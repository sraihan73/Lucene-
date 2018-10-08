using namespace std;

#include "IntervalScorer.h"

namespace org::apache::lucene::search::intervals
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using Similarity = org::apache::lucene::search::similarities::Similarity;

IntervalScorer::IntervalScorer(shared_ptr<Weight> weight,
                               shared_ptr<IntervalIterator> intervals,
                               shared_ptr<Similarity::SimScorer> simScorer)
    : org::apache::lucene::search::Scorer(weight), intervals(intervals),
      simScorer(simScorer)
{
}

int IntervalScorer::docID() { return intervals->docID(); }

float IntervalScorer::score() 
{
  ensureFreq();
  return simScorer->score(docID(), freq_);
}

shared_ptr<Explanation>
IntervalScorer::explain(const wstring &topLevel) 
{
  ensureFreq();
  shared_ptr<Explanation> freqExplanation =
      Explanation::match(freq_, L"intervalFreq=" + to_wstring(freq_));
  shared_ptr<Explanation> scoreExplanation =
      simScorer->explain(docID(), freqExplanation);
  return Explanation::match(scoreExplanation->getValue(),
                            topLevel + L", result of:", scoreExplanation);
}

float IntervalScorer::freq() 
{
  ensureFreq();
  return freq_;
}

void IntervalScorer::ensureFreq() 
{
  if (lastScoredDoc != docID()) {
    lastScoredDoc = docID();
    freq_ = 0;
    do {
      freq_ += (1.0 / (intervals->end() - intervals->start() + 1));
    } while (intervals->nextInterval() != IntervalIterator::NO_MORE_INTERVALS);
  }
}

shared_ptr<DocIdSetIterator> IntervalScorer::iterator()
{
  return TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator());
}

shared_ptr<TwoPhaseIterator> IntervalScorer::twoPhaseIterator()
{
  return make_shared<TwoPhaseIteratorAnonymousInnerClass>(shared_from_this(),
                                                          intervals);
}

IntervalScorer::TwoPhaseIteratorAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<IntervalScorer> outerInstance,
        shared_ptr<org::apache::lucene::search::intervals::IntervalIterator>
            intervals)
    : org::apache::lucene::search::TwoPhaseIterator(intervals)
{
  this->outerInstance = outerInstance;
}

bool IntervalScorer::TwoPhaseIteratorAnonymousInnerClass::matches() throw(
    IOException)
{
  return outerInstance->intervals->nextInterval() !=
         IntervalIterator::NO_MORE_INTERVALS;
}

float IntervalScorer::TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return outerInstance->intervals->matchCost();
}
} // namespace org::apache::lucene::search::intervals