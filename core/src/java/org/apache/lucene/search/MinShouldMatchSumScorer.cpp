using namespace std;

#include "MinShouldMatchSumScorer.h"

namespace org::apache::lucene::search
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
//    import static org.apache.lucene.search.DisiPriorityQueue.leftNode;
//    import static org.apache.lucene.search.DisiPriorityQueue.parentNode;
//    import static org.apache.lucene.search.DisiPriorityQueue.rightNode;

int64_t MinShouldMatchSumScorer::cost(shared_ptr<LongStream> costs,
                                        int numScorers, int minShouldMatch)
{
  // the idea here is the following: a bool query c1,c2,...cn with
  // minShouldMatch=m could be rewritten to: (c1 AND (c2..cn|msm=m-1)) OR (!c1
  // AND (c2..cn|msm=m)) if we assume that clauses come in ascending cost, then
  // the cost of the first part is the cost of c1 (because the cost of a
  // conjunction is the cost of the least costly clause) the cost of the second
  // part is the cost of finding m matches among the c2...cn remaining clauses
  // since it is a disjunction overall, the total cost is the sum of the costs
  // of these two parts

  // If we recurse infinitely, we find out that the cost of a msm query is the
  // sum of the costs of the num_scorers - minShouldMatch + 1 least costly
  // scorers
  shared_ptr<PriorityQueue<int64_t>> *const pq =
      make_shared<PriorityQueueAnonymousInnerClass>(numScorers -
                                                    minShouldMatch + 1);
  costs->forEach(pq::insertWithOverflow);
  return StreamSupport::stream(pq->spliterator(), false)
      .mapToLong(Number::longValue)
      .sum();
}

MinShouldMatchSumScorer::PriorityQueueAnonymousInnerClass::
    PriorityQueueAnonymousInnerClass(int numScorers)
    : org::apache::lucene::util::PriorityQueue<long>(numScorers -
                                                     minShouldMatch + 1)
{
}

bool MinShouldMatchSumScorer::PriorityQueueAnonymousInnerClass::lessThan(
    optional<int64_t> &a, optional<int64_t> &b)
{
  return a > b;
}

MinShouldMatchSumScorer::MinShouldMatchSumScorer(
    shared_ptr<Weight> weight,
    shared_ptr<deque<std::shared_ptr<Scorer>>> scorers, int minShouldMatch)
    : Scorer(weight), minShouldMatch(minShouldMatch),
      head(
          make_shared<DisiPriorityQueue>(scorers->size() - minShouldMatch + 1)),
      tail(std::deque<std::shared_ptr<DisiWrapper>>(minShouldMatch - 1)),
      cost(cost(scorers->stream()
                    .map_obj(Scorer::iterator)
                    .mapToLong(DocIdSetIterator::cost),
                scorers->size(), minShouldMatch))
{

  if (minShouldMatch > scorers->size()) {
    throw invalid_argument(
        L"minShouldMatch should be <= the number of scorers");
  }
  if (minShouldMatch < 1) {
    throw invalid_argument(L"minShouldMatch should be >= 1");
  }

  this->doc = -1;

  // there can be at most minShouldMatch - 1 scorers beyond the current position
  // otherwise we might be skipping over matching documents

  for (auto scorer : scorers) {
    addLead(make_shared<DisiWrapper>(scorer));
  }
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
MinShouldMatchSumScorer::getChildren() 
{
  deque<std::shared_ptr<ChildScorer>> matchingChildren =
      deque<std::shared_ptr<ChildScorer>>();
  updateFreq();
  for (shared_ptr<DisiWrapper> s = lead; s != nullptr; s = s->next) {
    matchingChildren.push_back(make_shared<ChildScorer>(s->scorer, L"SHOULD"));
  }
  return matchingChildren;
}

shared_ptr<DocIdSetIterator> MinShouldMatchSumScorer::iterator()
{
  return TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator());
}

shared_ptr<TwoPhaseIterator> MinShouldMatchSumScorer::twoPhaseIterator()
{
  shared_ptr<DocIdSetIterator> approximation =
      make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                          approximation);
}

MinShouldMatchSumScorer::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<MinShouldMatchSumScorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int MinShouldMatchSumScorer::DocIdSetIteratorAnonymousInnerClass::docID()
{
  assert(outerInstance->doc == outerInstance->lead->doc);
  return outerInstance->doc;
}

int MinShouldMatchSumScorer::DocIdSetIteratorAnonymousInnerClass::
    nextDoc() 
{
  // We are moving to the next doc ID, so scorers in 'lead' need to go in
  // 'tail'. If there is not enough space in 'tail', then we take the least
  // costly scorers and advance them.
  for (shared_ptr<DisiWrapper> s = outerInstance->lead; s != nullptr;
       s = s->next) {
    shared_ptr<DisiWrapper> *const evicted =
        outerInstance->insertTailWithOverFlow(s);
    if (evicted != nullptr) {
      if (evicted->doc == outerInstance->doc) {
        evicted->doc = evicted->iterator->nextDoc();
      } else {
        evicted->doc = evicted->iterator->advance(outerInstance->doc + 1);
      }
      outerInstance->head->push_back(evicted);
    }
  }

  outerInstance->setDocAndFreq();
  // It would be correct to return doNextCandidate() at this point but if you
  // call nextDoc as opposed to advance, it probably means that you really
  // need the next match. Returning 'doc' here would lead to a similar
  // iteration over sub postings overall except that the decision making would
  // happen at a higher level where more abstractions are involved and
  // benchmarks suggested it causes a significant performance hit.
  return outerInstance->doNext();
}

int MinShouldMatchSumScorer::DocIdSetIteratorAnonymousInnerClass::advance(
    int target) 
{
  // Same logic as in nextDoc
  for (shared_ptr<DisiWrapper> s = outerInstance->lead; s != nullptr;
       s = s->next) {
    shared_ptr<DisiWrapper> *const evicted =
        outerInstance->insertTailWithOverFlow(s);
    if (evicted != nullptr) {
      evicted->doc = evicted->iterator->advance(target);
      outerInstance->head->push_back(evicted);
    }
  }

  // But this time there might also be scorers in 'head' behind the desired
  // target so we need to do the same thing that we did on 'lead' on 'head'
  shared_ptr<DisiWrapper> headTop = outerInstance->head->top();
  while (headTop->doc < target) {
    shared_ptr<DisiWrapper> *const evicted =
        outerInstance->insertTailWithOverFlow(headTop);
    // We know that the tail is full since it contains at most
    // minShouldMatch - 1 entries and we just moved at least minShouldMatch
    // entries to it, so evicted is not null
    evicted->doc = evicted->iterator->advance(target);
    headTop = outerInstance->head->updateTop(evicted);
  }

  outerInstance->setDocAndFreq();
  return outerInstance->doNextCandidate();
}

int64_t MinShouldMatchSumScorer::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return outerInstance->cost_;
}

MinShouldMatchSumScorer::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<MinShouldMatchSumScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator> approximation)
    : TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
}

bool MinShouldMatchSumScorer::TwoPhaseIteratorAnonymousInnerClass::
    matches() 
{
  while (outerInstance->freq < outerInstance->minShouldMatch) {
    assert(outerInstance->freq > 0);
    if (outerInstance->freq + outerInstance->tailSize >=
        outerInstance->minShouldMatch) {
      // a match on doc is still possible, try to
      // advance scorers from the tail
      outerInstance->advanceTail();
    } else {
      return false;
    }
  }
  return true;
}

float MinShouldMatchSumScorer::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  // maximum number of scorer that matches() might advance
  return outerInstance->tail.size();
}

void MinShouldMatchSumScorer::addLead(shared_ptr<DisiWrapper> lead)
{
  lead->next = this->lead;
  this->lead = lead;
  freq += 1;
}

void MinShouldMatchSumScorer::pushBackLeads() 
{
  for (shared_ptr<DisiWrapper> s = lead; s != nullptr; s = s->next) {
    addTail(s);
  }
}

void MinShouldMatchSumScorer::advanceTail(shared_ptr<DisiWrapper> top) throw(
    IOException)
{
  top->doc = top->iterator->advance(doc);
  if (top->doc == doc) {
    addLead(top);
  } else {
    head->push_back(top);
  }
}

void MinShouldMatchSumScorer::advanceTail() 
{
  shared_ptr<DisiWrapper> *const top = popTail();
  advanceTail(top);
}

void MinShouldMatchSumScorer::setDocAndFreq()
{
  assert(head->size() > 0);

  // The top of `head` defines the next potential match
  // pop all documents which are on this doc
  lead = head->pop();
  lead->next.reset();
  freq = 1;
  doc = lead->doc;
  while (head->size() > 0 && head->top()->doc == doc) {
    addLead(head->pop());
  }
}

int MinShouldMatchSumScorer::doNext() 
{
  while (freq < minShouldMatch) {
    assert(freq > 0);
    if (freq + tailSize >= minShouldMatch) {
      // a match on doc is still possible, try to
      // advance scorers from the tail
      advanceTail();
    } else {
      // no match on doc is possible anymore, move to the next potential match
      pushBackLeads();
      setDocAndFreq();
    }
  }

  return doc;
}

int MinShouldMatchSumScorer::doNextCandidate() 
{
  while (freq + tailSize < minShouldMatch) {
    // no match on doc is possible, move to the next potential match
    pushBackLeads();
    setDocAndFreq();
  }

  return doc;
}

void MinShouldMatchSumScorer::updateFreq() 
{
  assert(freq >= minShouldMatch);
  // we return the next doc when there are minShouldMatch matching clauses
  // but some of the clauses in 'tail' might match as well
  // in general we want to advance least-costly clauses first in order to
  // skip over non-matching documents as fast as possible. However here,
  // we are advancing everything anyway so iterating over clauses in
  // (roughly) cost-descending order might help avoid some permutations in
  // the head heap
  for (int i = tailSize - 1; i >= 0; --i) {
    advanceTail(tail[i]);
  }
  tailSize = 0;
}

float MinShouldMatchSumScorer::score() 
{
  // we need to know about all matches
  updateFreq();
  double score = 0;
  for (shared_ptr<DisiWrapper> s = lead; s != nullptr; s = s->next) {
    score += s->scorer->score();
  }
  return static_cast<float>(score);
}

int MinShouldMatchSumScorer::docID()
{
  assert(doc == lead->doc);
  return doc;
}

shared_ptr<DisiWrapper>
MinShouldMatchSumScorer::insertTailWithOverFlow(shared_ptr<DisiWrapper> s)
{
  if (tailSize < tail.size()) {
    addTail(s);
    return nullptr;
  } else if (tail.size() >= 1) {
    shared_ptr<DisiWrapper> *const top = tail[0];
    if (top->cost < s->cost) {
      tail[0] = s;
      downHeapCost(tail, tailSize);
      return top;
    }
  }
  return s;
}

void MinShouldMatchSumScorer::addTail(shared_ptr<DisiWrapper> s)
{
  tail[tailSize] = s;
  upHeapCost(tail, tailSize);
  tailSize += 1;
}

shared_ptr<DisiWrapper> MinShouldMatchSumScorer::popTail()
{
  assert(tailSize > 0);
  shared_ptr<DisiWrapper> *const result = tail[0];
  tail[0] = tail[--tailSize];
  downHeapCost(tail, tailSize);
  return result;
}

void MinShouldMatchSumScorer::upHeapCost(
    std::deque<std::shared_ptr<DisiWrapper>> &heap, int i)
{
  shared_ptr<DisiWrapper> *const node = heap[i];
  constexpr int64_t nodeCost = node->cost;
  int j = parentNode(i);
  while (j >= 0 && nodeCost < heap[j]->cost) {
    heap[i] = heap[j];
    i = j;
    j = parentNode(j);
  }
  heap[i] = node;
}

void MinShouldMatchSumScorer::downHeapCost(
    std::deque<std::shared_ptr<DisiWrapper>> &heap, int size)
{
  int i = 0;
  shared_ptr<DisiWrapper> *const node = heap[0];
  int j = leftNode(i);
  if (j < size) {
    int k = rightNode(j);
    if (k < size && heap[k]->cost < heap[j]->cost) {
      j = k;
    }
    if (heap[j]->cost < node->cost) {
      do {
        heap[i] = heap[j];
        i = j;
        j = leftNode(i);
        k = rightNode(j);
        if (k < size && heap[k]->cost < heap[j]->cost) {
          j = k;
        }
      } while (j < size && heap[j]->cost < node->cost);
      heap[i] = node;
    }
  }
}
} // namespace org::apache::lucene::search