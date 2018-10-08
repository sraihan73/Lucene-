using namespace std;

#include "TwoPhaseIterator.h"

namespace org::apache::lucene::search
{

TwoPhaseIterator::TwoPhaseIterator(shared_ptr<DocIdSetIterator> approximation)
    : approximation(Objects::requireNonNull(approximation))
{
}

shared_ptr<DocIdSetIterator> TwoPhaseIterator::asDocIdSetIterator(
    shared_ptr<TwoPhaseIterator> twoPhaseIterator)
{
  return make_shared<TwoPhaseIteratorAsDocIdSetIterator>(twoPhaseIterator);
}

shared_ptr<TwoPhaseIterator>
TwoPhaseIterator::unwrap(shared_ptr<DocIdSetIterator> iterator)
{
  if (std::dynamic_pointer_cast<TwoPhaseIteratorAsDocIdSetIterator>(iterator) !=
      nullptr) {
    return (std::static_pointer_cast<TwoPhaseIteratorAsDocIdSetIterator>(
                iterator))
        ->twoPhaseIterator;
  } else {
    return nullptr;
  }
}

TwoPhaseIterator::TwoPhaseIteratorAsDocIdSetIterator::
    TwoPhaseIteratorAsDocIdSetIterator(
        shared_ptr<TwoPhaseIterator> twoPhaseIterator)
    : twoPhaseIterator(twoPhaseIterator),
      approximation(twoPhaseIterator->approximation_)
{
}

int TwoPhaseIterator::TwoPhaseIteratorAsDocIdSetIterator::docID()
{
  return approximation->docID();
}

int TwoPhaseIterator::TwoPhaseIteratorAsDocIdSetIterator::nextDoc() throw(
    IOException)
{
  return doNext(approximation->nextDoc());
}

int TwoPhaseIterator::TwoPhaseIteratorAsDocIdSetIterator::advance(
    int target) 
{
  return doNext(approximation->advance(target));
}

int TwoPhaseIterator::TwoPhaseIteratorAsDocIdSetIterator::doNext(int doc) throw(
    IOException)
{
  for (;; doc = approximation->nextDoc()) {
    if (doc == NO_MORE_DOCS) {
      return NO_MORE_DOCS;
    } else if (twoPhaseIterator->matches()) {
      return doc;
    }
  }
}

int64_t TwoPhaseIterator::TwoPhaseIteratorAsDocIdSetIterator::cost()
{
  return approximation->cost();
}

shared_ptr<DocIdSetIterator> TwoPhaseIterator::approximation()
{
  return approximation_;
}
} // namespace org::apache::lucene::search