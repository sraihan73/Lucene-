using namespace std;

#include "PhraseScorer.h"

namespace org::apache::lucene::search
{
using Similarity = org::apache::lucene::search::similarities::Similarity;

PhraseScorer::PhraseScorer(shared_ptr<Weight> weight,
                           shared_ptr<PhraseMatcher> matcher, bool needsScores,
                           shared_ptr<Similarity::SimScorer> simScorer)
    : Scorer(weight), matcher(matcher), needsScores(needsScores),
      simScorer(simScorer), matchCost(matcher->getMatchCost())
{
}

shared_ptr<TwoPhaseIterator> PhraseScorer::twoPhaseIterator()
{
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(
      shared_from_this(), matcher->approximation);
}

PhraseScorer::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<PhraseScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::DocIdSetIterator> approximation)
    : TwoPhaseIterator(approximation)
{
  this->outerInstance = outerInstance;
}

bool PhraseScorer::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  outerInstance->matcher->reset();
  outerInstance->freq = 0;
  return outerInstance->matcher->nextMatch();
}

float PhraseScorer::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return outerInstance->matchCost;
}

int PhraseScorer::docID() { return matcher->approximation->docID(); }

float PhraseScorer::score() 
{
  if (freq == 0) {
    freq = matcher->sloppyWeight(simScorer);
    while (matcher->nextMatch()) {
      freq += matcher->sloppyWeight(simScorer);
    }
  }
  return simScorer->score(docID(), freq);
}

shared_ptr<DocIdSetIterator> PhraseScorer::iterator()
{
  return TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator());
}

wstring PhraseScorer::toString() { return L"PhraseScorer(" + weight + L")"; }
} // namespace org::apache::lucene::search