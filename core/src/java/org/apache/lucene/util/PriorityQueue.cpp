using namespace std;

#include "PriorityQueue.h"

namespace org::apache::lucene::util
{

PriorityQueue<T>::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<PriorityQueue<std::shared_ptr<T>>> outerInstance)
{
  this->outerInstance = outerInstance;
  i = 1;
}

bool PriorityQueue<T>::IteratorAnonymousInnerClass::hasNext()
{
  return i <= outerInstance->size_;
}

shared_ptr<T> PriorityQueue<T>::IteratorAnonymousInnerClass::next()
{
  if (hasNext() == false) {
    // C++ TODO: The following line could not be converted:
    throw java.util.NoSuchElementException();
  }
  return outerInstance->heap[i++];
}
} // namespace org::apache::lucene::util