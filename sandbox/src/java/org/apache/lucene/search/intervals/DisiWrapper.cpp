using namespace std;

#include "DisiWrapper.h"

namespace org::apache::lucene::search::intervals
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

DisiWrapper::DisiWrapper(shared_ptr<IntervalIterator> iterator)
    : iterator(iterator), intervals(iterator), cost(iterator->cost()),
      matchCost(iterator->matchCost()), approximation(iterator)
{
  this->doc = -1;
}
} // namespace org::apache::lucene::search::intervals