using namespace std;

#include "PayloadHelper.h"

namespace org::apache::lucene::analysis::payloads
{

std::deque<char> PayloadHelper::encodeFloat(float payload)
{
  return encodeFloat(payload, std::deque<char>(4), 0);
}

std::deque<char>
PayloadHelper::encodeFloat(float payload, std::deque<char> &data, int offset)
{
  return encodeInt(Float::floatToIntBits(payload), data, offset);
}

std::deque<char> PayloadHelper::encodeInt(int payload)
{
  return encodeInt(payload, std::deque<char>(4), 0);
}

std::deque<char> PayloadHelper::encodeInt(int payload, std::deque<char> &data,
                                           int offset)
{
  data[offset] = static_cast<char>(payload >> 24);
  data[offset + 1] = static_cast<char>(payload >> 16);
  data[offset + 2] = static_cast<char>(payload >> 8);
  data[offset + 3] = static_cast<char>(payload);
  return data;
}

float PayloadHelper::decodeFloat(std::deque<char> &bytes)
{
  return decodeFloat(bytes, 0);
}

float PayloadHelper::decodeFloat(std::deque<char> &bytes, int offset)
{

  return Float::intBitsToFloat(decodeInt(bytes, offset));
}

int PayloadHelper::decodeInt(std::deque<char> &bytes, int offset)
{
  return ((bytes[offset] & 0xFF) << 24) | ((bytes[offset + 1] & 0xFF) << 16) |
         ((bytes[offset + 2] & 0xFF) << 8) | (bytes[offset + 3] & 0xFF);
}
} // namespace org::apache::lucene::analysis::payloads