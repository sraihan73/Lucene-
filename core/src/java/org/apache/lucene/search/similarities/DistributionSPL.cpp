using namespace std;

#include "DistributionSPL.h"

namespace org::apache::lucene::search::similarities
{

DistributionSPL::DistributionSPL() {}

float DistributionSPL::score(shared_ptr<BasicStats> stats, float tfn,
                             float lambda)
{
  if (lambda == 1.0f) {
    lambda = 0.99f;
  }
  return static_cast<float>(
      -log((pow(lambda, (tfn / (tfn + 1))) - lambda) / (1 - lambda)));
}

wstring DistributionSPL::toString() { return L"SPL"; }
} // namespace org::apache::lucene::search::similarities