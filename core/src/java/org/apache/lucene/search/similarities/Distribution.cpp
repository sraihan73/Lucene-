using namespace std;

#include "Distribution.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;

Distribution::Distribution() {}

shared_ptr<Explanation> Distribution::explain(shared_ptr<BasicStats> stats,
                                              float tfn, float lambda)
{
  return Explanation::match(score(stats, tfn, lambda),
                            getClass().getSimpleName());
}
} // namespace org::apache::lucene::search::similarities