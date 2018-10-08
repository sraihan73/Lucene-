using namespace std;

#include "ConstantScoreScorer.h"

namespace org::apache::lucene::search
{

ConstantScoreScorer::ConstantScoreScorer(shared_ptr<Weight> weight, float score,
                                         shared_ptr<DocIdSetIterator> disi)
    : Scorer(weight), score(score),
      twoPhaseIterator(this->twoPhaseIterator_.reset()), disi(disi)
{
}

ConstantScoreScorer::ConstantScoreScorer(
    shared_ptr<Weight> weight, float score,
    shared_ptr<TwoPhaseIterator> twoPhaseIterator)
    : Scorer(weight), score(score), twoPhaseIterator(twoPhaseIterator),
      disi(TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator))
{
}

shared_ptr<DocIdSetIterator> ConstantScoreScorer::iterator() { return disi; }

shared_ptr<TwoPhaseIterator> ConstantScoreScorer::twoPhaseIterator()
{
  return twoPhaseIterator_;
}

int ConstantScoreScorer::docID() { return disi->docID(); }

float ConstantScoreScorer::score()  { return score_; }
} // namespace org::apache::lucene::search