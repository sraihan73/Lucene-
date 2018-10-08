using namespace std;

#include "LegacyPrefixTree.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using BytesRef = org::apache::lucene::util::BytesRef;

LegacyPrefixTree::LegacyPrefixTree(shared_ptr<SpatialContext> ctx,
                                   int maxLevels)
    : SpatialPrefixTree(ctx, maxLevels)
{
}

double LegacyPrefixTree::getDistanceForLevel(int level)
{
  if (level < 1 || level > getMaxLevels()) {
    throw invalid_argument(L"Level must be in 1 to maxLevels range");
  }
  // TODO cache for each level
  shared_ptr<Cell> cell = getCell(ctx->getWorldBounds().getCenter(), level);
  shared_ptr<Rectangle> bbox = cell->getShape()->getBoundingBox();
  double width = bbox->getWidth();
  double height = bbox->getHeight();
  // Use standard cartesian hypotenuse. For geospatial, this answer is larger
  // than the correct one but it's okay to over-estimate.
  return sqrt(width * width + height * height);
}

shared_ptr<Cell> LegacyPrefixTree::readCell(shared_ptr<BytesRef> term,
                                            shared_ptr<Cell> scratch)
{
  shared_ptr<LegacyCell> cell = std::static_pointer_cast<LegacyCell>(scratch);
  if (cell == nullptr) {
    cell = std::static_pointer_cast<LegacyCell>(getWorldCell());
  }
  cell->readCell(term);
  return cell;
}

shared_ptr<CellIterator>
LegacyPrefixTree::getTreeCellIterator(shared_ptr<Shape> shape, int detailLevel)
{
  if (!(std::dynamic_pointer_cast<Point>(shape) != nullptr)) {
    return SpatialPrefixTree::getTreeCellIterator(shape, detailLevel);
  }

  // This specialization is here because the legacy implementations don't have a
  // fast implementation of
  // cell.getSubCells(point). It's fastest here to encode the full bytes for
  // detailLevel, and create subcells from the bytesRef in a loop. This avoids
  // an O(N^2) encode, and we have O(N) instead.

  shared_ptr<Cell> cell =
      getCell(std::static_pointer_cast<Point>(shape), detailLevel);
  assert(std::dynamic_pointer_cast<LegacyCell>(cell) != nullptr);
  shared_ptr<BytesRef> fullBytes = cell->getTokenBytesNoLeaf(nullptr);
  // fill in reverse order to be sorted
  std::deque<std::shared_ptr<Cell>> cells(detailLevel);
  for (int i = 1; i < detailLevel; i++) {
    fullBytes->length = i;
    shared_ptr<Cell> parentCell = readCell(fullBytes, nullptr);
    cells[i - 1] = parentCell;
  }
  cells[detailLevel - 1] = cell;
  return make_shared<FilterCellIterator>(Arrays::asList(cells).begin(),
                                         nullptr); // null filter
}
} // namespace org::apache::lucene::spatial::prefix::tree