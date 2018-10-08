using namespace std;

#include "BulkOperationPacked13.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked13::BulkOperationPacked13() : BulkOperationPacked(13) {}

void BulkOperationPacked13::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 51));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 38)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 25)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 12)) &
                         8191LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 4095LL) << 1) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 63)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 50)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 37)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 24)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 11)) &
                         8191LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 2047LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 49)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 36)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 23)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 10)) &
                         8191LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 1023LL) << 3) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 61)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 48)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 35)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 22)) &
                         8191LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 9)) &
        8191LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 511LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 47)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 34)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 21)) &
                         8191LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 8)) &
        8191LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 255LL) << 5) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 59)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 46)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 33)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 20)) &
                         8191LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 7)) &
        8191LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 127LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 45)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 32)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 19)) &
                         8191LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 6)) &
        8191LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 63LL) << 7) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 57)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 44)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 31)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 18)) &
                         8191LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 5)) &
        8191LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 31LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 43)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 30)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 17)) &
                         8191LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 4)) &
        8191LL);
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block8 & 15LL) << 9) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 55)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 42)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 29)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 16)) &
                         8191LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 3)) &
        8191LL);
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block9 & 7LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 41)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 28)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 15)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 2)) &
                         8191LL);
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block10 & 3LL) << 11) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 53)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 40)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 27)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 14)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 1)) &
                         8191LL);
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block11 & 1LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 39)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 26)) &
                         8191LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 13)) &
                         8191LL);
    values[valuesOffset++] = static_cast<int>(block12 & 8191LL);
  }
}

void BulkOperationPacked13::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 5) |
        (static_cast<int>(static_cast<unsigned int>(byte1) >> 3));
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte1 & 7) << 10) | (byte2 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte3) >> 6));
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte3 & 63) << 7) |
        (static_cast<int>(static_cast<unsigned int>(byte4) >> 1));
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 1) << 12) | (byte5 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte6) >> 4));
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte6 & 15) << 9) | (byte7 << 1) |
        (static_cast<int>(static_cast<unsigned int>(byte8) >> 7));
    constexpr int byte9 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte8 & 127) << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte9) >> 2));
    constexpr int byte10 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte11 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte9 & 3) << 11) | (byte10 << 3) |
        (static_cast<int>(static_cast<unsigned int>(byte11) >> 5));
    constexpr int byte12 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte11 & 31) << 8) | byte12;
  }
}

void BulkOperationPacked13::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 51);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                38)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                25)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                12)) &
        8191LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 4095LL) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 63));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                50)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                37)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                24)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                11)) &
        8191LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 2047LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                49)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                36)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                23)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                10)) &
        8191LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 1023LL) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 61));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                48)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                35)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                22)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 9)) &
        8191LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 511LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                47)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                34)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                21)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 8)) &
        8191LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 255LL) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 59));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                46)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                33)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                20)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 7)) &
        8191LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 127LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                45)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                32)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                19)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 6)) &
        8191LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 63LL) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 57));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                44)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                31)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                18)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 5)) &
        8191LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 31LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                43)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                30)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                17)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 4)) &
        8191LL;
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block8 & 15LL) << 9) |
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 55));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                42)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                29)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                16)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 3)) &
        8191LL;
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block9 & 7LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                41)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                28)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                15)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                2)) &
        8191LL;
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block10 & 3LL) << 11) |
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                53));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                40)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                27)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                14)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                1)) &
        8191LL;
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block11 & 1LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                39)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                26)) &
        8191LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                13)) &
        8191LL;
    values[valuesOffset++] = block12 & 8191LL;
  }
}

void BulkOperationPacked13::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte1) >> 3));
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte1 & 7) << 10) | (byte2 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte3) >> 6));
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte3 & 63) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte4) >> 1));
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 1) << 12) | (byte5 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte6) >> 4));
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte6 & 15) << 9) | (byte7 << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte8) >> 7));
    constexpr int64_t byte9 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte8 & 127) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte9) >> 2));
    constexpr int64_t byte10 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte11 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte9 & 3) << 11) | (byte10 << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte11) >> 5));
    constexpr int64_t byte12 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte11 & 31) << 8) | byte12;
  }
}
} // namespace org::apache::lucene::util::packed