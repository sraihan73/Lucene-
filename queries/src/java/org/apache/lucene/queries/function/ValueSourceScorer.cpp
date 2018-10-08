using namespace std;

#include "ValueSourceScorer.h"

namespace org::apache::lucene::queries::function
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;

ValueSourceScorer::ValueSourceScorer(
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<FunctionValues> values)
    : org::apache::lucene::search::Scorer(nullptr), values(values),
      twoPhaseIterator(make_shared<TwoPhaseIterator>(approximation)),
      disi(TwoPhaseIterator::asDocIdSetIterator(twoPhaseIterator_))
{
  shared_ptr<DocIdSetIterator> *const approximation = DocIdSetIterator::all(
      readerContext->reader()->maxDoc()); // no approximation!
  {public : bool matches() throws IOException{
      return ValueSourceScorer::this->matches(approximation->docID());
}

float matchCost()
{
  return 100; // TODO: use cost of ValueSourceScorer.this.matches()
}
}; // namespace org::apache::lucene::queries::function
}

shared_ptr<DocIdSetIterator> ValueSourceScorer::iterator() { return disi; }

shared_ptr<TwoPhaseIterator> ValueSourceScorer::twoPhaseIterator()
{
  return twoPhaseIterator_;
}

int ValueSourceScorer::docID() { return disi->docID(); }

float ValueSourceScorer::score() 
{
  // (same as FunctionQuery, but no qWeight)  TODO consider adding configurable
  // qWeight
  float score = values->floatVal(disi->docID());
  // Current Lucene priority queues can't handle NaN and -Infinity, so
  // map_obj to -Float.MAX_VALUE. This conditional handles both -infinity
  // and NaN since comparisons with NaN are always false.
  return score > -numeric_limits<float>::infinity()
             ? score
             : -numeric_limits<float>::max();
}
}