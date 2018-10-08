using namespace std;

#include "SpanOrQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using DisiPriorityQueue = org::apache::lucene::search::DisiPriorityQueue;
using DisiWrapper = org::apache::lucene::search::DisiWrapper;
using DisjunctionDISIApproximation =
    org::apache::lucene::search::DisjunctionDISIApproximation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;

SpanOrQuery::SpanOrQuery(deque<SpanQuery> &clauses)
{
  this->clauses = deque<>(clauses.size());
  for (auto seq : clauses) {
    addClause(seq);
  }
}

void SpanOrQuery::addClause(shared_ptr<SpanQuery> clause)
{
  if (field == L"") {
    field = clause->getField();
  } else if (clause->getField() != L"" && clause->getField() != field) {
    throw invalid_argument(L"Clauses must have same field.");
  }
  this->clauses.push_back(clause);
}

std::deque<std::shared_ptr<SpanQuery>> SpanOrQuery::getClauses()
{
  return clauses.toArray(
      std::deque<std::shared_ptr<SpanQuery>>(clauses.size()));
}

wstring SpanOrQuery::getField() { return field; }

shared_ptr<Query>
SpanOrQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<SpanOrQuery> rewritten = make_shared<SpanOrQuery>();
  bool actuallyRewritten = false;
  for (int i = 0; i < clauses.size(); i++) {
    shared_ptr<SpanQuery> c = clauses[i];
    shared_ptr<SpanQuery> query =
        std::static_pointer_cast<SpanQuery>(c->rewrite(reader));
    actuallyRewritten |= query != c;
    rewritten->addClause(query);
  }
  if (actuallyRewritten) {
    return rewritten;
  }
  return SpanQuery::rewrite(reader);
}

wstring SpanOrQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"spanOr([");
  deque<std::shared_ptr<SpanQuery>>::const_iterator i = clauses.begin();
  while (i != clauses.end()) {
    shared_ptr<SpanQuery> clause = *i;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(clause->toString(field));
    if ((*i)->hasNext()) {
      buffer->append(L", ");
    }
    i++;
  }
  buffer->append(L"])");
  return buffer->toString();
}

bool SpanOrQuery::equals(any other)
{
  return sameClassAs(other) &&
         clauses.equals(
             (any_cast<std::shared_ptr<SpanOrQuery>>(other)).clauses);
}

int SpanOrQuery::hashCode() { return classHash() ^ clauses.hashCode(); }

shared_ptr<SpanWeight>
SpanOrQuery::createWeight(shared_ptr<IndexSearcher> searcher, bool needsScores,
                          float boost) 
{
  deque<std::shared_ptr<SpanWeight>> subWeights =
      deque<std::shared_ptr<SpanWeight>>(clauses.size());
  for (auto q : clauses) {
    subWeights.push_back(q->createWeight(searcher, false, boost));
  }
  return make_shared<SpanOrWeight>(
      shared_from_this(), searcher,
      needsScores ? getTermContexts(subWeights) : nullptr, subWeights, boost);
}

SpanOrQuery::SpanOrWeight::SpanOrWeight(
    shared_ptr<SpanOrQuery> outerInstance, shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> &terms,
    deque<std::shared_ptr<SpanWeight>> &subWeights,
    float boost) 
    : SpanWeight(SpanOrQuery::this, searcher, terms, boost),
      subWeights(subWeights), outerInstance(outerInstance)
{
}

void SpanOrQuery::SpanOrWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto w : subWeights) {
    w->extractTerms(terms);
  }
}

bool SpanOrQuery::SpanOrWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  for (auto w : subWeights) {
    if (w->isCacheable(ctx) == false) {
      return false;
    }
  }
  return true;
}

void SpanOrQuery::SpanOrWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  for (auto w : subWeights) {
    w->extractTermContexts(contexts);
  }
}

shared_ptr<Spans> SpanOrQuery::SpanOrWeight::getSpans(
    shared_ptr<LeafReaderContext> context,
    Postings requiredPostings) 
{

  deque<std::shared_ptr<Spans>> subSpans =
      deque<std::shared_ptr<Spans>>(outerInstance->clauses.size());

  for (auto w : subWeights) {
    shared_ptr<Spans> spans = w->getSpans(context, requiredPostings);
    if (spans != nullptr) {
      subSpans.push_back(spans);
    }
  }

  if (subSpans.empty()) {
    return nullptr;
  } else if (subSpans.size() == 1) {
    return subSpans[0];
  }

  shared_ptr<DisiPriorityQueue> byDocQueue =
      make_shared<DisiPriorityQueue>(subSpans.size());
  for (auto spans : subSpans) {
    byDocQueue->push_back(make_shared<DisiWrapper>(spans));
  }

  shared_ptr<SpanPositionQueue> byPositionQueue =
      make_shared<SpanPositionQueue>(subSpans.size()); // when empty use -1

  return make_shared<SpansAnonymousInnerClass>(shared_from_this(), subSpans,
                                               byDocQueue, byPositionQueue);
}

SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::SpansAnonymousInnerClass(
    shared_ptr<SpanOrWeight> outerInstance,
    deque<std::shared_ptr<Spans>> &subSpans,
    shared_ptr<DisiPriorityQueue> byDocQueue,
    shared_ptr<org::apache::lucene::search::spans::SpanPositionQueue>
        byPositionQueue)
{
  this->outerInstance = outerInstance;
  this->subSpans = subSpans;
  this->byDocQueue = byDocQueue;
  this->byPositionQueue = byPositionQueue;
  topPositionSpans = nullptr;
  positionsCost = -1;
  lastDocTwoPhaseMatched = -1;
  cost = -1;
}

int SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  topPositionSpans = nullptr;
  shared_ptr<DisiWrapper> topDocSpans = byDocQueue->top();
  int currentDoc = topDocSpans->doc;
  do {
    topDocSpans->doc = topDocSpans->iterator->nextDoc();
    topDocSpans = byDocQueue->updateTop();
  } while (topDocSpans->doc == currentDoc);
  return topDocSpans->doc;
}

int SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::advance(
    int target) 
{
  topPositionSpans = nullptr;
  shared_ptr<DisiWrapper> topDocSpans = byDocQueue->top();
  do {
    topDocSpans->doc = topDocSpans->iterator->advance(target);
    topDocSpans = byDocQueue->updateTop();
  } while (topDocSpans->doc < target);
  return topDocSpans->doc;
}

int SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::docID()
{
  shared_ptr<DisiWrapper> topDocSpans = byDocQueue->top();
  return topDocSpans->doc;
}

shared_ptr<TwoPhaseIterator>
SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::asTwoPhaseIterator()
{
  float sumMatchCost = 0; // See also DisjunctionScorer.asTwoPhaseIterator()
  int64_t sumApproxCost = 0;

  for (auto w : byDocQueue) {
    if (w->twoPhaseView != nullptr) {
      int64_t costWeight = (w->cost <= 1) ? 1 : w->cost;
      sumMatchCost += w->twoPhaseView->matchCost() * costWeight;
      sumApproxCost += costWeight;
    }
  }

  if (sumApproxCost == 0) { // no sub spans supports approximations
    computePositionsCost();
    return nullptr;
  }

  constexpr float matchCost = sumMatchCost / sumApproxCost;

  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(
      shared_from_this(), make_shared<DisjunctionDISIApproximation>(byDocQueue),
      matchCost);
}

SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<SpansAnonymousInnerClass> outerInstance,
        shared_ptr<DisjunctionDISIApproximation> org, float matchCost)
    : org->apache.lucene.search.TwoPhaseIterator(DisjunctionDISIApproximation)
{
  this->outerInstance = outerInstance;
  this->matchCost = matchCost;
}

bool SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  return twoPhaseCurrentDocMatches();
}

float SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return matchCost;
}

void SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::computePositionsCost()
{
  float sumPositionsCost = 0;
  int64_t sumCost = 0;
  for (auto w : byDocQueue) {
    int64_t costWeight = (w->cost <= 1) ? 1 : w->cost;
    sumPositionsCost += w->spans->positionsCost() * costWeight;
    sumCost += costWeight;
  }
  positionsCost = sumPositionsCost / sumCost;
}

float SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::positionsCost()
{
  // This may be called when asTwoPhaseIterator returned null,
  // which happens when none of the sub spans supports approximations.
  assert(positionsCost > 0);
  return positionsCost;
}

bool SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::
    twoPhaseCurrentDocMatches() 
{
  shared_ptr<DisiWrapper> listAtCurrentDoc = byDocQueue->topList();
  // remove the head of the deque as long as it does not match
  constexpr int currentDoc = listAtCurrentDoc->doc;
  while (listAtCurrentDoc->twoPhaseView != nullptr) {
    if (listAtCurrentDoc->twoPhaseView->matches()) {
      // use this spans for positions at current doc:
      listAtCurrentDoc->lastApproxMatchDoc = currentDoc;
      break;
    }
    // do not use this spans for positions at current doc:
    listAtCurrentDoc->lastApproxNonMatchDoc = currentDoc;
    listAtCurrentDoc = listAtCurrentDoc->next;
    if (listAtCurrentDoc == nullptr) {
      return false;
    }
  }
  lastDocTwoPhaseMatched = currentDoc;
  topPositionSpans = nullptr;
  return true;
}

void SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::
    fillPositionQueue() 
{ // called at first nextStartPosition
  assert(byPositionQueue->empty());
  // add all matching Spans at current doc to byPositionQueue
  shared_ptr<DisiWrapper> listAtCurrentDoc = byDocQueue->topList();
  while (listAtCurrentDoc != nullptr) {
    shared_ptr<Spans> spansAtDoc = listAtCurrentDoc->spans;
    if (lastDocTwoPhaseMatched ==
        listAtCurrentDoc->doc) { // matched by DisjunctionDisiApproximation
      if (listAtCurrentDoc->twoPhaseView !=
          nullptr) { // matched by approximation
        if (listAtCurrentDoc->lastApproxNonMatchDoc ==
            listAtCurrentDoc->doc) { // matches() returned false
          spansAtDoc.reset();
        } else {
          if (listAtCurrentDoc->lastApproxMatchDoc != listAtCurrentDoc->doc) {
            if (!listAtCurrentDoc->twoPhaseView->matches()) {
              spansAtDoc.reset();
            }
          }
        }
      }
    }

    if (spansAtDoc != nullptr) {
      assert(spansAtDoc->docID() == listAtCurrentDoc->doc);
      assert(spansAtDoc->startPosition() == -1);
      spansAtDoc->nextStartPosition();
      assert(spansAtDoc->startPosition() != NO_MORE_POSITIONS);
      byPositionQueue->push_back(spansAtDoc);
    }
    listAtCurrentDoc = listAtCurrentDoc->next;
  }
  assert(byPositionQueue->size() > 0);
}

int SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::
    nextStartPosition() 
{
  if (topPositionSpans == nullptr) {
    byPositionQueue->clear();
    fillPositionQueue(); // fills byPositionQueue at first position
    topPositionSpans = byPositionQueue->top();
  } else {
    topPositionSpans::nextStartPosition();
    topPositionSpans = byPositionQueue->updateTop();
  }
  return topPositionSpans::startPosition();
}

int SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::startPosition()
{
  return topPositionSpans == nullptr ? -1 : topPositionSpans::startPosition();
}

int SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::endPosition()
{
  return topPositionSpans == nullptr ? -1 : topPositionSpans::endPosition();
}

int SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::width()
{
  return topPositionSpans::width();
}

void SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::collect(
    shared_ptr<SpanCollector> collector) 
{
  if (topPositionSpans != nullptr) {
    topPositionSpans::collect(collector);
  }
}

wstring SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::toString()
{
  return L"spanOr(" + outerInstance->outerInstance + L")@" + docID() + L": " +
         startPosition() + L" - " + endPosition();
}

int64_t SpanOrQuery::SpanOrWeight::SpansAnonymousInnerClass::cost()
{
  if (cost == -1) {
    cost = 0;
    for (auto spans : subSpans) {
      cost += spans->cost();
    }
  }
  return cost;
}
} // namespace org::apache::lucene::search::spans