using namespace std;

#include "BulkOperationPacked16.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked16::BulkOperationPacked16() : BulkOperationPacked(16) {}

void BulkOperationPacked16::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 48; shift >= 0; shift -= 16) {
      values[valuesOffset++] = static_cast<int>(
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          65535);
    }
  }
}

void BulkOperationPacked16::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    values[valuesOffset++] = ((blocks[blocksOffset++] & 0xFF) << 8) |
                             (blocks[blocksOffset++] & 0xFF);
  }
}

void BulkOperationPacked16::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 48; shift >= 0; shift -= 16) {
      values[valuesOffset++] =
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          65535;
    }
  }
}

void BulkOperationPacked16::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    values[valuesOffset++] = ((blocks[blocksOffset++] & 0xFFLL) << 8) |
                             (blocks[blocksOffset++] & 0xFFLL);
  }
}
} // namespace org::apache::lucene::util::packed