using namespace std;

#include "LMDirichletSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

LMDirichletSimilarity::LMDirichletSimilarity(
    shared_ptr<CollectionModel> collectionModel, float mu)
    : LMSimilarity(collectionModel), mu(mu)
{
}

LMDirichletSimilarity::LMDirichletSimilarity(float mu) : mu(mu) {}

LMDirichletSimilarity::LMDirichletSimilarity(
    shared_ptr<CollectionModel> collectionModel)
    : LMDirichletSimilarity(collectionModel, 2000)
{
}

LMDirichletSimilarity::LMDirichletSimilarity() : LMDirichletSimilarity(2000) {}

float LMDirichletSimilarity::score(shared_ptr<BasicStats> stats, float freq,
                                   float docLen)
{
  float score =
      stats->getBoost() *
      static_cast<float>(
          log(1 + freq / (mu * (std::static_pointer_cast<LMStats>(stats))
                                   ->getCollectionProbability())) +
          log(mu / (docLen + mu)));
  return score > 0.0f ? score : 0.0f;
}

void LMDirichletSimilarity::explain(deque<std::shared_ptr<Explanation>> &subs,
                                    shared_ptr<BasicStats> stats, int doc,
                                    float freq, float docLen)
{
  if (stats->getBoost() != 1.0f) {
    subs.push_back(Explanation::match(stats->getBoost(), L"boost"));
  }

  subs.push_back(Explanation::match(mu, L"mu"));
  shared_ptr<Explanation> weightExpl = Explanation::match(
      static_cast<float>(
          log(1 + freq / (mu * (std::static_pointer_cast<LMStats>(stats))
                                   ->getCollectionProbability()))),
      L"term weight");
  subs.push_back(weightExpl);
  subs.push_back(Explanation::match(static_cast<float>(log(mu / (docLen + mu))),
                                    L"document norm"));
  LMSimilarity::explain(subs, stats, doc, freq, docLen);
}

float LMDirichletSimilarity::getMu() { return mu; }

wstring LMDirichletSimilarity::getName()
{
  return wstring::format(Locale::ROOT, L"Dirichlet(%f)", getMu());
}
} // namespace org::apache::lucene::search::similarities