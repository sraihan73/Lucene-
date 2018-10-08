using namespace std;

#include "NormalizationH2.h"

namespace org::apache::lucene::search::similarities
{
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

NormalizationH2::NormalizationH2(float c) : c(c) {}

NormalizationH2::NormalizationH2() : NormalizationH2(1) {}

float NormalizationH2::tfn(shared_ptr<BasicStats> stats, float tf, float len)
{
  return static_cast<float>(tf *
                            log2(1 + c * stats->getAvgFieldLength() / len));
}

wstring NormalizationH2::toString() { return L"2"; }

float NormalizationH2::getC() { return c; }
} // namespace org::apache::lucene::search::similarities