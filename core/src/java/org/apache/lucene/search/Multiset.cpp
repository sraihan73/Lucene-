using namespace std;

#include "Multiset.h"

namespace org::apache::lucene::search
{

Multiset<T>::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<Multiset<std::shared_ptr<T>>> outerInstance,
    unordered_map<T, int>::const_iterator mapIterator)
{
  this->outerInstance = outerInstance;
  this->mapIterator = mapIterator;
}

bool Multiset<T>::IteratorAnonymousInnerClass::hasNext()
{
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  return remaining > 0 || mapIterator.hasNext();
}

shared_ptr<T> Multiset<T>::IteratorAnonymousInnerClass::next()
{
  if (remaining == 0) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    unordered_map::Entry<std::shared_ptr<T>, int> next = mapIterator.next();
    current = next.getKey();
    remaining = next.getValue();
  }
  assert(remaining > 0);
  remaining -= 1;
  return current;
}
} // namespace org::apache::lucene::search