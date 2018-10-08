using namespace std;

#include "AfterEffectB.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

AfterEffectB::AfterEffectB() {}

float AfterEffectB::score(shared_ptr<BasicStats> stats, float tfn)
{
  int64_t F = stats->getTotalTermFreq() + 1;
  int64_t n = stats->getDocFreq() + 1;
  return (F + 1) / (n * (tfn + 1));
}

shared_ptr<Explanation> AfterEffectB::explain(shared_ptr<BasicStats> stats,
                                              float tfn)
{
  return Explanation::match(
      score(stats, tfn), getClass().getSimpleName() + L", computed from: ",
      {Explanation::match(tfn, L"tfn"),
       Explanation::match(stats->getTotalTermFreq(), L"totalTermFreq"),
       Explanation::match(stats->getDocFreq(), L"docFreq")});
}

wstring AfterEffectB::toString() { return L"B"; }
} // namespace org::apache::lucene::search::similarities