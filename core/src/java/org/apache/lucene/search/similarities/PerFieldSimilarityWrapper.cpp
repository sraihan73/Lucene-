using namespace std;

#include "PerFieldSimilarityWrapper.h"

namespace org::apache::lucene::search::similarities
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;

PerFieldSimilarityWrapper::PerFieldSimilarityWrapper() {}

int64_t
PerFieldSimilarityWrapper::computeNorm(shared_ptr<FieldInvertState> state)
{
  return get(state->getName())->computeNorm(state);
}

shared_ptr<SimWeight> PerFieldSimilarityWrapper::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  shared_ptr<PerFieldSimWeight> weight = make_shared<PerFieldSimWeight>();
  weight->delegate_ = get(collectionStats->field());
  weight->delegateWeight =
      weight->delegate_->computeWeight(boost, collectionStats, {termStats});
  return weight;
}

shared_ptr<SimScorer> PerFieldSimilarityWrapper::simScorer(
    shared_ptr<SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<PerFieldSimWeight> perFieldWeight =
      std::static_pointer_cast<PerFieldSimWeight>(weight);
  return perFieldWeight->delegate_->simScorer(perFieldWeight->delegateWeight,
                                              context);
}
} // namespace org::apache::lucene::search::similarities