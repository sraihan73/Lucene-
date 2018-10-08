using namespace std;

#include "AxiomaticF2EXP.h"

namespace org::apache::lucene::search::similarities
{

AxiomaticF2EXP::AxiomaticF2EXP(float s, float k) : Axiomatic(s, 1, k) {}

AxiomaticF2EXP::AxiomaticF2EXP(float s) : AxiomaticF2EXP(s, 0.35f) {}

AxiomaticF2EXP::AxiomaticF2EXP() : Axiomatic() {}

wstring AxiomaticF2EXP::toString() { return L"F2EXP"; }

float AxiomaticF2EXP::tf(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return 1.0f;
}

float AxiomaticF2EXP::ln(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return 1.0f;
}

float AxiomaticF2EXP::tfln(shared_ptr<BasicStats> stats, float freq,
                           float docLen)
{
  return freq /
         (freq + this->s + this->s * docLen / stats->getAvgFieldLength());
}

float AxiomaticF2EXP::idf(shared_ptr<BasicStats> stats, float freq,
                          float docLen)
{
  return static_cast<float>(pow(
      (stats->getNumberOfDocuments() + 1.0) / stats->getDocFreq(), this->k));
}

float AxiomaticF2EXP::gamma(shared_ptr<BasicStats> stats, float freq,
                            float docLen)
{
  return 0.0f;
}
} // namespace org::apache::lucene::search::similarities