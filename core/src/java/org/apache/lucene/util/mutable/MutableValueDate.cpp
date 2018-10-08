using namespace std;

#include "MutableValueDate.h"

namespace org::apache::lucene::util::mutable_
{

any MutableValueDate::toObject() { return exists_ ? Date(value) : nullptr; }

shared_ptr<MutableValue> MutableValueDate::duplicate()
{
  shared_ptr<MutableValueDate> v = make_shared<MutableValueDate>();
  v->value = this->value;
  v->exists_ = this->exists_;
  return v;
}
} // namespace org::apache::lucene::util::mutable_