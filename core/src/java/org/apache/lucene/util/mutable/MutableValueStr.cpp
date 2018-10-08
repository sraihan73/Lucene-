using namespace std;

#include "MutableValueStr.h"

namespace org::apache::lucene::util::mutable_
{
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

any MutableValueStr::toObject()
{
  assert(exists_ || 0 == value->length());
  return exists_ ? value->get().utf8ToString() : nullptr;
}

void MutableValueStr::copy(shared_ptr<MutableValue> source)
{
  shared_ptr<MutableValueStr> s =
      std::static_pointer_cast<MutableValueStr>(source);
  exists_ = s->exists_;
  value->copyBytes(s->value);
}

shared_ptr<MutableValue> MutableValueStr::duplicate()
{
  shared_ptr<MutableValueStr> v = make_shared<MutableValueStr>();
  v->value->copyBytes(value);
  v->exists_ = this->exists_;
  return v;
}

bool MutableValueStr::equalsSameType(any other)
{
  assert(exists_ || 0 == value->length());
  shared_ptr<MutableValueStr> b =
      any_cast<std::shared_ptr<MutableValueStr>>(other);
  return value->get().equals(b->value->get()) && exists_ == b->exists_;
}

int MutableValueStr::compareSameType(any other)
{
  assert(exists_ || 0 == value->length());
  shared_ptr<MutableValueStr> b =
      any_cast<std::shared_ptr<MutableValueStr>>(other);
  int c = value->get()->compareTo(b->value->get());
  if (c != 0) {
    return c;
  }
  if (exists_ == b->exists_) {
    return 0;
  }
  return exists_ ? 1 : -1;
}

int MutableValueStr::hashCode()
{
  assert(exists_ || 0 == value->length());
  return value->get().hashCode();
}
} // namespace org::apache::lucene::util::mutable_