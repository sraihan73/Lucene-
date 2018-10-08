using namespace std;

#include "BaseGlobalOrdinalScorer.h"

namespace org::apache::lucene::search::join
{
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;

BaseGlobalOrdinalScorer::BaseGlobalOrdinalScorer(
    shared_ptr<Weight> weight, shared_ptr<SortedDocValues> values,
    shared_ptr<DocIdSetIterator> approximationScorer)
    : org::apache::lucene::search::Scorer(weight), values(values),
      approximation(approximationScorer)
{
}

float BaseGlobalOrdinalScorer::score()  { return score_; }

int BaseGlobalOrdinalScorer::docID() { return approximation->docID(); }

shared_ptr<DocIdSetIterator> BaseGlobalOrdinalScorer::iterator()
{
  return TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator());
}

shared_ptr<TwoPhaseIterator> BaseGlobalOrdinalScorer::twoPhaseIterator()
{
  return createTwoPhaseIterator(approximation);
}
} // namespace org::apache::lucene::search::join