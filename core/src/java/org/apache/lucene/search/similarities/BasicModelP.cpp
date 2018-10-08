using namespace std;

#include "BasicModelP.h"

namespace org::apache::lucene::search::similarities
{
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;
double BasicModelP::LOG2_E = log2(M_E);

BasicModelP::BasicModelP() {}

float BasicModelP::score(shared_ptr<BasicStats> stats, float tfn)
{
  float lambda = static_cast<float>(stats->getTotalTermFreq() + 1) /
                 (stats->getNumberOfDocuments() + 1);
  return static_cast<float>(tfn * log2(tfn / lambda) +
                            (lambda + 1 / (12 * tfn) - tfn) * LOG2_E +
                            0.5 * log2(2 * M_PI * tfn));
}

wstring BasicModelP::toString() { return L"P"; }
} // namespace org::apache::lucene::search::similarities