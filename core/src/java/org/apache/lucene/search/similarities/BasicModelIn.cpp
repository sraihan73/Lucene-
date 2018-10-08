using namespace std;

#include "BasicModelIn.h"

namespace org::apache::lucene::search::similarities
{
using Explanation = org::apache::lucene::search::Explanation;
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

BasicModelIn::BasicModelIn() {}

float BasicModelIn::score(shared_ptr<BasicStats> stats, float tfn)
{
  int64_t N = stats->getNumberOfDocuments();
  int64_t n = stats->getDocFreq();
  return tfn * static_cast<float>(log2((N + 1) / (n + 0.5)));
}

shared_ptr<Explanation> BasicModelIn::explain(shared_ptr<BasicStats> stats,
                                              float tfn)
{
  return Explanation::match(
      score(stats, tfn), getClass().getSimpleName() + L", computed from: ",
      {Explanation::match(stats->getNumberOfDocuments(), L"numberOfDocuments"),
       Explanation::match(stats->getDocFreq(), L"docFreq")});
}

wstring BasicModelIn::toString() { return L"I(n)"; }
} // namespace org::apache::lucene::search::similarities