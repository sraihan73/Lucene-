using namespace std;

#include "BasicModelIF.h"

namespace org::apache::lucene::search::similarities
{
//    import static org.apache.lucene.search.similarities.SimilarityBase.log2;

BasicModelIF::BasicModelIF() {}

float BasicModelIF::score(shared_ptr<BasicStats> stats, float tfn)
{
  int64_t N = stats->getNumberOfDocuments();
  int64_t F = stats->getTotalTermFreq();
  return tfn * static_cast<float>(log2(1 + (N + 1) / (F + 0.5)));
}

wstring BasicModelIF::toString() { return L"I(F)"; }
} // namespace org::apache::lucene::search::similarities