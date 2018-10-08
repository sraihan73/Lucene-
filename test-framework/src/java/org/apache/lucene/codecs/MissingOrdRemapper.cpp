using namespace std;

#include "MissingOrdRemapper.h"

namespace org::apache::lucene::codecs
{
using BytesRef = org::apache::lucene::util::BytesRef;

deque<std::shared_ptr<BytesRef>> MissingOrdRemapper::insertEmptyValue(
    deque<std::shared_ptr<BytesRef>> &iterable)
{
  return make_shared<IterableAnonymousInnerClass>(iterable);
}

MissingOrdRemapper::IterableAnonymousInnerClass::IterableAnonymousInnerClass(
    deque<std::shared_ptr<BytesRef>> &iterable)
{
  this->iterable = iterable;
}

shared_ptr<Iterator<std::shared_ptr<BytesRef>>>
MissingOrdRemapper::IterableAnonymousInnerClass::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

MissingOrdRemapper::IterableAnonymousInnerClass::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(
        shared_ptr<IterableAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  seenEmpty = false;
  in_ = outerInstance->iterable.begin();
}

bool MissingOrdRemapper::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::hasNext()
{
  return !seenEmpty || in_->hasNext();
}

shared_ptr<BytesRef> MissingOrdRemapper::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::next()
{
  if (!seenEmpty) {
    seenEmpty = true;
    return make_shared<BytesRef>();
  } else {
    return in_->next();
  }
}

void MissingOrdRemapper::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

deque<std::shared_ptr<Number>>
MissingOrdRemapper::mapMissingToOrd0(deque<std::shared_ptr<Number>> &iterable)
{
  return make_shared<IterableAnonymousInnerClass2>(iterable);
}

MissingOrdRemapper::IterableAnonymousInnerClass2::IterableAnonymousInnerClass2(
    deque<std::shared_ptr<Number>> &iterable)
{
  this->iterable = iterable;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
MissingOrdRemapper::IterableAnonymousInnerClass2::iterator()
{
  return make_shared<IteratorAnonymousInnerClass2>(shared_from_this());
}

MissingOrdRemapper::IterableAnonymousInnerClass2::IteratorAnonymousInnerClass2::
    IteratorAnonymousInnerClass2(
        shared_ptr<IterableAnonymousInnerClass2> outerInstance)
{
  this->outerInstance = outerInstance;
  in_ = outerInstance->iterable.begin();
}

bool MissingOrdRemapper::IterableAnonymousInnerClass2::
    IteratorAnonymousInnerClass2::hasNext()
{
  return in_->hasNext();
}

shared_ptr<Number> MissingOrdRemapper::IterableAnonymousInnerClass2::
    IteratorAnonymousInnerClass2::next()
{
  shared_ptr<Number> n = in_->next();
  if (n->longValue() == -1) {
    return 0;
  } else {
    return n;
  }
}

void MissingOrdRemapper::IterableAnonymousInnerClass2::
    IteratorAnonymousInnerClass2::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

deque<std::shared_ptr<Number>>
MissingOrdRemapper::mapAllOrds(deque<std::shared_ptr<Number>> &iterable)
{
  return make_shared<IterableAnonymousInnerClass3>(iterable);
}

MissingOrdRemapper::IterableAnonymousInnerClass3::IterableAnonymousInnerClass3(
    deque<std::shared_ptr<Number>> &iterable)
{
  this->iterable = iterable;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
MissingOrdRemapper::IterableAnonymousInnerClass3::iterator()
{
  return make_shared<IteratorAnonymousInnerClass3>(shared_from_this());
}

MissingOrdRemapper::IterableAnonymousInnerClass3::IteratorAnonymousInnerClass3::
    IteratorAnonymousInnerClass3(
        shared_ptr<IterableAnonymousInnerClass3> outerInstance)
{
  this->outerInstance = outerInstance;
  in_ = outerInstance->iterable.begin();
}

bool MissingOrdRemapper::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass3::hasNext()
{
  return in_->hasNext();
}

shared_ptr<Number> MissingOrdRemapper::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass3::next()
{
  shared_ptr<Number> n = in_->next();
  return n->longValue() + 1;
}

void MissingOrdRemapper::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass3::remove()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs