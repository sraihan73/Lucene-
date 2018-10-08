using namespace std;

#include "IBSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;
using NoNormalization =
    org::apache::lucene::search::similarities::Normalization::NoNormalization;

IBSimilarity::IBSimilarity(shared_ptr<Distribution> distribution,
                           shared_ptr<Lambda> lambda,
                           shared_ptr<Normalization> normalization)
    : distribution(distribution), lambda(lambda), normalization(normalization)
{
}

float IBSimilarity::score(shared_ptr<BasicStats> stats, float freq,
                          float docLen)
{
  return stats->getBoost() *
         distribution->score(stats, normalization->tfn(stats, freq, docLen),
                             lambda->lambda(stats));
}

void IBSimilarity::explain(deque<std::shared_ptr<Explanation>> &subs,
                           shared_ptr<BasicStats> stats, int doc, float freq,
                           float docLen)
{
  if (stats->getBoost() != 1.0f) {
    subs.push_back(Explanation::match(stats->getBoost(), L"boost"));
  }
  shared_ptr<Explanation> normExpl =
      normalization->explain(stats, freq, docLen);
  shared_ptr<Explanation> lambdaExpl = lambda->explain(stats);
  subs.push_back(normExpl);
  subs.push_back(lambdaExpl);
  subs.push_back(distribution->explain(stats, normExpl->getValue(),
                                       lambdaExpl->getValue()));
}

wstring IBSimilarity::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"IB " + distribution->toString() + L"-" + lambda->toString() +
         normalization->toString();
}

shared_ptr<Distribution> IBSimilarity::getDistribution()
{
  return distribution;
}

shared_ptr<Lambda> IBSimilarity::getLambda() { return lambda; }

shared_ptr<Normalization> IBSimilarity::getNormalization()
{
  return normalization;
}
} // namespace org::apache::lucene::search::similarities