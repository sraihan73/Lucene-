using namespace std;

#include "ConjunctionSpans.h"

namespace org::apache::lucene::search::spans
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using ConjunctionDISI = org::apache::lucene::search::ConjunctionDISI;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

ConjunctionSpans::ConjunctionSpans(deque<std::shared_ptr<Spans>> &subSpans)
    : subSpans(subSpans.toArray(
          std::deque<std::shared_ptr<Spans>>(subSpans.size()))),
      conjunction(ConjunctionDISI::intersectSpans(subSpans))
{
  if (subSpans.size() < 2) {
    throw invalid_argument(L"Less than 2 subSpans.size():" + subSpans.size());
  }
  this->atFirstInCurrentDoc =
      true; // ensure for doc -1 that start/end positions are -1
}

int ConjunctionSpans::docID() { return conjunction->docID(); }

int64_t ConjunctionSpans::cost() { return conjunction->cost(); }

int ConjunctionSpans::nextDoc() 
{
  return (conjunction->nextDoc() == NO_MORE_DOCS) ? NO_MORE_DOCS : toMatchDoc();
}

int ConjunctionSpans::advance(int target) 
{
  return (conjunction->advance(target) == NO_MORE_DOCS) ? NO_MORE_DOCS
                                                        : toMatchDoc();
}

int ConjunctionSpans::toMatchDoc() 
{
  oneExhaustedInCurrentDoc = false;
  while (true) {
    if (twoPhaseCurrentDocMatches()) {
      return docID();
    }
    if (conjunction->nextDoc() == NO_MORE_DOCS) {
      return NO_MORE_DOCS;
    }
  }
}

shared_ptr<TwoPhaseIterator> ConjunctionSpans::asTwoPhaseIterator()
{
  float totalMatchCost = 0;
  // Compute the matchCost as the total matchCost/positionsCostant of the sub
  // spans.
  for (auto spans : subSpans) {
    shared_ptr<TwoPhaseIterator> tpi = spans->asTwoPhaseIterator();
    if (tpi != nullptr) {
      totalMatchCost += tpi->matchCost();
    } else {
      totalMatchCost += spans->positionsCost();
    }
  }
  constexpr float matchCost = totalMatchCost;

  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(
      shared_from_this(), conjunction, matchCost);
}

ConjunctionSpans::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConjunctionSpans> outerInstance,
        shared_ptr<DocIdSetIterator> conjunction, float matchCost)
    : org::apache::lucene::search::TwoPhaseIterator(conjunction)
{
  this->outerInstance = outerInstance;
  this->matchCost = matchCost;
}

bool ConjunctionSpans::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  return outerInstance->twoPhaseCurrentDocMatches();
}

float ConjunctionSpans::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return matchCost;
}

float ConjunctionSpans::positionsCost()
{
  throw make_shared<UnsupportedOperationException>(); // asTwoPhaseIterator
                                                      // never returns null
                                                      // here.
}

std::deque<std::shared_ptr<Spans>> ConjunctionSpans::getSubSpans()
{
  return subSpans;
}
} // namespace org::apache::lucene::search::spans