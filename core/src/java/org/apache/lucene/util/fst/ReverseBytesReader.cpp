using namespace std;

#include "ReverseBytesReader.h"

namespace org::apache::lucene::util::fst
{

ReverseBytesReader::ReverseBytesReader(std::deque<char> &bytes) : bytes(bytes)
{
}

char ReverseBytesReader::readByte() { return bytes[pos--]; }

void ReverseBytesReader::readBytes(std::deque<char> &b, int offset, int len)
{
  for (int i = 0; i < len; i++) {
    b[offset + i] = bytes[pos--];
  }
}

void ReverseBytesReader::skipBytes(int64_t count) { pos -= count; }

int64_t ReverseBytesReader::getPosition() { return pos; }

void ReverseBytesReader::setPosition(int64_t pos)
{
  this->pos = static_cast<int>(pos);
}

bool ReverseBytesReader::reversed() { return true; }
} // namespace org::apache::lucene::util::fst