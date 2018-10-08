using namespace std;

#include "CharsRefBuilder.h"

namespace org::apache::lucene::util
{

const wstring CharsRefBuilder::NULL_STRING = L"null";

CharsRefBuilder::CharsRefBuilder() : ref(make_shared<CharsRef>()) {}

std::deque<wchar_t> CharsRefBuilder::chars() { return ref->chars; }

int CharsRefBuilder::length() { return ref->length_; }

void CharsRefBuilder::setLength(int length) { this->ref->length_ = length; }

wchar_t CharsRefBuilder::charAt(int offset) { return ref->chars[offset]; }

void CharsRefBuilder::setCharAt(int offset, wchar_t b)
{
  ref->chars[offset] = b;
}

void CharsRefBuilder::clear() { ref->length_ = 0; }

shared_ptr<CharsRefBuilder>
CharsRefBuilder::append(shared_ptr<std::wstring> csq)
{
  if (csq == nullptr) {
    return append(NULL_STRING);
  }
  return append(csq, 0, csq->length());
}

shared_ptr<CharsRefBuilder>
CharsRefBuilder::append(shared_ptr<std::wstring> csq, int start, int end)
{
  if (csq == nullptr) {
    return append(NULL_STRING);
  }
  grow(ref->length_ + end - start);
  for (int i = start; i < end; ++i) {
    setCharAt(ref->length_++, csq->charAt(i));
  }
  return shared_from_this();
}

shared_ptr<CharsRefBuilder> CharsRefBuilder::append(wchar_t c)
{
  grow(ref->length_ + 1);
  setCharAt(ref->length_++, c);
  return shared_from_this();
}

void CharsRefBuilder::copyChars(shared_ptr<CharsRef> other)
{
  copyChars(other->chars, other->offset, other->length_);
}

void CharsRefBuilder::grow(int newLength)
{
  ref->chars = ArrayUtil::grow(ref->chars, newLength);
}

void CharsRefBuilder::copyUTF8Bytes(std::deque<char> &bytes, int offset,
                                    int length)
{
  grow(length);
  ref->length_ = UnicodeUtil::UTF8toUTF16(bytes, offset, length, ref->chars);
}

void CharsRefBuilder::copyUTF8Bytes(shared_ptr<BytesRef> bytes)
{
  copyUTF8Bytes(bytes->bytes, bytes->offset, bytes->length);
}

void CharsRefBuilder::copyChars(std::deque<wchar_t> &otherChars,
                                int otherOffset, int otherLength)
{
  grow(otherLength);
  System::arraycopy(otherChars, otherOffset, ref->chars, 0, otherLength);
  ref->length_ = otherLength;
}

void CharsRefBuilder::append(std::deque<wchar_t> &otherChars, int otherOffset,
                             int otherLength)
{
  int newLen = ref->length_ + otherLength;
  grow(newLen);
  System::arraycopy(otherChars, otherOffset, ref->chars, ref->length_,
                    otherLength);
  ref->length_ = newLen;
}

shared_ptr<CharsRef> CharsRefBuilder::get()
{
  assert((ref->offset == 0,
          L"Modifying the offset of the returned ref is illegal"));
  return ref;
}

shared_ptr<CharsRef> CharsRefBuilder::toCharsRef()
{
  return make_shared<CharsRef>(Arrays::copyOf(ref->chars, ref->length_), 0,
                               ref->length_);
}

wstring CharsRefBuilder::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return get()->toString();
}

bool CharsRefBuilder::equals(any obj)
{
  throw make_shared<UnsupportedOperationException>();
}

int CharsRefBuilder::hashCode()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::util