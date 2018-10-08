using namespace std;

#include "BitUtil.h"

namespace org::apache::lucene::util
{

std::deque<int64_t> const BitUtil::MAGIC = {
    0x5555555555555555LL, 0x3333333333333333LL, 0x0F0F0F0F0F0F0F0FLL,
    0x00FF00FF00FF00FFLL, 0x0000FFFF0000FFFFLL, 0x00000000FFFFFFFFLL,
    0xAAAAAAAAAAAAAAAALL};
std::deque<short> const BitUtil::SHIFT = {1, 2, 4, 8, 16};

BitUtil::BitUtil() {} // no instance

int64_t BitUtil::pop_array(std::deque<int64_t> &arr, int wordOffset,
                             int numWords)
{
  int64_t popCount = 0;
  for (int i = wordOffset, end = wordOffset + numWords; i < end; ++i) {
    popCount += Long::bitCount(arr[i]);
  }
  return popCount;
}

int64_t BitUtil::pop_intersect(std::deque<int64_t> &arr1,
                                 std::deque<int64_t> &arr2, int wordOffset,
                                 int numWords)
{
  int64_t popCount = 0;
  for (int i = wordOffset, end = wordOffset + numWords; i < end; ++i) {
    popCount += Long::bitCount(arr1[i] & arr2[i]);
  }
  return popCount;
}

int64_t BitUtil::pop_union(std::deque<int64_t> &arr1,
                             std::deque<int64_t> &arr2, int wordOffset,
                             int numWords)
{
  int64_t popCount = 0;
  for (int i = wordOffset, end = wordOffset + numWords; i < end; ++i) {
    popCount += Long::bitCount(arr1[i] | arr2[i]);
  }
  return popCount;
}

int64_t BitUtil::pop_andnot(std::deque<int64_t> &arr1,
                              std::deque<int64_t> &arr2, int wordOffset,
                              int numWords)
{
  int64_t popCount = 0;
  for (int i = wordOffset, end = wordOffset + numWords; i < end; ++i) {
    popCount += Long::bitCount(arr1[i] & ~arr2[i]);
  }
  return popCount;
}

int64_t BitUtil::pop_xor(std::deque<int64_t> &arr1,
                           std::deque<int64_t> &arr2, int wordOffset,
                           int numWords)
{
  int64_t popCount = 0;
  for (int i = wordOffset, end = wordOffset + numWords; i < end; ++i) {
    popCount += Long::bitCount(arr1[i] ^ arr2[i]);
  }
  return popCount;
}

int BitUtil::nextHighestPowerOfTwo(int v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

int64_t BitUtil::nextHighestPowerOfTwo(int64_t v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v |= v >> 32;
  v++;
  return v;
}

int64_t BitUtil::interleave(int even, int odd)
{
  int64_t v1 = 0x00000000FFFFFFFFLL & even;
  int64_t v2 = 0x00000000FFFFFFFFLL & odd;
  v1 = (v1 | (v1 << SHIFT[4])) & MAGIC[4];
  v1 = (v1 | (v1 << SHIFT[3])) & MAGIC[3];
  v1 = (v1 | (v1 << SHIFT[2])) & MAGIC[2];
  v1 = (v1 | (v1 << SHIFT[1])) & MAGIC[1];
  v1 = (v1 | (v1 << SHIFT[0])) & MAGIC[0];
  v2 = (v2 | (v2 << SHIFT[4])) & MAGIC[4];
  v2 = (v2 | (v2 << SHIFT[3])) & MAGIC[3];
  v2 = (v2 | (v2 << SHIFT[2])) & MAGIC[2];
  v2 = (v2 | (v2 << SHIFT[1])) & MAGIC[1];
  v2 = (v2 | (v2 << SHIFT[0])) & MAGIC[0];

  return (v2 << 1) | v1;
}

int64_t BitUtil::deinterleave(int64_t b)
{
  b &= MAGIC[0];
  b = (b ^ (static_cast<int64_t>(static_cast<uint64_t>(b) >>
                                   SHIFT[0]))) &
      MAGIC[1];
  b = (b ^ (static_cast<int64_t>(static_cast<uint64_t>(b) >>
                                   SHIFT[1]))) &
      MAGIC[2];
  b = (b ^ (static_cast<int64_t>(static_cast<uint64_t>(b) >>
                                   SHIFT[2]))) &
      MAGIC[3];
  b = (b ^ (static_cast<int64_t>(static_cast<uint64_t>(b) >>
                                   SHIFT[3]))) &
      MAGIC[4];
  b = (b ^ (static_cast<int64_t>(static_cast<uint64_t>(b) >>
                                   SHIFT[4]))) &
      MAGIC[5];
  return b;
}

int64_t BitUtil::flipFlop(int64_t const b)
{
  return (static_cast<int64_t>(
             static_cast<uint64_t>((b & MAGIC[6])) >> 1)) |
         ((b & MAGIC[0]) << 1);
}

int BitUtil::zigZagEncode(int i) { return (i >> 31) ^ (i << 1); }

int64_t BitUtil::zigZagEncode(int64_t l) { return (l >> 63) ^ (l << 1); }

int BitUtil::zigZagDecode(int i)
{
  return ((static_cast<int>(static_cast<unsigned int>(i) >> 1)) ^ -(i & 1));
}

int64_t BitUtil::zigZagDecode(int64_t l)
{
  return ((static_cast<int64_t>(static_cast<uint64_t>(l) >> 1)) ^
          -(l & 1));
}
} // namespace org::apache::lucene::util