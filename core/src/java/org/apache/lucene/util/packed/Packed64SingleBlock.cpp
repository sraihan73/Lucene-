using namespace std;

#include "Packed64SingleBlock.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
std::deque<int> const Packed64SingleBlock::SUPPORTED_BITS_PER_VALUE =
    std::deque<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16, 21, 32};

bool Packed64SingleBlock::isSupported(int bitsPerValue)
{
  return Arrays::binarySearch(SUPPORTED_BITS_PER_VALUE, bitsPerValue) >= 0;
}

int Packed64SingleBlock::requiredCapacity(int valueCount, int valuesPerBlock)
{
  return valueCount / valuesPerBlock +
         (valueCount % valuesPerBlock == 0 ? 0 : 1);
}

Packed64SingleBlock::Packed64SingleBlock(int valueCount, int bitsPerValue)
    : PackedInts::MutableImpl(valueCount, bitsPerValue),
      blocks(
          std::deque<int64_t>(requiredCapacity(valueCount, valuesPerBlock)))
{
  assert(isSupported(bitsPerValue));
  constexpr int valuesPerBlock = 64 / bitsPerValue;
}

void Packed64SingleBlock::clear() { Arrays::fill(blocks, 0LL); }

int64_t Packed64SingleBlock::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
             2 * RamUsageEstimator::NUM_BYTES_INT +
             RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(blocks);
}

int Packed64SingleBlock::get(int index, std::deque<int64_t> &arr, int off,
                             int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  len = min(len, valueCount - index);
  assert(off + len <= arr.size());

  constexpr int originalIndex = index;

  // go to the next block boundary
  constexpr int valuesPerBlock = 64 / bitsPerValue;
  constexpr int offsetInBlock = index % valuesPerBlock;
  if (offsetInBlock != 0) {
    for (int i = offsetInBlock; i < valuesPerBlock && len > 0; ++i) {
      arr[off++] = get(index++);
      --len;
    }
    if (len == 0) {
      return index - originalIndex;
    }
  }

  // bulk get
  assert(index % valuesPerBlock == 0);
  shared_ptr<PackedInts::Decoder> *const decoder =
      BulkOperation::of(PackedInts::Format::PACKED_SINGLE_BLOCK, bitsPerValue);
  assert(decoder->longBlockCount() == 1);
  assert(decoder->longValueCount() == valuesPerBlock);
  constexpr int blockIndex = index / valuesPerBlock;
  constexpr int nblocks = (index + len) / valuesPerBlock - blockIndex;
  decoder->decode(blocks, blockIndex, arr, off, nblocks);
  constexpr int diff = nblocks * valuesPerBlock;
  index += diff;
  len -= diff;

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

int Packed64SingleBlock::set(int index, std::deque<int64_t> &arr, int off,
                             int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  len = min(len, valueCount - index);
  assert(off + len <= arr.size());

  constexpr int originalIndex = index;

  // go to the next block boundary
  constexpr int valuesPerBlock = 64 / bitsPerValue;
  constexpr int offsetInBlock = index % valuesPerBlock;
  if (offsetInBlock != 0) {
    for (int i = offsetInBlock; i < valuesPerBlock && len > 0; ++i) {
      set(index++, arr[off++]);
      --len;
    }
    if (len == 0) {
      return index - originalIndex;
    }
  }

  // bulk set
  assert(index % valuesPerBlock == 0);
  shared_ptr<BulkOperation> *const op =
      BulkOperation::of(PackedInts::Format::PACKED_SINGLE_BLOCK, bitsPerValue);
  assert(op->longBlockCount() == 1);
  assert(op->longValueCount() == valuesPerBlock);
  constexpr int blockIndex = index / valuesPerBlock;
  constexpr int nblocks = (index + len) / valuesPerBlock - blockIndex;
  op->encode(arr, off, blocks, blockIndex, nblocks);
  constexpr int diff = nblocks * valuesPerBlock;
  index += diff;
  len -= diff;

  if (index > originalIndex) {
    // stay at the block boundary
    return index - originalIndex;
  } else {
    // no progress so far => already at a block boundary but no full block to
    // set
    assert(index == originalIndex);
    return PackedInts::MutableImpl::set(index, arr, off, len);
  }
}

void Packed64SingleBlock::fill(int fromIndex, int toIndex, int64_t val)
{
  assert(fromIndex >= 0);
  assert(fromIndex <= toIndex);
  assert(PackedInts::unsignedBitsRequired(val) <= bitsPerValue);

  constexpr int valuesPerBlock = 64 / bitsPerValue;
  if (toIndex - fromIndex <= valuesPerBlock << 1) {
    // there needs to be at least one full block to set for the block
    // approach to be worth trying
    PackedInts::MutableImpl::fill(fromIndex, toIndex, val);
    return;
  }

  // set values naively until the next block start
  int fromOffsetInBlock = fromIndex % valuesPerBlock;
  if (fromOffsetInBlock != 0) {
    for (int i = fromOffsetInBlock; i < valuesPerBlock; ++i) {
      set(fromIndex++, val);
    }
    assert(fromIndex % valuesPerBlock == 0);
  }

  // bulk set of the inner blocks
  constexpr int fromBlock = fromIndex / valuesPerBlock;
  constexpr int toBlock = toIndex / valuesPerBlock;
  assert(fromBlock * valuesPerBlock == fromIndex);

  int64_t blockValue = 0LL;
  for (int i = 0; i < valuesPerBlock; ++i) {
    blockValue = blockValue | (val << (i * bitsPerValue));
  }
  Arrays::fill(blocks, fromBlock, toBlock, blockValue);

  // fill the gap
  for (int i = valuesPerBlock * toBlock; i < toIndex; ++i) {
    set(i, val);
  }
}

PackedInts::Format Packed64SingleBlock::getFormat()
{
  return PackedInts::Format::PACKED_SINGLE_BLOCK;
}

wstring Packed64SingleBlock::toString()
{
  return getClass().getSimpleName() + L"(bitsPerValue=" +
         to_wstring(bitsPerValue) + L",size=" + to_wstring(size()) +
         L",blocks=" + blocks.size() + L")";
}

shared_ptr<Packed64SingleBlock>
Packed64SingleBlock::create(shared_ptr<DataInput> in_, int valueCount,
                            int bitsPerValue) 
{
  shared_ptr<Packed64SingleBlock> reader = create(valueCount, bitsPerValue);
  for (int i = 0; i < reader->blocks.size(); ++i) {
    reader->blocks[i] = in_->readLong();
  }
  return reader;
}

shared_ptr<Packed64SingleBlock> Packed64SingleBlock::create(int valueCount,
                                                            int bitsPerValue)
{
  switch (bitsPerValue) {
  case 1:
    return make_shared<Packed64SingleBlock1>(valueCount);
  case 2:
    return make_shared<Packed64SingleBlock2>(valueCount);
  case 3:
    return make_shared<Packed64SingleBlock3>(valueCount);
  case 4:
    return make_shared<Packed64SingleBlock4>(valueCount);
  case 5:
    return make_shared<Packed64SingleBlock5>(valueCount);
  case 6:
    return make_shared<Packed64SingleBlock6>(valueCount);
  case 7:
    return make_shared<Packed64SingleBlock7>(valueCount);
  case 8:
    return make_shared<Packed64SingleBlock8>(valueCount);
  case 9:
    return make_shared<Packed64SingleBlock9>(valueCount);
  case 10:
    return make_shared<Packed64SingleBlock10>(valueCount);
  case 12:
    return make_shared<Packed64SingleBlock12>(valueCount);
  case 16:
    return make_shared<Packed64SingleBlock16>(valueCount);
  case 21:
    return make_shared<Packed64SingleBlock21>(valueCount);
  case 32:
    return make_shared<Packed64SingleBlock32>(valueCount);
  default:
    throw invalid_argument(L"Unsupported number of bits per value: " +
                           to_wstring(32));
  }
}

Packed64SingleBlock::Packed64SingleBlock1::Packed64SingleBlock1(int valueCount)
    : Packed64SingleBlock(valueCount, 1)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock1::get(int index)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 6);
  constexpr int b = index & 63;
  constexpr int shift = b << 0;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         1LL;
}

void Packed64SingleBlock::Packed64SingleBlock1::set(int index, int64_t value)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 6);
  constexpr int b = index & 63;
  constexpr int shift = b << 0;
  blocks[o] = (blocks[o] & ~(1LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock2::Packed64SingleBlock2(int valueCount)
    : Packed64SingleBlock(valueCount, 2)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock2::get(int index)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 5);
  constexpr int b = index & 31;
  constexpr int shift = b << 1;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         3LL;
}

void Packed64SingleBlock::Packed64SingleBlock2::set(int index, int64_t value)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 5);
  constexpr int b = index & 31;
  constexpr int shift = b << 1;
  blocks[o] = (blocks[o] & ~(3LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock3::Packed64SingleBlock3(int valueCount)
    : Packed64SingleBlock(valueCount, 3)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock3::get(int index)
{
  constexpr int o = index / 21;
  constexpr int b = index % 21;
  constexpr int shift = b * 3;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         7LL;
}

void Packed64SingleBlock::Packed64SingleBlock3::set(int index, int64_t value)
{
  constexpr int o = index / 21;
  constexpr int b = index % 21;
  constexpr int shift = b * 3;
  blocks[o] = (blocks[o] & ~(7LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock4::Packed64SingleBlock4(int valueCount)
    : Packed64SingleBlock(valueCount, 4)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock4::get(int index)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 4);
  constexpr int b = index & 15;
  constexpr int shift = b << 2;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         15LL;
}

void Packed64SingleBlock::Packed64SingleBlock4::set(int index, int64_t value)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 4);
  constexpr int b = index & 15;
  constexpr int shift = b << 2;
  blocks[o] = (blocks[o] & ~(15LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock5::Packed64SingleBlock5(int valueCount)
    : Packed64SingleBlock(valueCount, 5)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock5::get(int index)
{
  constexpr int o = index / 12;
  constexpr int b = index % 12;
  constexpr int shift = b * 5;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         31LL;
}

void Packed64SingleBlock::Packed64SingleBlock5::set(int index, int64_t value)
{
  constexpr int o = index / 12;
  constexpr int b = index % 12;
  constexpr int shift = b * 5;
  blocks[o] = (blocks[o] & ~(31LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock6::Packed64SingleBlock6(int valueCount)
    : Packed64SingleBlock(valueCount, 6)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock6::get(int index)
{
  constexpr int o = index / 10;
  constexpr int b = index % 10;
  constexpr int shift = b * 6;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         63LL;
}

void Packed64SingleBlock::Packed64SingleBlock6::set(int index, int64_t value)
{
  constexpr int o = index / 10;
  constexpr int b = index % 10;
  constexpr int shift = b * 6;
  blocks[o] = (blocks[o] & ~(63LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock7::Packed64SingleBlock7(int valueCount)
    : Packed64SingleBlock(valueCount, 7)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock7::get(int index)
{
  constexpr int o = index / 9;
  constexpr int b = index % 9;
  constexpr int shift = b * 7;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         127LL;
}

void Packed64SingleBlock::Packed64SingleBlock7::set(int index, int64_t value)
{
  constexpr int o = index / 9;
  constexpr int b = index % 9;
  constexpr int shift = b * 7;
  blocks[o] = (blocks[o] & ~(127LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock8::Packed64SingleBlock8(int valueCount)
    : Packed64SingleBlock(valueCount, 8)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock8::get(int index)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 3);
  constexpr int b = index & 7;
  constexpr int shift = b << 3;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         255LL;
}

void Packed64SingleBlock::Packed64SingleBlock8::set(int index, int64_t value)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 3);
  constexpr int b = index & 7;
  constexpr int shift = b << 3;
  blocks[o] = (blocks[o] & ~(255LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock9::Packed64SingleBlock9(int valueCount)
    : Packed64SingleBlock(valueCount, 9)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock9::get(int index)
{
  constexpr int o = index / 7;
  constexpr int b = index % 7;
  constexpr int shift = b * 9;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         511LL;
}

void Packed64SingleBlock::Packed64SingleBlock9::set(int index, int64_t value)
{
  constexpr int o = index / 7;
  constexpr int b = index % 7;
  constexpr int shift = b * 9;
  blocks[o] = (blocks[o] & ~(511LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock10::Packed64SingleBlock10(
    int valueCount)
    : Packed64SingleBlock(valueCount, 10)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock10::get(int index)
{
  constexpr int o = index / 6;
  constexpr int b = index % 6;
  constexpr int shift = b * 10;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         1023LL;
}

void Packed64SingleBlock::Packed64SingleBlock10::set(int index, int64_t value)
{
  constexpr int o = index / 6;
  constexpr int b = index % 6;
  constexpr int shift = b * 10;
  blocks[o] = (blocks[o] & ~(1023LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock12::Packed64SingleBlock12(
    int valueCount)
    : Packed64SingleBlock(valueCount, 12)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock12::get(int index)
{
  constexpr int o = index / 5;
  constexpr int b = index % 5;
  constexpr int shift = b * 12;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         4095LL;
}

void Packed64SingleBlock::Packed64SingleBlock12::set(int index, int64_t value)
{
  constexpr int o = index / 5;
  constexpr int b = index % 5;
  constexpr int shift = b * 12;
  blocks[o] = (blocks[o] & ~(4095LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock16::Packed64SingleBlock16(
    int valueCount)
    : Packed64SingleBlock(valueCount, 16)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock16::get(int index)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 2);
  constexpr int b = index & 3;
  constexpr int shift = b << 4;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         65535LL;
}

void Packed64SingleBlock::Packed64SingleBlock16::set(int index, int64_t value)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 2);
  constexpr int b = index & 3;
  constexpr int shift = b << 4;
  blocks[o] = (blocks[o] & ~(65535LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock21::Packed64SingleBlock21(
    int valueCount)
    : Packed64SingleBlock(valueCount, 21)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock21::get(int index)
{
  constexpr int o = index / 3;
  constexpr int b = index % 3;
  constexpr int shift = b * 21;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         2097151LL;
}

void Packed64SingleBlock::Packed64SingleBlock21::set(int index, int64_t value)
{
  constexpr int o = index / 3;
  constexpr int b = index % 3;
  constexpr int shift = b * 21;
  blocks[o] = (blocks[o] & ~(2097151LL << shift)) | (value << shift);
}

Packed64SingleBlock::Packed64SingleBlock32::Packed64SingleBlock32(
    int valueCount)
    : Packed64SingleBlock(valueCount, 32)
{
}

int64_t Packed64SingleBlock::Packed64SingleBlock32::get(int index)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 1);
  constexpr int b = index & 1;
  constexpr int shift = b << 5;
  return (static_cast<int64_t>(static_cast<uint64_t>(blocks[o]) >>
                                 shift)) &
         4294967295LL;
}

void Packed64SingleBlock::Packed64SingleBlock32::set(int index, int64_t value)
{
  constexpr int o = static_cast<int>(static_cast<unsigned int>(index) >> 1);
  constexpr int b = index & 1;
  constexpr int shift = b << 5;
  blocks[o] = (blocks[o] & ~(4294967295LL << shift)) | (value << shift);
}
} // namespace org::apache::lucene::util::packed