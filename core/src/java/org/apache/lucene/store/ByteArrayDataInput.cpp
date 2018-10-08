using namespace std;

#include "ByteArrayDataInput.h"

namespace org::apache::lucene::store
{
using BytesRef = org::apache::lucene::util::BytesRef;

ByteArrayDataInput::ByteArrayDataInput(std::deque<char> &bytes)
{
  reset(bytes);
}

ByteArrayDataInput::ByteArrayDataInput(std::deque<char> &bytes, int offset,
                                       int len)
{
  reset(bytes, offset, len);
}

ByteArrayDataInput::ByteArrayDataInput() { reset(BytesRef::EMPTY_BYTES); }

void ByteArrayDataInput::reset(std::deque<char> &bytes)
{
  reset(bytes, 0, bytes.size());
}

void ByteArrayDataInput::rewind() { pos = 0; }

int ByteArrayDataInput::getPosition() { return pos; }

void ByteArrayDataInput::setPosition(int pos) { this->pos = pos; }

void ByteArrayDataInput::reset(std::deque<char> &bytes, int offset, int len)
{
  this->bytes = bytes;
  pos = offset;
  limit = offset + len;
}

int ByteArrayDataInput::length() { return limit; }

bool ByteArrayDataInput::eof() { return pos == limit; }

void ByteArrayDataInput::skipBytes(int64_t count) { pos += count; }

short ByteArrayDataInput::readShort()
{
  return static_cast<short>(((bytes[pos++] & 0xFF) << 8) |
                            (bytes[pos++] & 0xFF));
}

int ByteArrayDataInput::readInt()
{
  return ((bytes[pos++] & 0xFF) << 24) | ((bytes[pos++] & 0xFF) << 16) |
         ((bytes[pos++] & 0xFF) << 8) | (bytes[pos++] & 0xFF);
}

int64_t ByteArrayDataInput::readLong()
{
  constexpr int i1 = ((bytes[pos++] & 0xff) << 24) |
                     ((bytes[pos++] & 0xff) << 16) |
                     ((bytes[pos++] & 0xff) << 8) | (bytes[pos++] & 0xff);
  constexpr int i2 = ((bytes[pos++] & 0xff) << 24) |
                     ((bytes[pos++] & 0xff) << 16) |
                     ((bytes[pos++] & 0xff) << 8) | (bytes[pos++] & 0xff);
  return ((static_cast<int64_t>(i1)) << 32) | (i2 & 0xFFFFFFFFLL);
}

int ByteArrayDataInput::readVInt()
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

int64_t ByteArrayDataInput::readVLong()
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

char ByteArrayDataInput::readByte() { return bytes[pos++]; }

void ByteArrayDataInput::readBytes(std::deque<char> &b, int offset, int len)
{
  System::arraycopy(bytes, pos, b, offset, len);
  pos += len;
}
} // namespace org::apache::lucene::store