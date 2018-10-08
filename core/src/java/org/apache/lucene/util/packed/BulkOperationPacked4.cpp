using namespace std;

#include "BulkOperationPacked4.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked4::BulkOperationPacked4() : BulkOperationPacked(4) {}

void BulkOperationPacked4::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset, std::deque<int> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 60; shift >= 0; shift -= 4) {
      values[valuesOffset++] = static_cast<int>(
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          15);
    }
  }
}

void BulkOperationPacked4::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int> &values, int valuesOffset,
                                  int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    constexpr char block = blocks[blocksOffset++];
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 4)) & 15;
    values[valuesOffset++] = block & 15;
  }
}

void BulkOperationPacked4::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 60; shift >= 0; shift -= 4) {
      values[valuesOffset++] =
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          15;
    }
  }
}

void BulkOperationPacked4::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    constexpr char block = blocks[blocksOffset++];
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 4)) & 15;
    values[valuesOffset++] = block & 15;
  }
}
} // namespace org::apache::lucene::util::packed