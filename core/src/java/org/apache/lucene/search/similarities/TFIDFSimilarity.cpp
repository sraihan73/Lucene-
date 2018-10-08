using namespace std;

#include "TFIDFSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;
using SmallFloat = org::apache::lucene::util::SmallFloat;
std::deque<float> const TFIDFSimilarity::OLD_NORM_TABLE =
    std::deque<float>(256);

TFIDFSimilarity::StaticConstructor::StaticConstructor()
{
  for (int i = 0; i < 256; i++) {
    OLD_NORM_TABLE[i] = SmallFloat::byte315ToFloat(static_cast<char>(i));
  }
}

TFIDFSimilarity::StaticConstructor TFIDFSimilarity::staticConstructor;

TFIDFSimilarity::TFIDFSimilarity() {}

void TFIDFSimilarity::setDiscountOverlaps(bool v) { discountOverlaps = v; }

bool TFIDFSimilarity::getDiscountOverlaps() { return discountOverlaps; }

shared_ptr<Explanation>
TFIDFSimilarity::idfExplain(shared_ptr<CollectionStatistics> collectionStats,
                            shared_ptr<TermStatistics> termStats)
{
  constexpr int64_t df = termStats->docFreq();
  constexpr int64_t docCount = collectionStats->docCount() == -1
                                     ? collectionStats->maxDoc()
                                     : collectionStats->docCount();
  constexpr float idf = this->idf(df, docCount);
  return Explanation::match(idf, L"idf(docFreq=" + to_wstring(df) +
                                     L", docCount=" + to_wstring(docCount) +
                                     L")");
}

shared_ptr<Explanation> TFIDFSimilarity::idfExplain(
    shared_ptr<CollectionStatistics> collectionStats,
    std::deque<std::shared_ptr<TermStatistics>> &termStats)
{
  double idf = 0; // sum into a double before casting into a float
  deque<std::shared_ptr<Explanation>> subs =
      deque<std::shared_ptr<Explanation>>();
  for (auto stat : termStats) {
    shared_ptr<Explanation> idfExplain =
        this->idfExplain(collectionStats, stat);
    subs.push_back(idfExplain);
    idf += idfExplain->getValue();
  }
  return Explanation::match(static_cast<float>(idf), L"idf(), sum of:", subs);
}

int64_t TFIDFSimilarity::computeNorm(shared_ptr<FieldInvertState> state)
{
  constexpr int numTerms;
  if (discountOverlaps) {
    numTerms = state->getLength() - state->getNumOverlap();
  } else {
    numTerms = state->getLength();
  }
  if (state->getIndexCreatedVersionMajor() >= 7) {
    return SmallFloat::intToByte4(numTerms);
  } else {
    return SmallFloat::floatToByte315(lengthNorm(numTerms));
  }
}

shared_ptr<SimWeight>
TFIDFSimilarity::computeWeight(float boost,
                               shared_ptr<CollectionStatistics> collectionStats,
                               deque<TermStatistics> &termStats)
{
  shared_ptr<Explanation> *const idf =
      termStats->length == 1 ? idfExplain(collectionStats, termStats[0])
                             : idfExplain(collectionStats, termStats);
  std::deque<float> normTable(256);
  for (int i = 1; i < 256; ++i) {
    int length = SmallFloat::byte4ToInt(static_cast<char>(i));
    float norm = lengthNorm(length);
    normTable[i] = norm;
  }
  normTable[0] = 1.0f / normTable[255];
  return make_shared<IDFStats>(collectionStats->field(), boost, idf, normTable);
}

shared_ptr<SimScorer> TFIDFSimilarity::simScorer(
    shared_ptr<SimWeight> stats,
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<IDFStats> idfstats = std::static_pointer_cast<IDFStats>(stats);
  const std::deque<float> normTable;
  if (context->reader()->getMetaData()->getCreatedVersionMajor() >= 7) {
    // the norms only encode the length, we need a translation table that
    // depends on how lengthNorm is implemented
    normTable = idfstats->normTable;
  } else {
    // the norm is directly encoded in the index
    normTable = OLD_NORM_TABLE;
  }
  return make_shared<TFIDFSimScorer>(
      shared_from_this(), idfstats,
      context->reader()->getNormValues(idfstats->field), normTable);
}

TFIDFSimilarity::TFIDFSimScorer::TFIDFSimScorer(
    shared_ptr<TFIDFSimilarity> outerInstance, shared_ptr<IDFStats> stats,
    shared_ptr<NumericDocValues> norms,
    std::deque<float> &normTable) 
    : stats(stats), weightValue(stats->queryWeight), norms(norms),
      normTable(normTable), outerInstance(outerInstance)
{
}

float TFIDFSimilarity::TFIDFSimScorer::score(int doc,
                                             float freq) 
{
  constexpr float raw =
      outerInstance->tf(freq) * weightValue; // compute tf(f)*weight

  if (norms == nullptr) {
    return raw;
  } else {
    float normValue;
    if (norms->advanceExact(doc)) {
      normValue = normTable[static_cast<int>(norms->longValue() & 0xFF)];
    } else {
      normValue = 0;
    }
    return raw * normValue; // normalize for field
  }
}

float TFIDFSimilarity::TFIDFSimScorer::computeSlopFactor(int distance)
{
  return outerInstance->sloppyFreq(distance);
}

float TFIDFSimilarity::TFIDFSimScorer::computePayloadFactor(
    int doc, int start, int end, shared_ptr<BytesRef> payload)
{
  return outerInstance->scorePayload(doc, start, end, payload);
}

shared_ptr<Explanation> TFIDFSimilarity::TFIDFSimScorer::explain(
    int doc, shared_ptr<Explanation> freq) 
{
  return outerInstance->explainScore(doc, freq, stats, norms, normTable);
}

TFIDFSimilarity::IDFStats::IDFStats(const wstring &field, float boost,
                                    shared_ptr<Explanation> idf,
                                    std::deque<float> &normTable)
    : field(field), idf(idf), boost(boost),
      queryWeight(boost * idf->getValue()), normTable(normTable)
{
  // TODO: Validate?
}

shared_ptr<Explanation>
TFIDFSimilarity::explainField(int doc, shared_ptr<Explanation> freq,
                              shared_ptr<IDFStats> stats,
                              shared_ptr<NumericDocValues> norms,
                              std::deque<float> &normTable) 
{
  shared_ptr<Explanation> tfExplanation = Explanation::match(
      tf(freq->getValue()),
      L"tf(freq=" + to_wstring(freq->getValue()) + L"), with freq of:", freq);
  float norm;
  if (norms == nullptr) {
    norm = 1.0f;
  } else if (norms->advanceExact(doc) == false) {
    norm = 0.0f;
  } else {
    norm = normTable[static_cast<int>(norms->longValue() & 0xFF)];
  }

  shared_ptr<Explanation> fieldNormExpl =
      Explanation::match(norm, L"fieldNorm(doc=" + to_wstring(doc) + L")");

  return Explanation::match(tfExplanation->getValue() * stats->idf.getValue() *
                                fieldNormExpl->getValue(),
                            L"fieldWeight in " + to_wstring(doc) +
                                L", product of:",
                            {tfExplanation, stats->idf, fieldNormExpl});
}

shared_ptr<Explanation>
TFIDFSimilarity::explainScore(int doc, shared_ptr<Explanation> freq,
                              shared_ptr<IDFStats> stats,
                              shared_ptr<NumericDocValues> norms,
                              std::deque<float> &normTable) 
{
  shared_ptr<Explanation> queryExpl =
      Explanation::match(stats->boost, L"boost");
  shared_ptr<Explanation> fieldExpl =
      explainField(doc, freq, stats, norms, normTable);
  if (stats->boost == 1.0f) {
    return fieldExpl;
  }
  return Explanation::match(queryExpl->getValue() * fieldExpl->getValue(),
                            L"score(doc=" + to_wstring(doc) + L",freq=" +
                                to_wstring(freq->getValue()) +
                                L"), product of:",
                            {queryExpl, fieldExpl});
}
} // namespace org::apache::lucene::search::similarities