using namespace std;

#include "MutableValueBool.h"

namespace org::apache::lucene::util::mutable_
{

any MutableValueBool::toObject()
{
  assert(exists_ || (false == value));
  return exists_ ? value : nullptr;
}

void MutableValueBool::copy(shared_ptr<MutableValue> source)
{
  shared_ptr<MutableValueBool> s =
      std::static_pointer_cast<MutableValueBool>(source);
  value = s->value;
  exists_ = s->exists_;
}

shared_ptr<MutableValue> MutableValueBool::duplicate()
{
  shared_ptr<MutableValueBool> v = make_shared<MutableValueBool>();
  v->value = this->value;
  v->exists_ = this->exists_;
  return v;
}

bool MutableValueBool::equalsSameType(any other)
{
  assert(exists_ || (false == value));
  shared_ptr<MutableValueBool> b =
      any_cast<std::shared_ptr<MutableValueBool>>(other);
  return value == b->value && exists_ == b->exists_;
}

int MutableValueBool::compareSameType(any other)
{
  assert(exists_ || (false == value));
  shared_ptr<MutableValueBool> b =
      any_cast<std::shared_ptr<MutableValueBool>>(other);
  if (value != b->value) {
    return value ? 1 : -1;
  }
  if (exists_ == b->exists_) {
    return 0;
  }
  return exists_ ? 1 : -1;
}

int MutableValueBool::hashCode()
{
  assert(exists_ || (false == value));
  return value ? 2 : (exists_ ? 1 : 0);
}
} // namespace org::apache::lucene::util::mutable_