using namespace std;

#include "NormalizationH3.h"

namespace org::apache::lucene::search::similarities
{

NormalizationH3::NormalizationH3() : NormalizationH3(800.0F) {}

NormalizationH3::NormalizationH3(float mu) : mu(mu) {}

float NormalizationH3::tfn(shared_ptr<BasicStats> stats, float tf, float len)
{
  return (tf + mu * ((stats->getTotalTermFreq() + 1.0F) /
                     (stats->getNumberOfFieldTokens() + 1.0F))) /
         (len + mu) * mu;
}

wstring NormalizationH3::toString() { return L"3(" + to_wstring(mu) + L")"; }

float NormalizationH3::getMu() { return mu; }
} // namespace org::apache::lucene::search::similarities