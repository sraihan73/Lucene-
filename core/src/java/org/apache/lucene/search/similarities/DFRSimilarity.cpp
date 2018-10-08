using namespace std;

#include "DFRSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;
using NoAfterEffect =
    org::apache::lucene::search::similarities::AfterEffect::NoAfterEffect;
using NoNormalization =
    org::apache::lucene::search::similarities::Normalization::NoNormalization;

DFRSimilarity::DFRSimilarity(shared_ptr<BasicModel> basicModel,
                             shared_ptr<AfterEffect> afterEffect,
                             shared_ptr<Normalization> normalization)
    : basicModel(basicModel), afterEffect(afterEffect),
      normalization(normalization)
{
  if (basicModel == nullptr || afterEffect == nullptr ||
      normalization == nullptr) {
    throw make_shared<NullPointerException>(L"null parameters not allowed.");
  }
}

float DFRSimilarity::score(shared_ptr<BasicStats> stats, float freq,
                           float docLen)
{
  float tfn = normalization->tfn(stats, freq, docLen);
  return stats->getBoost() * basicModel->score(stats, tfn) *
         afterEffect->score(stats, tfn);
}

void DFRSimilarity::explain(deque<std::shared_ptr<Explanation>> &subs,
                            shared_ptr<BasicStats> stats, int doc, float freq,
                            float docLen)
{
  if (stats->getBoost() != 1.0f) {
    subs.push_back(Explanation::match(stats->getBoost(), L"boost"));
  }

  shared_ptr<Explanation> normExpl =
      normalization->explain(stats, freq, docLen);
  float tfn = normExpl->getValue();
  subs.push_back(normExpl);
  subs.push_back(basicModel->explain(stats, tfn));
  subs.push_back(afterEffect->explain(stats, tfn));
}

wstring DFRSimilarity::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"DFR " + basicModel->toString() + afterEffect->toString() +
         normalization->toString();
}

shared_ptr<BasicModel> DFRSimilarity::getBasicModel() { return basicModel; }

shared_ptr<AfterEffect> DFRSimilarity::getAfterEffect() { return afterEffect; }

shared_ptr<Normalization> DFRSimilarity::getNormalization()
{
  return normalization;
}
} // namespace org::apache::lucene::search::similarities