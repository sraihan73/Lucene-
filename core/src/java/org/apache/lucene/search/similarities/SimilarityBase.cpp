using namespace std;

#include "SimilarityBase.h"

namespace org::apache::lucene::search::similarities
{
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;
using SmallFloat = org::apache::lucene::util::SmallFloat;
const double SimilarityBase::LOG_2 = log(2);

SimilarityBase::SimilarityBase() {}

void SimilarityBase::setDiscountOverlaps(bool v) { discountOverlaps = v; }

bool SimilarityBase::getDiscountOverlaps() { return discountOverlaps; }

shared_ptr<SimWeight>
SimilarityBase::computeWeight(float boost,
                              shared_ptr<CollectionStatistics> collectionStats,
                              deque<TermStatistics> &termStats)
{
  std::deque<std::shared_ptr<BasicStats>> stats(termStats->length);
  for (int i = 0; i < termStats->length; i++) {
    stats[i] = newStats(collectionStats->field(), boost);
    fillBasicStats(stats[i], collectionStats, termStats[i]);
  }
  return stats.size() == 1 ? stats[0]
                           : make_shared<MultiSimilarity::MultiStats>(stats);
}

shared_ptr<BasicStats> SimilarityBase::newStats(const wstring &field,
                                                float boost)
{
  return make_shared<BasicStats>(field, boost);
}

void SimilarityBase::fillBasicStats(
    shared_ptr<BasicStats> stats,
    shared_ptr<CollectionStatistics> collectionStats,
    shared_ptr<TermStatistics> termStats)
{
  // #positions(field) must be >= #positions(term)
  assert(collectionStats->sumTotalTermFreq() == -1 ||
         collectionStats->sumTotalTermFreq() >= termStats->totalTermFreq());
  int64_t numberOfDocuments = collectionStats->docCount() == -1
                                    ? collectionStats->maxDoc()
                                    : collectionStats->docCount();

  int64_t docFreq = termStats->docFreq();
  int64_t totalTermFreq = termStats->totalTermFreq();

  // frequencies are omitted, all postings have tf=1, so totalTermFreq = docFreq
  if (totalTermFreq == -1) {
    totalTermFreq = docFreq;
  }

  constexpr int64_t numberOfFieldTokens;
  constexpr float avgFieldLength;

  if (collectionStats->sumTotalTermFreq() == -1) {
    // frequencies are omitted, so sumTotalTermFreq = # postings
    if (collectionStats->sumDocFreq() == -1) {
      // theoretical case only: remove!
      numberOfFieldTokens = docFreq;
      avgFieldLength = 1.0f;
    } else {
      numberOfFieldTokens = collectionStats->sumDocFreq();
      avgFieldLength =
          static_cast<float>(collectionStats->sumDocFreq() /
                             static_cast<double>(numberOfDocuments));
    }
  } else {
    numberOfFieldTokens = collectionStats->sumTotalTermFreq();
    avgFieldLength = static_cast<float>(collectionStats->sumTotalTermFreq() /
                                        static_cast<double>(numberOfDocuments));
  }

  // TODO: add sumDocFreq for field (numberOfFieldPostings)
  stats->setNumberOfDocuments(numberOfDocuments);
  stats->setNumberOfFieldTokens(numberOfFieldTokens);
  stats->setAvgFieldLength(avgFieldLength);
  stats->setDocFreq(docFreq);
  stats->setTotalTermFreq(totalTermFreq);
}

void SimilarityBase::explain(deque<std::shared_ptr<Explanation>> &subExpls,
                             shared_ptr<BasicStats> stats, int doc, float freq,
                             float docLen)
{
}

shared_ptr<Explanation> SimilarityBase::explain(shared_ptr<BasicStats> stats,
                                                int doc,
                                                shared_ptr<Explanation> freq,
                                                float docLen)
{
  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  explain(subs, stats, doc, freq->getValue(), docLen);

  return Explanation::match(
      score(stats, freq->getValue(), docLen),
      L"score(" + getClass().getSimpleName() + L", doc=" + to_wstring(doc) +
          L", freq=" + to_wstring(freq->getValue()) + L"), computed from:",
      subs);
}

shared_ptr<SimScorer> SimilarityBase::simScorer(
    shared_ptr<SimWeight> stats,
    shared_ptr<LeafReaderContext> context) 
{
  int indexCreatedVersionMajor =
      context->reader()->getMetaData()->getCreatedVersionMajor();
  if (std::dynamic_pointer_cast<MultiSimilarity::MultiStats>(stats) !=
      nullptr) {
    // a multi term query (e.g. phrase). return the summation,
    // scoring almost as if it were bool query
    std::deque<std::shared_ptr<SimWeight>> subStats =
        (std::static_pointer_cast<MultiSimilarity::MultiStats>(stats))
            ->subStats;
    std::deque<std::shared_ptr<SimScorer>> subScorers(subStats.size());
    for (int i = 0; i < subScorers.size(); i++) {
      shared_ptr<BasicStats> basicstats =
          std::static_pointer_cast<BasicStats>(subStats[i]);
      subScorers[i] = make_shared<BasicSimScorer>(
          shared_from_this(), basicstats, indexCreatedVersionMajor,
          context->reader()->getNormValues(basicstats->field));
    }
    return make_shared<MultiSimilarity::MultiSimScorer>(subScorers);
  } else {
    shared_ptr<BasicStats> basicstats =
        std::static_pointer_cast<BasicStats>(stats);
    return make_shared<BasicSimScorer>(
        shared_from_this(), basicstats, indexCreatedVersionMajor,
        context->reader()->getNormValues(basicstats->field));
  }
}

std::deque<float> const SimilarityBase::OLD_LENGTH_TABLE =
    std::deque<float>(256);
std::deque<float> const SimilarityBase::LENGTH_TABLE = std::deque<float>(256);

SimilarityBase::StaticConstructor::StaticConstructor()
{
  for (int i = 1; i < 256; i++) {
    float f = SmallFloat::byte315ToFloat(static_cast<char>(i));
    OLD_LENGTH_TABLE[i] = 1.0f / (f * f);
  }
  OLD_LENGTH_TABLE[0] = 1.0f / OLD_LENGTH_TABLE[255]; // otherwise inf

  for (int i = 0; i < 256; i++) {
    LENGTH_TABLE[i] = SmallFloat::byte4ToInt(static_cast<char>(i));
  }
}

SimilarityBase::StaticConstructor SimilarityBase::staticConstructor;

int64_t SimilarityBase::computeNorm(shared_ptr<FieldInvertState> state)
{
  constexpr int numTerms;
  if (discountOverlaps) {
    numTerms = state->getLength() - state->getNumOverlap();
  } else {
    numTerms = state->getLength();
  }
  int indexCreatedVersionMajor = state->getIndexCreatedVersionMajor();
  if (indexCreatedVersionMajor >= 7) {
    return SmallFloat::intToByte4(numTerms);
  } else {
    return SmallFloat::floatToByte315(static_cast<float>(1 / sqrt(numTerms)));
  }
}

double SimilarityBase::log2(double x)
{
  // Put this to a 'util' class if we need more of these.
  return log(x) / LOG_2;
}

SimilarityBase::BasicSimScorer::BasicSimScorer(
    shared_ptr<SimilarityBase> outerInstance, shared_ptr<BasicStats> stats,
    int indexCreatedVersionMajor,
    shared_ptr<NumericDocValues> norms) 
    : stats(stats), norms(norms),
      normCache(indexCreatedVersionMajor >= 7 ? LENGTH_TABLE
                                              : OLD_LENGTH_TABLE),
      outerInstance(outerInstance)
{
}

float SimilarityBase::BasicSimScorer::getLengthValue(int doc) 
{
  if (norms == nullptr) {
    return 1.0F;
  }
  if (norms->advanceExact(doc)) {
    return normCache[Byte::toUnsignedInt(
        static_cast<char>(norms->longValue()))];
  } else {
    return 0;
  }
}

float SimilarityBase::BasicSimScorer::score(int doc,
                                            float freq) 
{
  // We have to supply something in case norms are omitted
  return outerInstance->score(stats, freq, getLengthValue(doc));
}

shared_ptr<Explanation> SimilarityBase::BasicSimScorer::explain(
    int doc, shared_ptr<Explanation> freq) 
{
  return outerInstance->explain(stats, doc, freq, getLengthValue(doc));
}

float SimilarityBase::BasicSimScorer::computeSlopFactor(int distance)
{
  return 1.0f / (distance + 1);
}

float SimilarityBase::BasicSimScorer::computePayloadFactor(
    int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return 1.0f;
}
} // namespace org::apache::lucene::search::similarities