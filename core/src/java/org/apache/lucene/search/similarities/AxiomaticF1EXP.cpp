using namespace std;

#include "AxiomaticF1EXP.h"

namespace org::apache::lucene::search::similarities
{

AxiomaticF1EXP::AxiomaticF1EXP(float s, float k) : Axiomatic(s, 1, k) {}

AxiomaticF1EXP::AxiomaticF1EXP(float s) : AxiomaticF1EXP(s, 0.35f) {}

AxiomaticF1EXP::AxiomaticF1EXP() : Axiomatic() {}

wstring AxiomaticF1EXP::toString() { return L"F1EXP"; }

float AxiomaticF1EXP::tf(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  if (freq <= 0.0) {
    return 0.0f;
  }
  return static_cast<float>(1 + log(1 + log(freq)));
}

float AxiomaticF1EXP::ln(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return (stats->getAvgFieldLength() + this->s) /
         (stats->getAvgFieldLength() + docLen * this->s);
}

float AxiomaticF1EXP::tfln(shared_ptr<BasicStats> stats, float freq,
                           float docLen)
{
  return 1.0f;
}

float AxiomaticF1EXP::idf(shared_ptr<BasicStats> stats, float freq,
                          float docLen)
{
  return static_cast<float>(pow(
      (stats->getNumberOfDocuments() + 1.0) / stats->getDocFreq(), this->k));
}

float AxiomaticF1EXP::gamma(shared_ptr<BasicStats> stats, float freq,
                            float docLen)
{
  return 0.0f;
}
} // namespace org::apache::lucene::search::similarities