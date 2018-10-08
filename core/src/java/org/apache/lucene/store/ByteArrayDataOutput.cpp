using namespace std;

#include "ByteArrayDataOutput.h"

namespace org::apache::lucene::store
{
using BytesRef = org::apache::lucene::util::BytesRef;

ByteArrayDataOutput::ByteArrayDataOutput(std::deque<char> &bytes)
{
  reset(bytes);
}

ByteArrayDataOutput::ByteArrayDataOutput(std::deque<char> &bytes, int offset,
                                         int len)
{
  reset(bytes, offset, len);
}

ByteArrayDataOutput::ByteArrayDataOutput() { reset(BytesRef::EMPTY_BYTES); }

void ByteArrayDataOutput::reset(std::deque<char> &bytes)
{
  reset(bytes, 0, bytes.size());
}

void ByteArrayDataOutput::reset(std::deque<char> &bytes, int offset, int len)
{
  this->bytes = bytes;
  pos = offset;
  limit = offset + len;
}

int ByteArrayDataOutput::getPosition() { return pos; }

void ByteArrayDataOutput::writeByte(char b)
{
  assert(pos < limit);
  bytes[pos++] = b;
}

void ByteArrayDataOutput::writeBytes(std::deque<char> &b, int offset,
                                     int length)
{
  assert(pos + length <= limit);
  System::arraycopy(b, offset, bytes, pos, length);
  pos += length;
}
} // namespace org::apache::lucene::store