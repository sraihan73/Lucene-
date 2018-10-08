using namespace std;

#include "FakeScorer.h"

namespace org::apache::lucene::search
{

FakeScorer::FakeScorer() : Scorer(nullptr) {}

int FakeScorer::docID() { return doc; }

float FakeScorer::score() { return score_; }

shared_ptr<DocIdSetIterator> FakeScorer::iterator()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Weight> FakeScorer::getWeight()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<deque<std::shared_ptr<ChildScorer>>> FakeScorer::getChildren()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::search