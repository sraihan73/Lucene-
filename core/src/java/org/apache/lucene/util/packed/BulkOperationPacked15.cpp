using namespace std;

#include "BulkOperationPacked15.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked15::BulkOperationPacked15() : BulkOperationPacked(15) {}

void BulkOperationPacked15::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 49));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 34)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 19)) &
                         32767LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 4)) &
        32767LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 15LL) << 11) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 53)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 38)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 23)) &
                         32767LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 8)) &
        32767LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 255LL) << 7) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 57)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 42)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 27)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 12)) &
                         32767LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 4095LL) << 3) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 61)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 46)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 31)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 16)) &
                         32767LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 1)) &
        32767LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 1LL) << 14) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 50)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 35)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 20)) &
                         32767LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 5)) &
        32767LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 31LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 39)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 24)) &
                         32767LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 9)) &
        32767LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 511LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 43)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 28)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 13)) &
                         32767LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 8191LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 47)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 32)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 17)) &
                         32767LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 2)) &
        32767LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 3LL) << 13) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 51)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 36)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 21)) &
                         32767LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 6)) &
        32767LL);
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block8 & 63LL) << 9) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 55)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 40)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 25)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 10)) &
                         32767LL);
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block9 & 1023LL) << 5) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 59)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 44)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 29)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 14)) &
                         32767LL);
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block10 & 16383LL) << 1) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 63)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 48)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 33)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 18)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 3)) &
                         32767LL);
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block11 & 7LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 37)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 22)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 7)) &
                         32767LL);
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block12 & 127LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 41)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 26)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 11)) &
                         32767LL);
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block13 & 2047LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 45)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 30)) &
                         32767LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 15)) &
                         32767LL);
    values[valuesOffset++] = static_cast<int>(block14 & 32767LL);
  }
}

void BulkOperationPacked15::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 7) |
        (static_cast<int>(static_cast<unsigned int>(byte1) >> 1));
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte1 & 1) << 14) | (byte2 << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte3) >> 2));
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte3 & 3) << 13) | (byte4 << 5) |
        (static_cast<int>(static_cast<unsigned int>(byte5) >> 3));
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 7) << 12) | (byte6 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte7) >> 4));
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte9 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte7 & 15) << 11) | (byte8 << 3) |
        (static_cast<int>(static_cast<unsigned int>(byte9) >> 5));
    constexpr int byte10 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte11 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte9 & 31) << 10) | (byte10 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte11) >> 6));
    constexpr int byte12 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte13 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte11 & 63) << 9) | (byte12 << 1) |
        (static_cast<int>(static_cast<unsigned int>(byte13) >> 7));
    constexpr int byte14 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte13 & 127) << 8) | byte14;
  }
}

void BulkOperationPacked15::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 49);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                34)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                19)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 4)) &
        32767LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 15LL) << 11) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 53));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                38)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                23)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 8)) &
        32767LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 255LL) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 57));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                42)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                27)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                12)) &
        32767LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 4095LL) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 61));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                46)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                31)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                16)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 1)) &
        32767LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 1LL) << 14) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 50));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                35)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                20)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 5)) &
        32767LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 31LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                39)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                24)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 9)) &
        32767LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 511LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                43)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                28)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                13)) &
        32767LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 8191LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                47)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                32)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                17)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 2)) &
        32767LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 3LL) << 13) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 51));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                36)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                21)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 6)) &
        32767LL;
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block8 & 63LL) << 9) |
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 55));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                40)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                25)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                10)) &
        32767LL;
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block9 & 1023LL) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                59));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                44)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                29)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                14)) &
        32767LL;
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block10 & 16383LL) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                63));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                48)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                33)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                18)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                3)) &
        32767LL;
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block11 & 7LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                37)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                22)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                7)) &
        32767LL;
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block12 & 127LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                41)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                26)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                11)) &
        32767LL;
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block13 & 2047LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                45)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                30)) &
        32767LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                15)) &
        32767LL;
    values[valuesOffset++] = block14 & 32767LL;
  }
}

void BulkOperationPacked15::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte1) >> 1));
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte1 & 1) << 14) | (byte2 << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte3) >> 2));
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte3 & 3) << 13) | (byte4 << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte5) >> 3));
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 7) << 12) | (byte6 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte7) >> 4));
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte9 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte7 & 15) << 11) | (byte8 << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte9) >> 5));
    constexpr int64_t byte10 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte11 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte9 & 31) << 10) | (byte10 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte11) >> 6));
    constexpr int64_t byte12 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte13 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte11 & 63) << 9) | (byte12 << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte13) >> 7));
    constexpr int64_t byte14 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte13 & 127) << 8) | byte14;
  }
}
} // namespace org::apache::lucene::util::packed