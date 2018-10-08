using namespace std;

#include "BulkOperationPacked8.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked8::BulkOperationPacked8() : BulkOperationPacked(8) {}

void BulkOperationPacked8::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset, std::deque<int> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 56; shift >= 0; shift -= 8) {
      values[valuesOffset++] = static_cast<int>(
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          255);
    }
  }
}

void BulkOperationPacked8::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int> &values, int valuesOffset,
                                  int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    values[valuesOffset++] = blocks[blocksOffset++] & 0xFF;
  }
}

void BulkOperationPacked8::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 56; shift >= 0; shift -= 8) {
      values[valuesOffset++] =
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          255;
    }
  }
}

void BulkOperationPacked8::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    values[valuesOffset++] = blocks[blocksOffset++] & 0xFF;
  }
}
} // namespace org::apache::lucene::util::packed