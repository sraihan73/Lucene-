using namespace std;

#include "LMSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;

LMSimilarity::LMSimilarity(shared_ptr<CollectionModel> collectionModel)
    : collectionModel(collectionModel)
{
}

LMSimilarity::LMSimilarity() : LMSimilarity(new DefaultCollectionModel()) {}

shared_ptr<BasicStats> LMSimilarity::newStats(const wstring &field, float boost)
{
  return make_shared<LMStats>(field, boost);
}

void LMSimilarity::fillBasicStats(
    shared_ptr<BasicStats> stats,
    shared_ptr<CollectionStatistics> collectionStats,
    shared_ptr<TermStatistics> termStats)
{
  SimilarityBase::fillBasicStats(stats, collectionStats, termStats);
  shared_ptr<LMStats> lmStats = std::static_pointer_cast<LMStats>(stats);
  lmStats->setCollectionProbability(collectionModel->computeProbability(stats));
}

void LMSimilarity::explain(deque<std::shared_ptr<Explanation>> &subExpls,
                           shared_ptr<BasicStats> stats, int doc, float freq,
                           float docLen)
{
  subExpls.push_back(Explanation::match(
      collectionModel->computeProbability(stats), L"collection probability"));
}

wstring LMSimilarity::toString()
{
  wstring coll = collectionModel->getName();
  if (coll != L"") {
    return wstring::format(Locale::ROOT, L"LM %s - %s", getName(), coll);
  } else {
    return wstring::format(Locale::ROOT, L"LM %s", getName());
  }
}

LMSimilarity::LMStats::LMStats(const wstring &field, float boost)
    : BasicStats(field, boost)
{
}

float LMSimilarity::LMStats::getCollectionProbability()
{
  return collectionProbability;
}

void LMSimilarity::LMStats::setCollectionProbability(
    float collectionProbability)
{
  this->collectionProbability = collectionProbability;
}

LMSimilarity::DefaultCollectionModel::DefaultCollectionModel() {}

float LMSimilarity::DefaultCollectionModel::computeProbability(
    shared_ptr<BasicStats> stats)
{
  return (stats->getTotalTermFreq() + 1.0F) /
         (stats->getNumberOfFieldTokens() + 1.0F);
}

wstring LMSimilarity::DefaultCollectionModel::getName() { return L""; }
} // namespace org::apache::lucene::search::similarities