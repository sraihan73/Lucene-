using namespace std;

#include "BulkOperationPacked.h"

namespace org::apache::lucene::util::packed
{

BulkOperationPacked::BulkOperationPacked(int bitsPerValue)
    : bitsPerValue(bitsPerValue), longBlockCount(blocks),
      longValueCount(64 * longBlockCount_ / bitsPerValue),
      intMask(static_cast<int>(mask))
{
  assert(bitsPerValue > 0 && bitsPerValue <= 64);
  int blocks = bitsPerValue;
  while ((blocks & 1) == 0) {
    blocks = static_cast<int>(static_cast<unsigned int>(blocks) >> 1);
  }
  int byteBlockCount = 8 * longBlockCount_;
  int byteValueCount = longValueCount_;
  while ((byteBlockCount & 1) == 0 && (byteValueCount & 1) == 0) {
    byteBlockCount =
        static_cast<int>(static_cast<unsigned int>(byteBlockCount) >> 1);
    byteValueCount =
        static_cast<int>(static_cast<unsigned int>(byteValueCount) >> 1);
  }
  this->byteBlockCount_ = byteBlockCount;
  this->byteValueCount_ = byteValueCount;
  if (bitsPerValue == 64) {
    this->mask = ~0LL;
  } else {
    this->mask = (1LL << bitsPerValue) - 1;
  }
  assert(longValueCount_ * bitsPerValue == 64 * longBlockCount_);
}

int BulkOperationPacked::longBlockCount() { return longBlockCount_; }

int BulkOperationPacked::longValueCount() { return longValueCount_; }

int BulkOperationPacked::byteBlockCount() { return byteBlockCount_; }

int BulkOperationPacked::byteValueCount() { return byteValueCount_; }

void BulkOperationPacked::decode(std::deque<int64_t> &blocks,
                                 int blocksOffset,
                                 std::deque<int64_t> &values,
                                 int valuesOffset, int iterations)
{
  int bitsLeft = 64;
  for (int i = 0; i < longValueCount_ * iterations; ++i) {
    bitsLeft -= bitsPerValue;
    if (bitsLeft < 0) {
      values[valuesOffset++] =
          ((blocks[blocksOffset++] & ((1LL << (bitsPerValue + bitsLeft)) - 1))
           << -bitsLeft) |
          (static_cast<int64_t>(
              static_cast<uint64_t>(blocks[blocksOffset]) >>
              (64 + bitsLeft)));
      bitsLeft += 64;
    } else {
      values[valuesOffset++] =
          (static_cast<int64_t>(
              static_cast<uint64_t>(blocks[blocksOffset]) >>
              bitsLeft)) &
          mask;
    }
  }
}

void BulkOperationPacked::decode(std::deque<char> &blocks, int blocksOffset,
                                 std::deque<int64_t> &values,
                                 int valuesOffset, int iterations)
{
  int64_t nextValue = 0LL;
  int bitsLeft = bitsPerValue;
  for (int i = 0; i < iterations * byteBlockCount_; ++i) {
    constexpr int64_t bytes = blocks[blocksOffset++] & 0xFFLL;
    if (bitsLeft > 8) {
      // just buffer
      bitsLeft -= 8;
      nextValue |= bytes << bitsLeft;
    } else {
      // flush
      int bits = 8 - bitsLeft;
      values[valuesOffset++] =
          nextValue | (static_cast<int64_t>(
                          static_cast<uint64_t>(bytes) >> bits));
      while (bits >= bitsPerValue) {
        bits -= bitsPerValue;
        values[valuesOffset++] =
            (static_cast<int64_t>(static_cast<uint64_t>(bytes) >>
                                    bits)) &
            mask;
      }
      // then buffer
      bitsLeft = bitsPerValue - bits;
      nextValue = (bytes & ((1LL << bits) - 1)) << bitsLeft;
    }
  }
  assert(bitsLeft == bitsPerValue);
}

void BulkOperationPacked::decode(std::deque<int64_t> &blocks,
                                 int blocksOffset, std::deque<int> &values,
                                 int valuesOffset, int iterations)
{
  if (bitsPerValue > 32) {
    throw make_shared<UnsupportedOperationException>(
        L"Cannot decode " + to_wstring(bitsPerValue) +
        L"-bits values into an int[]");
  }
  int bitsLeft = 64;
  for (int i = 0; i < longValueCount_ * iterations; ++i) {
    bitsLeft -= bitsPerValue;
    if (bitsLeft < 0) {
      values[valuesOffset++] = static_cast<int>(
          ((blocks[blocksOffset++] & ((1LL << (bitsPerValue + bitsLeft)) - 1))
           << -bitsLeft) |
          (static_cast<int64_t>(
              static_cast<uint64_t>(blocks[blocksOffset]) >>
              (64 + bitsLeft))));
      bitsLeft += 64;
    } else {
      values[valuesOffset++] = static_cast<int>(
          (static_cast<int64_t>(
              static_cast<uint64_t>(blocks[blocksOffset]) >>
              bitsLeft)) &
          mask);
    }
  }
}

void BulkOperationPacked::decode(std::deque<char> &blocks, int blocksOffset,
                                 std::deque<int> &values, int valuesOffset,
                                 int iterations)
{
  int nextValue = 0;
  int bitsLeft = bitsPerValue;
  for (int i = 0; i < iterations * byteBlockCount_; ++i) {
    constexpr int bytes = blocks[blocksOffset++] & 0xFF;
    if (bitsLeft > 8) {
      // just buffer
      bitsLeft -= 8;
      nextValue |= bytes << bitsLeft;
    } else {
      // flush
      int bits = 8 - bitsLeft;
      values[valuesOffset++] =
          nextValue |
          (static_cast<int>(static_cast<unsigned int>(bytes) >> bits));
      while (bits >= bitsPerValue) {
        bits -= bitsPerValue;
        values[valuesOffset++] =
            (static_cast<int>(static_cast<unsigned int>(bytes) >> bits)) &
            intMask;
      }
      // then buffer
      bitsLeft = bitsPerValue - bits;
      nextValue = (bytes & ((1 << bits) - 1)) << bitsLeft;
    }
  }
  assert(bitsLeft == bitsPerValue);
}

void BulkOperationPacked::encode(std::deque<int64_t> &values,
                                 int valuesOffset,
                                 std::deque<int64_t> &blocks,
                                 int blocksOffset, int iterations)
{
  int64_t nextBlock = 0;
  int bitsLeft = 64;
  for (int i = 0; i < longValueCount_ * iterations; ++i) {
    bitsLeft -= bitsPerValue;
    if (bitsLeft > 0) {
      nextBlock |= values[valuesOffset++] << bitsLeft;
    } else if (bitsLeft == 0) {
      nextBlock |= values[valuesOffset++];
      blocks[blocksOffset++] = nextBlock;
      nextBlock = 0;
      bitsLeft = 64;
    } else { // bitsLeft < 0
      nextBlock |= static_cast<int64_t>(
          static_cast<uint64_t>(values[valuesOffset]) >> -bitsLeft);
      blocks[blocksOffset++] = nextBlock;
      nextBlock = (values[valuesOffset++] & ((1LL << -bitsLeft) - 1))
                  << (64 + bitsLeft);
      bitsLeft += 64;
    }
  }
}

void BulkOperationPacked::encode(std::deque<int> &values, int valuesOffset,
                                 std::deque<int64_t> &blocks,
                                 int blocksOffset, int iterations)
{
  int64_t nextBlock = 0;
  int bitsLeft = 64;
  for (int i = 0; i < longValueCount_ * iterations; ++i) {
    bitsLeft -= bitsPerValue;
    if (bitsLeft > 0) {
      nextBlock |= (values[valuesOffset++] & 0xFFFFFFFFLL) << bitsLeft;
    } else if (bitsLeft == 0) {
      nextBlock |= (values[valuesOffset++] & 0xFFFFFFFFLL);
      blocks[blocksOffset++] = nextBlock;
      nextBlock = 0;
      bitsLeft = 64;
    } else { // bitsLeft < 0
      nextBlock |=
          static_cast<int64_t>(static_cast<uint64_t>(
                                     (values[valuesOffset] & 0xFFFFFFFFLL)) >>
                                 -bitsLeft);
      blocks[blocksOffset++] = nextBlock;
      nextBlock = (values[valuesOffset++] & ((1LL << -bitsLeft) - 1))
                  << (64 + bitsLeft);
      bitsLeft += 64;
    }
  }
}

void BulkOperationPacked::encode(std::deque<int64_t> &values,
                                 int valuesOffset, std::deque<char> &blocks,
                                 int blocksOffset, int iterations)
{
  int nextBlock = 0;
  int bitsLeft = 8;
  for (int i = 0; i < byteValueCount_ * iterations; ++i) {
    constexpr int64_t v = values[valuesOffset++];
    assert(PackedInts::unsignedBitsRequired(v) <= bitsPerValue);
    if (bitsPerValue < bitsLeft) {
      // just buffer
      nextBlock |= v << (bitsLeft - bitsPerValue);
      bitsLeft -= bitsPerValue;
    } else {
      // flush as many blocks as possible
      int bits = bitsPerValue - bitsLeft;
      blocks[blocksOffset++] = static_cast<char>(
          nextBlock |
          (static_cast<int64_t>(static_cast<uint64_t>(v) >> bits)));
      while (bits >= 8) {
        bits -= 8;
        blocks[blocksOffset++] = static_cast<char>(
            static_cast<int64_t>(static_cast<uint64_t>(v) >> bits));
      }
      // then buffer
      bitsLeft = 8 - bits;
      nextBlock = static_cast<int>((v & ((1LL << bits) - 1)) << bitsLeft);
    }
  }
  assert(bitsLeft == 8);
}

void BulkOperationPacked::encode(std::deque<int> &values, int valuesOffset,
                                 std::deque<char> &blocks, int blocksOffset,
                                 int iterations)
{
  int nextBlock = 0;
  int bitsLeft = 8;
  for (int i = 0; i < byteValueCount_ * iterations; ++i) {
    constexpr int v = values[valuesOffset++];
    assert(PackedInts::bitsRequired(v & 0xFFFFFFFFLL) <= bitsPerValue);
    if (bitsPerValue < bitsLeft) {
      // just buffer
      nextBlock |= v << (bitsLeft - bitsPerValue);
      bitsLeft -= bitsPerValue;
    } else {
      // flush as many blocks as possible
      int bits = bitsPerValue - bitsLeft;
      blocks[blocksOffset++] = static_cast<char>(
          nextBlock | (static_cast<int>(static_cast<unsigned int>(v) >> bits)));
      while (bits >= 8) {
        bits -= 8;
        blocks[blocksOffset++] = static_cast<char>(
            static_cast<int>(static_cast<unsigned int>(v) >> bits));
      }
      // then buffer
      bitsLeft = 8 - bits;
      nextBlock = (v & ((1 << bits) - 1)) << bitsLeft;
    }
  }
  assert(bitsLeft == 8);
}
} // namespace org::apache::lucene::util::packed