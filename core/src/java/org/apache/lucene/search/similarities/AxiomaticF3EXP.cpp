using namespace std;

#include "AxiomaticF3EXP.h"

namespace org::apache::lucene::search::similarities
{

AxiomaticF3EXP::AxiomaticF3EXP(float s, int queryLen, float k)
    : Axiomatic(s, queryLen, k)
{
}

AxiomaticF3EXP::AxiomaticF3EXP(float s, int queryLen)
    : AxiomaticF3EXP(s, queryLen, 0.35f)
{
}

wstring AxiomaticF3EXP::toString() { return L"F3EXP"; }

float AxiomaticF3EXP::tf(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  if (freq <= 0.0) {
    return 0.0f;
  }
  return static_cast<float>(1 + log(1 + log(freq)));
}

float AxiomaticF3EXP::ln(shared_ptr<BasicStats> stats, float freq, float docLen)
{
  return 1.0f;
}

float AxiomaticF3EXP::tfln(shared_ptr<BasicStats> stats, float freq,
                           float docLen)
{
  return 1.0f;
}

float AxiomaticF3EXP::idf(shared_ptr<BasicStats> stats, float freq,
                          float docLen)
{
  return static_cast<float>(pow(
      (stats->getNumberOfDocuments() + 1.0) / stats->getDocFreq(), this->k));
}

float AxiomaticF3EXP::gamma(shared_ptr<BasicStats> stats, float freq,
                            float docLen)
{
  return (docLen - this->queryLen) * this->s * this->queryLen /
         stats->getAvgFieldLength();
}
} // namespace org::apache::lucene::search::similarities