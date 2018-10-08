using namespace std;

#include "IntsRefBuilder.h"

namespace org::apache::lucene::util
{

IntsRefBuilder::IntsRefBuilder() : ref(make_shared<IntsRef>()) {}

std::deque<int> IntsRefBuilder::ints() { return ref->ints; }

int IntsRefBuilder::length() { return ref->length; }

void IntsRefBuilder::setLength(int length) { this->ref->length = length; }

void IntsRefBuilder::clear() { setLength(0); }

int IntsRefBuilder::intAt(int offset) { return ref->ints[offset]; }

void IntsRefBuilder::setIntAt(int offset, int b) { ref->ints[offset] = b; }

void IntsRefBuilder::append(int i)
{
  grow(ref->length + 1);
  ref->ints[ref->length++] = i;
}

void IntsRefBuilder::grow(int newLength)
{
  ref->ints = ArrayUtil::grow(ref->ints, newLength);
}

void IntsRefBuilder::copyInts(std::deque<int> &otherInts, int otherOffset,
                              int otherLength)
{
  grow(otherLength);
  System::arraycopy(otherInts, otherOffset, ref->ints, 0, otherLength);
  ref->length = otherLength;
}

void IntsRefBuilder::copyInts(shared_ptr<IntsRef> ints)
{
  copyInts(ints->ints, ints->offset, ints->length);
}

void IntsRefBuilder::copyUTF8Bytes(shared_ptr<BytesRef> bytes)
{
  grow(bytes->length);
  ref->length = UnicodeUtil::UTF8toUTF32(bytes, ref->ints);
}

shared_ptr<IntsRef> IntsRefBuilder::get()
{
  assert((ref->offset == 0,
          L"Modifying the offset of the returned ref is illegal"));
  return ref;
}

shared_ptr<IntsRef> IntsRefBuilder::toIntsRef()
{
  return IntsRef::deepCopyOf(get());
}

bool IntsRefBuilder::equals(any obj)
{
  throw make_shared<UnsupportedOperationException>();
}

int IntsRefBuilder::hashCode()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::util