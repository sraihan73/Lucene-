using namespace std;

#include "BulkOperationPacked1.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked1::BulkOperationPacked1() : BulkOperationPacked(1) {}

void BulkOperationPacked1::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset, std::deque<int> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 63; shift >= 0; shift -= 1) {
      values[valuesOffset++] = static_cast<int>(
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          1);
    }
  }
}

void BulkOperationPacked1::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int> &values, int valuesOffset,
                                  int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    constexpr char block = blocks[blocksOffset++];
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 7)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 6)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 5)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 4)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 3)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 2)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 1)) & 1;
    values[valuesOffset++] = block & 1;
  }
}

void BulkOperationPacked1::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    for (int shift = 63; shift >= 0; shift -= 1) {
      values[valuesOffset++] =
          (static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                  shift)) &
          1;
    }
  }
}

void BulkOperationPacked1::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int j = 0; j < iterations; ++j) {
    constexpr char block = blocks[blocksOffset++];
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 7)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 6)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 5)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 4)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 3)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 2)) & 1;
    values[valuesOffset++] =
        (static_cast<int>(static_cast<unsigned int>(block) >> 1)) & 1;
    values[valuesOffset++] = block & 1;
  }
}
} // namespace org::apache::lucene::util::packed