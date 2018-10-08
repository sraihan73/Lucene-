using namespace std;

#include "CustomScoreProvider.h"

namespace org::apache::lucene::queries
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionQuery = org::apache::lucene::queries::function::FunctionQuery;
using Explanation = org::apache::lucene::search::Explanation;

CustomScoreProvider::CustomScoreProvider(shared_ptr<LeafReaderContext> context)
    : context(context)
{
}

float CustomScoreProvider::customScore(
    int doc, float subQueryScore,
    std::deque<float> &valSrcScores) 
{
  if (valSrcScores.size() == 1) {
    return customScore(doc, subQueryScore, valSrcScores[0]);
  }
  if (valSrcScores.empty()) {
    return customScore(doc, subQueryScore, 1);
  }
  float score = subQueryScore;
  for (auto valSrcScore : valSrcScores) {
    score *= valSrcScore;
  }
  return score;
}

float CustomScoreProvider::customScore(int doc, float subQueryScore,
                                       float valSrcScore) 
{
  return subQueryScore * valSrcScore;
}

shared_ptr<Explanation> CustomScoreProvider::customExplain(
    int doc, shared_ptr<Explanation> subQueryExpl,
    std::deque<std::shared_ptr<Explanation>> &valSrcExpls) 
{
  if (valSrcExpls.size() == 1) {
    return customExplain(doc, subQueryExpl, valSrcExpls[0]);
  }
  if (valSrcExpls.empty()) {
    return subQueryExpl;
  }
  float valSrcScore = 1;
  for (auto valSrcExpl : valSrcExpls) {
    valSrcScore *= valSrcExpl->getValue();
  }

  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  subs.push_back(subQueryExpl);
  for (auto valSrcExpl : valSrcExpls) {
    subs.push_back(valSrcExpl);
  }
  return Explanation::match(valSrcScore * subQueryExpl->getValue(),
                            L"custom score: product of:", subs);
}

shared_ptr<Explanation> CustomScoreProvider::customExplain(
    int doc, shared_ptr<Explanation> subQueryExpl,
    shared_ptr<Explanation> valSrcExpl) 
{
  float valSrcScore = 1;
  if (valSrcExpl != nullptr) {
    valSrcScore *= valSrcExpl->getValue();
  }
  return Explanation::match(valSrcScore * subQueryExpl->getValue(),
                            L"custom score: product of:",
                            {subQueryExpl, valSrcExpl});
}
} // namespace org::apache::lucene::queries