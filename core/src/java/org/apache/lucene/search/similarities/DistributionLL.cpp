using namespace std;

#include "DistributionLL.h"

namespace org::apache::lucene::search::similarities
{

DistributionLL::DistributionLL() {}

float DistributionLL::score(shared_ptr<BasicStats> stats, float tfn,
                            float lambda)
{
  return static_cast<float>(-log(lambda / (tfn + lambda)));
}

wstring DistributionLL::toString() { return L"LL"; }
} // namespace org::apache::lucene::search::similarities