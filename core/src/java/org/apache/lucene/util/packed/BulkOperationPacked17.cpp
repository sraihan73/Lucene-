using namespace std;

#include "BulkOperationPacked17.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked17::BulkOperationPacked17() : BulkOperationPacked(17) {}

void BulkOperationPacked17::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset, std::deque<int> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 47));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 30)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 13)) &
                         131071LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 8191LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 43)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 26)) &
                         131071LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 9)) &
        131071LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 511LL) << 8) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 56)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 39)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 22)) &
                         131071LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 5)) &
        131071LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 31LL) << 12) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 52)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 35)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 18)) &
                         131071LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 1)) &
        131071LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 1LL) << 16) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 48)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 31)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 14)) &
                         131071LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 16383LL) << 3) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 61)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 44)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 27)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 10)) &
                         131071LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 1023LL) << 7) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 57)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 40)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 23)) &
                         131071LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 6)) &
        131071LL);
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block6 & 63LL) << 11) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 53)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 36)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block7) >> 19)) &
                         131071LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 2)) &
        131071LL);
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block7 & 3LL) << 15) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 49)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 32)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block8) >> 15)) &
                         131071LL);
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block8 & 32767LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 45)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 28)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block9) >> 11)) &
                         131071LL);
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block9 & 2047LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 41)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 24)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block10) >> 7)) &
                         131071LL);
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block10 & 127LL) << 10) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 54)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 37)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 20)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block11) >> 3)) &
                         131071LL);
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block11 & 7LL) << 14) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 50)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 33)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block12) >> 16)) &
                         131071LL);
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block12 & 65535LL) << 1) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 63)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 46)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 29)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block13) >> 12)) &
                         131071LL);
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block13 & 4095LL) << 5) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 59)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 42)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 25)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block14) >> 8)) &
                         131071LL);
    constexpr int64_t block15 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block14 & 255LL) << 9) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 55)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 38)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 21)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block15) >> 4)) &
                         131071LL);
    constexpr int64_t block16 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block15 & 15LL) << 13) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 51)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 34)) &
                         131071LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block16) >> 17)) &
                         131071LL);
    values[valuesOffset++] = static_cast<int>(block16 & 131071LL);
  }
}

void BulkOperationPacked17::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int> &values, int valuesOffset,
                                   int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 9) | (byte1 << 1) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 7));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 127) << 10) | (byte3 << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte4) >> 6));
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 63) << 11) | (byte5 << 3) |
        (static_cast<int>(static_cast<unsigned int>(byte6) >> 5));
    constexpr int byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte6 & 31) << 12) | (byte7 << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte8) >> 4));
    constexpr int byte9 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte10 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte8 & 15) << 13) | (byte9 << 5) |
        (static_cast<int>(static_cast<unsigned int>(byte10) >> 3));
    constexpr int byte11 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte12 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte10 & 7) << 14) | (byte11 << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte12) >> 2));
    constexpr int byte13 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte14 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte12 & 3) << 15) | (byte13 << 7) |
        (static_cast<int>(static_cast<unsigned int>(byte14) >> 1));
    constexpr int byte15 = blocks[blocksOffset++] & 0xFF;
    constexpr int byte16 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte14 & 1) << 16) | (byte15 << 8) | byte16;
  }
}

void BulkOperationPacked17::decode(std::deque<int64_t> &blocks,
                                   int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 47);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                30)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                13)) &
        131071LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 8191LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                43)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                26)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 9)) &
        131071LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 511LL) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 56));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                39)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                22)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 5)) &
        131071LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 31LL) << 12) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 52));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                35)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                18)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 1)) &
        131071LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 1LL) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 48));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                31)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                14)) &
        131071LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 16383LL) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 61));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                44)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                27)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                10)) &
        131071LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 1023LL) << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 57));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                40)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                23)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 6)) &
        131071LL;
    constexpr int64_t block7 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block6 & 63LL) << 11) |
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 53));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                36)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >>
                                19)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block7) >> 2)) &
        131071LL;
    constexpr int64_t block8 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block7 & 3LL) << 15) |
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >> 49));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                32)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block8) >>
                                15)) &
        131071LL;
    constexpr int64_t block9 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block8 & 32767LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >> 62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                45)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                28)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block9) >>
                                11)) &
        131071LL;
    constexpr int64_t block10 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block9 & 2047LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                41)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                24)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block10) >>
                                7)) &
        131071LL;
    constexpr int64_t block11 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block10 & 127LL) << 10) |
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                54));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                37)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                20)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block11) >>
                                3)) &
        131071LL;
    constexpr int64_t block12 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block11 & 7LL) << 14) |
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                50));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                33)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block12) >>
                                16)) &
        131071LL;
    constexpr int64_t block13 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block12 & 65535LL) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                63));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                46)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                29)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block13) >>
                                12)) &
        131071LL;
    constexpr int64_t block14 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block13 & 4095LL) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                59));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                42)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                25)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block14) >>
                                8)) &
        131071LL;
    constexpr int64_t block15 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block14 & 255LL) << 9) |
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                55));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                38)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                21)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block15) >>
                                4)) &
        131071LL;
    constexpr int64_t block16 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block15 & 15LL) << 13) |
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                51));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                34)) &
        131071LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block16) >>
                                17)) &
        131071LL;
    values[valuesOffset++] = block16 & 131071LL;
  }
}

void BulkOperationPacked17::decode(std::deque<char> &blocks, int blocksOffset,
                                   std::deque<int64_t> &values,
                                   int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        (byte0 << 9) | (byte1 << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 7));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 127) << 10) | (byte3 << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte4) >> 6));
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 63) << 11) | (byte5 << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte6) >> 5));
    constexpr int64_t byte7 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte8 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte6 & 31) << 12) | (byte7 << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte8) >> 4));
    constexpr int64_t byte9 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte10 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte8 & 15) << 13) | (byte9 << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte10) >> 3));
    constexpr int64_t byte11 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte12 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte10 & 7) << 14) | (byte11 << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte12) >> 2));
    constexpr int64_t byte13 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte14 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte12 & 3) << 15) | (byte13 << 7) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte14) >> 1));
    constexpr int64_t byte15 = blocks[blocksOffset++] & 0xFF;
    constexpr int64_t byte16 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] = ((byte14 & 1) << 16) | (byte15 << 8) | byte16;
  }
}
} // namespace org::apache::lucene::util::packed