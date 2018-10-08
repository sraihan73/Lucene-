using namespace std;

#include "ClassicSimilarity.h"

namespace org::apache::lucene::search::similarities
{
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;

ClassicSimilarity::ClassicSimilarity() {}

float ClassicSimilarity::lengthNorm(int numTerms)
{
  return static_cast<float>(1.0 / sqrt(numTerms));
}

float ClassicSimilarity::tf(float freq)
{
  return static_cast<float>(sqrt(freq));
}

float ClassicSimilarity::sloppyFreq(int distance)
{
  return 1.0f / (distance + 1);
}

float ClassicSimilarity::scorePayload(int doc, int start, int end,
                                      shared_ptr<BytesRef> payload)
{
  return 1;
}

shared_ptr<Explanation>
ClassicSimilarity::idfExplain(shared_ptr<CollectionStatistics> collectionStats,
                              shared_ptr<TermStatistics> termStats)
{
  constexpr int64_t df = termStats->docFreq();
  constexpr int64_t docCount = collectionStats->docCount() == -1
                                     ? collectionStats->maxDoc()
                                     : collectionStats->docCount();
  constexpr float idf = this->idf(df, docCount);
  return Explanation::match(
      idf, L"idf, computed as log((docCount+1)/(docFreq+1)) + 1 from:",
      {Explanation::match(df, L"docFreq"),
       Explanation::match(docCount, L"docCount")});
}

float ClassicSimilarity::idf(int64_t docFreq, int64_t docCount)
{
  return static_cast<float>(
      log((docCount + 1) / static_cast<double>(docFreq + 1)) + 1.0);
}

wstring ClassicSimilarity::toString() { return L"ClassicSimilarity"; }
} // namespace org::apache::lucene::search::similarities