using namespace std;

#include "BytesRefBuilder.h"

namespace org::apache::lucene::util
{

BytesRefBuilder::BytesRefBuilder() : ref(make_shared<BytesRef>()) {}

std::deque<char> BytesRefBuilder::bytes() { return ref->bytes; }

int BytesRefBuilder::length() { return ref->length; }

void BytesRefBuilder::setLength(int length) { this->ref->length = length; }

char BytesRefBuilder::byteAt(int offset) { return ref->bytes[offset]; }

void BytesRefBuilder::setByteAt(int offset, char b) { ref->bytes[offset] = b; }

void BytesRefBuilder::grow(int capacity)
{
  ref->bytes = ArrayUtil::grow(ref->bytes, capacity);
}

void BytesRefBuilder::append(char b)
{
  grow(ref->length + 1);
  ref->bytes[ref->length++] = b;
}

void BytesRefBuilder::append(std::deque<char> &b, int off, int len)
{
  grow(ref->length + len);
  System::arraycopy(b, off, ref->bytes, ref->length, len);
  ref->length += len;
}

void BytesRefBuilder::append(shared_ptr<BytesRef> ref)
{
  append(ref->bytes, ref->offset, ref->length);
}

void BytesRefBuilder::append(shared_ptr<BytesRefBuilder> builder)
{
  append(builder->get());
}

void BytesRefBuilder::clear() { setLength(0); }

void BytesRefBuilder::copyBytes(std::deque<char> &b, int off, int len)
{
  clear();
  append(b, off, len);
}

void BytesRefBuilder::copyBytes(shared_ptr<BytesRef> ref)
{
  clear();
  append(ref);
}

void BytesRefBuilder::copyBytes(shared_ptr<BytesRefBuilder> builder)
{
  clear();
  append(builder);
}

void BytesRefBuilder::copyChars(shared_ptr<std::wstring> text)
{
  copyChars(text, 0, text->length());
}

void BytesRefBuilder::copyChars(shared_ptr<std::wstring> text, int off, int len)
{
  grow(UnicodeUtil::maxUTF8Length(len));
  ref->length = UnicodeUtil::UTF16toUTF8(text, off, len, ref->bytes);
}

void BytesRefBuilder::copyChars(std::deque<wchar_t> &text, int off, int len)
{
  grow(UnicodeUtil::maxUTF8Length(len));
  ref->length = UnicodeUtil::UTF16toUTF8(text, off, len, ref->bytes);
}

shared_ptr<BytesRef> BytesRefBuilder::get()
{
  assert((ref->offset == 0,
          L"Modifying the offset of the returned ref is illegal"));
  return ref;
}

shared_ptr<BytesRef> BytesRefBuilder::toBytesRef()
{
  return make_shared<BytesRef>(Arrays::copyOf(ref->bytes, ref->length));
}

bool BytesRefBuilder::equals(any obj)
{
  throw make_shared<UnsupportedOperationException>();
}

int BytesRefBuilder::hashCode()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::util