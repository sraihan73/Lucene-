using namespace std;

#include "Geo3DPointDistanceComparator.h"

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
using GeoDistanceShape = org::apache::lucene::spatial3d::geom::GeoDistanceShape;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using XYZBounds = org::apache::lucene::spatial3d::geom::XYZBounds;

Geo3DPointDistanceComparator::Geo3DPointDistanceComparator(
    const wstring &field, shared_ptr<GeoDistanceShape> distanceShape,
    int numHits)
    : field(field), distanceShape(distanceShape),
      values(std::deque<double>(numHits))
{
}

void Geo3DPointDistanceComparator::setScorer(shared_ptr<Scorer> scorer) {}

int Geo3DPointDistanceComparator::compare(int slot1, int slot2)
{
  return Double::compare(values[slot1], values[slot2]);
}

void Geo3DPointDistanceComparator::setBottom(int slot)
{
  bottomDistance = values[slot];
  // make bounding box(es) to exclude non-competitive hits, but start
  // sampling if we get called way too much: don't make gobs of bounding
  // boxes if comparator hits a worst case order (e.g. backwards distance order)
  if (setBottomCounter < 1024 || (setBottomCounter & 0x3F) == 0x3F) {
    // Update bounds
    shared_ptr<XYZBounds> *const bounds = make_shared<XYZBounds>();
    distanceShape->getDistanceBounds(bounds, DistanceStyle::ARC,
                                     bottomDistance);
    priorityQueueBounds = bounds;
  }
  setBottomCounter++;
}

void Geo3DPointDistanceComparator::setTopValue(optional<double> &value)
{
  topValue = value.value();
}

int Geo3DPointDistanceComparator::compareBottom(int doc) 
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

    if (x > priorityQueueBounds->getMaximumX() ||
        x < priorityQueueBounds->getMinimumX() ||
        y > priorityQueueBounds->getMaximumY() ||
        y < priorityQueueBounds->getMinimumY() ||
        z > priorityQueueBounds->getMaximumZ() ||
        z < priorityQueueBounds->getMinimumZ()) {
      continue;
    }

    cmp = max(
        cmp, Double::compare(bottomDistance, distanceShape->computeDistance(
                                                 DistanceStyle::ARC, x, y, z)));
  }
  return cmp;
}

void Geo3DPointDistanceComparator::copy(int slot, int doc) 
{
  values[slot] = computeMinimumDistance(doc);
}

shared_ptr<LeafFieldComparator> Geo3DPointDistanceComparator::getLeafComparator(
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

optional<double> Geo3DPointDistanceComparator::value(int slot)
{
  // Return the arc distance
  return static_cast<Double>(values[slot] * PlanetModel::WGS84_MEAN);
}

int Geo3DPointDistanceComparator::compareTop(int doc) 
{
  return Double::compare(topValue, computeMinimumDistance(doc));
}

double Geo3DPointDistanceComparator::computeMinimumDistance(
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
      constexpr double distance = distanceShape->computeDistance(
          DistanceStyle::ARC, Geo3DDocValuesField::decodeXValue(encoded),
          Geo3DDocValuesField::decodeYValue(encoded),
          Geo3DDocValuesField::decodeZValue(encoded));
      minValue = min(minValue, distance);
    }
  }
  return minValue;
}
} // namespace org::apache::lucene::spatial3d