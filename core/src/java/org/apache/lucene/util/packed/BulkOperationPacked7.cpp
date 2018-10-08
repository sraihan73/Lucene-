using namespace std;

#include "BulkOperationPacked7.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked7::BulkOperationPacked7() : BulkOperationPacked(7) {}

void BulkOperationPacked7::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset, std::deque<int> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 57));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 50)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 43)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 36)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 29)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 22)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block0) >> 15)) &
                         127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 8)) &
        127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 1)) &
        127LL);
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block0 & 1LL) << 6) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 58)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 51)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 44)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 37)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 30)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 23)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block1) >> 16)) &
                         127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 9)) &
        127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 2)) &
        127LL);
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block1 & 3LL) << 5) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 59)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 52)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 45)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 38)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 31)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 24)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 17)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block2) >> 10)) &
                         127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 3)) &
        127LL);
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block2 & 7LL) << 4) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 60)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 53)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 46)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 39)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 32)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 25)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 18)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block3) >> 11)) &
                         127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 4)) &
        127LL);
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block3 & 15LL) << 3) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 61)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 54)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 47)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 40)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 33)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 26)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 19)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block4) >> 12)) &
                         127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 5)) &
        127LL);
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block4 & 31LL) << 2) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 62)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 55)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 48)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 41)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 34)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 27)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 20)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block5) >> 13)) &
                         127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 6)) &
        127LL);
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int>(((block5 & 63LL) << 1) |
                         (static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 63)));
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 56)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 49)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 42)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 35)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 28)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 21)) &
                         127LL);
    values[valuesOffset++] =
        static_cast<int>((static_cast<int64_t>(
                             static_cast<uint64_t>(block6) >> 14)) &
                         127LL);
    values[valuesOffset++] = static_cast<int>(
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 7)) &
        127LL);
    values[valuesOffset++] = static_cast<int>(block6 & 127LL);
  }
}

void BulkOperationPacked7::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int> &values, int valuesOffset,
                                  int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int byte0 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        static_cast<int>(static_cast<unsigned int>(byte0) >> 1);
    constexpr int byte1 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte0 & 1) << 6) |
        (static_cast<int>(static_cast<unsigned int>(byte1) >> 2));
    constexpr int byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte1 & 3) << 5) |
        (static_cast<int>(static_cast<unsigned int>(byte2) >> 3));
    constexpr int byte3 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 7) << 4) |
        (static_cast<int>(static_cast<unsigned int>(byte3) >> 4));
    constexpr int byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte3 & 15) << 3) |
        (static_cast<int>(static_cast<unsigned int>(byte4) >> 5));
    constexpr int byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 31) << 2) |
        (static_cast<int>(static_cast<unsigned int>(byte5) >> 6));
    constexpr int byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 63) << 1) |
        (static_cast<int>(static_cast<unsigned int>(byte6) >> 7));
    values[valuesOffset++] = byte6 & 127;
  }
}

void BulkOperationPacked7::decode(std::deque<int64_t> &blocks,
                                  int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block0 = blocks[blocksOffset++];
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(block0) >> 57);
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                50)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                43)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                36)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                29)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                22)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >>
                                15)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 8)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block0) >> 1)) &
        127LL;
    constexpr int64_t block1 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block0 & 1LL) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 58));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                51)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                44)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                37)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                30)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                23)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >>
                                16)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 9)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block1) >> 2)) &
        127LL;
    constexpr int64_t block2 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block1 & 3LL) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 59));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                52)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                45)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                38)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                31)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                24)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                17)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >>
                                10)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block2) >> 3)) &
        127LL;
    constexpr int64_t block3 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block2 & 7LL) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 60));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                53)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                46)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                39)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                32)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                25)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                18)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >>
                                11)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block3) >> 4)) &
        127LL;
    constexpr int64_t block4 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block3 & 15LL) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 61));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                54)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                47)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                40)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                33)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                26)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                19)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >>
                                12)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block4) >> 5)) &
        127LL;
    constexpr int64_t block5 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block4 & 31LL) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 62));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                55)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                48)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                41)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                34)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                27)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                20)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >>
                                13)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block5) >> 6)) &
        127LL;
    constexpr int64_t block6 = blocks[blocksOffset++];
    values[valuesOffset++] =
        ((block5 & 63LL) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 63));
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                56)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                49)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                42)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                35)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                28)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                21)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >>
                                14)) &
        127LL;
    values[valuesOffset++] =
        (static_cast<int64_t>(static_cast<uint64_t>(block6) >> 7)) &
        127LL;
    values[valuesOffset++] = block6 & 127LL;
  }
}

void BulkOperationPacked7::decode(std::deque<char> &blocks, int blocksOffset,
                                  std::deque<int64_t> &values,
                                  int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t byte0 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        static_cast<int64_t>(static_cast<uint64_t>(byte0) >> 1);
    constexpr int64_t byte1 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte0 & 1) << 6) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte1) >> 2));
    constexpr int64_t byte2 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte1 & 3) << 5) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte2) >> 3));
    constexpr int64_t byte3 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte2 & 7) << 4) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte3) >> 4));
    constexpr int64_t byte4 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte3 & 15) << 3) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte4) >> 5));
    constexpr int64_t byte5 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte4 & 31) << 2) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte5) >> 6));
    constexpr int64_t byte6 = blocks[blocksOffset++] & 0xFF;
    values[valuesOffset++] =
        ((byte5 & 63) << 1) |
        (static_cast<int64_t>(static_cast<uint64_t>(byte6) >> 7));
    values[valuesOffset++] = byte6 & 127;
  }
}
} // namespace org::apache::lucene::util::packed