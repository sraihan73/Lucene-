using namespace std;

#include "SingletonCellIterator.h"

namespace org::apache::lucene::spatial::prefix::tree
{

SingletonCellIterator::SingletonCellIterator(shared_ptr<Cell> cell)
{
  this->nextCell = cell; // preload nextCell
}

bool SingletonCellIterator::hasNext()
{
  thisCell_.reset();
  return nextCell != nullptr;
}
} // namespace org::apache::lucene::spatial::prefix::tree