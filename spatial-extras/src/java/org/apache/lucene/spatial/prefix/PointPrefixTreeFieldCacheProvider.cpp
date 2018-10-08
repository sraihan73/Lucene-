using namespace std;

#include "PointPrefixTreeFieldCacheProvider.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::shape::Point;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using ShapeFieldCacheProvider =
    org::apache::lucene::spatial::util::ShapeFieldCacheProvider;
using BytesRef = org::apache::lucene::util::BytesRef;

PointPrefixTreeFieldCacheProvider::PointPrefixTreeFieldCacheProvider(
    shared_ptr<SpatialPrefixTree> grid, const wstring &shapeField,
    int defaultSize)
    : org::apache::lucene::spatial::util::ShapeFieldCacheProvider<
          org::locationtech::spatial4j::shape::Point>(shapeField, defaultSize),
      grid(grid)
{
}

shared_ptr<Point>
PointPrefixTreeFieldCacheProvider::readShape(shared_ptr<BytesRef> term)
{
  scanCell = grid->readCell(term, scanCell);
  if (scanCell->getLevel() == grid->getMaxLevels()) {
    return scanCell->getShape()->getCenter();
  }
  return nullptr;
}
} // namespace org::apache::lucene::spatial::prefix