using namespace std;

#include "CellToBytesRefIterator50.h"

namespace org::apache::lucene::spatial::prefix
{
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using BytesRef = org::apache::lucene::util::BytesRef;

shared_ptr<BytesRef> CellToBytesRefIterator50::next()
{
  if (repeatCell != nullptr) {
    bytesRef = repeatCell->getTokenBytesWithLeaf(bytesRef);
    repeatCell.reset();
    return bytesRef;
  }
  if (!cellIter->hasNext()) {
    return nullptr;
  }
  shared_ptr<Cell> cell = cellIter->next();
  bytesRef = cell->getTokenBytesNoLeaf(bytesRef);
  if (cell->isLeaf()) {
    repeatCell = cell;
  }
  return bytesRef;
}
} // namespace org::apache::lucene::spatial::prefix