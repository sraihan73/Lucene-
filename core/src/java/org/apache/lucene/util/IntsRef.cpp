using namespace std;

#include "IntsRef.h"

namespace org::apache::lucene::util
{

std::deque<int> const IntsRef::EMPTY_INTS = std::deque<int>(0);

IntsRef::IntsRef() { ints = EMPTY_INTS; }

IntsRef::IntsRef(int capacity) { ints = std::deque<int>(capacity); }

IntsRef::IntsRef(std::deque<int> &ints, int offset, int length)
{
  this->ints = ints;
  this->offset = offset;
  this->length = length;
  assert(isValid());
}

shared_ptr<IntsRef> IntsRef::clone()
{
  return make_shared<IntsRef>(ints, offset, length);
}

int IntsRef::hashCode()
{
  constexpr int prime = 31;
  int result = 0;
  constexpr int end = offset + length;
  for (int i = offset; i < end; i++) {
    result = prime * result + ints[i];
  }
  return result;
}

bool IntsRef::equals(any other)
{
  if (other == nullptr) {
    return false;
  }
  if (std::dynamic_pointer_cast<IntsRef>(other) != nullptr) {
    return this->intsEquals(any_cast<std::shared_ptr<IntsRef>>(other));
  }
  return false;
}

bool IntsRef::intsEquals(shared_ptr<IntsRef> other)
{
  return FutureArrays::equals(this->ints, this->offset,
                              this->offset + this->length, other->ints,
                              other->offset, other->offset + other->length);
}

int IntsRef::compareTo(shared_ptr<IntsRef> other)
{
  return FutureArrays::compare(this->ints, this->offset,
                               this->offset + this->length, other->ints,
                               other->offset, other->offset + other->length);
}

wstring IntsRef::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L'[');
  constexpr int end = offset + length;
  for (int i = offset; i < end; i++) {
    if (i > offset) {
      sb->append(L' ');
    }
    sb->append(Integer::toHexString(ints[i]));
  }
  sb->append(L']');
  return sb->toString();
}

shared_ptr<IntsRef> IntsRef::deepCopyOf(shared_ptr<IntsRef> other)
{
  return make_shared<IntsRef>(
      Arrays::copyOfRange(other->ints, other->offset,
                          other->offset + other->length),
      0, other->length);
}

bool IntsRef::isValid()
{
  if (ints.empty()) {
    throw make_shared<IllegalStateException>(L"ints is null");
  }
  if (length < 0) {
    throw make_shared<IllegalStateException>(L"length is negative: " +
                                             to_wstring(length));
  }
  if (length > ints.size()) {
    throw make_shared<IllegalStateException>(L"length is out of bounds: " +
                                             to_wstring(length) +
                                             L",ints.length=" + ints.size());
  }
  if (offset < 0) {
    throw make_shared<IllegalStateException>(L"offset is negative: " +
                                             to_wstring(offset));
  }
  if (offset > ints.size()) {
    throw make_shared<IllegalStateException>(L"offset out of bounds: " +
                                             to_wstring(offset) +
                                             L",ints.length=" + ints.size());
  }
  if (offset + length < 0) {
    throw make_shared<IllegalStateException>(
        L"offset+length is negative: offset=" + to_wstring(offset) +
        L",length=" + to_wstring(length));
  }
  if (offset + length > ints.size()) {
    throw make_shared<IllegalStateException>(
        L"offset+length out of bounds: offset=" + to_wstring(offset) +
        L",length=" + to_wstring(length) + L",ints.length=" + ints.size());
  }
  return true;
}
} // namespace org::apache::lucene::util