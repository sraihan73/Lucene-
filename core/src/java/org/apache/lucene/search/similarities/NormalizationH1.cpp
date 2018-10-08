using namespace std;

#include "NormalizationH1.h"

namespace org::apache::lucene::search::similarities
{

NormalizationH1::NormalizationH1(float c) : c(c) {}

NormalizationH1::NormalizationH1() : NormalizationH1(1) {}

float NormalizationH1::tfn(shared_ptr<BasicStats> stats, float tf, float len)
{
  return tf * c * stats->getAvgFieldLength() / len;
}

wstring NormalizationH1::toString() { return L"1"; }

float NormalizationH1::getC() { return c; }
} // namespace org::apache::lucene::search::similarities