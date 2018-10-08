using namespace std;

#include "Packed16ThreeBlocks.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

Packed16ThreeBlocks::Packed16ThreeBlocks(int valueCount)
    : PackedInts::MutableImpl(valueCount, 48),
      blocks(std::deque<short>(valueCount * 3))
{
  if (valueCount > MAX_SIZE) {
    throw out_of_range(L"MAX_SIZE exceeded");
  }
}

Packed16ThreeBlocks::Packed16ThreeBlocks(int packedIntsVersion,
                                         shared_ptr<DataInput> in_,
                                         int valueCount) 
    : Packed16ThreeBlocks(valueCount)
{
  for (int i = 0; i < 3 * valueCount; ++i) {
    blocks[i] = in_->readShort();
  }
}

int64_t Packed16ThreeBlocks::get(int index)
{
  constexpr int o = index * 3;
  return (blocks[o] & 0xFFFFLL) << 32 | (blocks[o + 1] & 0xFFFFLL) << 16 |
         (blocks[o + 2] & 0xFFFFLL);
}

int Packed16ThreeBlocks::get(int index, std::deque<int64_t> &arr, int off,
                             int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int gets = min(valueCount - index, len);
  for (int i = index * 3, end = (index + gets) * 3; i < end; i += 3) {
    arr[off++] = (blocks[i] & 0xFFFFLL) << 32 |
                 (blocks[i + 1] & 0xFFFFLL) << 16 | (blocks[i + 2] & 0xFFFFLL);
  }
  return gets;
}

void Packed16ThreeBlocks::set(int index, int64_t value)
{
  constexpr int o = index * 3;
  blocks[o] = static_cast<short>(
      static_cast<int64_t>(static_cast<uint64_t>(value) >> 32));
  blocks[o + 1] = static_cast<short>(
      static_cast<int64_t>(static_cast<uint64_t>(value) >> 16));
  blocks[o + 2] = static_cast<short>(value);
}

int Packed16ThreeBlocks::set(int index, std::deque<int64_t> &arr, int off,
                             int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int sets = min(valueCount - index, len);
  for (int i = off, o = index * 3, end = off + sets; i < end; ++i) {
    constexpr int64_t value = arr[i];
    blocks[o++] = static_cast<short>(
        static_cast<int64_t>(static_cast<uint64_t>(value) >> 32));
    blocks[o++] = static_cast<short>(
        static_cast<int64_t>(static_cast<uint64_t>(value) >> 16));
    blocks[o++] = static_cast<short>(value);
  }
  return sets;
}

void Packed16ThreeBlocks::fill(int fromIndex, int toIndex, int64_t val)
{
  constexpr short block1 = static_cast<short>(
      static_cast<int64_t>(static_cast<uint64_t>(val) >> 32));
  constexpr short block2 = static_cast<short>(
      static_cast<int64_t>(static_cast<uint64_t>(val) >> 16));
  constexpr short block3 = static_cast<short>(val);
  for (int i = fromIndex * 3, end = toIndex * 3; i < end; i += 3) {
    blocks[i] = block1;
    blocks[i + 1] = block2;
    blocks[i + 2] = block3;
  }
}

void Packed16ThreeBlocks::clear()
{
  Arrays::fill(blocks, static_cast<short>(0));
}

int64_t Packed16ThreeBlocks::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
             2 * RamUsageEstimator::NUM_BYTES_INT +
             RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(blocks);
}

wstring Packed16ThreeBlocks::toString()
{
  return getClass().getSimpleName() + L"(bitsPerValue=" +
         to_wstring(bitsPerValue) + L",size=" + to_wstring(size()) +
         L",blocks=" + blocks.size() + L")";
}
} // namespace org::apache::lucene::util::packed