using namespace std;

#include "BM25Similarity.h"

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

BM25Similarity::BM25Similarity(float k1, float b) : k1(k1), b(b)
{
  if (Float::isFinite(k1) == false || k1 < 0) {
    throw invalid_argument(L"illegal k1 value: " + to_wstring(k1) +
                           L", must be a non-negative finite value");
  }
  if (isnan(b) || b < 0 || b > 1) {
    throw invalid_argument(L"illegal b value: " + to_wstring(b) +
                           L", must be between 0 and 1");
  }
}

BM25Similarity::BM25Similarity() : BM25Similarity(1.2f, 0.75f) {}

float BM25Similarity::idf(int64_t docFreq, int64_t docCount)
{
  return static_cast<float>(
      log(1 + (docCount - docFreq + 0.5) / (docFreq + 0.5)));
}

float BM25Similarity::sloppyFreq(int distance) { return 1.0f / (distance + 1); }

float BM25Similarity::scorePayload(int doc, int start, int end,
                                   shared_ptr<BytesRef> payload)
{
  return 1;
}

float BM25Similarity::avgFieldLength(
    shared_ptr<CollectionStatistics> collectionStats)
{
  constexpr int64_t sumTotalTermFreq;
  if (collectionStats->sumTotalTermFreq() == -1) {
    // frequencies are omitted (tf=1), its # of postings
    if (collectionStats->sumDocFreq() == -1) {
      // theoretical case only: remove!
      return 1.0f;
    }
    sumTotalTermFreq = collectionStats->sumDocFreq();
  } else {
    sumTotalTermFreq = collectionStats->sumTotalTermFreq();
  }
  constexpr int64_t docCount = collectionStats->docCount() == -1
                                     ? collectionStats->maxDoc()
                                     : collectionStats->docCount();
  return static_cast<float>(sumTotalTermFreq / static_cast<double>(docCount));
}

void BM25Similarity::setDiscountOverlaps(bool v) { discountOverlaps = v; }

bool BM25Similarity::getDiscountOverlaps() { return discountOverlaps; }

std::deque<float> const BM25Similarity::OLD_LENGTH_TABLE =
    std::deque<float>(256);
std::deque<float> const BM25Similarity::LENGTH_TABLE = std::deque<float>(256);

BM25Similarity::StaticConstructor::StaticConstructor()
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

BM25Similarity::StaticConstructor BM25Similarity::staticConstructor;

int64_t BM25Similarity::computeNorm(shared_ptr<FieldInvertState> state)
{
  constexpr int numTerms = discountOverlaps
                               ? state->getLength() - state->getNumOverlap()
                               : state->getLength();
  int indexCreatedVersionMajor = state->getIndexCreatedVersionMajor();
  if (indexCreatedVersionMajor >= 7) {
    return SmallFloat::intToByte4(numTerms);
  } else {
    return SmallFloat::floatToByte315(static_cast<float>(1 / sqrt(numTerms)));
  }
}

shared_ptr<Explanation>
BM25Similarity::idfExplain(shared_ptr<CollectionStatistics> collectionStats,
                           shared_ptr<TermStatistics> termStats)
{
  constexpr int64_t df = termStats->docFreq();
  constexpr int64_t docCount = collectionStats->docCount() == -1
                                     ? collectionStats->maxDoc()
                                     : collectionStats->docCount();
  constexpr float idf = this->idf(df, docCount);
  return Explanation::match(idf,
                            L"idf, computed as log(1 + (docCount - docFreq + "
                            L"0.5) / (docFreq + 0.5)) from:",
                            {Explanation::match(df, L"docFreq"),
                             Explanation::match(docCount, L"docCount")});
}

shared_ptr<Explanation> BM25Similarity::idfExplain(
    shared_ptr<CollectionStatistics> collectionStats,
    std::deque<std::shared_ptr<TermStatistics>> &termStats)
{
  double idf = 0; // sum into a double before casting into a float
  deque<std::shared_ptr<Explanation>> details =
      deque<std::shared_ptr<Explanation>>();
  for (auto stat : termStats) {
    shared_ptr<Explanation> idfExplain =
        this->idfExplain(collectionStats, stat);
    details.push_back(idfExplain);
    idf += idfExplain->getValue();
  }
  return Explanation::match(static_cast<float>(idf), L"idf(), sum of:",
                            details);
}

shared_ptr<SimWeight>
BM25Similarity::computeWeight(float boost,
                              shared_ptr<CollectionStatistics> collectionStats,
                              deque<TermStatistics> &termStats)
{
  shared_ptr<Explanation> idf = termStats->length == 1
                                    ? idfExplain(collectionStats, termStats[0])
                                    : idfExplain(collectionStats, termStats);
  float avgdl = avgFieldLength(collectionStats);

  std::deque<float> oldCache(256);
  std::deque<float> cache(256);
  for (int i = 0; i < cache.size(); i++) {
    oldCache[i] = k1 * ((1 - b) + b * OLD_LENGTH_TABLE[i] / avgdl);
    cache[i] = k1 * ((1 - b) + b * LENGTH_TABLE[i] / avgdl);
  }
  return make_shared<BM25Stats>(collectionStats->field(), boost, idf, avgdl,
                                oldCache, cache);
}

shared_ptr<SimScorer> BM25Similarity::simScorer(
    shared_ptr<SimWeight> stats,
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<BM25Stats> bm25stats = std::static_pointer_cast<BM25Stats>(stats);
  return make_shared<BM25DocScorer>(
      shared_from_this(), bm25stats,
      context->reader()->getMetaData()->getCreatedVersionMajor(),
      context->reader()->getNormValues(bm25stats->field));
}

BM25Similarity::BM25DocScorer::BM25DocScorer(
    shared_ptr<BM25Similarity> outerInstance, shared_ptr<BM25Stats> stats,
    int indexCreatedVersionMajor,
    shared_ptr<NumericDocValues> norms) 
    : stats(stats), weightValue(stats->weight * (outerInstance->k1 + 1)),
      norms(norms), outerInstance(outerInstance)
{
  if (indexCreatedVersionMajor >= 7) {
    lengthCache = LENGTH_TABLE;
    cache = stats->cache;
  } else {
    lengthCache = OLD_LENGTH_TABLE;
    cache = stats->oldCache;
  }
}

float BM25Similarity::BM25DocScorer::score(int doc,
                                           float freq) 
{
  // if there are no norms, we act as if b=0
  float norm;
  if (norms == nullptr) {
    norm = outerInstance->k1;
  } else {
    if (norms->advanceExact(doc)) {
      norm = cache[(static_cast<char>(norms->longValue())) & 0xFF];
    } else {
      norm = cache[0];
    }
  }
  return weightValue * freq / (freq + norm);
}

shared_ptr<Explanation> BM25Similarity::BM25DocScorer::explain(
    int doc, shared_ptr<Explanation> freq) 
{
  return outerInstance->explainScore(doc, freq, stats, norms, lengthCache);
}

float BM25Similarity::BM25DocScorer::computeSlopFactor(int distance)
{
  return outerInstance->sloppyFreq(distance);
}

float BM25Similarity::BM25DocScorer::computePayloadFactor(
    int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return outerInstance->scorePayload(doc, start, end, payload);
}

BM25Similarity::BM25Stats::BM25Stats(const wstring &field, float boost,
                                     shared_ptr<Explanation> idf, float avgdl,
                                     std::deque<float> &oldCache,
                                     std::deque<float> &cache)
    : idf(idf), avgdl(avgdl), boost(boost), weight(idf->getValue() * boost),
      field(field), oldCache(oldCache), cache(cache)
{
}

shared_ptr<Explanation> BM25Similarity::explainTFNorm(
    int doc, shared_ptr<Explanation> freq, shared_ptr<BM25Stats> stats,
    shared_ptr<NumericDocValues> norms,
    std::deque<float> &lengthCache) 
{
  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  subs.push_back(freq);
  subs.push_back(Explanation::match(k1, L"parameter k1"));
  if (norms == nullptr) {
    subs.push_back(
        Explanation::match(0, L"parameter b (norms omitted for field)"));
    return Explanation::match(
        (freq->getValue() * (k1 + 1)) / (freq->getValue() + k1),
        L"tfNorm, computed as (freq * (k1 + 1)) / (freq + k1) from:", subs);
  } else {
    char norm;
    if (norms->advanceExact(doc)) {
      norm = static_cast<char>(norms->longValue());
    } else {
      norm = 0;
    }
    float doclen = lengthCache[norm & 0xff];
    subs.push_back(Explanation::match(b, L"parameter b"));
    subs.push_back(Explanation::match(stats->avgdl, L"avgFieldLength"));
    subs.push_back(Explanation::match(doclen, L"fieldLength"));
    return Explanation::match(
        (freq->getValue() * (k1 + 1)) /
            (freq->getValue() + k1 * (1 - b + b * doclen / stats->avgdl)),
        L"tfNorm, computed as (freq * (k1 + 1)) / (freq + k1 * (1 - b + b * "
        L"fieldLength / avgFieldLength)) from:",
        subs);
  }
}

shared_ptr<Explanation>
BM25Similarity::explainScore(int doc, shared_ptr<Explanation> freq,
                             shared_ptr<BM25Stats> stats,
                             shared_ptr<NumericDocValues> norms,
                             std::deque<float> &lengthCache) 
{
  shared_ptr<Explanation> boostExpl =
      Explanation::match(stats->boost, L"boost");
  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  if (boostExpl->getValue() != 1.0f) {
    subs.push_back(boostExpl);
  }
  subs.push_back(stats->idf);
  shared_ptr<Explanation> tfNormExpl =
      explainTFNorm(doc, freq, stats, norms, lengthCache);
  subs.push_back(tfNormExpl);
  return Explanation::match(
      boostExpl->getValue() * stats->idf.getValue() * tfNormExpl->getValue(),
      L"score(doc=" + to_wstring(doc) + L",freq=" + freq + L"), product of:",
      subs);
}

wstring BM25Similarity::toString()
{
  return L"BM25(k1=" + to_wstring(k1) + L",b=" + to_wstring(b) + L")";
}

float BM25Similarity::getK1() { return k1; }

float BM25Similarity::getB() { return b; }
} // namespace org::apache::lucene::search::similarities