using namespace std;

#include "FilterScorer.h"

namespace org::apache::lucene::search
{

FilterScorer::FilterScorer(shared_ptr<Scorer> in_)
    : Scorer(in_->weight), in_(in_)
{
}

FilterScorer::FilterScorer(shared_ptr<Scorer> in_, shared_ptr<Weight> weight)
    : Scorer(weight), in_(in_)
{
  if (in_ == nullptr) {
    throw make_shared<NullPointerException>(L"wrapped Scorer must not be null");
  }
}

float FilterScorer::score()  { return in_->score(); }

int FilterScorer::docID() { return in_->docID(); }

shared_ptr<DocIdSetIterator> FilterScorer::iterator() { return in_->begin(); }

shared_ptr<TwoPhaseIterator> FilterScorer::twoPhaseIterator()
{
  return in_->twoPhaseIterator();
}
} // namespace org::apache::lucene::search