using namespace std;

#include "TreeCellIterator.h"

namespace org::apache::lucene::spatial::prefix::tree
{
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

TreeCellIterator::TreeCellIterator(shared_ptr<Shape> shapeFilter,
                                   int detailLevel, shared_ptr<Cell> parentCell)
    : shapeFilter(shapeFilter),
      iterStack(std::deque<std::shared_ptr<CellIterator>>(detailLevel))
{
  assert(parentCell->getLevel() == 0);
  iterStack[0] = parentCell->getNextLevelCells(shapeFilter);
  stackIdx = 0; // always points to an iter (non-null)
  // note: not obvious but needed to visit the first cell before trying to
  // descend
  descend = false;
}

bool TreeCellIterator::hasNext()
{
  if (nextCell != nullptr) {
    return true;
  }
  while (true) {
    if (stackIdx == -1) // the only condition in which we return false
    {
      return false;
    }
    // If we can descend...
    if (descend && !(stackIdx == iterStack.size() - 1 ||
                     iterStack[stackIdx]->thisCell().isLeaf())) {
      shared_ptr<CellIterator> nextIter =
          iterStack[stackIdx]->thisCell().getNextLevelCells(shapeFilter);
      // push stack
      iterStack[++stackIdx] = nextIter;
    }
    // Get sibling...
    if (iterStack[stackIdx]->hasNext()) {
      nextCell = iterStack[stackIdx]->next();
      // at detailLevel
      if (stackIdx == iterStack.size() - 1 &&
          !(std::dynamic_pointer_cast<Point>(shapeFilter) !=
            nullptr)) // point check is a kludge
      {
        nextCell->setLeaf(); // because at bottom
      }
      break;
    }
    // Couldn't get next; go up...
    // pop stack
    iterStack[stackIdx--].reset();
    descend = false; // so that we don't re-descend where we just were
  }
  assert(nextCell != nullptr);
  descend = true; // reset
  return true;
}

void TreeCellIterator::remove()
{
  assert(thisCell() != nullptr && nextCell == nullptr);
  descend = false;
}
} // namespace org::apache::lucene::spatial::prefix::tree