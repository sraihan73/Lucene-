using namespace std;

#include "BulkOperationPacked2.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked2::BulkOperationPacked2() : BulkOperationPacked(2) {}

void BulkOperationPacked2::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset, std::deque<int> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 62; shift >= 0; shift -= 2) {
      values[valuesOffset++] = static_cast<int>(
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          3);
    }
  }
}

void BulkOperationPacked2::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int> &values, int valuesOffset,
                                  int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    constexpr char block = blocks[blocksOffset++];
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 6)) & 3;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 4)) & 3;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 2)) & 3;
    values[valuesOffset++] = block & 3;
  }
}

void BulkOperationPacked2::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 62; shift >= 0; shift -= 2) {
      values[valuesOffset++] =
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          3;
    }
  }
}

void BulkOperationPacked2::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    constexpr char block = blocks[blocksOffset++];
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 6)) & 3;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 4)) & 3;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 2)) & 3;
    values[valuesOffset++] = block & 3;
  }
}
} // namespace org::apache::lucene::util::packed