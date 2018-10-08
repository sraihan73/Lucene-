using namespace std;

#include "ByteArrayIndexInput.h"

namespace org::apache::lucene::store
{

ByteArrayIndexInput::ByteArrayIndexInput(const wstring &description,
                                         std::deque<char> &bytes)
    : IndexInput(description)
{
  this->bytes = bytes;
  this->limit = bytes.size();
}

int64_t ByteArrayIndexInput::getFilePointer() { return pos; }

void ByteArrayIndexInput::seek(int64_t pos)
{
  this->pos = static_cast<int>(pos);
}

void ByteArrayIndexInput::reset(std::deque<char> &bytes, int offset, int len)
{
  this->bytes = bytes;
  pos = offset;
  limit = offset + len;
}

int64_t ByteArrayIndexInput::length() { return limit; }

bool ByteArrayIndexInput::eof() { return pos == limit; }

void ByteArrayIndexInput::skipBytes(int64_t count) { pos += count; }

short ByteArrayIndexInput::readShort()
{
  return static_cast<short>(((bytes[pos++] & 0xFF) << 8) |
                            (bytes[pos++] & 0xFF));
}

int ByteArrayIndexInput::readInt()
{
  return ((bytes[pos++] & 0xFF) << 24) | ((bytes[pos++] & 0xFF) << 16) |
         ((bytes[pos++] & 0xFF) << 8) | (bytes[pos++] & 0xFF);
}

int64_t ByteArrayIndexInput::readLong()
{
  constexpr int i1 = ((bytes[pos++] & 0xff) << 24) |
                     ((bytes[pos++] & 0xff) << 16) |
                     ((bytes[pos++] & 0xff) << 8) | (bytes[pos++] & 0xff);
  constexpr int i2 = ((bytes[pos++] & 0xff) << 24) |
                     ((bytes[pos++] & 0xff) << 16) |
                     ((bytes[pos++] & 0xff) << 8) | (bytes[pos++] & 0xff);
  return ((static_cast<int64_t>(i1)) << 32) | (i2 & 0xFFFFFFFFLL);
}

int ByteArrayIndexInput::readVInt()
{
  char b = bytes[pos++];
  if (b >= 0) {
    return b;
  }
  int i = b & 0x7F;
  b = bytes[pos++];
  i |= (b & 0x7F) << 7;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7F) << 14;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7F) << 21;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  // Warning: the next ands use 0x0F / 0xF0 - beware copy/paste errors:
  i |= (b & 0x0F) << 28;
  if ((b & 0xF0) == 0) {
    return i;
  }
  throw runtime_error(L"Invalid vInt detected (too many bits)");
}

int64_t ByteArrayIndexInput::readVLong()
{
  char b = bytes[pos++];
  if (b >= 0) {
    return b;
  }
  int64_t i = b & 0x7FLL;
  b = bytes[pos++];
  i |= (b & 0x7FLL) << 7;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7FLL) << 14;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7FLL) << 21;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7FLL) << 28;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7FLL) << 35;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7FLL) << 42;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7FLL) << 49;
  if (b >= 0) {
    return i;
  }
  b = bytes[pos++];
  i |= (b & 0x7FLL) << 56;
  if (b >= 0) {
    return i;
  }
  throw runtime_error(L"Invalid vLong detected (negative values disallowed)");
}

char ByteArrayIndexInput::readByte() { return bytes[pos++]; }

void ByteArrayIndexInput::readBytes(std::deque<char> &b, int offset, int len)
{
  System::arraycopy(bytes, pos, b, offset, len);
  pos += len;
}

ByteArrayIndexInput::~ByteArrayIndexInput() {}

shared_ptr<IndexInput>
ByteArrayIndexInput::slice(const wstring &sliceDescription, int64_t offset,
                           int64_t length) 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::store