using namespace std;

#include "RandomSimilarity.h"

namespace org::apache::lucene::search::similarities
{

RandomSimilarity::RandomSimilarity(shared_ptr<Random> random)
    : knownSims(deque<>(allSims)), perFieldSeed(random->nextInt()),
      shouldQueryNorm(random->nextBoolean())
{
  Collections::shuffle(knownSims, random);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Similarity> RandomSimilarity::get(const wstring &field)
{
  assert(field != L"");
  shared_ptr<Similarity> sim = previousMappings[field];
  if (sim == nullptr) {
    sim = knownSims[max(0, abs(perFieldSeed ^ field.hashCode())) %
                    knownSims.size()];
    previousMappings.emplace(field, sim);
  }
  return sim;
}

std::deque<std::shared_ptr<BasicModel>> RandomSimilarity::BASIC_MODELS = {
    make_shared<BasicModelG>(), make_shared<BasicModelIF>(),
    make_shared<BasicModelIn>(), make_shared<BasicModelIne>()};
std::deque<std::shared_ptr<AfterEffect>> RandomSimilarity::AFTER_EFFECTS = {
    make_shared<AfterEffectB>(), make_shared<AfterEffectL>(),
    make_shared<AfterEffect::NoAfterEffect>()};
std::deque<std::shared_ptr<Normalization>> RandomSimilarity::NORMALIZATIONS = {
    make_shared<NormalizationH1>(), make_shared<NormalizationH2>(),
    make_shared<NormalizationH3>(), make_shared<NormalizationZ>()};
std::deque<std::shared_ptr<Distribution>> RandomSimilarity::DISTRIBUTIONS = {
    make_shared<DistributionLL>(), make_shared<DistributionSPL>()};
std::deque<std::shared_ptr<Lambda>> RandomSimilarity::LAMBDAS = {
    make_shared<LambdaDF>(), make_shared<LambdaTTF>()};
std::deque<std::shared_ptr<Independence>>
    RandomSimilarity::INDEPENDENCE_MEASURES = {
        make_shared<IndependenceStandardized>(),
        make_shared<IndependenceSaturated>(),
        make_shared<IndependenceChiSquared>()};
deque<std::shared_ptr<Similarity>> RandomSimilarity::allSims;

RandomSimilarity::StaticConstructor::StaticConstructor()
{
  allSims = deque<>();
  allSims.push_back(make_shared<ClassicSimilarity>());
  allSims.push_back(make_shared<BM25Similarity>());
  // We cannot do this, because this similarity behaves in "non-traditional"
  // ways: allSims.add(new BooleanSimilarity());
  for (auto basicModel : BASIC_MODELS) {
    for (auto afterEffect : AFTER_EFFECTS) {
      for (auto normalization : NORMALIZATIONS) {
        allSims.push_back(
            make_shared<DFRSimilarity>(basicModel, afterEffect, normalization));
      }
    }
  }
  for (auto distribution : DISTRIBUTIONS) {
    for (auto lambda : LAMBDAS) {
      for (auto normalization : NORMALIZATIONS) {
        allSims.push_back(
            make_shared<IBSimilarity>(distribution, lambda, normalization));
      }
    }
  }
  /* TODO: enable Dirichlet
  allSims.add(new LMDirichletSimilarity()); */
  allSims.push_back(make_shared<LMJelinekMercerSimilarity>(0.1f));
  allSims.push_back(make_shared<LMJelinekMercerSimilarity>(0.7f));
  for (auto independence : INDEPENDENCE_MEASURES) {
    allSims.push_back(make_shared<DFISimilarity>(independence));
  }
}

RandomSimilarity::StaticConstructor RandomSimilarity::staticConstructor;

// C++ WARNING: The following method was originally marked 'synchronized':
wstring RandomSimilarity::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"RandomSimilarity(queryNorm=" +
         StringHelper::toString(shouldQueryNorm) + L"): " +
         previousMappings.toString();
}
} // namespace org::apache::lucene::search::similarities