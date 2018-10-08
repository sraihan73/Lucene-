using namespace std;

#include "AfterEffectL.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

AfterEffectL::AfterEffectL() {}

float AfterEffectL::score(shared_ptr<BasicStats> stats, float tfn)
{
  return 1 / (tfn + 1);
}

shared_ptr<Explanation> AfterEffectL::explain(shared_ptr<BasicStats> stats,
                                              float tfn)
{
  return Explanation::match(score(stats, tfn),
                            getClass().getSimpleName() + L", computed from: ",
                            Explanation::match(tfn, L"tfn"));
}

wstring AfterEffectL::toString() { return L"L"; }
} // namespace org::apache::lucene::search::similarities