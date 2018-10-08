using namespace std;

#include "BulkOperationPacked22.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked22::BulkOperationPacked22() : BulkOperationPacked(22) {}

void BulkOperationPacked22::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 42));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 20)) &
                         4194303LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 1048575LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 40)) &
                         4194303LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 18)) &
                         4194303LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 262143LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 38)) &
                         4194303LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 16)) &
                         4194303LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 65535LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 36)) &
                         4194303LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 14)) &
                         4194303LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 16383LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 34)) &
                         4194303LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 12)) &
                         4194303LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 4095LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 32)) &
                         4194303LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 10)) &
                         4194303LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 1023LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 30)) &
                         4194303LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 8)) &
        4194303LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 255LL) << 14) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 50)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 28)) &
                         4194303LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 6)) &
        4194303LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 63LL) << 16) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 48)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 26)) &
                         4194303LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 4)) &
        4194303LL);
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block8 & 15LL) << 18) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 46)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 24)) &
                         4194303LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 2)) &
        4194303LL);
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block9 & 3LL) << 20) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 44)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 22)) &
                         4194303LL);
    values[valuesOffset++] = static_cast<int>(block10 & 4194303LL);
  }
}

void BulkOperationPacked22::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 14) | (byte1 << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 2));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 3) << 20) | (byte3 << 12) | (byte4 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte5) >> 4));
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 15) << 18) | (byte6 << 10) | (byte7 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte8) >> 6));
    constexpr int byte9 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte10 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte8 & 63) << 16) | (byte9 << 8) | byte10;
  }
}

void BulkOperationPacked22::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 42);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                20)) &
        4194303LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 1048575LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                40)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                18)) &
        4194303LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 262143LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                38)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                16)) &
        4194303LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 65535LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                36)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                14)) &
        4194303LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 16383LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                34)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                12)) &
        4194303LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 4095LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                32)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                10)) &
        4194303LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 1023LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                30)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 8)) &
        4194303LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 255LL) << 14) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 50));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                28)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 6)) &
        4194303LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 63LL) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 48));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                26)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 4)) &
        4194303LL;
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block8 & 15LL) << 18) |
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 46));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                24)) &
        4194303LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 2)) &
        4194303LL;
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block9 & 3LL) << 20) |
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                44));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                22)) &
        4194303LL;
    values[valuesOffset++] = block10 & 4194303LL;
  }
}

void BulkOperationPacked22::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 14) | (byte1 << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 2));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 3) << 20) | (byte3 << 12) | (byte4 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte5) >> 4));
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 15) << 18) | (byte6 << 10) | (byte7 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte8) >> 6));
    constexpr int64_t byte9 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte10 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte8 & 63) << 16) | (byte9 << 8) | byte10;
  }
}
} // namespace org::apache::lucene::util::packed