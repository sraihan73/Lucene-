using namespace std;

#include "SpanPositionQueue.h"

namespace org::apache::lucene::search::spans
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

SpanPositionQueue::SpanPositionQueue(int maxSize)
    : org::apache::lucene::util::PriorityQueue<Spans>(maxSize, false)
{
}

bool SpanPositionQueue::lessThan(shared_ptr<Spans> s1, shared_ptr<Spans> s2)
{
  int start1 = s1->startPosition();
  int start2 = s2->startPosition();
  return (start1 < start2)
             ? true
             : (start1 == start2) ? s1->endPosition() < s2->endPosition()
                                  : false;
}
} // namespace org::apache::lucene::search::spans