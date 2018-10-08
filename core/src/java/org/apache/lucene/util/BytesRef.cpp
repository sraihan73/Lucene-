using namespace std;

#include "BytesRef.h"

namespace org::apache::lucene::util
{

std::deque<char> const BytesRef::EMPTY_BYTES = std::deque<char>(0);

BytesRef::BytesRef() : BytesRef(EMPTY_BYTES) {}

BytesRef::BytesRef(std::deque<char> &bytes, int offset, int length)
{
  this->bytes = bytes;
  this->offset = offset;
  this->length = length;
  assert(isValid());
}

BytesRef::BytesRef(std::deque<char> &bytes) : BytesRef(bytes, 0, bytes.length)
{
}

BytesRef::BytesRef(int capacity) { this->bytes = std::deque<char>(capacity); }

BytesRef::BytesRef(shared_ptr<std::wstring> text)
    : BytesRef(new byte[UnicodeUtil::maxUTF8Length(text->length())])
{
  length = UnicodeUtil::UTF16toUTF8(text, 0, text->length(), bytes);
}

bool BytesRef::bytesEquals(shared_ptr<BytesRef> other)
{
  return FutureArrays::equals(this->bytes, this->offset,
                              this->offset + this->length, other->bytes,
                              other->offset, other->offset + other->length);
}

shared_ptr<BytesRef> BytesRef::clone()
{
  return make_shared<BytesRef>(bytes, offset, length);
}

int BytesRef::hashCode()
{
  return StringHelper::murmurhash3_x86_32(shared_from_this(),
                                          StringHelper::GOOD_FAST_HASH_SEED);
}

bool BytesRef::equals(any other)
{
  if (other == nullptr) {
    return false;
  }
  if (std::dynamic_pointer_cast<BytesRef>(other) != nullptr) {
    return this->bytesEquals(any_cast<std::shared_ptr<BytesRef>>(other));
  }
  return false;
}

wstring BytesRef::utf8ToString()
{
  const std::deque<wchar_t> ref = std::deque<wchar_t>(length);
  constexpr int len = UnicodeUtil::UTF8toUTF16(bytes, offset, length, ref);
  return wstring(ref, 0, len);
}

wstring BytesRef::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L'[');
  constexpr int end = offset + length;
  for (int i = offset; i < end; i++) {
    if (i > offset) {
      sb->append(L' ');
    }
    sb->append(Integer::toHexString(bytes[i] & 0xff));
  }
  sb->append(L']');
  return sb->toString();
}

int BytesRef::compareTo(shared_ptr<BytesRef> other)
{
  return FutureArrays::compareUnsigned(
      this->bytes, this->offset, this->offset + this->length, other->bytes,
      other->offset, other->offset + other->length);
}

shared_ptr<BytesRef> BytesRef::deepCopyOf(shared_ptr<BytesRef> other)
{
  shared_ptr<BytesRef> copy = make_shared<BytesRef>();
  copy->bytes = Arrays::copyOfRange(other->bytes, other->offset,
                                    other->offset + other->length);
  copy->offset = 0;
  copy->length = other->length;
  return copy;
}

bool BytesRef::isValid()
{
  if (bytes.empty()) {
    throw make_shared<IllegalStateException>(L"bytes is null");
  }
  if (length < 0) {
    throw make_shared<IllegalStateException>(L"length is negative: " +
                                             to_wstring(length));
  }
  if (length > bytes.size()) {
    throw make_shared<IllegalStateException>(L"length is out of bounds: " +
                                             to_wstring(length) +
                                             L",bytes.length=" + bytes.size());
  }
  if (offset < 0) {
    throw make_shared<IllegalStateException>(L"offset is negative: " +
                                             to_wstring(offset));
  }
  if (offset > bytes.size()) {
    throw make_shared<IllegalStateException>(L"offset out of bounds: " +
                                             to_wstring(offset) +
                                             L",bytes.length=" + bytes.size());
  }
  if (offset + length < 0) {
    throw make_shared<IllegalStateException>(
        L"offset+length is negative: offset=" + to_wstring(offset) +
        L",length=" + to_wstring(length));
  }
  if (offset + length > bytes.size()) {
    throw make_shared<IllegalStateException>(
        L"offset+length out of bounds: offset=" + to_wstring(offset) +
        L",length=" + to_wstring(length) + L",bytes.length=" + bytes.size());
  }
  return true;
}
} // namespace org::apache::lucene::util