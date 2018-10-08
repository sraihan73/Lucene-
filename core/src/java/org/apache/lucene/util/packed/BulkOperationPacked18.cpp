using namespace std;

#include "BulkOperationPacked18.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked18::BulkOperationPacked18() : BulkOperationPacked(18) {}

void BulkOperationPacked18::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 46));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 28)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 10)) &
                         262143LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 1023LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 38)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 20)) &
                         262143LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 2)) &
        262143LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 3LL) << 16) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 48)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 30)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 12)) &
                         262143LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 4095LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 40)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 22)) &
                         262143LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 4)) &
        262143LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 15LL) << 14) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 50)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 32)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 14)) &
                         262143LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 16383LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 42)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 24)) &
                         262143LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 6)) &
        262143LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 63LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 34)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 16)) &
                         262143LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 65535LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 44)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 26)) &
                         262143LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 8)) &
        262143LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 255LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 36)) &
                         262143LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 18)) &
                         262143LL);
    values[valuesOffset++] = static_cast<int>(block8 & 262143LL);
  }
}

void BulkOperationPacked18::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 10) | (byte1 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 6));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 63) << 12) | (byte3 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte4) >> 4));
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 15) << 14) | (byte5 << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte6) >> 2));
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte6 & 3) << 16) | (byte7 << 8) | byte8;
  }
}

void BulkOperationPacked18::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 46);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                28)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                10)) &
        262143LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 1023LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                38)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                20)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 2)) &
        262143LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 3LL) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 48));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                30)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                12)) &
        262143LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 4095LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                40)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                22)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 4)) &
        262143LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 15LL) << 14) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 50));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                32)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                14)) &
        262143LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 16383LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                42)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                24)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 6)) &
        262143LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 63LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                34)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                16)) &
        262143LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 65535LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                44)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                26)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 8)) &
        262143LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 255LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                36)) &
        262143LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                18)) &
        262143LL;
    values[valuesOffset++] = block8 & 262143LL;
  }
}

void BulkOperationPacked18::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 10) | (byte1 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 6));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 63) << 12) | (byte3 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte4) >> 4));
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 15) << 14) | (byte5 << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte6) >> 2));
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte6 & 3) << 16) | (byte7 << 8) | byte8;
  }
}
} // namespace org::apache::lucene::util::packed