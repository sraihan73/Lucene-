using namespace std;

#include "TopOrdAndFloatQueue.h"

namespace org::apache::lucene::facet
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

TopOrdAndFloatQueue::OrdAndValue::OrdAndValue() {}

TopOrdAndFloatQueue::TopOrdAndFloatQueue(int topN)
    : org::apache::lucene::util::PriorityQueue<
          TopOrdAndFloatQueue::OrdAndValue>(topN, false)
{
}

bool TopOrdAndFloatQueue::lessThan(shared_ptr<OrdAndValue> a,
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