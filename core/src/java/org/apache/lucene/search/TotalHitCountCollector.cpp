using namespace std;

#include "TotalHitCountCollector.h"

namespace org::apache::lucene::search
{

int TotalHitCountCollector::getTotalHits() { return totalHits; }

void TotalHitCountCollector::collect(int doc) { totalHits++; }

bool TotalHitCountCollector::needsScores() { return false; }
} // namespace org::apache::lucene::search