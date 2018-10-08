using namespace std;

#include "MultiSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;

MultiSimilarity::MultiSimilarity(std::deque<std::shared_ptr<Similarity>> &sims)
    : sims(sims)
{
}

int64_t MultiSimilarity::computeNorm(shared_ptr<FieldInvertState> state)
{
  return sims[0]->computeNorm(state);
}

shared_ptr<SimWeight>
MultiSimilarity::computeWeight(float boost,
                               shared_ptr<CollectionStatistics> collectionStats,
                               deque<TermStatistics> &termStats)
{
  std::deque<std::shared_ptr<SimWeight>> subStats(sims.size());
  for (int i = 0; i < subStats.size(); i++) {
    subStats[i] = sims[i]->computeWeight(boost, collectionStats, {termStats});
  }
  return make_shared<MultiStats>(subStats);
}

shared_ptr<SimScorer> MultiSimilarity::simScorer(
    shared_ptr<SimWeight> stats,
    shared_ptr<LeafReaderContext> context) 
{
  std::deque<std::shared_ptr<SimScorer>> subScorers(sims.size());
  for (int i = 0; i < subScorers.size(); i++) {
    subScorers[i] = sims[i]->simScorer(
        (std::static_pointer_cast<MultiStats>(stats))->subStats[i], context);
  }
  return make_shared<MultiSimScorer>(subScorers);
}

MultiSimilarity::MultiSimScorer::MultiSimScorer(
    std::deque<std::shared_ptr<SimScorer>> &subScorers)
    : subScorers(subScorers)
{
}

float MultiSimilarity::MultiSimScorer::score(int doc,
                                             float freq) 
{
  float sum = 0.0f;
  for (auto subScorer : subScorers) {
    sum += subScorer->score(doc, freq);
  }
  return sum;
}

shared_ptr<Explanation> MultiSimilarity::MultiSimScorer::explain(
    int doc, shared_ptr<Explanation> freq) 
{
  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  for (auto subScorer : subScorers) {
    subs.push_back(subScorer->explain(doc, freq));
  }
  return Explanation::match(score(doc, freq->getValue()), L"sum of:", subs);
}

float MultiSimilarity::MultiSimScorer::computeSlopFactor(int distance)
{
  return subScorers[0]->computeSlopFactor(distance);
}

float MultiSimilarity::MultiSimScorer::computePayloadFactor(
    int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return subScorers[0]->computePayloadFactor(doc, start, end, payload);
}

MultiSimilarity::MultiStats::MultiStats(
    std::deque<std::shared_ptr<SimWeight>> &subStats)
    : subStats(subStats)
{
}
} // namespace org::apache::lucene::search::similarities