using namespace std;

#include "AxiomaticF3LOG.h"

namespace org::apache::lucene::search::similarities
{

AxiomaticF3LOG::AxiomaticF3LOG(float s, int queryLen) : Axiomatic(s, queryLen)
{
}

wstring AxiomaticF3LOG::toString() { return L"F3LOG"; }

float AxiomaticF3LOG::tf(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  if (freq <= 0.0) {
    return 0.0f;
  }
  return static_cast<float>(1 + log(1 + log(freq)));
}

float AxiomaticF3LOG::ln(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return 1.0f;
}

float AxiomaticF3LOG::tfln(shared_ptr<BasicStats> stats, float freq,
                           float docLen)
{
  return 1.0f;
}

float AxiomaticF3LOG::idf(shared_ptr<BasicStats> stats, float freq,
                          float docLen)
{
  return static_cast<float>(
      log((stats->getNumberOfDocuments() + 1.0) / stats->getDocFreq()));
}

float AxiomaticF3LOG::gamma(shared_ptr<BasicStats> stats, float freq,
                            float docLen)
{
  return (docLen - this->queryLen) * this->s * this->queryLen /
         stats->getAvgFieldLength();
}
} // namespace org::apache::lucene::search::similarities