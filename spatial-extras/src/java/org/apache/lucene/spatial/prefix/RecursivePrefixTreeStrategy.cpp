using namespace std;

#include "RecursivePrefixTreeStrategy.h"

namespace org::apache::lucene::spatial::prefix
{
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellCanPrune = org::apache::lucene::spatial::prefix::tree::CellCanPrune;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using UnsupportedSpatialOperation =
    org::apache::lucene::spatial::query::UnsupportedSpatialOperation;
using org::locationtech::spatial4j::shape::Shape;

RecursivePrefixTreeStrategy::RecursivePrefixTreeStrategy(
    shared_ptr<SpatialPrefixTree> grid, const wstring &fieldName)
    : PrefixTreeStrategy(grid, fieldName)
{
  prefixGridScanLevel =
      grid->getMaxLevels() -
      4; // TODO this default constant is dependent on the prefix grid size
}

int RecursivePrefixTreeStrategy::getPrefixGridScanLevel()
{
  return prefixGridScanLevel;
}

void RecursivePrefixTreeStrategy::setPrefixGridScanLevel(
    int prefixGridScanLevel)
{
  // TODO if negative then subtract from maxlevels
  this->prefixGridScanLevel = prefixGridScanLevel;
}

bool RecursivePrefixTreeStrategy::isMultiOverlappingIndexedShapes()
{
  return multiOverlappingIndexedShapes;
}

void RecursivePrefixTreeStrategy::setMultiOverlappingIndexedShapes(
    bool multiOverlappingIndexedShapes)
{
  this->multiOverlappingIndexedShapes = multiOverlappingIndexedShapes;
}

bool RecursivePrefixTreeStrategy::isPruneLeafyBranches()
{
  return pruneLeafyBranches;
}

void RecursivePrefixTreeStrategy::setPruneLeafyBranches(bool pruneLeafyBranches)
{
  this->pruneLeafyBranches = pruneLeafyBranches;
}

wstring RecursivePrefixTreeStrategy::toString()
{
  shared_ptr<StringBuilder> str =
      (make_shared<StringBuilder>(getClass().getSimpleName()))->append(L'(');
  // C++ TODO: There is no native C++ equivalent to 'toString':
  str->append(L"SPG:(")->append(grid->toString())->append(L')');
  if (pointsOnly) {
    str->append(L",pointsOnly");
  }
  if (pruneLeafyBranches) {
    str->append(L",pruneLeafyBranches");
  }
  if (prefixGridScanLevel != grid->getMaxLevels() - 4) {
    str->append(L",prefixGridScanLevel:")
        ->append(L"" + to_wstring(prefixGridScanLevel));
  }
  if (!multiOverlappingIndexedShapes) {
    str->append(L",!multiOverlappingIndexedShapes");
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return str->append(L')')->toString();
}

shared_ptr<Iterator<std::shared_ptr<Cell>>>
RecursivePrefixTreeStrategy::createCellIteratorToIndex(
    shared_ptr<Shape> shape, int detailLevel,
    shared_ptr<Iterator<std::shared_ptr<Cell>>> reuse)
{
  if (!pruneLeafyBranches || isGridAlignedShape(shape)) {
    return PrefixTreeStrategy::createCellIteratorToIndex(shape, detailLevel,
                                                         reuse);
  }

  deque<std::shared_ptr<Cell>> cells = deque<std::shared_ptr<Cell>>(4096);
  recursiveTraverseAndPrune(grid->getWorldCell(), shape, detailLevel, cells);
  return cells.begin();
}

bool RecursivePrefixTreeStrategy::recursiveTraverseAndPrune(
    shared_ptr<Cell> cell, shared_ptr<Shape> shape, int detailLevel,
    deque<std::shared_ptr<Cell>> &result)
{

  if (cell->getLevel() == detailLevel) {
    cell->setLeaf(); // FYI might already be a leaf
  }
  if (cell->isLeaf()) {
    result.push_back(cell);
    return true;
  }
  if (cell->getLevel() != 0) {
    result.push_back(cell);
  }

  int leaves = 0;
  shared_ptr<CellIterator> subCells = cell->getNextLevelCells(shape);
  while (subCells->hasNext()) {
    shared_ptr<Cell> subCell = subCells->next();
    if (recursiveTraverseAndPrune(subCell, shape, detailLevel, result)) {
      leaves++;
    }
    subCells++;
  }

  if (!(std::dynamic_pointer_cast<CellCanPrune>(cell) != nullptr)) {
    // Cannot prune so return false
    return false;
  }

  // can we prune?
  if (leaves ==
          (std::static_pointer_cast<CellCanPrune>(cell))->getSubCellsSize() &&
      cell->getLevel() != 0) {
    // Optimization: substitute the parent as a leaf instead of adding all
    // children as leaves

    // remove the leaves
    do {
      result.pop_back(); // remove last
    } while (--leaves > 0);
    // add cell as the leaf
    cell->setLeaf();
    return true;
  }
  return false;
}

shared_ptr<Query>
RecursivePrefixTreeStrategy::makeQuery(shared_ptr<SpatialArgs> args)
{
  shared_ptr<SpatialOperation> *const op = args->getOperation();

  shared_ptr<Shape> shape = args->getShape();
  int detailLevel =
      grid->getLevelForDistance(args->resolveDistErr(ctx, distErrPct));

  if (op == SpatialOperation::Intersects) {
    if (isGridAlignedShape(args->getShape())) {
      return makeGridShapeIntersectsQuery(args->getShape());
    }
    return make_shared<IntersectsPrefixTreeQuery>(
        shape, getFieldName(), grid, detailLevel, prefixGridScanLevel);
  } else if (op == SpatialOperation::IsWithin) {
    return make_shared<WithinPrefixTreeQuery>(
        shape, getFieldName(), grid, detailLevel, prefixGridScanLevel,
        -1); //-1 flag is slower but ensures correct results
  } else if (op == SpatialOperation::Contains) {
    return make_shared<ContainsPrefixTreeQuery>(shape, getFieldName(), grid,
                                                detailLevel,
                                                multiOverlappingIndexedShapes);
  }
  throw make_shared<UnsupportedSpatialOperation>(op);
}

bool RecursivePrefixTreeStrategy::isGridAlignedShape(shared_ptr<Shape> shape)
{
  return isPointShape(shape);
}

shared_ptr<Query> RecursivePrefixTreeStrategy::makeGridShapeIntersectsQuery(
    shared_ptr<Shape> gridShape)
{
  assert(isGridAlignedShape(gridShape));
  if (isPointsOnly()) {
    // Awesome; this will be equivalent to a TermQuery.
    Iterator<std::shared_ptr<Cell>> cellIterator =
        grid->getTreeCellIterator(gridShape, grid->getMaxLevels());
    // get last cell
    shared_ptr<Cell> cell = cellIterator->next();
    while (cellIterator->hasNext()) {
      int prevLevel = cell->getLevel();
      cell = cellIterator->next();
      assert(prevLevel < cell->getLevel());
      cellIterator++;
    }
    assert(cell->isLeaf());
    return make_shared<TermQuery>(make_shared<Term>(
        getFieldName(), cell->getTokenBytesWithLeaf(nullptr)));
  } else {
    // Well there could be parent cells. But we can reduce the "scan level"
    // which will be slower for a point query.
    // TODO: AVPTQ will still scan the bottom nonetheless; file an issue to
    // eliminate that
    return make_shared<IntersectsPrefixTreeQuery>(
        gridShape, getFieldName(), grid, getGrid()->getMaxLevels(),
        getGrid()->getMaxLevels() + 1);
  }
}
} // namespace org::apache::lucene::spatial::prefix