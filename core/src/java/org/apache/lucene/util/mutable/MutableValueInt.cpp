using namespace std;

#include "MutableValueInt.h"

namespace org::apache::lucene::util::mutable_
{

any MutableValueInt::toObject()
{
  assert(exists_ || 0 == value);
  return exists_ ? value : nullptr;
}

void MutableValueInt::copy(shared_ptr<MutableValue> source)
{
  shared_ptr<MutableValueInt> s =
      std::static_pointer_cast<MutableValueInt>(source);
  value = s->value;
  exists_ = s->exists_;
}

shared_ptr<MutableValue> MutableValueInt::duplicate()
{
  shared_ptr<MutableValueInt> v = make_shared<MutableValueInt>();
  v->value = this->value;
  v->exists_ = this->exists_;
  return v;
}

bool MutableValueInt::equalsSameType(any other)
{
  assert(exists_ || 0 == value);
  shared_ptr<MutableValueInt> b =
      any_cast<std::shared_ptr<MutableValueInt>>(other);
  return value == b->value && exists_ == b->exists_;
}

int MutableValueInt::compareSameType(any other)
{
  assert(exists_ || 0 == value);
  shared_ptr<MutableValueInt> b =
      any_cast<std::shared_ptr<MutableValueInt>>(other);
  int ai = value;
  int bi = b->value;
  if (ai < bi) {
    return -1;
  } else if (ai > bi) {
    return 1;
  }

  if (exists_ == b->exists_) {
    return 0;
  }
  return exists_ ? 1 : -1;
}

int MutableValueInt::hashCode()
{
  assert(exists_ || 0 == value);
  // TODO: if used in HashMap, it already mixes the value... maybe use a
  // straight value?
  return (value >> 8) + (value >> 16);
}
} // namespace org::apache::lucene::util::mutable_