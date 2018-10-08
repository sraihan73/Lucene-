using namespace std;

#include "LMJelinekMercerSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

LMJelinekMercerSimilarity::LMJelinekMercerSimilarity(
    shared_ptr<CollectionModel> collectionModel, float lambda)
    : LMSimilarity(collectionModel), lambda(lambda)
{
}

LMJelinekMercerSimilarity::LMJelinekMercerSimilarity(float lambda)
    : lambda(lambda)
{
}

float LMJelinekMercerSimilarity::score(shared_ptr<BasicStats> stats, float freq,
                                       float docLen)
{
  return stats->getBoost() *
         static_cast<float>(
             log(1 + ((1 - lambda) * freq / docLen) /
                         (lambda * (std::static_pointer_cast<LMStats>(stats))
                                       ->getCollectionProbability())));
}

void LMJelinekMercerSimilarity::explain(
    deque<std::shared_ptr<Explanation>> &subs, shared_ptr<BasicStats> stats,
    int doc, float freq, float docLen)
{
  if (stats->getBoost() != 1.0f) {
    subs.push_back(Explanation::match(stats->getBoost(), L"boost"));
  }
  subs.push_back(Explanation::match(lambda, L"lambda"));
  LMSimilarity::explain(subs, stats, doc, freq, docLen);
}

float LMJelinekMercerSimilarity::getLambda() { return lambda; }

wstring LMJelinekMercerSimilarity::getName()
{
  return wstring::format(Locale::ROOT, L"Jelinek-Mercer(%f)", getLambda());
}
} // namespace org::apache::lucene::search::similarities