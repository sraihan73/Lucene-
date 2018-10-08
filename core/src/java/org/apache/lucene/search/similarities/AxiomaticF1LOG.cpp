using namespace std;

#include "AxiomaticF1LOG.h"

namespace org::apache::lucene::search::similarities
{

AxiomaticF1LOG::AxiomaticF1LOG(float s) : Axiomatic(s) {}

AxiomaticF1LOG::AxiomaticF1LOG() : Axiomatic() {}

wstring AxiomaticF1LOG::toString() { return L"F1LOG"; }

float AxiomaticF1LOG::tf(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  if (freq <= 0.0) {
    return 0.0f;
  }
  return static_cast<float>(1 + log(1 + log(freq)));
}

float AxiomaticF1LOG::ln(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return (stats->getAvgFieldLength() + this->s) /
         (stats->getAvgFieldLength() + docLen * this->s);
}

float AxiomaticF1LOG::tfln(shared_ptr<BasicStats> stats, float freq,
                           float docLen)
{
  return 1.0f;
}

float AxiomaticF1LOG::idf(shared_ptr<BasicStats> stats, float freq,
                          float docLen)
{
  return static_cast<float>(
      log((stats->getNumberOfDocuments() + 1.0) / stats->getDocFreq()));
}

float AxiomaticF1LOG::gamma(shared_ptr<BasicStats> stats, float freq,
                            float docLen)
{
  return 0.0f;
}
} // namespace org::apache::lucene::search::similarities