using namespace std;

#include "BulkOperationPacked11.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked11::BulkOperationPacked11() : BulkOperationPacked(11) {}

void BulkOperationPacked11::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 53));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 42)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 31)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 20)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 9)) &
        2047LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 511LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 51)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 40)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 29)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 18)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 7)) &
        2047LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 127LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 49)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 38)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 27)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 16)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 5)) &
        2047LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 31LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 47)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 36)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 25)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 14)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 3)) &
        2047LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 7LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 45)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 34)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 23)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 12)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 1)) &
        2047LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 1LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 43)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 32)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 21)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 10)) &
                         2047LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 1023LL) << 1) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 63)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 52)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 41)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 30)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 19)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 8)) &
        2047LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 255LL) << 3) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 61)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 50)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 39)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 28)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 17)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 6)) &
        2047LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 63LL) << 5) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 59)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 48)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 37)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 26)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 15)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 4)) &
        2047LL);
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block8 & 15LL) << 7) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 57)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 46)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 35)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 24)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 13)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 2)) &
        2047LL);
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block9 & 3LL) << 9) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 55)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 44)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 33)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 22)) &
                         2047LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 11)) &
                         2047LL);
    values[valuesOffset++] = static_cast<int>(block10 & 2047LL);
  }
}

void BulkOperationPacked11::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 3) |
        (static_cast<int>(static_cast<unsigned int>(byte1) >> 5));
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte1 & 31) << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 2));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 3) << 9) | (byte3 << 1) |
        (static_cast<int>(static_cast<unsigned int>(byte4) >> 7));
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 127) << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte5) >> 4));
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 15) << 7) |
        (static_cast<int>(static_cast<unsigned int>(byte6) >> 1));
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte6 & 1) << 10) | (byte7 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte8) >> 6));
    constexpr int byte9 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte8 & 63) << 5) |
        (static_cast<int>(static_cast<unsigned int>(byte9) >> 3));
    constexpr int byte10 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte9 & 7) << 8) | byte10;
  }
}

void BulkOperationPacked11::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 53);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                42)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                31)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                20)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 9)) &
        2047LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 511LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                51)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                40)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                29)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                18)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 7)) &
        2047LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 127LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                49)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                38)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                27)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                16)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 5)) &
        2047LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 31LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                47)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                36)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                25)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                14)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 3)) &
        2047LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 7LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                45)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                34)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                23)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                12)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 1)) &
        2047LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 1LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                43)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                32)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                21)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                10)) &
        2047LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 1023LL) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 63));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                52)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                41)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                30)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                19)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 8)) &
        2047LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 255LL) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 61));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                50)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                39)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                28)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                17)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 6)) &
        2047LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 63LL) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 59));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                48)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                37)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                26)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                15)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 4)) &
        2047LL;
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block8 & 15LL) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 57));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                46)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                35)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                24)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                13)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 2)) &
        2047LL;
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block9 & 3LL) << 9) |
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                55));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                44)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                33)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                22)) &
        2047LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                11)) &
        2047LL;
    values[valuesOffset++] = block10 & 2047LL;
  }
}

void BulkOperationPacked11::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte1) >> 5));
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte1 & 31) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 2));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 3) << 9) | (byte3 << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte4) >> 7));
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 127) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte5) >> 4));
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 15) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte6) >> 1));
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte6 & 1) << 10) | (byte7 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte8) >> 6));
    constexpr int64_t byte9 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte8 & 63) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte9) >> 3));
    constexpr int64_t byte10 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte9 & 7) << 8) | byte10;
  }
}
} // namespace org::apache::lucene::util::packed