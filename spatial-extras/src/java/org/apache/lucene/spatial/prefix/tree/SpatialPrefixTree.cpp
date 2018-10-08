using namespace std;

#include "SpatialPrefixTree.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;
using BytesRef = org::apache::lucene::util::BytesRef;

SpatialPrefixTree::SpatialPrefixTree(shared_ptr<SpatialContext> ctx,
                                     int maxLevels)
    : maxLevels(maxLevels), ctx(ctx)
{
  assert(maxLevels > 0);
}

shared_ptr<SpatialContext> SpatialPrefixTree::getSpatialContext()
{
  return ctx;
}

int SpatialPrefixTree::getMaxLevels() { return maxLevels; }

wstring SpatialPrefixTree::toString()
{
  return getClass().getSimpleName() + L"(maxLevels:" + to_wstring(maxLevels) +
         L",ctx:" + ctx + L")";
}

shared_ptr<CellIterator>
SpatialPrefixTree::getTreeCellIterator(shared_ptr<Shape> shape, int detailLevel)
{
  if (detailLevel > maxLevels) {
    throw invalid_argument(L"detailLevel > maxLevels");
  }
  return make_shared<TreeCellIterator>(shape, detailLevel, getWorldCell());
}
} // namespace org::apache::lucene::spatial::prefix::tree