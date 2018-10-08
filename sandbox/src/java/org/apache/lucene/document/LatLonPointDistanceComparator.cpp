using namespace std;

#include "LatLonPointDistanceComparator.h"

namespace org::apache::lucene::document
{
using Rectangle = org::apache::lucene::geo::Rectangle;
using DocValues = org::apache::lucene::index::DocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using SloppyMath = org::apache::lucene::util::SloppyMath;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

LatLonPointDistanceComparator::LatLonPointDistanceComparator(
    const wstring &field, double latitude, double longitude, int numHits)
    : field(field), latitude(latitude), longitude(longitude),
      values(std::deque<double>(numHits))
{
}

void LatLonPointDistanceComparator::setScorer(shared_ptr<Scorer> scorer) {}

int LatLonPointDistanceComparator::compare(int slot1, int slot2)
{
  return Double::compare(values[slot1], values[slot2]);
}

void LatLonPointDistanceComparator::setBottom(int slot)
{
  bottom = values[slot];
  // make bounding box(es) to exclude non-competitive hits, but start
  // sampling if we get called way too much: don't make gobs of bounding
  // boxes if comparator hits a worst case order (e.g. backwards distance order)
  if (setBottomCounter < 1024 || (setBottomCounter & 0x3F) == 0x3F) {
    shared_ptr<Rectangle> box =
        Rectangle::fromPointDistance(latitude, longitude, haversin2(bottom));
    // pre-encode our box to our integer encoding, so we don't have to decode
    // to double values for uncompetitive hits. This has some cost!
    minLat = encodeLatitude(box->minLat);
    maxLat = encodeLatitude(box->maxLat);
    if (box->crossesDateline()) {
      // box1
      minLon = numeric_limits<int>::min();
      maxLon = encodeLongitude(box->maxLon);
      // box2
      minLon2 = encodeLongitude(box->minLon);
    } else {
      minLon = encodeLongitude(box->minLon);
      maxLon = encodeLongitude(box->maxLon);
      // disable box2
      minLon2 = numeric_limits<int>::max();
    }
  }
  setBottomCounter++;
}

void LatLonPointDistanceComparator::setTopValue(optional<double> &value)
{
  topValue = value.value();
}

void LatLonPointDistanceComparator::setValues() 
{
  if (valuesDocID != currentDocs->docID()) {
    assert((valuesDocID < currentDocs->docID(),
            L" valuesDocID=" + to_wstring(valuesDocID) + L" vs " +
                to_wstring(currentDocs->docID())));
    valuesDocID = currentDocs->docID();
    int count = currentDocs->docValueCount();
    if (count > currentValues.size()) {
      currentValues = std::deque<int64_t>(
          ArrayUtil::oversize(count, RamUsageEstimator::NUM_BYTES_LONG));
    }
    for (int i = 0; i < count; i++) {
      currentValues[i] = currentDocs->nextValue();
    }
  }
}

int LatLonPointDistanceComparator::compareBottom(int doc) 
{
  if (doc > currentDocs->docID()) {
    currentDocs->advance(doc);
  }
  if (doc < currentDocs->docID()) {
    return Double::compare(bottom, numeric_limits<double>::infinity());
  }

  setValues();

  int numValues = currentDocs->docValueCount();

  int cmp = -1;
  for (int i = 0; i < numValues; i++) {
    int64_t encoded = currentValues[i];

    // test bounding box
    int latitudeBits = static_cast<int>(encoded >> 32);
    if (latitudeBits < minLat || latitudeBits > maxLat) {
      continue;
    }
    int longitudeBits = static_cast<int>(encoded & 0xFFFFFFFF);
    if ((longitudeBits < minLon || longitudeBits > maxLon) &&
        (longitudeBits < minLon2)) {
      continue;
    }

    // only compute actual distance if its inside "competitive bounding box"
    double docLatitude = decodeLatitude(latitudeBits);
    double docLongitude = decodeLongitude(longitudeBits);
    cmp = max(cmp, Double::compare(bottom, SloppyMath::haversinSortKey(
                                               latitude, longitude, docLatitude,
                                               docLongitude)));
    // once we compete in the PQ, no need to continue.
    if (cmp > 0) {
      return cmp;
    }
  }
  return cmp;
}

void LatLonPointDistanceComparator::copy(int slot, int doc) 
{
  values[slot] = sortKey(doc);
}

shared_ptr<LeafFieldComparator>
LatLonPointDistanceComparator::getLeafComparator(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> reader = context->reader();
  shared_ptr<FieldInfo> info = reader->getFieldInfos()->fieldInfo(field);
  if (info != nullptr) {
    LatLonDocValuesField::checkCompatible(info);
  }
  currentDocs = DocValues::getSortedNumeric(reader, field);
  valuesDocID = -1;
  return shared_from_this();
}

optional<double> LatLonPointDistanceComparator::value(int slot)
{
  return static_cast<Double>(haversin2(values[slot]));
}

int LatLonPointDistanceComparator::compareTop(int doc) 
{
  return Double::compare(topValue, haversin2(sortKey(doc)));
}

double LatLonPointDistanceComparator::sortKey(int doc) 
{
  if (doc > currentDocs->docID()) {
    currentDocs->advance(doc);
  }
  double minValue = numeric_limits<double>::infinity();
  if (doc == currentDocs->docID()) {
    setValues();
    int numValues = currentDocs->docValueCount();
    for (int i = 0; i < numValues; i++) {
      int64_t encoded = currentValues[i];
      double docLatitude = decodeLatitude(static_cast<int>(encoded >> 32));
      double docLongitude =
          decodeLongitude(static_cast<int>(encoded & 0xFFFFFFFF));
      minValue =
          min(minValue, SloppyMath::haversinSortKey(latitude, longitude,
                                                    docLatitude, docLongitude));
    }
  }
  return minValue;
}

double LatLonPointDistanceComparator::haversin2(double partial_)
{
  if (isinf(partial_)) {
    return partial_;
  }
  return SloppyMath::haversinMeters(partial_);
}
} // namespace org::apache::lucene::document