using namespace std;

#include "AfterEffect.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

AfterEffect::AfterEffect() {}

AfterEffect::NoAfterEffect::NoAfterEffect() {}

float AfterEffect::NoAfterEffect::score(shared_ptr<BasicStats> stats, float tfn)
{
  return 1.0f;
}

shared_ptr<Explanation>
AfterEffect::NoAfterEffect::explain(shared_ptr<BasicStats> stats, float tfn)
{
  return Explanation::match(1, L"no aftereffect");
}

wstring AfterEffect::NoAfterEffect::toString() { return L""; }
} // namespace org::apache::lucene::search::similarities