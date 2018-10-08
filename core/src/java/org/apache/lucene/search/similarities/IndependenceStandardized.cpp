using namespace std;

#include "IndependenceStandardized.h"

namespace org::apache::lucene::search::similarities
{

IndependenceStandardized::IndependenceStandardized() {}

float IndependenceStandardized::score(float freq, float expected)
{
  return (freq - expected) / static_cast<float>(sqrt(expected));
}

wstring IndependenceStandardized::toString() { return L"Standardized"; }
} // namespace org::apache::lucene::search::similarities