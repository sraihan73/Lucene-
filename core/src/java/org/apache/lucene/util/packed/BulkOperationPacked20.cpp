using namespace std;

#include "BulkOperationPacked20.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked20::BulkOperationPacked20() : BulkOperationPacked(20) {}

void BulkOperationPacked20::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 44));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 24)) &
                         1048575LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 4)) &
        1048575LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 15LL) << 16) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 48)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 28)) &
                         1048575LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 8)) &
        1048575LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 255LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 32)) &
                         1048575LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 12)) &
                         1048575LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 4095LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 36)) &
                         1048575LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 16)) &
                         1048575LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 65535LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 40)) &
                         1048575LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 20)) &
                         1048575LL);
    values[valuesOffset++] = static_cast<int>(block4 & 1048575LL);
  }
}

void BulkOperationPacked20::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 12) | (byte1 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 4));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte2 & 15) << 16) | (byte3 << 8) | byte4;
  }
}

void BulkOperationPacked20::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 44);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                24)) &
        1048575LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 4)) &
        1048575LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 15LL) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 48));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                28)) &
        1048575LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 8)) &
        1048575LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 255LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                32)) &
        1048575LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                12)) &
        1048575LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 4095LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                36)) &
        1048575LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                16)) &
        1048575LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 65535LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                40)) &
        1048575LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                20)) &
        1048575LL;
    values[valuesOffset++] = block4 & 1048575LL;
  }
}

void BulkOperationPacked20::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 12) | (byte1 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 4));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte2 & 15) << 16) | (byte3 << 8) | byte4;
  }
}
} // namespace org::apache::lucene::util::packed