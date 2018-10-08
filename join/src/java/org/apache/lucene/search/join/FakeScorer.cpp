using namespace std;

#include "FakeScorer.h"

namespace org::apache::lucene::search::join
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;

FakeScorer::FakeScorer() : org::apache::lucene::search::Scorer(nullptr) {}

int FakeScorer::docID() { return doc; }

shared_ptr<DocIdSetIterator> FakeScorer::iterator()
{
  throw make_shared<UnsupportedOperationException>();
}

float FakeScorer::score()  { return score_; }
} // namespace org::apache::lucene::search::join