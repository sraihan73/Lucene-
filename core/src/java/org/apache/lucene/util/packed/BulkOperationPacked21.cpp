using namespace std;

#include "BulkOperationPacked21.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked21::BulkOperationPacked21() : BulkOperationPacked(21) {}

void BulkOperationPacked21::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 43));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 22)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 1)) &
        2097151LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 1LL) << 20) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 44)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 23)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 2)) &
        2097151LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 3LL) << 19) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 45)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 24)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 3)) &
        2097151LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 7LL) << 18) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 46)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 25)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 4)) &
        2097151LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 15LL) << 17) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 47)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 26)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 5)) &
        2097151LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 31LL) << 16) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 48)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 27)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 6)) &
        2097151LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 63LL) << 15) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 49)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 28)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 7)) &
        2097151LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 127LL) << 14) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 50)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 29)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 8)) &
        2097151LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 255LL) << 13) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 51)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 30)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 9)) &
        2097151LL);
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block8 & 511LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 31)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 10)) &
                         2097151LL);
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block9 & 1023LL) << 11) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 53)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 32)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 11)) &
                         2097151LL);
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block10 & 2047LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 33)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 12)) &
                         2097151LL);
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block11 & 4095LL) << 9) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 55)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 34)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 13)) &
                         2097151LL);
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block12 & 8191LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 35)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 14)) &
                         2097151LL);
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block13 & 16383LL) << 7) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 57)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 36)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 15)) &
                         2097151LL);
    constexpr int64_t block15 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block14 & 32767LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 37)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 16)) &
                         2097151LL);
    constexpr int64_t block16 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block15 & 65535LL) << 5) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 59)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 38)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 17)) &
                         2097151LL);
    constexpr int64_t block17 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block16 & 131071LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 39)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 18)) &
                         2097151LL);
    constexpr int64_t block18 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block17 & 262143LL) << 3) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block18) >> 61)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block18) >> 40)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block18) >> 19)) &
                         2097151LL);
    constexpr int64_t block19 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block18 & 524287LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block19) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block19) >> 41)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block19) >> 20)) &
                         2097151LL);
    constexpr int64_t block20 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block19 & 1048575LL) << 1) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block20) >> 63)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block20) >> 42)) &
                         2097151LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block20) >> 21)) &
                         2097151LL);
    values[valuesOffset++] = static_cast<int>(block20 & 2097151LL);
  }
}

void BulkOperationPacked21::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 13) | (byte1 << 5) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 3));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 7) << 18) | (byte3 << 10) | (byte4 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte5) >> 6));
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 63) << 15) | (byte6 << 7) |
        (static_cast<int>(static_cast<unsigned int>(byte7) >> 1));
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte9 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte10 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte7 & 1) << 20) | (byte8 << 12) | (byte9 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte10) >> 4));
    constexpr int byte11 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte12 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte13 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte10 & 15) << 17) | (byte11 << 9) | (byte12 << 1) |
        (static_cast<int>(static_cast<unsigned int>(byte13) >> 7));
    constexpr int byte14 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte15 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte13 & 127) << 14) | (byte14 << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte15) >> 2));
    constexpr int byte16 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte17 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte18 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte15 & 3) << 19) | (byte16 << 11) | (byte17 << 3) |
        (static_cast<int>(static_cast<unsigned int>(byte18) >> 5));
    constexpr int byte19 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte20 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte18 & 31) << 16) | (byte19 << 8) | byte20;
  }
}

void BulkOperationPacked21::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 43);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                22)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 1)) &
        2097151LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 1LL) << 20) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 44));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                23)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 2)) &
        2097151LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 3LL) << 19) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 45));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                24)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 3)) &
        2097151LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 7LL) << 18) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 46));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                25)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 4)) &
        2097151LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 15LL) << 17) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 47));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                26)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 5)) &
        2097151LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 31LL) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 48));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                27)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 6)) &
        2097151LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 63LL) << 15) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 49));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                28)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 7)) &
        2097151LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 127LL) << 14) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 50));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                29)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 8)) &
        2097151LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 255LL) << 13) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 51));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                30)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 9)) &
        2097151LL;
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block8 & 511LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                31)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                10)) &
        2097151LL;
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block9 & 1023LL) << 11) |
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                53));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                32)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                11)) &
        2097151LL;
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block10 & 2047LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                33)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                12)) &
        2097151LL;
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block11 & 4095LL) << 9) |
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                55));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                34)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                13)) &
        2097151LL;
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block12 & 8191LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                35)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                14)) &
        2097151LL;
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block13 & 16383LL) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                57));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                36)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                15)) &
        2097151LL;
    constexpr int64_t block15 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block14 & 32767LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                37)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                16)) &
        2097151LL;
    constexpr int64_t block16 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block15 & 65535LL) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                59));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                38)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                17)) &
        2097151LL;
    constexpr int64_t block17 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block16 & 131071LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                39)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                18)) &
        2097151LL;
    constexpr int64_t block18 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block17 & 262143LL) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(block18) >>
                                61));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block18) >>
                                40)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block18) >>
                                19)) &
        2097151LL;
    constexpr int64_t block19 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block18 & 524287LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block19) >>
                                62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block19) >>
                                41)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block19) >>
                                20)) &
        2097151LL;
    constexpr int64_t block20 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block19 & 1048575LL) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(block20) >>
                                63));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block20) >>
                                42)) &
        2097151LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block20) >>
                                21)) &
        2097151LL;
    values[valuesOffset++] = block20 & 2097151LL;
  }
}

void BulkOperationPacked21::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 13) | (byte1 << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 3));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 7) << 18) | (byte3 << 10) | (byte4 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte5) >> 6));
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 63) << 15) | (byte6 << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte7) >> 1));
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte9 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte10 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte7 & 1) << 20) | (byte8 << 12) | (byte9 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte10) >> 4));
    constexpr int64_t byte11 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte12 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte13 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte10 & 15) << 17) | (byte11 << 9) | (byte12 << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte13) >> 7));
    constexpr int64_t byte14 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte15 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte13 & 127) << 14) | (byte14 << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte15) >> 2));
    constexpr int64_t byte16 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte17 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte18 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte15 & 3) << 19) | (byte16 << 11) | (byte17 << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte18) >> 5));
    constexpr int64_t byte19 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte20 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte18 & 31) << 16) | (byte19 << 8) | byte20;
  }
}
} // namespace org::apache::lucene::util::packed