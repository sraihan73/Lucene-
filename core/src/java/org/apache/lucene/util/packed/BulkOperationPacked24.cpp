using namespace std;

#include "BulkOperationPacked24.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked24::BulkOperationPacked24() : BulkOperationPacked(24) {}

void BulkOperationPacked24::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 40));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 16)) &
                         16777215LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 65535LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 32)) &
                         16777215LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 8)) &
        16777215LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 255LL) << 16) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 48)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 24)) &
                         16777215LL);
    values[valuesOffset++] = static_cast<int>(block2 & 16777215LL);
  }
}

void BulkOperationPacked24::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = (byte0 << 16) | (byte1 << 8) | byte2;
  }
}

void BulkOperationPacked24::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 40);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                16)) &
        16777215LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 65535LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                32)) &
        16777215LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 8)) &
        16777215LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 255LL) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 48));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                24)) &
        16777215LL;
    values[valuesOffset++] = block2 & 16777215LL;
  }
}

void BulkOperationPacked24::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = (byte0 << 16) | (byte1 << 8) | byte2;
  }
}
} // namespace org::apache::lucene::util::packed