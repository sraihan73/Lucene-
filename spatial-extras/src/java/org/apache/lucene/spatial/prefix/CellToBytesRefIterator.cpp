using namespace std;

#include "CellToBytesRefIterator.h"

namespace org::apache::lucene::spatial::prefix
{
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

void CellToBytesRefIterator::reset(
    shared_ptr<Iterator<std::shared_ptr<Cell>>> cellIter)
{
  this->cellIter = cellIter;
}

shared_ptr<BytesRef> CellToBytesRefIterator::next()
{
  if (!cellIter->hasNext()) {
    return nullptr;
  }
  return cellIter->next().getTokenBytesWithLeaf(bytesRef);
}
} // namespace org::apache::lucene::spatial::prefix