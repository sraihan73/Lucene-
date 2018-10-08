using namespace std;

#include "WeakIdentityMap.h"

namespace org::apache::lucene::util
{

WeakIdentityMap<K, V>::IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
    shared_ptr<WeakIdentityMap<std::shared_ptr<K>, std::shared_ptr<V>>>
        outerInstance,
    shared_ptr<Iterator<std::shared_ptr<IdentityWeakReference>>> iterator)
{
  this->outerInstance = outerInstance;
  this->iterator = iterator;
  next = nullptr;
  nextIsSet = false;
}

bool WeakIdentityMap<K, V>::IteratorAnonymousInnerClass::hasNext()
{
  return nextIsSet || setNext();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public K next()
shared_ptr<K> WeakIdentityMap<K, V>::IteratorAnonymousInnerClass::next()
{
  if (!hasNext()) {
    // C++ TODO: The following line could not be converted:
    throw java.util.NoSuchElementException();
  }
  assert(nextIsSet);
  try {
    return std::static_pointer_cast<K>(next);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // release strong reference and invalidate current value:
    nextIsSet = false;
    next = nullptr;
  }
}

void WeakIdentityMap<K, V>::IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

bool WeakIdentityMap<K, V>::IteratorAnonymousInnerClass::setNext()
{
  assert(!nextIsSet);
  while (iterator->hasNext()) {
    next = iterator->next()->get();
    if (next == nullptr) {
      // the key was already GCed, we can remove it from backing map_obj:
      iterator->remove();
    } else {
      // unfold "null" special value:
      if (next == NULL) {
        next = nullptr;
      }
      return nextIsSet = true;
    }
    iterator++;
  }
  return false;
}

WeakIdentityMap<K, V>::IdentityWeakReference::IdentityWeakReference(
    any obj, shared_ptr<ReferenceQueue<any>> queue)
    : WeakReference<Object>(obj == nullptr ? NULL : obj, queue),
      hash(System::identityHashCode(obj))
{
}

int WeakIdentityMap<K, V>::IdentityWeakReference::hashCode() { return hash; }

bool WeakIdentityMap<K, V>::IdentityWeakReference::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (std::dynamic_pointer_cast<IdentityWeakReference>(o) != nullptr) {
    shared_ptr<IdentityWeakReference> *const ref =
        any_cast<std::shared_ptr<IdentityWeakReference>>(o);
    if (this->get() == ref->get()) {
      return true;
    }
  }
  return false;
}
} // namespace org::apache::lucene::util