using namespace std;

#include "Geo3DPointOutsideDistanceComparator.h"

namespace org::apache::lucene::spatial3d
{
using DocValues = org::apache::lucene::index::DocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using DistanceStyle = org::apache::lucene::spatial3d::geom::DistanceStyle;
using GeoOutsideDistance =
    org::apache::lucene::spatial3d::geom::GeoOutsideDistance;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;

Geo3DPointOutsideDistanceComparator::Geo3DPointOutsideDistanceComparator(
    const wstring &field, shared_ptr<GeoOutsideDistance> distanceShape,
    int numHits)
    : field(field), distanceShape(distanceShape),
      values(std::deque<double>(numHits))
{
}

void Geo3DPointOutsideDistanceComparator::setScorer(shared_ptr<Scorer> scorer)
{
}

int Geo3DPointOutsideDistanceComparator::compare(int slot1, int slot2)
{
  return Double::compare(values[slot1], values[slot2]);
}

void Geo3DPointOutsideDistanceComparator::setBottom(int slot)
{
  bottomDistance = values[slot];
}

void Geo3DPointOutsideDistanceComparator::setTopValue(optional<double> &value)
{
  topValue = value.value();
}

int Geo3DPointOutsideDistanceComparator::compareBottom(int doc) throw(
    IOException)
{
  if (doc > currentDocs->docID()) {
    currentDocs->advance(doc);
  }
  if (doc < currentDocs->docID()) {
    return Double::compare(bottomDistance, numeric_limits<double>::infinity());
  }

  int numValues = currentDocs->docValueCount();
  assert(numValues > 0);

  int cmp = -1;
  for (int i = 0; i < numValues; i++) {
    int64_t encoded = currentDocs->nextValue();

    // Test against bounds.
    // First we need to decode...
    constexpr double x = Geo3DDocValuesField::decodeXValue(encoded);
    constexpr double y = Geo3DDocValuesField::decodeYValue(encoded);
    constexpr double z = Geo3DDocValuesField::decodeZValue(encoded);

    cmp = max(cmp, Double::compare(bottomDistance,
                                   distanceShape->computeOutsideDistance(
                                       DistanceStyle::ARC, x, y, z)));
  }
  return cmp;
}

void Geo3DPointOutsideDistanceComparator::copy(int slot,
                                               int doc) 
{
  values[slot] = computeMinimumDistance(doc);
}

shared_ptr<LeafFieldComparator>
Geo3DPointOutsideDistanceComparator::getLeafComparator(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> reader = context->reader();
  shared_ptr<FieldInfo> info = reader->getFieldInfos()->fieldInfo(field);
  if (info != nullptr) {
    Geo3DDocValuesField::checkCompatible(info);
  }
  currentDocs = DocValues::getSortedNumeric(reader, field);
  return shared_from_this();
}

optional<double> Geo3DPointOutsideDistanceComparator::value(int slot)
{
  // Return the arc distance
  return static_cast<Double>(values[slot] * PlanetModel::WGS84_MEAN);
}

int Geo3DPointOutsideDistanceComparator::compareTop(int doc) 
{
  return Double::compare(topValue, computeMinimumDistance(doc));
}

double Geo3DPointOutsideDistanceComparator::computeMinimumDistance(
    int const doc) 
{
  if (doc > currentDocs->docID()) {
    currentDocs->advance(doc);
  }
  double minValue = numeric_limits<double>::infinity();
  if (doc == currentDocs->docID()) {
    constexpr int numValues = currentDocs->docValueCount();
    for (int i = 0; i < numValues; i++) {
      constexpr int64_t encoded = currentDocs->nextValue();
      constexpr double distance = distanceShape->computeOutsideDistance(
          DistanceStyle::ARC, Geo3DDocValuesField::decodeXValue(encoded),
          Geo3DDocValuesField::decodeYValue(encoded),
          Geo3DDocValuesField::decodeZValue(encoded));
      minValue = min(minValue, distance);
    }
  }
  return minValue;
}
} // namespace org::apache::lucene::spatial3d