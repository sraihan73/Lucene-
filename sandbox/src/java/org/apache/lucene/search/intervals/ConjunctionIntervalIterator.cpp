using namespace std;

#include "ConjunctionIntervalIterator.h"

namespace org::apache::lucene::search::intervals
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

ConjunctionIntervalIterator::ConjunctionIntervalIterator(
    deque<std::shared_ptr<IntervalIterator>> &subIterators)
    : approximation(ConjunctionDISI::intersectIterators(subIterators)),
      subIterators(subIterators), cost(costsum)
{
  float costsum = 0;
  for (auto it : subIterators) {
    costsum += it->matchCost();
  }
}

int ConjunctionIntervalIterator::docID() { return approximation->docID(); }

int ConjunctionIntervalIterator::nextDoc() 
{
  int doc = approximation->nextDoc();
  reset();
  return doc;
}

int ConjunctionIntervalIterator::advance(int target) 
{
  int doc = approximation->advance(target);
  reset();
  return doc;
}

int64_t ConjunctionIntervalIterator::cost() { return approximation->cost(); }

float ConjunctionIntervalIterator::matchCost() { return cost_; }
} // namespace org::apache::lucene::search::intervals