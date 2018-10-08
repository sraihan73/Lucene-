using namespace std;

#include "BasicModelBE.h"

namespace org::apache::lucene::search::similarities
{
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

BasicModelBE::BasicModelBE() {}

float BasicModelBE::score(shared_ptr<BasicStats> stats, float tfn)
{
  double F = stats->getTotalTermFreq() + 1 + tfn;
  // approximation only holds true when F << N, so we use N += F
  double N = F + stats->getNumberOfDocuments();
  return static_cast<float>(-log2((N - 1) * M_E) +
                            f(N + F - 1, N + F - tfn - 2) - f(F, F - tfn));
}

double BasicModelBE::f(double n, double m)
{
  return (m + 0.5) * log2(n / m) + (n - m) * log2(n);
}

wstring BasicModelBE::toString() { return L"Be"; }
} // namespace org::apache::lucene::search::similarities