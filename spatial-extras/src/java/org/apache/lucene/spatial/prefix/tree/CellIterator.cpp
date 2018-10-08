using namespace std;

#include "CellIterator.h"

namespace org::apache::lucene::spatial::prefix::tree
{

shared_ptr<Cell> CellIterator::thisCell()
{
  assert((thisCell_ != nullptr,
          L"Only call thisCell() after next(), not hasNext()"));
  return thisCell_;
}

shared_ptr<Cell> CellIterator::nextFrom(shared_ptr<Cell> fromCell)
{
  while (true) {
    if (!hasNext()) {
      return nullptr;
    }
    shared_ptr<Cell> c = next(); // will update thisCell
    if (c->compareToNoLeaf(fromCell) >= 0) {
      return c;
    }
  }
}

void CellIterator::remove() { assert(thisCell_ != nullptr); }

shared_ptr<Cell> CellIterator::next()
{
  if (nextCell == nullptr) {
    if (!hasNext()) {
      // C++ TODO: The following line could not be converted:
      throw java.util.NoSuchElementException();
    }
  }
  thisCell_ = nextCell;
  nextCell.reset();
  return thisCell_;
}
} // namespace org::apache::lucene::spatial::prefix::tree