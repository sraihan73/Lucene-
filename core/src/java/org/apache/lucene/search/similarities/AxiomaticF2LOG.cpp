using namespace std;

#include "AxiomaticF2LOG.h"

namespace org::apache::lucene::search::similarities
{

AxiomaticF2LOG::AxiomaticF2LOG(float s) : Axiomatic(s) {}

AxiomaticF2LOG::AxiomaticF2LOG() : Axiomatic() {}

wstring AxiomaticF2LOG::toString() { return L"F2LOG"; }

float AxiomaticF2LOG::tf(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return 1.0f;
}

float AxiomaticF2LOG::ln(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return 1.0f;
}

float AxiomaticF2LOG::tfln(shared_ptr<BasicStats> stats, float freq,
                           float docLen)
{
  return freq /
         (freq + this->s + this->s * docLen / stats->getAvgFieldLength());
}

float AxiomaticF2LOG::idf(shared_ptr<BasicStats> stats, float freq,
                          float docLen)
{
  return static_cast<float>(
      log((stats->getNumberOfDocuments() + 1.0) / stats->getDocFreq()));
}

float AxiomaticF2LOG::gamma(shared_ptr<BasicStats> stats, float freq,
                            float docLen)
{
  return 0.0f;
}
} // namespace org::apache::lucene::search::similarities