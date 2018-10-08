using namespace std;

#include "DisjunctionIntervalsSource.h"

namespace org::apache::lucene::search::intervals
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

DisjunctionIntervalsSource::DisjunctionIntervalsSource(
    deque<std::shared_ptr<IntervalsSource>> &subSources)
    : subSources(subSources)
{
}

shared_ptr<IntervalIterator> DisjunctionIntervalsSource::intervals(
    const wstring &field, shared_ptr<LeafReaderContext> ctx) 
{
  deque<std::shared_ptr<IntervalIterator>> subIterators =
      deque<std::shared_ptr<IntervalIterator>>();
  for (auto subSource : subSources) {
    shared_ptr<IntervalIterator> it = subSource->intervals(field, ctx);
    if (it != nullptr) {
      subIterators.push_back(it);
    }
  }
  if (subIterators.empty()) {
    return nullptr;
  }
  return make_shared<DisjunctionIntervalIterator>(subIterators);
}

bool DisjunctionIntervalsSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<DisjunctionIntervalsSource> that =
      any_cast<std::shared_ptr<DisjunctionIntervalsSource>>(o);
  return Objects::equals(subSources, that->subSources);
}

int DisjunctionIntervalsSource::hashCode() { return Objects::hash(subSources); }

wstring DisjunctionIntervalsSource::toString()
{
  return subSources.stream()
      .map_obj(any::toString)
      .collect(Collectors::joining(L",", L"or(", L")"));
}

void DisjunctionIntervalsSource::extractTerms(
    const wstring &field, shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto source : subSources) {
    source->extractTerms(field, terms);
  }
}

DisjunctionIntervalsSource::DisjunctionIntervalIterator::
    DisjunctionIntervalIterator(
        deque<std::shared_ptr<IntervalIterator>> &iterators)
    : approximation(make_shared<DisjunctionDISIApproximation>(disiQueue)),
      intervalQueue(
          make_shared<PriorityQueue<std::shared_ptr<IntervalIterator>>>(
              iterators.size())),
      disiQueue(make_shared<DisiPriorityQueue>(iterators.size())),
      iterators(iterators), matchCost(costsum)
{
  for (auto it : iterators) {
    disiQueue->push_back(make_shared<DisiWrapper>(it));
  }
  {protected : bool lessThan(IntervalIterator a, IntervalIterator b){
      return a::end() < b::end() ||
             (a::end() == b::end() && a::start() >= b::start());
}
}; // namespace org::apache::lucene::search::intervals
float costsum = 0;
for (auto it : iterators) {
  costsum += it->cost();
}
}

float DisjunctionIntervalsSource::DisjunctionIntervalIterator::matchCost()
{
  return matchCost_;
}

int DisjunctionIntervalsSource::DisjunctionIntervalIterator::start()
{
  return current->start();
}

int DisjunctionIntervalsSource::DisjunctionIntervalIterator::end()
{
  return current->end();
}

void DisjunctionIntervalsSource::DisjunctionIntervalIterator::reset() throw(
    IOException)
{
  intervalQueue->clear();
  for (shared_ptr<DisiWrapper> dw = disiQueue->topList(); dw != nullptr;
       dw = dw->next) {
    dw->intervals.nextInterval();
    intervalQueue->push_back(dw->intervals);
  }
  current = EMPTY;
}

int DisjunctionIntervalsSource::DisjunctionIntervalIterator::
    nextInterval() 
{
  if (current == EMPTY) {
    if (intervalQueue->size() > 0) {
      current = intervalQueue->top();
    }
    return current->start();
  }
  int start = current->start(), end = current->end();
  while (intervalQueue->size() > 0 &&
         contains(intervalQueue->top(), start, end)) {
    shared_ptr<IntervalIterator> it = intervalQueue->pop();
    if (it != nullptr && it->nextInterval() != NO_MORE_INTERVALS) {
      intervalQueue->push_back(it);
    }
  }
  if (intervalQueue->empty()) {
    current = EMPTY;
    return NO_MORE_INTERVALS;
  }
  current = intervalQueue->top();
  return current->start();
}

bool DisjunctionIntervalsSource::DisjunctionIntervalIterator::contains(
    shared_ptr<IntervalIterator> it, int start, int end)
{
  return start >= it->start() && start <= it->end() && end >= it->start() &&
         end <= it->end();
}

int DisjunctionIntervalsSource::DisjunctionIntervalIterator::docID()
{
  return approximation->docID();
}

int DisjunctionIntervalsSource::DisjunctionIntervalIterator::nextDoc() throw(
    IOException)
{
  int doc = approximation->nextDoc();
  reset();
  return doc;
}

int DisjunctionIntervalsSource::DisjunctionIntervalIterator::advance(
    int target) 
{
  int doc = approximation->advance(target);
  reset();
  return doc;
}

int64_t DisjunctionIntervalsSource::DisjunctionIntervalIterator::cost()
{
  return approximation->cost();
}

const shared_ptr<IntervalIterator> DisjunctionIntervalsSource::EMPTY =
    make_shared<IntervalIteratorAnonymousInnerClass>();

DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::
    IntervalIteratorAnonymousInnerClass()
{
}

int DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::docID()
{
  throw make_shared<UnsupportedOperationException>();
}

int DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::
    nextDoc() 
{
  throw make_shared<UnsupportedOperationException>();
}

int DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::advance(
    int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t
DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::cost()
{
  throw make_shared<UnsupportedOperationException>();
}

int DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::start()
{
  return -1;
}

int DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::end()
{
  return -1;
}

int DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::
    nextInterval()
{
  return NO_MORE_INTERVALS;
}

float DisjunctionIntervalsSource::IntervalIteratorAnonymousInnerClass::
    matchCost()
{
  return 0;
}
}