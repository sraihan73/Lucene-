using namespace std;

#include "BasicModelD.h"

namespace org::apache::lucene::search::similarities
{
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

BasicModelD::BasicModelD() {}

float BasicModelD::score(shared_ptr<BasicStats> stats, float tfn)
{
  // we have to ensure phi is always < 1 for tiny TTF values, otherwise nphi can
  // go negative, resulting in NaN. cleanest way is to unconditionally always
  // add tfn to totalTermFreq to create a 'normalized' F.
  double F = stats->getTotalTermFreq() + 1 + tfn;
  double phi = static_cast<double>(tfn) / F;
  double nphi = 1 - phi;
  double p = 1.0 / (stats->getNumberOfDocuments() + 1);
  double D = phi * log2(phi / p) + nphi * log2(nphi / (1 - p));
  return static_cast<float>(D * F + 0.5 * log2(1 + 2 * M_PI * tfn * nphi));
}

wstring BasicModelD::toString() { return L"D"; }
} // namespace org::apache::lucene::search::similarities