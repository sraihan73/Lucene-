using namespace std;

#include "PhraseMatcher.h"

namespace org::apache::lucene::search
{
using Similarity = org::apache::lucene::search::similarities::Similarity;

PhraseMatcher::PhraseMatcher(shared_ptr<DocIdSetIterator> approximation,
                             float matchCost)
    : approximation(approximation), matchCost(matchCost)
{
  assert(TwoPhaseIterator::unwrap(approximation) == nullptr);
}

float PhraseMatcher::getMatchCost() { return matchCost; }
} // namespace org::apache::lucene::search