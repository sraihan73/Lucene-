using namespace std;

#include "SuggestWordQueue.h"

namespace org::apache::lucene::search::spell
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
const shared_ptr<java::util::Comparator<std::shared_ptr<SuggestWord>>>
    SuggestWordQueue::DEFAULT_COMPARATOR =
        make_shared<SuggestWordScoreComparator>();

SuggestWordQueue::SuggestWordQueue(int size)
    : org::apache::lucene::util::PriorityQueue<SuggestWord>(size)
{
  comparator = DEFAULT_COMPARATOR;
}

SuggestWordQueue::SuggestWordQueue(
    int size, shared_ptr<Comparator<std::shared_ptr<SuggestWord>>> comparator)
    : org::apache::lucene::util::PriorityQueue<SuggestWord>(size)
{
  this->comparator = comparator;
}

bool SuggestWordQueue::lessThan(shared_ptr<SuggestWord> wa,
                                shared_ptr<SuggestWord> wb)
{
  int val = comparator->compare(wa, wb);
  return val < 0;
}
} // namespace org::apache::lucene::search::spell