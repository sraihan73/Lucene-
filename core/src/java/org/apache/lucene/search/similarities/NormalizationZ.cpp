using namespace std;

#include "NormalizationZ.h"

namespace org::apache::lucene::search::similarities
{

NormalizationZ::NormalizationZ() : NormalizationZ(0.30F) {}

NormalizationZ::NormalizationZ(float z) : z(z) {}

float NormalizationZ::tfn(shared_ptr<BasicStats> stats, float tf, float len)
{
  return static_cast<float>(tf * pow(stats->avgFieldLength / len, z));
}

wstring NormalizationZ::toString() { return L"Z(" + to_wstring(z) + L")"; }

float NormalizationZ::getZ() { return z; }
} // namespace org::apache::lucene::search::similarities