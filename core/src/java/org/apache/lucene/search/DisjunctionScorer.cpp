using namespace std;

#include "DisjunctionScorer.h"

namespace org::apache::lucene::search
{
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

DisjunctionScorer::DisjunctionScorer(
    shared_ptr<Weight> weight, deque<std::shared_ptr<Scorer>> &subScorers,
    bool needsScores)
    : Scorer(weight), needsScores(needsScores),
      subScorers(make_shared<DisiPriorityQueue>(subScorers.size())),
      approximation(make_shared<DisjunctionDISIApproximation>(this->subScorers))
{
  if (subScorers.size() <= 1) {
    throw invalid_argument(L"There must be at least 2 subScorers");
  }
  for (auto scorer : subScorers) {
    shared_ptr<DisiWrapper> *const w = make_shared<DisiWrapper>(scorer);
    this->subScorers->push_back(w);
  }

  bool hasApproximation = false;
  float sumMatchCost = 0;
  int64_t sumApproxCost = 0;
  // Compute matchCost as the average over the matchCost of the subScorers.
  // This is weighted by the cost, which is an expected number of matching
  // documents.
  for (auto w : this->subScorers) {
    int64_t costWeight = (w->cost <= 1) ? 1 : w->cost;
    sumApproxCost += costWeight;
    if (w->twoPhaseView != nullptr) {
      hasApproximation = true;
      sumMatchCost += w->matchCost * costWeight;
    }
  }

  if (hasApproximation == false) { // no sub scorer supports approximations
    twoPhase.reset();
  } else {
    constexpr float matchCost = sumMatchCost / sumApproxCost;
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    twoPhase =
        make_shared<TwoPhase>(shared_from_this(), approximation, matchCost);
  }
}

shared_ptr<DocIdSetIterator> DisjunctionScorer::iterator()
{
  if (twoPhase != nullptr) {
    return TwoPhaseIterator::asDocIdSetIterator(twoPhase);
  } else {
    return approximation;
  }
}

shared_ptr<TwoPhaseIterator> DisjunctionScorer::twoPhaseIterator()
{
  return twoPhase;
}

DisjunctionScorer::TwoPhase::TwoPhase(
    shared_ptr<DisjunctionScorer> outerInstance,
    shared_ptr<DocIdSetIterator> approximation, float matchCost)
    : TwoPhaseIterator(approximation), matchCost(matchCost),
      unverifiedMatches(
          make_shared<PriorityQueue<std::shared_ptr<DisiWrapper>>>(
              outerInstance->subScorers->size())),
      outerInstance(outerInstance)
{
  {protected : bool lessThan(DisiWrapper a,
                             DisiWrapper b){return a::matchCost < b::matchCost;
}
}; // namespace org::apache::lucene::search
}

shared_ptr<DisiWrapper>
DisjunctionScorer::TwoPhase::getSubMatches() 
{
  // iteration order does not matter
  for (auto w : unverifiedMatches) {
    if (w->twoPhaseView->matches()) {
      w->next = verifiedMatches;
      verifiedMatches = w;
    }
  }
  unverifiedMatches->clear();
  return verifiedMatches;
}

bool DisjunctionScorer::TwoPhase::matches() 
{
  verifiedMatches.reset();
  unverifiedMatches->clear();

  for (shared_ptr<DisiWrapper> w = outerInstance->subScorers->topList();
       w != nullptr;) {
    shared_ptr<DisiWrapper> next = w->next;

    if (w->twoPhaseView == nullptr) {
      // implicitly verified, move it to verifiedMatches
      w->next = verifiedMatches;
      verifiedMatches = w;

      if (outerInstance->needsScores == false) {
        // we can stop here
        return true;
      }
    } else {
      unverifiedMatches->push_back(w);
    }
    w = next;
  }

  if (verifiedMatches != nullptr) {
    return true;
  }

  // verify subs that have an two-phase iterator
  // least-costly ones first
  while (unverifiedMatches->size() > 0) {
    shared_ptr<DisiWrapper> w = unverifiedMatches->pop();
    if (w->twoPhaseView->matches()) {
      w->next.reset();
      verifiedMatches = w;
      return true;
    }
  }

  return false;
}

float DisjunctionScorer::TwoPhase::matchCost() { return matchCost_; }

int DisjunctionScorer::docID() { return subScorers->top()->doc; }

shared_ptr<DisiWrapper> DisjunctionScorer::getSubMatches() 
{
  if (twoPhase == nullptr) {
    return subScorers->topList();
  } else {
    return twoPhase->getSubMatches();
  }
}

float DisjunctionScorer::score() 
{
  return score(getSubMatches());
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
DisjunctionScorer::getChildren() 
{
  deque<std::shared_ptr<ChildScorer>> children =
      deque<std::shared_ptr<ChildScorer>>();
  for (shared_ptr<DisiWrapper> scorer = getSubMatches(); scorer != nullptr;
       scorer = scorer->next) {
    children.push_back(make_shared<ChildScorer>(scorer->scorer, L"SHOULD"));
  }
  return children;
}
}