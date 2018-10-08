using namespace std;

#include "DFISimilarity.h"

namespace org::apache::lucene::search::similarities
{

DFISimilarity::DFISimilarity(shared_ptr<Independence> independenceMeasure)
    : independence(independenceMeasure)
{
}

float DFISimilarity::score(shared_ptr<BasicStats> stats, float freq,
                           float docLen)
{

  constexpr float expected = (stats->getTotalTermFreq() + 1) * docLen /
                             (stats->getNumberOfFieldTokens() + 1);

  // if the observed frequency is less than or equal to the expected value, then
  // return zero.
  if (freq <= expected) {
    return 0;
  }

  constexpr float measure = independence->score(freq, expected);

  return stats->getBoost() * static_cast<float>(log2(measure + 1));
}

shared_ptr<Independence> DFISimilarity::getIndependence()
{
  return independence;
}

wstring DFISimilarity::toString() { return L"DFI(" + independence + L")"; }
} // namespace org::apache::lucene::search::similarities