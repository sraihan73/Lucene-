using namespace std;

#include "MutableValueLong.h"

namespace org::apache::lucene::util::mutable_
{

any MutableValueLong::toObject()
{
  assert(exists_ || 0LL == value);
  return exists_ ? value : nullptr;
}

void MutableValueLong::copy(shared_ptr<MutableValue> source)
{
  shared_ptr<MutableValueLong> s =
      std::static_pointer_cast<MutableValueLong>(source);
  exists_ = s->exists_;
  value = s->value;
}

shared_ptr<MutableValue> MutableValueLong::duplicate()
{
  shared_ptr<MutableValueLong> v = make_shared<MutableValueLong>();
  v->value = this->value;
  v->exists_ = this->exists_;
  return v;
}

bool MutableValueLong::equalsSameType(any other)
{
  assert(exists_ || 0LL == value);
  shared_ptr<MutableValueLong> b =
      any_cast<std::shared_ptr<MutableValueLong>>(other);
  return value == b->value && exists_ == b->exists_;
}

int MutableValueLong::compareSameType(any other)
{
  assert(exists_ || 0LL == value);
  shared_ptr<MutableValueLong> b =
      any_cast<std::shared_ptr<MutableValueLong>>(other);
  int64_t bv = b->value;
  if (value < bv) {
    return -1;
  }
  if (value > bv) {
    return 1;
  }
  if (exists_ == b->exists_) {
    return 0;
  }
  return exists_ ? 1 : -1;
}

int MutableValueLong::hashCode()
{
  assert(exists_ || 0LL == value);
  return static_cast<int>(value) + static_cast<int>(value >> 32);
}
} // namespace org::apache::lucene::util::mutable_