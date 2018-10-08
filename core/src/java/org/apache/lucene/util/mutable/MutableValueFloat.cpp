using namespace std;

#include "MutableValueFloat.h"

namespace org::apache::lucene::util::mutable_
{

any MutableValueFloat::toObject()
{
  assert(exists_ || 0.0F == value);
  return exists_ ? value : nullptr;
}

void MutableValueFloat::copy(shared_ptr<MutableValue> source)
{
  shared_ptr<MutableValueFloat> s =
      std::static_pointer_cast<MutableValueFloat>(source);
  value = s->value;
  exists_ = s->exists_;
}

shared_ptr<MutableValue> MutableValueFloat::duplicate()
{
  shared_ptr<MutableValueFloat> v = make_shared<MutableValueFloat>();
  v->value = this->value;
  v->exists_ = this->exists_;
  return v;
}

bool MutableValueFloat::equalsSameType(any other)
{
  assert(exists_ || 0.0F == value);
  shared_ptr<MutableValueFloat> b =
      any_cast<std::shared_ptr<MutableValueFloat>>(other);
  return value == b->value && exists_ == b->exists_;
}

int MutableValueFloat::compareSameType(any other)
{
  assert(exists_ || 0.0F == value);
  shared_ptr<MutableValueFloat> b =
      any_cast<std::shared_ptr<MutableValueFloat>>(other);
  int c = Float::compare(value, b->value);
  if (c != 0) {
    return c;
  }
  if (exists_ == b->exists_) {
    return 0;
  }
  return exists_ ? 1 : -1;
}

int MutableValueFloat::hashCode()
{
  assert(exists_ || 0.0F == value);
  return Float::floatToIntBits(value);
}
} // namespace org::apache::lucene::util::mutable_