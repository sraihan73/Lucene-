using namespace std;

#include "BulkOperationPackedSingleBlock.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPackedSingleBlock::BulkOperationPackedSingleBlock(int bitsPerValue)
    : bitsPerValue(bitsPerValue), valueCount(64 / bitsPerValue),
      mask((1LL << bitsPerValue) - 1)
{
}

int BulkOperationPackedSingleBlock::longBlockCount() { return BLOCK_COUNT; }

int BulkOperationPackedSingleBlock::byteBlockCount() { return BLOCK_COUNT * 8; }

int BulkOperationPackedSingleBlock::longValueCount() { return valueCount; }

int BulkOperationPackedSingleBlock::byteValueCount() { return valueCount; }

int64_t BulkOperationPackedSingleBlock::readLong(std::deque<char> &blocks,
                                                   int blocksOffset)
{
  return (blocks[blocksOffset++] & 0xFFLL) << 56 |
         (blocks[blocksOffset++] & 0xFFLL) << 48 |
         (blocks[blocksOffset++] & 0xFFLL) << 40 |
         (blocks[blocksOffset++] & 0xFFLL) << 32 |
         (blocks[blocksOffset++] & 0xFFLL) << 24 |
         (blocks[blocksOffset++] & 0xFFLL) << 16 |
         (blocks[blocksOffset++] & 0xFFLL) << 8 |
         blocks[blocksOffset++] & 0xFFLL;
}

int BulkOperationPackedSingleBlock::decode(int64_t block,
                                           std::deque<int64_t> &values,
                                           int valuesOffset)
{
  values[valuesOffset++] = block & mask;
  for (int j = 1; j < valueCount; ++j) {
    block = static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                   bitsPerValue);
    values[valuesOffset++] = block & mask;
  }
  return valuesOffset;
}

int BulkOperationPackedSingleBlock::decode(int64_t block,
                                           std::deque<int> &values,
                                           int valuesOffset)
{
  values[valuesOffset++] = static_cast<int>(block & mask);
  for (int j = 1; j < valueCount; ++j) {
    block = static_cast<int64_t>(static_cast<uint64_t>(block) >>
                                   bitsPerValue);
    values[valuesOffset++] = static_cast<int>(block & mask);
  }
  return valuesOffset;
}

int64_t BulkOperationPackedSingleBlock::encode(std::deque<int64_t> &values,
                                                 int valuesOffset)
{
  int64_t block = values[valuesOffset++];
  for (int j = 1; j < valueCount; ++j) {
    block |= values[valuesOffset++] << (j * bitsPerValue);
  }
  return block;
}

int64_t BulkOperationPackedSingleBlock::encode(std::deque<int> &values,
                                                 int valuesOffset)
{
  int64_t block = values[valuesOffset++] & 0xFFFFFFFFLL;
  for (int j = 1; j < valueCount; ++j) {
    block |= (values[valuesOffset++] & 0xFFFFFFFFLL) << (j * bitsPerValue);
  }
  return block;
}

void BulkOperationPackedSingleBlock::decode(std::deque<int64_t> &blocks,
                                            int blocksOffset,
                                            std::deque<int64_t> &values,
                                            int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    valuesOffset = decode(block, values, valuesOffset);
  }
}

void BulkOperationPackedSingleBlock::decode(std::deque<char> &blocks,
                                            int blocksOffset,
                                            std::deque<int64_t> &values,
                                            int valuesOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = readLong(blocks, blocksOffset);
    blocksOffset += 8;
    valuesOffset = decode(block, values, valuesOffset);
  }
}

void BulkOperationPackedSingleBlock::decode(std::deque<int64_t> &blocks,
                                            int blocksOffset,
                                            std::deque<int> &values,
                                            int valuesOffset, int iterations)
{
  if (bitsPerValue > 32) {
    throw make_shared<UnsupportedOperationException>(
        L"Cannot decode " + to_wstring(bitsPerValue) +
        L"-bits values into an int[]");
  }
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = blocks[blocksOffset++];
    valuesOffset = decode(block, values, valuesOffset);
  }
}

void BulkOperationPackedSingleBlock::decode(std::deque<char> &blocks,
                                            int blocksOffset,
                                            std::deque<int> &values,
                                            int valuesOffset, int iterations)
{
  if (bitsPerValue > 32) {
    throw make_shared<UnsupportedOperationException>(
        L"Cannot decode " + to_wstring(bitsPerValue) +
        L"-bits values into an int[]");
  }
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = readLong(blocks, blocksOffset);
    blocksOffset += 8;
    valuesOffset = decode(block, values, valuesOffset);
  }
}

void BulkOperationPackedSingleBlock::encode(std::deque<int64_t> &values,
                                            int valuesOffset,
                                            std::deque<int64_t> &blocks,
                                            int blocksOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    blocks[blocksOffset++] = encode(values, valuesOffset);
    valuesOffset += valueCount;
  }
}

void BulkOperationPackedSingleBlock::encode(std::deque<int> &values,
                                            int valuesOffset,
                                            std::deque<int64_t> &blocks,
                                            int blocksOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    blocks[blocksOffset++] = encode(values, valuesOffset);
    valuesOffset += valueCount;
  }
}

void BulkOperationPackedSingleBlock::encode(std::deque<int64_t> &values,
                                            int valuesOffset,
                                            std::deque<char> &blocks,
                                            int blocksOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = encode(values, valuesOffset);
    valuesOffset += valueCount;
    blocksOffset = writeLong(block, blocks, blocksOffset);
  }
}

void BulkOperationPackedSingleBlock::encode(std::deque<int> &values,
                                            int valuesOffset,
                                            std::deque<char> &blocks,
                                            int blocksOffset, int iterations)
{
  for (int i = 0; i < iterations; ++i) {
    constexpr int64_t block = encode(values, valuesOffset);
    valuesOffset += valueCount;
    blocksOffset = writeLong(block, blocks, blocksOffset);
  }
}
} // namespace org::apache::lucene::util::packed