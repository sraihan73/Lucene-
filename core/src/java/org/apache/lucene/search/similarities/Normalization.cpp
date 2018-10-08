using namespace std;

#include "Normalization.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

Normalization::Normalization() {}

shared_ptr<Explanation> Normalization::explain(shared_ptr<BasicStats> stats,
                                               float tf, float len)
{
  return Explanation::match(
      tfn(stats, tf, len), getClass().getSimpleName() + L", computed from: ",
      {Explanation::match(tf, L"tf"),
       Explanation::match(stats->getAvgFieldLength(), L"avgFieldLength"),
       Explanation::match(len, L"len")});
}

Normalization::NoNormalization::NoNormalization() {}

float Normalization::NoNormalization::tfn(shared_ptr<BasicStats> stats,
                                          float tf, float len)
{
  return tf;
}

shared_ptr<Explanation>
Normalization::NoNormalization::explain(shared_ptr<BasicStats> stats, float tf,
                                        float len)
{
  return Explanation::match(1, L"no normalization");
}

wstring Normalization::NoNormalization::toString() { return L""; }
} // namespace org::apache::lucene::search::similarities