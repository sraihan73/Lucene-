using namespace std;

#include "BooleanSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<Similarity> BooleanSimilarity::BM25_SIM =
    make_shared<BM25Similarity>();

BooleanSimilarity::BooleanSimilarity() {}

int64_t BooleanSimilarity::computeNorm(shared_ptr<FieldInvertState> state)
{
  return BM25_SIM->computeNorm(state);
}

shared_ptr<SimWeight> BooleanSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  return make_shared<BooleanWeight>(boost);
}

BooleanSimilarity::BooleanWeight::BooleanWeight(float boost) : boost(boost) {}

shared_ptr<SimScorer> BooleanSimilarity::simScorer(
    shared_ptr<SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  constexpr float boost =
      (std::static_pointer_cast<BooleanWeight>(weight))->boost;

  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this(), boost);
}

BooleanSimilarity::SimScorerAnonymousInnerClass::SimScorerAnonymousInnerClass(
    shared_ptr<BooleanSimilarity> outerInstance, float boost)
{
  this->outerInstance = outerInstance;
  this->boost = boost;
}

float BooleanSimilarity::SimScorerAnonymousInnerClass::score(
    int doc, float freq) 
{
  return boost;
}

shared_ptr<Explanation>
BooleanSimilarity::SimScorerAnonymousInnerClass::explain(
    int doc, shared_ptr<Explanation> freq) 
{
  shared_ptr<Explanation> queryBoostExpl =
      Explanation::match(boost, L"query boost");
  return Explanation::match(queryBoostExpl->getValue(),
                            L"score(" + getClass().getSimpleName() + L", doc=" +
                                to_wstring(doc) + L"), computed from:",
                            queryBoostExpl);
}

float BooleanSimilarity::SimScorerAnonymousInnerClass::computeSlopFactor(
    int distance)
{
  return 1.0f;
}

float BooleanSimilarity::SimScorerAnonymousInnerClass::computePayloadFactor(
    int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return 1.0f;
}
} // namespace org::apache::lucene::search::similarities