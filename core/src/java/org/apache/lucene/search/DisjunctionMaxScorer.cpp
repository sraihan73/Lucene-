using namespace std;

#include "DisjunctionMaxScorer.h"

namespace org::apache::lucene::search
{

DisjunctionMaxScorer::DisjunctionMaxScorer(
    shared_ptr<Weight> weight, float tieBreakerMultiplier,
    deque<std::shared_ptr<Scorer>> &subScorers, bool needsScores)
    : DisjunctionScorer(weight, subScorers, needsScores),
      tieBreakerMultiplier(tieBreakerMultiplier)
{
}

float DisjunctionMaxScorer::score(shared_ptr<DisiWrapper> topList) throw(
    IOException)
{
  float scoreSum = 0;
  float scoreMax = -numeric_limits<float>::infinity();
  for (shared_ptr<DisiWrapper> w = topList; w != nullptr; w = w->next) {
    constexpr float subScore = w->scorer->score();
    scoreSum += subScore;
    if (subScore > scoreMax) {
      scoreMax = subScore;
    }
  }
  return scoreMax + (scoreSum - scoreMax) * tieBreakerMultiplier;
}
} // namespace org::apache::lucene::search