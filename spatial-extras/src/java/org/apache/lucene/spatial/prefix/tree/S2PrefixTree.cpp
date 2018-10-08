using namespace std;

#include "S2PrefixTree.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using com::google::common::geometry::S2CellId;
using com::google::common::geometry::S2LatLng;
using com::google::common::geometry::S2Projections;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

int S2PrefixTree::Factory::getLevelForDistance(double degrees)
{
  shared_ptr<S2PrefixTree> grid =
      make_shared<S2PrefixTree>(ctx, S2PrefixTree::getMaxLevels(1));
  return grid->getLevelForDistance(degrees);
}

shared_ptr<SpatialPrefixTree> S2PrefixTree::Factory::newSPT()
{
  return make_shared<S2PrefixTree>(
      ctx, maxLevels ? maxLevels : S2PrefixTree::getMaxLevels(1));
}

S2PrefixTree::S2PrefixTree(shared_ptr<SpatialContext> ctx, int maxLevels)
    : S2PrefixTree(ctx, maxLevels, 1)
{
}

S2PrefixTree::S2PrefixTree(shared_ptr<SpatialContext> ctx, int maxLevels,
                           int arity)
    : SpatialPrefixTree(ctx, maxLevels),
      s2ShapeFactory(
          std::static_pointer_cast<S2ShapeFactory>(ctx->getShapeFactory())),
      arity(arity)
{
  if (!(std::dynamic_pointer_cast<S2ShapeFactory>(ctx->getShapeFactory()) !=
        nullptr)) {
    throw invalid_argument(
        L"Spatial context does not support S2 spatial index.");
  }
  if (arity < 1 || arity > 3) {
    throw invalid_argument(L"Invalid value for S2 tree arity. Possible values "
                           L"are 1, 2 or 3. Provided value is " +
                           to_wstring(arity) + L".");
  }
}

int S2PrefixTree::getMaxLevels(int arity)
{
  return S2CellId::MAX_LEVEL / arity + 1;
}

int S2PrefixTree::getLevelForDistance(double dist)
{
  if (dist == 0) {
    return maxLevels;
  }
  int level = S2Projections::MAX_WIDTH::getMinLevel(
      dist * DistanceUtils::DEGREES_TO_RADIANS);
  int roundLevel = level % arity != 0 ? 1 : 0;
  level = level / arity + roundLevel;
  return min(maxLevels, level + 1);
}

double S2PrefixTree::getDistanceForLevel(int level)
{
  if (level == 0) {
    return 180;
  }
  return S2Projections::MAX_WIDTH::getValue(arity * (level - 1)) *
         DistanceUtils::RADIANS_TO_DEGREES;
}

shared_ptr<Cell> S2PrefixTree::getWorldCell()
{
  return make_shared<S2PrefixTreeCell>(shared_from_this(), nullptr);
}

shared_ptr<Cell> S2PrefixTree::readCell(shared_ptr<BytesRef> term,
                                        shared_ptr<Cell> scratch)
{
  shared_ptr<S2PrefixTreeCell> cell =
      std::static_pointer_cast<S2PrefixTreeCell>(scratch);
  if (cell == nullptr) {
    cell = std::static_pointer_cast<S2PrefixTreeCell>(getWorldCell());
  }
  cell->readCell(shared_from_this(), term);
  return cell;
}

shared_ptr<CellIterator>
S2PrefixTree::getTreeCellIterator(shared_ptr<Shape> shape, int detailLevel)
{
  if (!(std::dynamic_pointer_cast<Point>(shape) != nullptr)) {
    return SpatialPrefixTree::getTreeCellIterator(shape, detailLevel);
  }
  shared_ptr<Point> p = std::static_pointer_cast<Point>(shape);
  shared_ptr<S2CellId> id =
      S2CellId::fromLatLng(S2LatLng::fromDegrees(p->getY(), p->getX()))
          .parent(arity * (detailLevel - 1));
  deque<std::shared_ptr<Cell>> cells =
      deque<std::shared_ptr<Cell>>(detailLevel);
  for (int i = 0; i < detailLevel - 1; i++) {
    cells.push_back(make_shared<S2PrefixTreeCell>(shared_from_this(),
                                                  id->parent(i * arity)));
  }
  cells.push_back(make_shared<S2PrefixTreeCell>(shared_from_this(), id));
  return make_shared<FilterCellIterator>(cells.begin(), nullptr);
}
} // namespace org::apache::lucene::spatial::prefix::tree