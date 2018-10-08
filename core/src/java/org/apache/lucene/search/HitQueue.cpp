using namespace std;

#include "HitQueue.h"

namespace org::apache::lucene::search
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

HitQueue::HitQueue(int size, bool prePopulate)
    : org::apache::lucene::util::PriorityQueue<ScoreDoc>(size, prePopulate)
{
}

shared_ptr<ScoreDoc> HitQueue::getSentinelObject()
{
  // Always set the doc Id to MAX_VALUE so that it won't be favored by
  // lessThan. This generally should not happen since if score is not NEG_INF,
  // TopScoreDocCollector will always add the object to the queue.
  return make_shared<ScoreDoc>(numeric_limits<int>::max(),
                               -numeric_limits<float>::infinity());
}

bool HitQueue::lessThan(shared_ptr<ScoreDoc> hitA, shared_ptr<ScoreDoc> hitB)
{
  if (hitA->score == hitB->score) {
    return hitA->doc > hitB->doc;
  } else {
    return hitA->score < hitB->score;
  }
}
} // namespace org::apache::lucene::search