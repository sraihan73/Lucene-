using namespace std;

#include "NumberRangePrefixTreeStrategy.h"

namespace org::apache::lucene::spatial::prefix
{
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using NumberRangePrefixTree =
    org::apache::lucene::spatial::prefix::tree::NumberRangePrefixTree;
using Bits = org::apache::lucene::util::Bits;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;
//    import static
//    org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape;

NumberRangePrefixTreeStrategy::NumberRangePrefixTreeStrategy(
    shared_ptr<NumberRangePrefixTree> prefixTree, const wstring &fieldName)
    : RecursivePrefixTreeStrategy(prefixTree, fieldName)
{
  setPruneLeafyBranches(false);
  setPrefixGridScanLevel(prefixTree->getMaxLevels() -
                         2); // user might want to change, however
  setPointsOnly(false);
  setDistErrPct(0);
}

shared_ptr<NumberRangePrefixTree> NumberRangePrefixTreeStrategy::getGrid()
{
  return std::static_pointer_cast<NumberRangePrefixTree>(
      RecursivePrefixTreeStrategy::getGrid());
}

bool NumberRangePrefixTreeStrategy::isPointShape(shared_ptr<Shape> shape)
{
  if (std::dynamic_pointer_cast<NumberRangePrefixTree::UnitNRShape>(shape) !=
      nullptr) {
    return (std::static_pointer_cast<NumberRangePrefixTree::UnitNRShape>(shape))
               ->getLevel() == grid->getMaxLevels();
  } else {
    return false;
  }
}

bool NumberRangePrefixTreeStrategy::isGridAlignedShape(shared_ptr<Shape> shape)
{
  // any UnitNRShape other than the world is a single cell/term
  if (std::dynamic_pointer_cast<NumberRangePrefixTree::UnitNRShape>(shape) !=
      nullptr) {
    return (std::static_pointer_cast<NumberRangePrefixTree::UnitNRShape>(shape))
               ->getLevel() > 0;
  } else {
    return false;
  }
}

shared_ptr<DoubleValuesSource>
NumberRangePrefixTreeStrategy::makeDistanceValueSource(
    shared_ptr<Point> queryPoint, double multiplier)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Facets> NumberRangePrefixTreeStrategy::calcFacets(
    shared_ptr<IndexReaderContext> context, shared_ptr<Bits> topAcceptDocs,
    shared_ptr<NumberRangePrefixTree::UnitNRShape> start,
    shared_ptr<NumberRangePrefixTree::UnitNRShape> end) 
{
  shared_ptr<Shape> facetRange = getGrid()->toRangeShape(start, end);
  int detailLevel = max(start->getLevel(), end->getLevel()) + 1;
  return calcFacets(context, topAcceptDocs, facetRange, detailLevel);
}

shared_ptr<Facets> NumberRangePrefixTreeStrategy::calcFacets(
    shared_ptr<IndexReaderContext> context, shared_ptr<Bits> topAcceptDocs,
    shared_ptr<Shape> facetRange, int const level) 
{
  shared_ptr<Facets> *const facets = make_shared<Facets>(level);
  PrefixTreeFacetCounter::compute(shared_from_this(), context, topAcceptDocs,
                                  facetRange, level,
                                  make_shared<FacetVisitorAnonymousInnerClass>(
                                      shared_from_this(), level, facets));
  return facets;
}

NumberRangePrefixTreeStrategy::FacetVisitorAnonymousInnerClass::
    FacetVisitorAnonymousInnerClass(
        shared_ptr<NumberRangePrefixTreeStrategy> outerInstance, int level,
        shared_ptr<org::apache::lucene::spatial::prefix::
                       NumberRangePrefixTreeStrategy::Facets>
            facets)
{
  this->outerInstance = outerInstance;
  this->level = level;
  this->facets = facets;
}

void NumberRangePrefixTreeStrategy::FacetVisitorAnonymousInnerClass::visit(
    shared_ptr<Cell> cell, int count)
{
  if (cell->getLevel() <
      level - 1) { // some ancestor of parent facet level, direct or distant
    parentFacet = nullptr; // reset
    parentShape = nullptr; // reset
    facets->topLeaves += count;
  } else if (cell->getLevel() == level - 1) { // parent
    // set up FacetParentVal
    setupParent(std::static_pointer_cast<NumberRangePrefixTree::UnitNRShape>(
        cell->getShape()));
    parentFacet::parentLeaves += count;
  } else { // at facet level
    shared_ptr<NumberRangePrefixTree::UnitNRShape> unitShape =
        std::static_pointer_cast<NumberRangePrefixTree::UnitNRShape>(
            cell->getShape());
    shared_ptr<NumberRangePrefixTree::UnitNRShape> unitShapeParent =
        unitShape->getShapeAtLevel(unitShape->getLevel() - 1);
    if (parentFacet == nullptr || !parentShape::equals(unitShapeParent)) {
      setupParent(unitShapeParent);
    }
    // lazy init childCounts
    if (parentFacet->childCounts == nullptr) {
      parentFacet->childCounts = std::deque<int>(parentFacet::childCountsLen);
    }
    parentFacet::childCounts[unitShape->getValAtLevel(cell->getLevel())] +=
        count;
  }
}

void NumberRangePrefixTreeStrategy::FacetVisitorAnonymousInnerClass::
    setupParent(shared_ptr<NumberRangePrefixTree::UnitNRShape> unitShape)
{
  parentShape = unitShape->clone();
  // Look for existing parentFacet (from previous segment), or create anew if
  // needed
  parentFacet = facets->parents->get(parentShape);
  if (parentFacet == nullptr) { // didn't find one; make a new one
    parentFacet = make_shared<Facets::FacetParentVal>();
    parentFacet->childCountsLen =
        outerInstance->getGrid()->getNumSubCells(parentShape);
    facets->parents->put(parentShape, parentFacet);
  }
}

NumberRangePrefixTreeStrategy::Facets::Facets(int detailLevel)
    : detailLevel(detailLevel)
{
}

wstring NumberRangePrefixTreeStrategy::Facets::toString()
{
  shared_ptr<StringBuilder> buf = make_shared<StringBuilder>(2048);
  buf->append(L"Facets: level=" + to_wstring(detailLevel) + L" topLeaves=" +
              to_wstring(topLeaves) + L" parentCount=" + parents->size());
  for (auto entry : parents) {
    buf->append(L'\n');
    if (buf->length() > 1000) {
      buf->append(L"...");
      break;
    }
    shared_ptr<FacetParentVal> *const pVal = entry.second;
    buf->append(L' ')->append(entry.first + L" leafCount=" +
                              to_wstring(pVal->parentLeaves));
    if (pVal->childCounts.size() > 0) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      buf->append(L' ')->append(Arrays->toString(pVal->childCounts));
    }
  }
  return buf->toString();
}
} // namespace org::apache::lucene::spatial::prefix