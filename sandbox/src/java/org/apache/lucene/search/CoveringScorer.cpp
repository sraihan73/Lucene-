using namespace std;

#include "CoveringScorer.h"

namespace org::apache::lucene::search
{

CoveringScorer::CoveringScorer(
    shared_ptr<Weight> weight,
    shared_ptr<deque<std::shared_ptr<Scorer>>> scorers,
    shared_ptr<LongValues> minMatchValues, int maxDoc)
    : Scorer(weight), numScorers(scorers->size()), maxDoc(maxDoc),
      minMatchValues(minMatchValues),
      subScorers(make_shared<DisiPriorityQueue>(scorers->size())),
      cost(scorers->stream()
               .map_obj(Scorer::iterator)
               .mapToLong(DocIdSetIterator::cost)
               .sum())
{

  this->doc = -1;

  for (auto scorer : scorers) {
    subScorers->push_back(make_shared<DisiWrapper>(scorer));
  }
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
CoveringScorer::getChildren() 
{
  deque<std::shared_ptr<ChildScorer>> matchingChildren =
      deque<std::shared_ptr<ChildScorer>>();
  setTopListAndFreqIfNecessary();
  for (shared_ptr<DisiWrapper> s = topList; s != nullptr; s = s->next) {
    matchingChildren.push_back(make_shared<ChildScorer>(s->scorer, L"SHOULD"));
  }
  return matchingChildren;
}

CoveringScorer::DocIdSetIteratorAnonymousInnerClass::
    DocIdSetIteratorAnonymousInnerClass()
{
}

int CoveringScorer::DocIdSetIteratorAnonymousInnerClass::docID()
{
  return outerInstance->doc;
}

int CoveringScorer::DocIdSetIteratorAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  return advance(outerInstance->docID() + 1);
}

int CoveringScorer::DocIdSetIteratorAnonymousInnerClass::advance(
    int target) 
{
  // reset state
  outerInstance->matches = false;
  outerInstance->topList.smartpointerreset();

  outerInstance->doc = target;
  setMinMatch();

  shared_ptr<DisiWrapper> top = outerInstance->subScorers.top();
  int numMatches = 0;
  int maxPotentialMatches = outerInstance->numScorers;
  while (top->doc < target) {
    if (maxPotentialMatches < outerInstance->minMatch) {
      // No need to keep trying to advance to `target` since no match is
      // possible.
      if (target >= outerInstance->maxDoc - 1) {
        outerInstance->doc = NO_MORE_DOCS;
      } else {
        outerInstance->doc = target + 1;
      }
      setMinMatch();
      return outerInstance->doc;
    }
    top->doc = top->iterator->advance(target);
    bool match = top->doc == target;
    top = outerInstance->subScorers.updateTop();
    if (match) {
      numMatches++;
      if (numMatches >= outerInstance->minMatch) {
        // success, no need to check other iterators
        outerInstance->matches = true;
        return outerInstance->doc;
      }
    } else {
      maxPotentialMatches--;
    }
  }

  outerInstance->doc = top->doc;
  setMinMatch();
  return outerInstance->doc;
}

void CoveringScorer::DocIdSetIteratorAnonymousInnerClass::setMinMatch() throw(
    IOException)
{
  if (outerInstance->doc >= outerInstance->maxDoc) {
    // advanceExact may not be called on out-of-range doc ids
    outerInstance->minMatch = 1;
  } else if (outerInstance->minMatchValues.advanceExact(outerInstance->doc)) {
    // values < 1 are treated as 1: we require at least one match
    outerInstance->minMatch = max(1, outerInstance->minMatchValues.longValue());
  } else {
    // this will make sure the document does not match
    outerInstance->minMatch = numeric_limits<int64_t>::max();
  }
}

int64_t CoveringScorer::DocIdSetIteratorAnonymousInnerClass::cost()
{
  return outerInstance->maxDoc;
}

CoveringScorer::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(shared_ptr<UnknownType> approximation)
    : TwoPhaseIterator(approximation)
{
}

bool CoveringScorer::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  if (outerInstance->matches) {
    return true;
  }
  if (outerInstance->topList == nullptr) {
    outerInstance->advanceAll(outerInstance->doc);
  }
  if (outerInstance->subScorers.top().doc != outerInstance->doc) {
    assert(outerInstance->subScorers.top().doc > outerInstance->doc);
    return false;
  }
  outerInstance->setTopListAndFreq();
  assert(outerInstance->topList->doc == outerInstance->doc);
  return outerInstance->matches =
             outerInstance->freq >= outerInstance->minMatch;
}

float CoveringScorer::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return outerInstance->numScorers;
}

shared_ptr<DocIdSetIterator> CoveringScorer::iterator()
{
  return TwoPhaseIterator::asDocIdSetIterator(twoPhase);
}

shared_ptr<TwoPhaseIterator> CoveringScorer::twoPhaseIterator()
{
  return twoPhase;
}

void CoveringScorer::advanceAll(int target) 
{
  shared_ptr<DisiWrapper> top = subScorers->top();
  while (top->doc < target) {
    top->doc = top->iterator->advance(target);
    top = subScorers->updateTop();
  }
}

void CoveringScorer::setTopListAndFreq()
{
  topList = subScorers->topList();
  freq = 0;
  for (shared_ptr<DisiWrapper> w = topList; w != nullptr; w = w->next) {
    freq++;
  }
}

void CoveringScorer::setTopListAndFreqIfNecessary() 
{
  if (topList == nullptr) {
    advanceAll(doc);
    setTopListAndFreq();
  }
}

float CoveringScorer::score() 
{
  // we need to know about all matches
  setTopListAndFreqIfNecessary();
  double score = 0;
  for (shared_ptr<DisiWrapper> w = topList; w != nullptr; w = w->next) {
    score += w->scorer->score();
  }
  return static_cast<float>(score);
}

int CoveringScorer::docID() { return doc; }
} // namespace org::apache::lucene::search