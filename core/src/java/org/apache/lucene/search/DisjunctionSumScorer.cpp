using namespace std;

#include "DisjunctionSumScorer.h"

namespace org::apache::lucene::search
{

DisjunctionSumScorer::DisjunctionSumScorer(
    shared_ptr<Weight> weight, deque<std::shared_ptr<Scorer>> &subScorers,
    bool needsScores)
    : DisjunctionScorer(weight, subScorers, needsScores)
{
}

float DisjunctionSumScorer::score(shared_ptr<DisiWrapper> topList) throw(
    IOException)
{
  double score = 0;
  for (shared_ptr<DisiWrapper> w = topList; w != nullptr; w = w->next) {
    score += w->scorer->score();
  }
  return static_cast<float>(score);
}
} // namespace org::apache::lucene::search