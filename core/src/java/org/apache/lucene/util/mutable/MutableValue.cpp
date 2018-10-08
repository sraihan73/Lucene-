using namespace std;

#include "MutableValue.h"

namespace org::apache::lucene::util::mutable_
{

bool MutableValue::exists() { return exists_; }

int MutableValue::compareTo(shared_ptr<MutableValue> other)
{
  type_info c1 = this->getClass();
  type_info c2 = other->getClass();
  if (c1 != c2) {
    int c = c1.hash_code() - c2.hash_code();
    if (c == 0) {
      c = c1.getCanonicalName()->compareTo(c2.getCanonicalName());
    }
    return c;
  }
  return compareSameType(other);
}

bool MutableValue::equals(any other)
{
  return (getClass() == other.type()) && this->equalsSameType(other);
}

wstring MutableValue::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return exists() ? toObject().toString() : L"(null)";
}
} // namespace org::apache::lucene::util::mutable_