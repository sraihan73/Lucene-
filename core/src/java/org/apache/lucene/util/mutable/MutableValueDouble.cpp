using namespace std;

#include "MutableValueDouble.h"

namespace org::apache::lucene::util::mutable_
{

any MutableValueDouble::toObject()
{
  assert(exists_ || 0.0 == value);
  return exists_ ? value : nullptr;
}

void MutableValueDouble::copy(shared_ptr<MutableValue> source)
{
  shared_ptr<MutableValueDouble> s =
      std::static_pointer_cast<MutableValueDouble>(source);
  value = s->value;
  exists_ = s->exists_;
}

shared_ptr<MutableValue> MutableValueDouble::duplicate()
{
  shared_ptr<MutableValueDouble> v = make_shared<MutableValueDouble>();
  v->value = this->value;
  v->exists_ = this->exists_;
  return v;
}

bool MutableValueDouble::equalsSameType(any other)
{
  assert(exists_ || 0.0 == value);
  shared_ptr<MutableValueDouble> b =
      any_cast<std::shared_ptr<MutableValueDouble>>(other);
  return value == b->value && exists_ == b->exists_;
}

int MutableValueDouble::compareSameType(any other)
{
  assert(exists_ || 0.0 == value);
  shared_ptr<MutableValueDouble> b =
      any_cast<std::shared_ptr<MutableValueDouble>>(other);
  int c = Double::compare(value, b->value);
  if (c != 0) {
    return c;
  }
  if (exists_ == b->exists_) {
    return 0;
  }
  return exists_ ? 1 : -1;
}

int MutableValueDouble::hashCode()
{
  assert(exists_ || 0.0 == value);
  int64_t x = Double::doubleToLongBits(value);
  return static_cast<int>(x) + static_cast<int>(static_cast<int64_t>(
                                   static_cast<uint64_t>(x) >> 32));
}
} // namespace org::apache::lucene::util::mutable_