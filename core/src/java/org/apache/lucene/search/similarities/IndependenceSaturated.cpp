using namespace std;

#include "IndependenceSaturated.h"

namespace org::apache::lucene::search::similarities
{

IndependenceSaturated::IndependenceSaturated() {}

float IndependenceSaturated::score(float freq, float expected)
{
  return (freq - expected) / expected;
}

wstring IndependenceSaturated::toString() { return L"Saturated"; }
} // namespace org::apache::lucene::search::similarities