using namespace std;

#include "ForwardBytesReader.h"

namespace org::apache::lucene::util::fst
{

ForwardBytesReader::ForwardBytesReader(std::deque<char> &bytes) : bytes(bytes)
{
}

char ForwardBytesReader::readByte() { return bytes[pos++]; }

void ForwardBytesReader::readBytes(std::deque<char> &b, int offset, int len)
{
  System::arraycopy(bytes, pos, b, offset, len);
  pos += len;
}

void ForwardBytesReader::skipBytes(int64_t count) { pos += count; }

int64_t ForwardBytesReader::getPosition() { return pos; }

void ForwardBytesReader::setPosition(int64_t pos)
{
  this->pos = static_cast<int>(pos);
}

bool ForwardBytesReader::reversed() { return false; }
} // namespace org::apache::lucene::util::fst