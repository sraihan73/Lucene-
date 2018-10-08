using namespace std;

#include "LatLonPoint.h"

namespace org::apache::lucene::document
{
using Lucene60PointsFormat =
    org::apache::lucene::codecs::lucene60::Lucene60PointsFormat;
using GeoUtils = org::apache::lucene::geo::GeoUtils;
using Polygon = org::apache::lucene::geo::Polygon;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PointValues = org::apache::lucene::index::PointValues;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using PointRangeQuery = org::apache::lucene::search::PointRangeQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitudeCeil;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitudeCeil;
const shared_ptr<FieldType> LatLonPoint::TYPE = make_shared<FieldType>();

LatLonPoint::StaticConstructor::StaticConstructor()
{
  TYPE->setDimensions(2, Integer::BYTES);
  TYPE->freeze();
}

LatLonPoint::StaticConstructor LatLonPoint::staticConstructor;

void LatLonPoint::setLocationValue(double latitude, double longitude)
{
  const std::deque<char> bytes;

  if (fieldsData == nullptr) {
    bytes = std::deque<char>(8);
    fieldsData = make_shared<BytesRef>(bytes);
  } else {
    bytes = (any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes;
  }

  int latitudeEncoded = encodeLatitude(latitude);
  int longitudeEncoded = encodeLongitude(longitude);
  NumericUtils::intToSortableBytes(latitudeEncoded, bytes, 0);
  NumericUtils::intToSortableBytes(longitudeEncoded, bytes, Integer::BYTES);
}

LatLonPoint::LatLonPoint(const wstring &name, double latitude, double longitude)
    : Field(name, TYPE)
{
  setLocationValue(latitude, longitude);
}

wstring LatLonPoint::toString()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  result->append(getClass().getSimpleName());
  result->append(L" <");
  result->append(name_);
  result->append(L':');

  std::deque<char> bytes =
      (any_cast<std::shared_ptr<BytesRef>>(fieldsData)).bytes;
  result->append(decodeLatitude(bytes, 0));
  result->append(L',');
  result->append(decodeLongitude(bytes, Integer::BYTES));

  result->append(L'>');
  return result->toString();
}

std::deque<char> LatLonPoint::encode(double latitude, double longitude)
{
  std::deque<char> bytes(2 * Integer::BYTES);
  NumericUtils::intToSortableBytes(encodeLatitude(latitude), bytes, 0);
  NumericUtils::intToSortableBytes(encodeLongitude(longitude), bytes,
                                   Integer::BYTES);
  return bytes;
}

std::deque<char> LatLonPoint::encodeCeil(double latitude, double longitude)
{
  std::deque<char> bytes(2 * Integer::BYTES);
  NumericUtils::intToSortableBytes(encodeLatitudeCeil(latitude), bytes, 0);
  NumericUtils::intToSortableBytes(encodeLongitudeCeil(longitude), bytes,
                                   Integer::BYTES);
  return bytes;
}

void LatLonPoint::checkCompatible(shared_ptr<FieldInfo> fieldInfo)
{
  // point/dv properties could be "unset", if you e.g. used only StoredField
  // with this same name in the segment.
  if (fieldInfo->getPointDimensionCount() != 0 &&
      fieldInfo->getPointDimensionCount() != TYPE->pointDimensionCount()) {
    throw invalid_argument(L"field=\"" + fieldInfo->name +
                           L"\" was indexed with numDims=" +
                           to_wstring(fieldInfo->getPointDimensionCount()) +
                           L" but this point type has numDims=" +
                           to_wstring(TYPE->pointDimensionCount()) +
                           L", is the field really a LatLonPoint?");
  }
  if (fieldInfo->getPointNumBytes() != 0 &&
      fieldInfo->getPointNumBytes() != TYPE->pointNumBytes()) {
    throw invalid_argument(L"field=\"" + fieldInfo->name +
                           L"\" was indexed with bytesPerDim=" +
                           to_wstring(fieldInfo->getPointNumBytes()) +
                           L" but this point type has bytesPerDim=" +
                           to_wstring(TYPE->pointNumBytes()) +
                           L", is the field really a LatLonPoint?");
  }
}

shared_ptr<Query> LatLonPoint::newBoxQuery(const wstring &field,
                                           double minLatitude,
                                           double maxLatitude,
                                           double minLongitude,
                                           double maxLongitude)
{
  // exact double values of lat=90.0D and lon=180.0D must be treated special as
  // they are not represented in the encoding and should not drag in extra bogus
  // junk! TODO: should encodeCeil just throw ArithmeticException to be less
  // trappy here?
  if (minLatitude == 90.0) {
    // range cannot match as 90.0 can never exist
    return make_shared<MatchNoDocsQuery>(
        L"LatLonPoint.newBoxQuery with minLatitude=90.0");
  }
  if (minLongitude == 180.0) {
    if (maxLongitude == 180.0) {
      // range cannot match as 180.0 can never exist
      return make_shared<MatchNoDocsQuery>(
          L"LatLonPoint.newBoxQuery with minLongitude=maxLongitude=180.0");
    } else if (maxLongitude < minLongitude) {
      // encodeCeil() with dateline wrapping!
      minLongitude = -180.0;
    }
  }
  std::deque<char> lower = encodeCeil(minLatitude, minLongitude);
  std::deque<char> upper = encode(maxLatitude, maxLongitude);
  // Crosses date line: we just rewrite into OR of two bboxes, with longitude as
  // an open range:
  if (maxLongitude < minLongitude) {
    // Disable coord here because a multi-valued doc could match both rects and
    // get unfairly boosted:
    shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();

    // E.g.: maxLon = -179, minLon = 179
    std::deque<char> leftOpen = lower.clone();
    // leave longitude open
    NumericUtils::intToSortableBytes(numeric_limits<int>::min(), leftOpen,
                                     Integer::BYTES);
    shared_ptr<Query> left = newBoxInternal(field, leftOpen, upper);
    q->add(make_shared<BooleanClause>(left, BooleanClause::Occur::SHOULD));

    std::deque<char> rightOpen = upper.clone();
    // leave longitude open
    NumericUtils::intToSortableBytes(numeric_limits<int>::max(), rightOpen,
                                     Integer::BYTES);
    shared_ptr<Query> right = newBoxInternal(field, lower, rightOpen);
    q->add(make_shared<BooleanClause>(right, BooleanClause::Occur::SHOULD));
    return make_shared<ConstantScoreQuery>(q->build());
  } else {
    return newBoxInternal(field, lower, upper);
  }
}

shared_ptr<Query> LatLonPoint::newBoxInternal(const wstring &field,
                                              std::deque<char> &min,
                                              std::deque<char> &max)
{
  return make_shared<PointRangeQueryAnonymousInnerClass>(field, min, max);
}

LatLonPoint::PointRangeQueryAnonymousInnerClass::
    PointRangeQueryAnonymousInnerClass(const wstring &field, deque<char> &min,
                                       deque<char> &max)
    : org::apache::lucene::search::PointRangeQuery(field, min, max, 2)
{
}

wstring LatLonPoint::PointRangeQueryAnonymousInnerClass::toString(
    int dimension, std::deque<char> &value)
{
  if (dimension == 0) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Double::toString(decodeLatitude(value, 0));
  } else if (dimension == 1) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return Double::toString(decodeLongitude(value, 0));
  } else {
    throw make_shared<AssertionError>();
  }
}

shared_ptr<Query> LatLonPoint::newDistanceQuery(const wstring &field,
                                                double latitude,
                                                double longitude,
                                                double radiusMeters)
{
  return make_shared<LatLonPointDistanceQuery>(field, latitude, longitude,
                                               radiusMeters);
}

shared_ptr<Query> LatLonPoint::newPolygonQuery(const wstring &field,
                                               deque<Polygon> &polygons)
{
  return make_shared<LatLonPointInPolygonQuery>(field, polygons);
}

shared_ptr<TopFieldDocs>
LatLonPoint::nearest(shared_ptr<IndexSearcher> searcher, const wstring &field,
                     double latitude, double longitude,
                     int n) 
{
  GeoUtils::checkLatitude(latitude);
  GeoUtils::checkLongitude(longitude);
  if (n < 1) {
    throw invalid_argument(L"n must be at least 1; got " + to_wstring(n));
  }
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
  if (searcher == nullptr) {
    throw invalid_argument(L"searcher must not be null");
  }
  deque<std::shared_ptr<BKDReader>> readers =
      deque<std::shared_ptr<BKDReader>>();
  deque<int> docBases = deque<int>();
  deque<std::shared_ptr<Bits>> liveDocs = deque<std::shared_ptr<Bits>>();
  int totalHits = 0;
  for (auto leaf : searcher->getIndexReader()->leaves()) {
    shared_ptr<PointValues> points = leaf->reader()->getPointValues(field);
    if (points != nullptr) {
      if (std::dynamic_pointer_cast<BKDReader>(points) != nullptr == false) {
        throw invalid_argument(L"can only run on Lucene60PointsReader points "
                               L"implementation, but got " +
                               points);
      }
      totalHits += points->getDocCount();
      shared_ptr<BKDReader> reader =
          std::static_pointer_cast<BKDReader>(points);
      if (reader != nullptr) {
        readers.push_back(reader);
        docBases.push_back(leaf->docBase);
        liveDocs.push_back(leaf->reader()->getLiveDocs());
      }
    }
  }

  std::deque<std::shared_ptr<NearestNeighbor::NearestHit>> hits =
      NearestNeighbor::nearest(latitude, longitude, readers, liveDocs, docBases,
                               n);

  // Convert to TopFieldDocs:
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs(hits.size());
  for (int i = 0; i < hits.size(); i++) {
    shared_ptr<NearestNeighbor::NearestHit> hit = hits[i];
    scoreDocs[i] = make_shared<FieldDoc>(
        hit->docID, 0.0f,
        std::deque<any>{static_cast<Double>(hit->distanceMeters)});
  }
  return make_shared<TopFieldDocs>(totalHits, scoreDocs, nullptr, 0.0f);
}
} // namespace org::apache::lucene::document