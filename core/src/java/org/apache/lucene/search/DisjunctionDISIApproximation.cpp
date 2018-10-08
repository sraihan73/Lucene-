using namespace std;

#include "DisjunctionDISIApproximation.h"

namespace org::apache::lucene::search
{

DisjunctionDISIApproximation::DisjunctionDISIApproximation(
    shared_ptr<DisiPriorityQueue> subIterators)
    : subIterators(subIterators), cost(cost_)
{
  int64_t cost = 0;
  for (auto w : subIterators) {
    cost += w->cost;
  }
}

int64_t DisjunctionDISIApproximation::cost() { return cost_; }

int DisjunctionDISIApproximation::docID() { return subIterators->top()->doc; }

int DisjunctionDISIApproximation::nextDoc() 
{
  shared_ptr<DisiWrapper> top = subIterators->top();
  constexpr int doc = top->doc;
  do {
    top->doc = top->approximation->nextDoc();
    top = subIterators->updateTop();
  } while (top->doc == doc);

  return top->doc;
}

int DisjunctionDISIApproximation::advance(int target) 
{
  shared_ptr<DisiWrapper> top = subIterators->top();
  do {
    top->doc = top->approximation->advance(target);
    top = subIterators->updateTop();
  } while (top->doc < target);

  return top->doc;
}
} // namespace org::apache::lucene::search