using namespace std;

#include "ReqExclScorer.h"

namespace org::apache::lucene::search
{

ReqExclScorer::ReqExclScorer(shared_ptr<Scorer> reqScorer,
                             shared_ptr<Scorer> exclScorer)
    : Scorer(reqScorer->weight), reqScorer(reqScorer),
      reqTwoPhaseIterator(reqScorer->twoPhaseIterator()),
      exclTwoPhaseIterator(exclScorer->twoPhaseIterator())
{
  if (reqTwoPhaseIterator == nullptr) {
    reqApproximation = reqScorer->begin();
  } else {
    reqApproximation = reqTwoPhaseIterator->approximation();
  }
  if (exclTwoPhaseIterator == nullptr) {
    exclApproximation = exclScorer->begin();
  } else {
    exclApproximation = exclTwoPhaseIterator->approximation();
  }
}

bool ReqExclScorer::matchesOrNull(shared_ptr<TwoPhaseIterator> it) throw(
    IOException)
{
  return it == nullptr || it->matches();
}

shared_ptr<DocIdSetIterator> ReqExclScorer::iterator()
{
  return TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator());
}

int ReqExclScorer::docID() { return reqApproximation->docID(); }

float ReqExclScorer::score() 
{
  return reqScorer->score(); // reqScorer may be null when next() or skipTo()
                             // already return false
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
ReqExclScorer::getChildren()
{
  return Collections::singleton(make_shared<ChildScorer>(reqScorer, L"MUST"));
}

float ReqExclScorer::matchCost(
    shared_ptr<DocIdSetIterator> reqApproximation,
    shared_ptr<TwoPhaseIterator> reqTwoPhaseIterator,
    shared_ptr<DocIdSetIterator> exclApproximation,
    shared_ptr<TwoPhaseIterator> exclTwoPhaseIterator)
{
  float matchCost = 2; // we perform 2 comparisons to advance exclApproximation
  if (reqTwoPhaseIterator != nullptr) {
    // this two-phase iterator must always be matched
    matchCost += reqTwoPhaseIterator->matchCost();
  }

  // match cost of the prohibited clause: we need to advance the approximation
  // and match the two-phased iterator
  constexpr float exclMatchCost =
      ADVANCE_COST +
      (exclTwoPhaseIterator == nullptr ? 0 : exclTwoPhaseIterator->matchCost());

  // upper value for the ratio of documents that reqApproximation matches that
  // exclApproximation also matches
  float ratio;
  if (reqApproximation->cost() <= 0) {
    ratio = 1.0f;
  } else if (exclApproximation->cost() <= 0) {
    ratio = 0.0f;
  } else {
    ratio = static_cast<float>(
                min(reqApproximation->cost(), exclApproximation->cost())) /
            reqApproximation->cost();
  }
  matchCost += ratio * exclMatchCost;

  return matchCost;
}

shared_ptr<TwoPhaseIterator> ReqExclScorer::twoPhaseIterator()
{
  constexpr float matchCost =
      ReqExclScorer::matchCost(reqApproximation, reqTwoPhaseIterator,
                               exclApproximation, exclTwoPhaseIterator);

  if (reqTwoPhaseIterator == nullptr ||
      (exclTwoPhaseIterator != nullptr &&
       reqTwoPhaseIterator->matchCost() <= exclTwoPhaseIterator->matchCost())) {
    // reqTwoPhaseIterator is LESS costly than exclTwoPhaseIterator, check it
    // first
    return make_shared<TwoPhaseIteratorAnonymousInnerClass>(
        shared_from_this(), reqApproximation, matchCost);
  } else {
    // reqTwoPhaseIterator is MORE costly than exclTwoPhaseIterator, check it
    // last
    return make_shared<TwoPhaseIteratorAnonymousInnerClass2>(
        shared_from_this(), reqApproximation, matchCost);
  }
}

ReqExclScorer::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ReqExclScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            reqApproximation,
        float matchCost)
    : TwoPhaseIterator(reqApproximation)
{
  this->outerInstance = outerInstance;
  this->matchCost = matchCost;
}

bool ReqExclScorer::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  constexpr int doc = outerInstance->reqApproximation->docID();
  // check if the doc is not excluded
  int exclDoc = outerInstance->exclApproximation->docID();
  if (exclDoc < doc) {
    exclDoc = outerInstance->exclApproximation->advance(doc);
  }
  if (exclDoc != doc) {
    return matchesOrNull(outerInstance->reqTwoPhaseIterator);
  }
  return matchesOrNull(outerInstance->reqTwoPhaseIterator) &&
         !matchesOrNull(outerInstance->exclTwoPhaseIterator);
}

float ReqExclScorer::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return matchCost;
}

ReqExclScorer::TwoPhaseIteratorAnonymousInnerClass2::
    TwoPhaseIteratorAnonymousInnerClass2(
        shared_ptr<ReqExclScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            reqApproximation,
        float matchCost)
    : TwoPhaseIterator(reqApproximation)
{
  this->outerInstance = outerInstance;
  this->matchCost = matchCost;
}

bool ReqExclScorer::TwoPhaseIteratorAnonymousInnerClass2::matches() throw(
    IOException)
{
  constexpr int doc = outerInstance->reqApproximation->docID();
  // check if the doc is not excluded
  int exclDoc = outerInstance->exclApproximation->docID();
  if (exclDoc < doc) {
    exclDoc = outerInstance->exclApproximation->advance(doc);
  }
  if (exclDoc != doc) {
    return matchesOrNull(outerInstance->reqTwoPhaseIterator);
  }
  return !matchesOrNull(outerInstance->exclTwoPhaseIterator) &&
         matchesOrNull(outerInstance->reqTwoPhaseIterator);
}

float ReqExclScorer::TwoPhaseIteratorAnonymousInnerClass2::matchCost()
{
  return matchCost;
}
} // namespace org::apache::lucene::search