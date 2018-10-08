using namespace std;

#include "BooleanScorer.h"

namespace org::apache::lucene::search
{
using Bits = org::apache::lucene::util::Bits;
using FutureObjects = org::apache::lucene::util::FutureObjects;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

BooleanScorer::BulkScorerAndDoc::BulkScorerAndDoc(
    shared_ptr<BooleanScorer> outerInstance, shared_ptr<BulkScorer> scorer)
    : scorer(scorer), cost(scorer->cost()), outerInstance(outerInstance)
{
  this->next = -1;
}

void BooleanScorer::BulkScorerAndDoc::advance(int min) 
{
  score(outerInstance->orCollector, nullptr, min, min);
}

void BooleanScorer::BulkScorerAndDoc::score(shared_ptr<LeafCollector> collector,
                                            shared_ptr<Bits> acceptDocs,
                                            int min, int max) 
{
  next = scorer->score(collector, acceptDocs, min, max);
}

int64_t
BooleanScorer::cost(shared_ptr<deque<std::shared_ptr<BulkScorer>>> scorers,
                    int minShouldMatch)
{
  shared_ptr<PriorityQueue<std::shared_ptr<BulkScorer>>> *const pq =
      make_shared<PriorityQueueAnonymousInnerClass>(scorers->size() -
                                                    minShouldMatch + 1);
  for (auto scorer : scorers) {
    pq->insertWithOverflow(scorer);
  }
  int64_t cost = 0;
  for (shared_ptr<BulkScorer> scorer = pq->pop(); scorer != nullptr;
       scorer = pq->pop()) {
    cost += scorer->cost();
  }
  return cost;
}

BooleanScorer::PriorityQueueAnonymousInnerClass::
    PriorityQueueAnonymousInnerClass(shared_ptr<UnknownType> size)
    : org::apache::lucene::util::PriorityQueue<BulkScorer>(size() -
                                                           minShouldMatch + 1)
{
}

bool BooleanScorer::PriorityQueueAnonymousInnerClass::lessThan(
    shared_ptr<BulkScorer> a, shared_ptr<BulkScorer> b)
{
  return a->cost() > b->cost();
}

BooleanScorer::HeadPriorityQueue::HeadPriorityQueue(int maxSize)
    : org::apache::lucene::util::PriorityQueue<BulkScorerAndDoc>(maxSize)
{
}

bool BooleanScorer::HeadPriorityQueue::lessThan(shared_ptr<BulkScorerAndDoc> a,
                                                shared_ptr<BulkScorerAndDoc> b)
{
  return a->next < b->next;
}

BooleanScorer::TailPriorityQueue::TailPriorityQueue(int maxSize)
    : org::apache::lucene::util::PriorityQueue<BulkScorerAndDoc>(maxSize)
{
}

bool BooleanScorer::TailPriorityQueue::lessThan(shared_ptr<BulkScorerAndDoc> a,
                                                shared_ptr<BulkScorerAndDoc> b)
{
  return a->cost < b->cost;
}

shared_ptr<BulkScorerAndDoc> BooleanScorer::TailPriorityQueue::get(int i)
{
  FutureObjects::checkIndex(i, size());
  return any_cast<std::shared_ptr<BulkScorerAndDoc>>(getHeapArray()[1 + i]);
}

BooleanScorer::OrCollector::OrCollector(shared_ptr<BooleanScorer> outerInstance)
    : outerInstance(outerInstance)
{
}

void BooleanScorer::OrCollector::setScorer(shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void BooleanScorer::OrCollector::collect(int doc) 
{
  constexpr int i = doc & MASK;
  constexpr int idx = static_cast<int>(static_cast<unsigned int>(i) >> 6);
  outerInstance->matching[idx] |= 1LL << i;
  shared_ptr<Bucket> *const bucket = outerInstance->buckets[i];
  bucket->freq++;
  bucket->score += scorer->score();
}

BooleanScorer::BooleanScorer(
    shared_ptr<BooleanWeight> weight,
    shared_ptr<deque<std::shared_ptr<BulkScorer>>> scorers,
    int minShouldMatch, bool needsScores)
    : leads(std::deque<std::shared_ptr<BulkScorerAndDoc>>(scorers->size())),
      head(
          make_shared<HeadPriorityQueue>(scorers->size() - minShouldMatch + 1)),
      tail(make_shared<TailPriorityQueue>(minShouldMatch - 1)),
      minShouldMatch(minShouldMatch), cost(cost(scorers, minShouldMatch))
{
  if (minShouldMatch < 1 || minShouldMatch > scorers->size()) {
    throw invalid_argument(
        L"minShouldMatch should be within 1..num_scorers. Got " +
        to_wstring(minShouldMatch));
  }
  if (scorers->size() <= 1) {
    throw invalid_argument(
        L"This scorer can only be used with two scorers or more, got " +
        scorers->size());
  }
  for (int i = 0; i < buckets.size(); i++) {
    buckets[i] = make_shared<Bucket>();
  }
  for (auto scorer : scorers) {
    if (needsScores == false) {
      // OrCollector calls score() all the time so we have to explicitly
      // disable scoring in order to avoid decoding useless norms
      scorer = BooleanWeight::disableScoring(scorer);
    }
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    shared_ptr<BulkScorerAndDoc> *const evicted = tail->insertWithOverflow(
        make_shared<BulkScorerAndDoc>(shared_from_this(), scorer));
    if (evicted != nullptr) {
      head->push_back(evicted);
    }
  }
}

int64_t BooleanScorer::cost() { return cost_; }

void BooleanScorer::scoreDocument(shared_ptr<LeafCollector> collector, int base,
                                  int i) 
{
  shared_ptr<FakeScorer> *const fakeScorer = this->fakeScorer;
  shared_ptr<Bucket> *const bucket = buckets[i];
  if (bucket->freq >= minShouldMatch) {
    fakeScorer->score = static_cast<float>(bucket->score);
    constexpr int doc = base | i;
    fakeScorer->doc = doc;
    collector->collect(doc);
  }
  bucket->freq = 0;
  bucket->score = 0;
}

void BooleanScorer::scoreMatches(shared_ptr<LeafCollector> collector,
                                 int base) 
{
  std::deque<int64_t> matching = this->matching;
  for (int idx = 0; idx < matching.size(); idx++) {
    int64_t bits = matching[idx];
    while (bits != 0LL) {
      int ntz = Long::numberOfTrailingZeros(bits);
      int doc = idx << 6 | ntz;
      scoreDocument(collector, base, doc);
      bits ^= 1LL << ntz;
    }
  }
}

void BooleanScorer::scoreWindowIntoBitSetAndReplay(
    shared_ptr<LeafCollector> collector, shared_ptr<Bits> acceptDocs, int base,
    int min, int max, std::deque<std::shared_ptr<BulkScorerAndDoc>> &scorers,
    int numScorers) 
{
  for (int i = 0; i < numScorers; ++i) {
    shared_ptr<BulkScorerAndDoc> *const scorer = scorers[i];
    assert(scorer->next < max);
    scorer->score(orCollector, acceptDocs, min, max);
  }

  scoreMatches(collector, base);
  Arrays::fill(matching, 0LL);
}

shared_ptr<BulkScorerAndDoc> BooleanScorer::advance(int min) 
{
  assert(tail->size() == minShouldMatch - 1);
  shared_ptr<HeadPriorityQueue> *const head = this->head;
  shared_ptr<TailPriorityQueue> *const tail = this->tail;
  shared_ptr<BulkScorerAndDoc> headTop = head->top();
  shared_ptr<BulkScorerAndDoc> tailTop = tail->top();
  while (headTop->next < min) {
    if (tailTop == nullptr || headTop->cost <= tailTop->cost) {
      headTop->advance(min);
      headTop = head->updateTop();
    } else {
      // swap the top of head and tail
      shared_ptr<BulkScorerAndDoc> *const previousHeadTop = headTop;
      tailTop->advance(min);
      headTop = head->updateTop(tailTop);
      tailTop = tail->updateTop(previousHeadTop);
    }
  }
  return headTop;
}

void BooleanScorer::scoreWindowMultipleScorers(
    shared_ptr<LeafCollector> collector, shared_ptr<Bits> acceptDocs,
    int windowBase, int windowMin, int windowMax,
    int maxFreq) 
{
  while (maxFreq < minShouldMatch && maxFreq + tail->size() >= minShouldMatch) {
    // a match is still possible
    shared_ptr<BulkScorerAndDoc> *const candidate = tail->pop();
    candidate->advance(windowMin);
    if (candidate->next < windowMax) {
      leads[maxFreq++] = candidate;
    } else {
      head->push_back(candidate);
    }
  }

  if (maxFreq >= minShouldMatch) {
    // There might be matches in other scorers from the tail too
    for (int i = 0; i < tail->size(); ++i) {
      leads[maxFreq++] = tail[i];
    }
    tail->clear();

    scoreWindowIntoBitSetAndReplay(collector, acceptDocs, windowBase, windowMin,
                                   windowMax, leads, maxFreq);
  }

  // Push back scorers into head and tail
  for (int i = 0; i < maxFreq; ++i) {
    shared_ptr<BulkScorerAndDoc> *const evicted =
        head->insertWithOverflow(leads[i]);
    if (evicted != nullptr) {
      tail->push_back(evicted);
    }
  }
}

void BooleanScorer::scoreWindowSingleScorer(
    shared_ptr<BulkScorerAndDoc> bulkScorer,
    shared_ptr<LeafCollector> collector, shared_ptr<Bits> acceptDocs,
    int windowMin, int windowMax, int max) 
{
  assert(tail->empty());
  constexpr int nextWindowBase = head->top()->next & ~MASK;
  constexpr int end = max(windowMax, min(max, nextWindowBase));

  bulkScorer->score(collector, acceptDocs, windowMin, end);

  // reset the scorer that should be used for the general case
  collector->setScorer(fakeScorer);
}

shared_ptr<BulkScorerAndDoc> BooleanScorer::scoreWindow(
    shared_ptr<BulkScorerAndDoc> top, shared_ptr<LeafCollector> collector,
    shared_ptr<Bits> acceptDocs, int min, int max) 
{
  constexpr int windowBase =
      top->next & ~MASK; // find the window that the next match belongs to
  constexpr int windowMin = max(min, windowBase);
  constexpr int windowMax = min(max, windowBase + SIZE);

  // Fill 'leads' with all scorers from 'head' that are in the right window
  leads[0] = head->pop();
  int maxFreq = 1;
  while (head->size() > 0 && head->top()->next < windowMax) {
    leads[maxFreq++] = head->pop();
  }

  if (minShouldMatch == 1 && maxFreq == 1) {
    // special case: only one scorer can match in the current window,
    // we can collect directly
    shared_ptr<BulkScorerAndDoc> *const bulkScorer = leads[0];
    scoreWindowSingleScorer(bulkScorer, collector, acceptDocs, windowMin,
                            windowMax, max);
    return head->push_back(bulkScorer);
  } else {
    // general case, collect through a bit set first and then replay
    scoreWindowMultipleScorers(collector, acceptDocs, windowBase, windowMin,
                               windowMax, maxFreq);
    return head->top();
  }
}

int BooleanScorer::score(shared_ptr<LeafCollector> collector,
                         shared_ptr<Bits> acceptDocs, int min,
                         int max) 
{
  fakeScorer->doc = -1;
  collector->setScorer(fakeScorer);

  shared_ptr<BulkScorerAndDoc> top = advance(min);
  while (top->next < max) {
    top = scoreWindow(top, collector, acceptDocs, min, max);
  }

  return top->next;
}
} // namespace org::apache::lucene::search