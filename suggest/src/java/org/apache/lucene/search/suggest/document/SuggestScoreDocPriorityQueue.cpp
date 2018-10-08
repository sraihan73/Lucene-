using namespace std;

#include "SuggestScoreDocPriorityQueue.h"

namespace org::apache::lucene::search::suggest::document
{
using SuggestScoreDoc = org::apache::lucene::search::suggest::document::
    TopSuggestDocs::SuggestScoreDoc;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

SuggestScoreDocPriorityQueue::SuggestScoreDocPriorityQueue(int size)
    : org::apache::lucene::util::PriorityQueue<
          org::apache::lucene::search::suggest::document::TopSuggestDocs::
              SuggestScoreDoc>(size)
{
}

bool SuggestScoreDocPriorityQueue::lessThan(shared_ptr<SuggestScoreDoc> a,
                                            shared_ptr<SuggestScoreDoc> b)
{
  if (a->score == b->score) {
    // prefer smaller doc id, in case of a tie
    return a->doc > b->doc;
  }
  return a->score < b->score;
}

std::deque<std::shared_ptr<SuggestScoreDoc>>
SuggestScoreDocPriorityQueue::getResults()
{
  int size = this->size();
  std::deque<std::shared_ptr<SuggestScoreDoc>> res(size);
  for (int i = size - 1; i >= 0; i--) {
    res[i] = pop();
  }
  return res;
}
} // namespace org::apache::lucene::search::suggest::document