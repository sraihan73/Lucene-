using namespace std;

#include "ConjunctionScorer.h"

namespace org::apache::lucene::search
{

ConjunctionScorer::ConjunctionScorer(
    shared_ptr<Weight> weight,
    shared_ptr<deque<std::shared_ptr<Scorer>>> required,
    shared_ptr<deque<std::shared_ptr<Scorer>>> scorers)
    : Scorer(weight), disi(ConjunctionDISI::intersectScorers(required)),
      scorers(scorers->toArray(
          std::deque<std::shared_ptr<Scorer>>(scorers->size()))),
      required(required)
{
  assert(required->containsAll(scorers));
}

shared_ptr<TwoPhaseIterator> ConjunctionScorer::twoPhaseIterator()
{
  return TwoPhaseIterator::unwrap(disi);
}

shared_ptr<DocIdSetIterator> ConjunctionScorer::iterator() { return disi; }

int ConjunctionScorer::docID() { return disi->docID(); }

float ConjunctionScorer::score() 
{
  double sum = 0.0;
  for (auto scorer : scorers) {
    sum += scorer->score();
  }
  return static_cast<float>(sum);
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>>
ConjunctionScorer::getChildren()
{
  deque<std::shared_ptr<ChildScorer>> children =
      deque<std::shared_ptr<ChildScorer>>();
  for (auto scorer : required) {
    children.push_back(make_shared<ChildScorer>(scorer, L"MUST"));
  }
  return children;
}

ConjunctionScorer::DocsAndFreqs::DocsAndFreqs(
    shared_ptr<DocIdSetIterator> iterator)
    : cost(iterator->cost()), iterator(iterator)
{
}
} // namespace org::apache::lucene::search