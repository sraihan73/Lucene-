using namespace std;

#include "Packed64.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

Packed64::Packed64(int valueCount, int bitsPerValue)
    : PackedInts::MutableImpl(valueCount, bitsPerValue),
      blocks(std::deque<int64_t>(longCount)),
      maskRight(static_cast<int>(
          static_cast<unsigned int>(~0LL << (BLOCK_SIZE - bitsPerValue)) >>
          (BLOCK_SIZE - bitsPerValue))),
      bpvMinusBlockSize(bitsPerValue - BLOCK_SIZE)
{
  constexpr PackedInts::Format format = PackedInts::Format::PACKED;
  constexpr int longCount =
      format.longCount(PackedInts::VERSION_CURRENT, valueCount, bitsPerValue);
}

Packed64::Packed64(int packedIntsVersion, shared_ptr<DataInput> in_,
                   int valueCount, int bitsPerValue) 
    : PackedInts::MutableImpl(valueCount, bitsPerValue),
      blocks(std::deque<int64_t>(longCount)),
      maskRight(static_cast<int>(
          static_cast<unsigned int>(~0LL << (BLOCK_SIZE - bitsPerValue)) >>
          (BLOCK_SIZE - bitsPerValue))),
      bpvMinusBlockSize(bitsPerValue - BLOCK_SIZE)
{
  constexpr PackedInts::Format format = PackedInts::Format::PACKED;
  constexpr int64_t byteCount = format.byteCount(
      packedIntsVersion, valueCount, bitsPerValue); // to know how much to read
  constexpr int longCount =
      format.longCount(PackedInts::VERSION_CURRENT, valueCount,
                       bitsPerValue); // to size the array
  // read as many longs as we can
  for (int i = 0; i < byteCount / 8; ++i) {
    blocks[i] = in_->readLong();
  }
  constexpr int remaining = static_cast<int>(byteCount % 8);
  if (remaining != 0) {
    // read the last bytes
    int64_t lastLong = 0;
    for (int i = 0; i < remaining; ++i) {
      lastLong |= (in_->readByte() & 0xFFLL) << (56 - i * 8);
    }
    blocks[blocks.size() - 1] = lastLong;
  }
}

int64_t Packed64::get(int const index)
{
  // The abstract index in a bit stream
  constexpr int64_t majorBitPos =
      static_cast<int64_t>(index) * bitsPerValue;
  // The index in the backing long-array
  constexpr int elementPos = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(majorBitPos) >> BLOCK_BITS));
  // The number of value-bits in the second long
  constexpr int64_t endBits = (majorBitPos & MOD_MASK) + bpvMinusBlockSize;

  if (endBits <= 0) { // Single block
    return (static_cast<int64_t>(
               static_cast<uint64_t>(blocks[elementPos]) >>
               -endBits)) &
           maskRight;
  }
  // Two blocks
  return ((blocks[elementPos] << endBits) |
          (static_cast<int64_t>(
              static_cast<uint64_t>(blocks[elementPos + 1]) >>
              (BLOCK_SIZE - endBits)))) &
         maskRight;
}

int Packed64::get(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  len = min(len, valueCount - index);
  assert(off + len <= arr.size());

  constexpr int originalIndex = index;
  shared_ptr<PackedInts::Decoder> *const decoder =
      BulkOperation::of(PackedInts::Format::PACKED, bitsPerValue);

  // go to the next block where the value does not span across two blocks
  constexpr int offsetInBlocks = index % decoder->longValueCount();
  if (offsetInBlocks != 0) {
    for (int i = offsetInBlocks; i < decoder->longValueCount() && len > 0;
         ++i) {
      arr[off++] = get(index++);
      --len;
    }
    if (len == 0) {
      return index - originalIndex;
    }
  }

  // bulk get
  assert(index % decoder->longValueCount() == 0);
  int blockIndex = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(
          (static_cast<int64_t>(index) * bitsPerValue)) >>
      BLOCK_BITS));
  assert((static_cast<int64_t>(index) * bitsPerValue) & MOD_MASK) == 0;
  constexpr int iterations = len / decoder->longValueCount();
  decoder->decode(blocks, blockIndex, arr, off, iterations);
  constexpr int gotValues = iterations * decoder->longValueCount();
  index += gotValues;
  len -= gotValues;
  assert(len >= 0);

  if (index > originalIndex) {
    // stay at the block boundary
    return index - originalIndex;
  } else {
    // no progress so far => already at a block boundary but no full block to
    // get
    assert(index == originalIndex);
    return PackedInts::MutableImpl::get(index, arr, off, len);
  }
}

void Packed64::set(int const index, int64_t const value)
{
  // The abstract index in a contiguous bit stream
  constexpr int64_t majorBitPos =
      static_cast<int64_t>(index) * bitsPerValue;
  // The index in the backing long-array
  constexpr int elementPos = static_cast<int>(
      static_cast<int64_t>(static_cast<uint64_t>(majorBitPos) >>
                             BLOCK_BITS)); // / BLOCK_SIZE
  // The number of value-bits in the second long
  constexpr int64_t endBits = (majorBitPos & MOD_MASK) + bpvMinusBlockSize;

  if (endBits <= 0) { // Single block
    blocks[elementPos] =
        blocks[elementPos] & ~(maskRight << -endBits) | (value << -endBits);
    return;
  }
  // Two blocks
  blocks[elementPos] =
      blocks[elementPos] &
          ~(static_cast<int64_t>(static_cast<uint64_t>(maskRight) >>
                                   endBits)) |
      (static_cast<int64_t>(static_cast<uint64_t>(value) >>
                              endBits));
  blocks[elementPos + 1] =
      blocks[elementPos + 1] &
          (static_cast<int64_t>(static_cast<uint64_t>(~0LL) >>
                                  endBits)) |
      (value << (BLOCK_SIZE - endBits));
}

int Packed64::set(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  len = min(len, valueCount - index);
  assert(off + len <= arr.size());

  constexpr int originalIndex = index;
  shared_ptr<PackedInts::Encoder> *const encoder =
      BulkOperation::of(PackedInts::Format::PACKED, bitsPerValue);

  // go to the next block where the value does not span across two blocks
  constexpr int offsetInBlocks = index % encoder->longValueCount();
  if (offsetInBlocks != 0) {
    for (int i = offsetInBlocks; i < encoder->longValueCount() && len > 0;
         ++i) {
      set(index++, arr[off++]);
      --len;
    }
    if (len == 0) {
      return index - originalIndex;
    }
  }

  // bulk set
  assert(index % encoder->longValueCount() == 0);
  int blockIndex = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(
          (static_cast<int64_t>(index) * bitsPerValue)) >>
      BLOCK_BITS));
  assert((static_cast<int64_t>(index) * bitsPerValue) & MOD_MASK) == 0;
  constexpr int iterations = len / encoder->longValueCount();
  encoder->encode(arr, off, blocks, blockIndex, iterations);
  constexpr int setValues = iterations * encoder->longValueCount();
  index += setValues;
  len -= setValues;
  assert(len >= 0);

  if (index > originalIndex) {
    // stay at the block boundary
    return index - originalIndex;
  } else {
    // no progress so far => already at a block boundary but no full block to
    // get
    assert(index == originalIndex);
    return PackedInts::MutableImpl::set(index, arr, off, len);
  }
}

wstring Packed64::toString()
{
  return L"Packed64(bitsPerValue=" + to_wstring(bitsPerValue) + L",size=" +
         to_wstring(size()) + L",blocks=" + blocks.size() + L")";
}

int64_t Packed64::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER + 3 * Integer::BYTES +
             Long::BYTES + RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(blocks);
}

void Packed64::fill(int fromIndex, int toIndex, int64_t val)
{
  assert(PackedInts::unsignedBitsRequired(val) <= getBitsPerValue());
  assert(fromIndex <= toIndex);

  // minimum number of values that use an exact number of full blocks
  constexpr int nAlignedValues = 64 / gcd(64, bitsPerValue);
  constexpr int span = toIndex - fromIndex;
  if (span <= 3 * nAlignedValues) {
    // there needs be at least 2 * nAlignedValues aligned values for the
    // block approach to be worth trying
    PackedInts::MutableImpl::fill(fromIndex, toIndex, val);
    return;
  }

  // fill the first values naively until the next block start
  constexpr int fromIndexModNAlignedValues = fromIndex % nAlignedValues;
  if (fromIndexModNAlignedValues != 0) {
    for (int i = fromIndexModNAlignedValues; i < nAlignedValues; ++i) {
      set(fromIndex++, val);
    }
  }
  assert(fromIndex % nAlignedValues == 0);

  // compute the long[] blocks for nAlignedValues consecutive values and
  // use them to set as many values as possible without applying any mask
  // or shift
  constexpr int nAlignedBlocks = (nAlignedValues * bitsPerValue) >> 6;
  const std::deque<int64_t> nAlignedValuesBlocks;
  {
    shared_ptr<Packed64> values =
        make_shared<Packed64>(nAlignedValues, bitsPerValue);
    for (int i = 0; i < nAlignedValues; ++i) {
      values->set(i, val);
    }
    nAlignedValuesBlocks = values->blocks;
    assert(nAlignedBlocks <= nAlignedValuesBlocks.size());
  }
  constexpr int startBlock = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(
          (static_cast<int64_t>(fromIndex) * bitsPerValue)) >>
      6));
  constexpr int endBlock = static_cast<int>(static_cast<int64_t>(
      static_cast<uint64_t>(
          (static_cast<int64_t>(toIndex) * bitsPerValue)) >>
      6));
  for (int block = startBlock; block < endBlock; ++block) {
    constexpr int64_t blockValue =
        nAlignedValuesBlocks[block % nAlignedBlocks];
    blocks[block] = blockValue;
  }

  // fill the gap
  for (int i = static_cast<int>((static_cast<int64_t>(endBlock) << 6) /
                                bitsPerValue);
       i < toIndex; ++i) {
    set(i, val);
  }
}

int Packed64::gcd(int a, int b)
{
  if (a < b) {
    return gcd(b, a);
  } else if (b == 0) {
    return a;
  } else {
    return gcd(b, a % b);
  }
}

void Packed64::clear() { Arrays::fill(blocks, 0LL); }
} // namespace org::apache::lucene::util::packed