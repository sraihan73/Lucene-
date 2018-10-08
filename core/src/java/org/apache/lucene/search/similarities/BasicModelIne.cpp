using namespace std;

#include "BasicModelIne.h"

namespace org::apache::lucene::search::similarities
{
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

BasicModelIne::BasicModelIne() {}

float BasicModelIne::score(shared_ptr<BasicStats> stats, float tfn)
{
  int64_t N = stats->getNumberOfDocuments();
  int64_t F = stats->getTotalTermFreq();
  double ne = N * (1 - pow((N - 1) / static_cast<double>(N), F));
  return tfn * static_cast<float>(log2((N + 1) / (ne + 0.5)));
}

wstring BasicModelIne::toString() { return L"I(ne)"; }
} // namespace org::apache::lucene::search::similarities