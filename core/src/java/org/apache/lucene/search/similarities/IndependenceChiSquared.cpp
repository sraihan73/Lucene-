using namespace std;

#include "IndependenceChiSquared.h"

namespace org::apache::lucene::search::similarities
{

IndependenceChiSquared::IndependenceChiSquared() {}

float IndependenceChiSquared::score(float freq, float expected)
{
  return (freq - expected) * (freq - expected) / expected;
}

wstring IndependenceChiSquared::toString() { return L"ChiSquared"; }
} // namespace org::apache::lucene::search::similarities