using namespace std;

#include "Scorer.h"

namespace org::apache::lucene::search
{

Scorer::Scorer(shared_ptr<Weight> weight) : weight(weight) {}

shared_ptr<Weight> Scorer::getWeight() { return weight; }

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
Scorer::getChildren() 
{
  return Collections::emptyList();
}

Scorer::ChildScorer::ChildScorer(shared_ptr<Scorer> child,
                                 const wstring &relationship)
    : child(child), relationship(relationship)
{
}

shared_ptr<TwoPhaseIterator> Scorer::twoPhaseIterator() { return nullptr; }
} // namespace org::apache::lucene::search