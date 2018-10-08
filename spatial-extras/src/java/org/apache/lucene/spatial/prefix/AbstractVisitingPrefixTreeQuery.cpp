using namespace std;

#include "AbstractVisitingPrefixTreeQuery.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using BytesRef = org::apache::lucene::util::BytesRef;

AbstractVisitingPrefixTreeQuery::AbstractVisitingPrefixTreeQuery(
    shared_ptr<Shape> queryShape, const wstring &fieldName,
    shared_ptr<SpatialPrefixTree> grid, int detailLevel,
    int prefixGridScanLevel)
    : AbstractPrefixTreeQuery(queryShape, fieldName, grid, detailLevel),
      prefixGridScanLevel(
          max(0, min(prefixGridScanLevel, grid->getMaxLevels() - 1)))
{
  assert(detailLevel <= grid->getMaxLevels());
}

AbstractVisitingPrefixTreeQuery::VisitorTemplate::VisitorTemplate(
    shared_ptr<AbstractVisitingPrefixTreeQuery> outerInstance,
    shared_ptr<LeafReaderContext> context) 
    : BaseTermsEnumTraverser(outerInstance, context),
      outerInstance(outerInstance)
{
}

shared_ptr<DocIdSet>
AbstractVisitingPrefixTreeQuery::VisitorTemplate::getDocIdSet() throw(
    IOException)
{
  assert((curVNode == nullptr, L"Called more than once?"));
  if (termsEnum == nullptr) {
    return nullptr;
  }
  if (!nextTerm()) { // advances
    return nullptr;
  }

  curVNode = make_shared<VNode>(nullptr);
  curVNode->reset(outerInstance->grid->getWorldCell());

  start();

  addIntersectingChildren();

  while (thisTerm != nullptr) { // terminates for other reasons too!

    // Advance curVNode pointer
    if (curVNode->children != nullptr) {
      //-- HAVE CHILDREN: DESCEND
      assert(curVNode->children
                 ->hasNext()); // if we put it there then it has something
      preSiblings(curVNode);
      curVNode = curVNode->children->next();
    } else {
      //-- NO CHILDREN: ADVANCE TO NEXT SIBLING
      shared_ptr<VNode> parentVNode = curVNode->parent;
      while (true) {
        if (parentVNode == nullptr) {
          goto mainBreak; // all done
        }
        if (parentVNode->children->hasNext()) {
          // advance next sibling
          curVNode = parentVNode->children->next();
          break;
        } else {
          // reached end of siblings; pop up
          postSiblings(parentVNode);
          parentVNode->children.reset(); // GC
          parentVNode = parentVNode->parent;
        }
      }
    }

    // Seek to curVNode's cell (or skip if termsEnum has moved beyond)
    constexpr int compare = indexedCell->compareToNoLeaf(curVNode->cell);
    if (compare > 0) {
      // The indexed cell is after; continue loop to next query cell
      continue;
    }
    if (compare < 0) {
      // The indexed cell is before; seek ahead to query cell:
      //      Seek !
      curVNode->cell->getTokenBytesNoLeaf(curVNodeTerm);
      TermsEnum::SeekStatus seekStatus = termsEnum->seekCeil(curVNodeTerm);
      if (seekStatus == TermsEnum::SeekStatus::END) {
        break; // all done
      }
      thisTerm = termsEnum->term();
      indexedCell = outerInstance->grid->readCell(thisTerm, indexedCell);
      if (seekStatus == TermsEnum::SeekStatus::NOT_FOUND) {
        // Did we find a leaf of the cell we were looking for or something
        // after?
        if (!indexedCell->isLeaf() ||
            indexedCell->compareToNoLeaf(curVNode->cell) != 0) {
          continue; // The indexed cell is after; continue loop to next query
                    // cell
        }
      }
    }
    // indexedCell == queryCell (disregarding leaf).

    // If indexedCell is a leaf then there's no prefix (prefix sorts before) --
    // just visit and continue
    if (indexedCell->isLeaf()) {
      visitLeaf(indexedCell); // TODO or query cell? Though shouldn't matter.
      if (!nextTerm()) {
        break;
      }
      continue;
    }
    // If a prefix (non-leaf) then visit; see if we descend.
    constexpr bool descend = visitPrefix(
        curVNode->cell); // need to use curVNode.cell not indexedCell
    if (!nextTerm()) {
      break;
    }
    // Check for adjacent leaf with the same prefix
    if (indexedCell->isLeaf() &&
        indexedCell->getLevel() == curVNode->cell->getLevel()) {
      visitLeaf(indexedCell); // TODO or query cell? Though shouldn't matter.
      if (!nextTerm()) {
        break;
      }
    }

    if (descend) {
      addIntersectingChildren();
    }

  mainContinue:;
  }
mainBreak: // main loop

  return finish();
}

void AbstractVisitingPrefixTreeQuery::VisitorTemplate::
    addIntersectingChildren() 
{
  assert(thisTerm != nullptr);
  shared_ptr<Cell> cell = curVNode->cell;
  if (cell->getLevel() >= outerInstance->detailLevel) {
    throw make_shared<IllegalStateException>(L"Spatial logic error");
  }

  // Decide whether to continue to divide & conquer, or whether it's time to
  // scan through terms beneath this cell.
  // Scanning is a performance optimization trade-off.

  // TODO use termsEnum.docFreq() as heuristic
  bool scan = cell->getLevel() >=
              outerInstance->prefixGridScanLevel; // simple heuristic

  if (!scan) {
    // Divide & conquer (ultimately termsEnum.seek())

    Iterator<std::shared_ptr<Cell>> subCellsIter = findSubCellsToVisit(cell);
    if (!subCellsIter->hasNext()) // not expected
    {
      return;
    }
    curVNode->children = make_shared<VNodeCellIterator>(
        shared_from_this(), subCellsIter, make_shared<VNode>(curVNode));

  } else {
    // Scan (loop of termsEnum.next())

    this->scan(outerInstance->detailLevel);
  }
}

shared_ptr<CellIterator>
AbstractVisitingPrefixTreeQuery::VisitorTemplate::findSubCellsToVisit(
    shared_ptr<Cell> cell)
{
  return cell->getNextLevelCells(outerInstance->queryShape);
}

void AbstractVisitingPrefixTreeQuery::VisitorTemplate::scan(
    int scanDetailLevel) 
{
  // note: this can be a do-while instead in 6x; 5x has a back-compat with
  // redundant leaves -- LUCENE-4942
  while (curVNode->cell->isPrefixOf(indexedCell)) {
    if (indexedCell->getLevel() == scanDetailLevel ||
        (indexedCell->getLevel() < scanDetailLevel && indexedCell->isLeaf())) {
      visitScanned(indexedCell);
    }
    // advance
    if (!nextTerm()) {
      break;
    }
  }
}

bool AbstractVisitingPrefixTreeQuery::VisitorTemplate::nextTerm() throw(
    IOException)
{
  if ((thisTerm = termsEnum->next()) == nullptr) {
    return false;
  }
  indexedCell = outerInstance->grid->readCell(thisTerm, indexedCell);
  return true;
}

AbstractVisitingPrefixTreeQuery::VisitorTemplate::VNodeCellIterator::
    VNodeCellIterator(
        shared_ptr<AbstractVisitingPrefixTreeQuery::VisitorTemplate>
            outerInstance,
        shared_ptr<Iterator<std::shared_ptr<Cell>>> cellIter,
        shared_ptr<VNode> vNode)
    : cellIter(cellIter), vNode(vNode), outerInstance(outerInstance)
{
}

bool AbstractVisitingPrefixTreeQuery::VisitorTemplate::VNodeCellIterator::
    hasNext()
{
  return cellIter->hasNext();
}

shared_ptr<VNode>
AbstractVisitingPrefixTreeQuery::VisitorTemplate::VNodeCellIterator::next()
{
  assert(hasNext());
  vNode->reset(cellIter->next());
  return vNode;
}

void AbstractVisitingPrefixTreeQuery::VisitorTemplate::VNodeCellIterator::
    remove()
{ // it always removes
}

void AbstractVisitingPrefixTreeQuery::VisitorTemplate::visitScanned(
    shared_ptr<Cell> cell) 
{
  shared_ptr<SpatialRelation> *const relate =
      cell->getShape()->relate(outerInstance->queryShape);
  if (relate->intersects()) {
    cell->setShapeRel(relate); // just being pedantic
    if (cell->isLeaf()) {
      visitLeaf(cell);
    } else {
      visitPrefix(cell);
    }
  }
}

void AbstractVisitingPrefixTreeQuery::VisitorTemplate::preSiblings(
    shared_ptr<VNode> vNode) 
{
}

void AbstractVisitingPrefixTreeQuery::VisitorTemplate::postSiblings(
    shared_ptr<VNode> vNode) 
{
}

AbstractVisitingPrefixTreeQuery::VNode::VNode(shared_ptr<VNode> parent)
    : parent(parent)
{ // remember to call reset(cell) after
}

void AbstractVisitingPrefixTreeQuery::VNode::reset(shared_ptr<Cell> cell)
{
  assert(cell != nullptr);
  this->cell = cell;
  assert(children == nullptr);
}
} // namespace org::apache::lucene::spatial::prefix