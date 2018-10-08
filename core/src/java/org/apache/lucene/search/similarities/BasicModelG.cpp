using namespace std;

#include "BasicModelG.h"

namespace org::apache::lucene::search::similarities
{
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

BasicModelG::BasicModelG() {}

float BasicModelG::score(shared_ptr<BasicStats> stats, float tfn)
{
  // just like in BE, approximation only holds true when F << N, so we use
  // lambda = F / (N + F)
  double F = stats->getTotalTermFreq() + 1;
  double N = stats->getNumberOfDocuments();
  double lambda = F / (N + F);
  // -log(1 / (lambda + 1)) -> log(lambda + 1)
  return static_cast<float>(log2(lambda + 1) +
                            tfn * log2((1 + lambda) / lambda));
}

wstring BasicModelG::toString() { return L"G"; }
} // namespace org::apache::lucene::search::similarities