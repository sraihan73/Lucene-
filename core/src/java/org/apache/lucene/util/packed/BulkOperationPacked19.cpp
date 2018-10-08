using namespace std;

#include "BulkOperationPacked19.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked19::BulkOperationPacked19() : BulkOperationPacked(19) {}

void BulkOperationPacked19::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 45));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 26)) &
                         524287LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 7)) &
        524287LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 127LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 33)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 14)) &
                         524287LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 16383LL) << 5) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 59)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 40)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 21)) &
                         524287LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 2)) &
        524287LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 3LL) << 17) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 47)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 28)) &
                         524287LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 9)) &
        524287LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 511LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 35)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 16)) &
                         524287LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 65535LL) << 3) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 61)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 42)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 23)) &
                         524287LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 4)) &
        524287LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 15LL) << 15) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 49)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 30)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 11)) &
                         524287LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 2047LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 37)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 18)) &
                         524287LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 262143LL) << 1) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 63)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 44)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 25)) &
                         524287LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 6)) &
        524287LL);
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block8 & 63LL) << 13) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 51)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 32)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 13)) &
                         524287LL);
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block9 & 8191LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 39)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 20)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 1)) &
                         524287LL);
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block10 & 1LL) << 18) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 46)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 27)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 8)) &
                         524287LL);
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block11 & 255LL) << 11) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 53)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 34)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 15)) &
                         524287LL);
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block12 & 32767LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 41)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 22)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 3)) &
                         524287LL);
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block13 & 7LL) << 16) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 48)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 29)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 10)) &
                         524287LL);
    constexpr int64_t block15 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block14 & 1023LL) << 9) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 55)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 36)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 17)) &
                         524287LL);
    constexpr int64_t block16 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block15 & 131071LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 43)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 24)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 5)) &
                         524287LL);
    constexpr int64_t block17 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block16 & 31LL) << 14) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 50)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 31)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 12)) &
                         524287LL);
    constexpr int64_t block18 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block17 & 4095LL) << 7) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block18) >> 57)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block18) >> 38)) &
                         524287LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block18) >> 19)) &
                         524287LL);
    values[valuesOffset++] = static_cast<int>(block18 & 524287LL);
  }
}

void BulkOperationPacked19::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 11) | (byte1 << 3) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 5));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 31) << 14) | (byte3 << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte4) >> 2));
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 3) << 17) | (byte5 << 9) | (byte6 << 1) |
        (static_cast<int>(static_cast<unsigned int>(byte7) >> 7));
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte9 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte7 & 127) << 12) | (byte8 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte9) >> 4));
    constexpr int byte10 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte11 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte9 & 15) << 15) | (byte10 << 7) |
        (static_cast<int>(static_cast<unsigned int>(byte11) >> 1));
    constexpr int byte12 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte13 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte14 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte11 & 1) << 18) | (byte12 << 10) | (byte13 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte14) >> 6));
    constexpr int byte15 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte16 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte14 & 63) << 13) | (byte15 << 5) |
        (static_cast<int>(static_cast<unsigned int>(byte16) >> 3));
    constexpr int byte17 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte18 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte16 & 7) << 16) | (byte17 << 8) | byte18;
  }
}

void BulkOperationPacked19::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 45);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                26)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 7)) &
        524287LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 127LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                33)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                14)) &
        524287LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 16383LL) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 59));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                40)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                21)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 2)) &
        524287LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 3LL) << 17) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 47));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                28)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 9)) &
        524287LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 511LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                35)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                16)) &
        524287LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 65535LL) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 61));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                42)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                23)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 4)) &
        524287LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 15LL) << 15) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 49));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                30)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                11)) &
        524287LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 2047LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                37)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                18)) &
        524287LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 262143LL) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 63));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                44)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                25)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 6)) &
        524287LL;
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block8 & 63LL) << 13) |
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 51));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                32)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                13)) &
        524287LL;
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block9 & 8191LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                39)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                20)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                1)) &
        524287LL;
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block10 & 1LL) << 18) |
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                46));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                27)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                8)) &
        524287LL;
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block11 & 255LL) << 11) |
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                53));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                34)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                15)) &
        524287LL;
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block12 & 32767LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                41)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                22)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                3)) &
        524287LL;
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block13 & 7LL) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                48));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                29)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                10)) &
        524287LL;
    constexpr int64_t block15 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block14 & 1023LL) << 9) |
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                55));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                36)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                17)) &
        524287LL;
    constexpr int64_t block16 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block15 & 131071LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                43)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                24)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                5)) &
        524287LL;
    constexpr int64_t block17 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block16 & 31LL) << 14) |
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                50));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                31)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                12)) &
        524287LL;
    constexpr int64_t block18 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block17 & 4095LL) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(block18) >>
                                57));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block18) >>
                                38)) &
        524287LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block18) >>
                                19)) &
        524287LL;
    values[valuesOffset++] = block18 & 524287LL;
  }
}

void BulkOperationPacked19::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 11) | (byte1 << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 5));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 31) << 14) | (byte3 << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte4) >> 2));
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 3) << 17) | (byte5 << 9) | (byte6 << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte7) >> 7));
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte9 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte7 & 127) << 12) | (byte8 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte9) >> 4));
    constexpr int64_t byte10 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte11 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte9 & 15) << 15) | (byte10 << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte11) >> 1));
    constexpr int64_t byte12 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte13 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte14 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte11 & 1) << 18) | (byte12 << 10) | (byte13 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte14) >> 6));
    constexpr int64_t byte15 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte16 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte14 & 63) << 13) | (byte15 << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte16) >> 3));
    constexpr int64_t byte17 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte18 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte16 & 7) << 16) | (byte17 << 8) | byte18;
  }
}
} // namespace org::apache::lucene::util::packed