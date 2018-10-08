using namespace std;

#include "TopOrdAndIntQueue.h"

namespace org::apache::lucene::facet
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

TopOrdAndIntQueue::OrdAndValue::OrdAndValue() {}

TopOrdAndIntQueue::TopOrdAndIntQueue(int topN)
    : org::apache::lucene::util::PriorityQueue<TopOrdAndIntQueue::OrdAndValue>(
          topN, false)
{
}

bool TopOrdAndIntQueue::lessThan(shared_ptr<OrdAndValue> a,
                                 shared_ptr<OrdAndValue> b)
{
  if (a->value < b->value) {
    return true;
  } else if (a->value > b->value) {
    return false;
  } else {
    return a->ord > b->ord;
  }
}
} // namespace org::apache::lucene::facet