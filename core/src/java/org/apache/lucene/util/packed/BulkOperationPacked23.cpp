using namespace std;

#include "BulkOperationPacked23.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked23::BulkOperationPacked23() : BulkOperationPacked(23) {}

void BulkOperationPacked23::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 41));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 18)) &
                         8388607LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 262143LL) << 5) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 59)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 36)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 13)) &
                         8388607LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 8191LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 31)) &
                         8388607LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 8)) &
        8388607LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 255LL) << 15) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 49)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 26)) &
                         8388607LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 3)) &
        8388607LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 7LL) << 20) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 44)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 21)) &
                         8388607LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 2097151LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 39)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 16)) &
                         8388607LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 65535LL) << 7) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 57)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 34)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 11)) &
                         8388607LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 2047LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 29)) &
                         8388607LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 6)) &
        8388607LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 63LL) << 17) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 47)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 24)) &
                         8388607LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 1)) &
        8388607LL);
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block8 & 1LL) << 22) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 42)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 19)) &
                         8388607LL);
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block9 & 524287LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 37)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 14)) &
                         8388607LL);
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block10 & 16383LL) << 9) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 55)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 32)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 9)) &
                         8388607LL);
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block11 & 511LL) << 14) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 50)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 27)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 4)) &
                         8388607LL);
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block12 & 15LL) << 19) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 45)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 22)) &
                         8388607LL);
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block13 & 4194303LL) << 1) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 63)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 40)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 17)) &
                         8388607LL);
    constexpr int64_t block15 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block14 & 131071LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 35)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 12)) &
                         8388607LL);
    constexpr int64_t block16 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block15 & 4095LL) << 11) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 53)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 30)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 7)) &
                         8388607LL);
    constexpr int64_t block17 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block16 & 127LL) << 16) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 48)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 25)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block17) >> 2)) &
                         8388607LL);
    constexpr int64_t block18 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block17 & 3LL) << 21) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block18) >> 43)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block18) >> 20)) &
                         8388607LL);
    constexpr int64_t block19 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block18 & 1048575LL) << 3) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block19) >> 61)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block19) >> 38)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block19) >> 15)) &
                         8388607LL);
    constexpr int64_t block20 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block19 & 32767LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block20) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block20) >> 33)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block20) >> 10)) &
                         8388607LL);
    constexpr int64_t block21 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block20 & 1023LL) << 13) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block21) >> 51)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block21) >> 28)) &
                         8388607LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block21) >> 5)) &
                         8388607LL);
    constexpr int64_t block22 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block21 & 31LL) << 18) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block22) >> 46)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block22) >> 23)) &
                         8388607LL);
    values[valuesOffset++] = static_cast<int>(block22 & 8388607LL);
  }
}

void BulkOperationPacked23::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 15) | (byte1 << 7) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 1));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 1) << 22) | (byte3 << 14) | (byte4 << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte5) >> 2));
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 3) << 21) | (byte6 << 13) | (byte7 << 5) |
        (static_cast<int>(static_cast<unsigned int>(byte8) >> 3));
    constexpr int byte9 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte10 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte11 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte8 & 7) << 20) | (byte9 << 12) | (byte10 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte11) >> 4));
    constexpr int byte12 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte13 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte14 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte11 & 15) << 19) | (byte12 << 11) | (byte13 << 3) |
        (static_cast<int>(static_cast<unsigned int>(byte14) >> 5));
    constexpr int byte15 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte16 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte17 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte14 & 31) << 18) | (byte15 << 10) | (byte16 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte17) >> 6));
    constexpr int byte18 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte19 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte20 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte17 & 63) << 17) | (byte18 << 9) | (byte19 << 1) |
        (static_cast<int>(static_cast<unsigned int>(byte20) >> 7));
    constexpr int byte21 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte22 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte20 & 127) << 16) | (byte21 << 8) | byte22;
  }
}

void BulkOperationPacked23::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 41);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                18)) &
        8388607LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 262143LL) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 59));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                36)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                13)) &
        8388607LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 8191LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                31)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 8)) &
        8388607LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 255LL) << 15) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 49));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                26)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 3)) &
        8388607LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 7LL) << 20) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 44));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                21)) &
        8388607LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 2097151LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                39)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                16)) &
        8388607LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 65535LL) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 57));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                34)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                11)) &
        8388607LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 2047LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                29)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 6)) &
        8388607LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 63LL) << 17) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 47));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                24)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 1)) &
        8388607LL;
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block8 & 1LL) << 22) |
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 42));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                19)) &
        8388607LL;
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block9 & 524287LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                37)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                14)) &
        8388607LL;
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block10 & 16383LL) << 9) |
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                55));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                32)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                9)) &
        8388607LL;
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block11 & 511LL) << 14) |
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                50));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                27)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                4)) &
        8388607LL;
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block12 & 15LL) << 19) |
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                45));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                22)) &
        8388607LL;
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block13 & 4194303LL) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                63));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                40)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                17)) &
        8388607LL;
    constexpr int64_t block15 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block14 & 131071LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                35)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                12)) &
        8388607LL;
    constexpr int64_t block16 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block15 & 4095LL) << 11) |
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                53));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                30)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                7)) &
        8388607LL;
    constexpr int64_t block17 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block16 & 127LL) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                48));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                25)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block17) >>
                                2)) &
        8388607LL;
    constexpr int64_t block18 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block17 & 3LL) << 21) |
        (static_cast<int64_t>(static_cast<uint64_t>(block18) >>
                                43));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block18) >>
                                20)) &
        8388607LL;
    constexpr int64_t block19 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block18 & 1048575LL) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(block19) >>
                                61));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block19) >>
                                38)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block19) >>
                                15)) &
        8388607LL;
    constexpr int64_t block20 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block19 & 32767LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block20) >>
                                56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block20) >>
                                33)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block20) >>
                                10)) &
        8388607LL;
    constexpr int64_t block21 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block20 & 1023LL) << 13) |
        (static_cast<int64_t>(static_cast<uint64_t>(block21) >>
                                51));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block21) >>
                                28)) &
        8388607LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block21) >>
                                5)) &
        8388607LL;
    constexpr int64_t block22 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block21 & 31LL) << 18) |
        (static_cast<int64_t>(static_cast<uint64_t>(block22) >>
                                46));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block22) >>
                                23)) &
        8388607LL;
    values[valuesOffset++] = block22 & 8388607LL;
  }
}

void BulkOperationPacked23::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 15) | (byte1 << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 1));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 1) << 22) | (byte3 << 14) | (byte4 << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte5) >> 2));
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 3) << 21) | (byte6 << 13) | (byte7 << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte8) >> 3));
    constexpr int64_t byte9 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte10 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte11 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte8 & 7) << 20) | (byte9 << 12) | (byte10 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte11) >> 4));
    constexpr int64_t byte12 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte13 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte14 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte11 & 15) << 19) | (byte12 << 11) | (byte13 << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte14) >> 5));
    constexpr int64_t byte15 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte16 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte17 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte14 & 31) << 18) | (byte15 << 10) | (byte16 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte17) >> 6));
    constexpr int64_t byte18 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte19 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte20 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte17 & 63) << 17) | (byte18 << 9) | (byte19 << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte20) >> 7));
    constexpr int64_t byte21 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte22 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte20 & 127) << 16) | (byte21 << 8) | byte22;
  }
}
} // namespace org::apache::lucene::util::packed