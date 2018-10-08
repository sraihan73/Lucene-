using namespace std;

#include "Packed8ThreeBlocks.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

Packed8ThreeBlocks::Packed8ThreeBlocks(int valueCount)
    : PackedInts::MutableImpl(valueCount, 24),
      blocks(std::deque<char>(valueCount * 3))
{
  if (valueCount > MAX_SIZE) {
    throw out_of_range(L"MAX_SIZE exceeded");
  }
}

Packed8ThreeBlocks::Packed8ThreeBlocks(int packedIntsVersion,
                                       shared_ptr<DataInput> in_,
                                       int valueCount) 
    : Packed8ThreeBlocks(valueCount)
{
  in_->readBytes(blocks, 0, 3 * valueCount);
}

int64_t Packed8ThreeBlocks::get(int index)
{
  constexpr int o = index * 3;
  return (blocks[o] & 0xFFLL) << 16 | (blocks[o + 1] & 0xFFLL) << 8 |
         (blocks[o + 2] & 0xFFLL);
}

int Packed8ThreeBlocks::get(int index, std::deque<int64_t> &arr, int off,
                            int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int gets = min(valueCount - index, len);
  for (int i = index * 3, end = (index + gets) * 3; i < end; i += 3) {
    arr[off++] = (blocks[i] & 0xFFLL) << 16 | (blocks[i + 1] & 0xFFLL) << 8 |
                 (blocks[i + 2] & 0xFFLL);
  }
  return gets;
}

void Packed8ThreeBlocks::set(int index, int64_t value)
{
  constexpr int o = index * 3;
  blocks[o] = static_cast<char>(
      static_cast<int64_t>(static_cast<uint64_t>(value) >> 16));
  blocks[o + 1] = static_cast<char>(
      static_cast<int64_t>(static_cast<uint64_t>(value) >> 8));
  blocks[o + 2] = static_cast<char>(value);
}

int Packed8ThreeBlocks::set(int index, std::deque<int64_t> &arr, int off,
                            int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int sets = min(valueCount - index, len);
  for (int i = off, o = index * 3, end = off + sets; i < end; ++i) {
    constexpr int64_t value = arr[i];
    blocks[o++] = static_cast<char>(
        static_cast<int64_t>(static_cast<uint64_t>(value) >> 16));
    blocks[o++] = static_cast<char>(
        static_cast<int64_t>(static_cast<uint64_t>(value) >> 8));
    blocks[o++] = static_cast<char>(value);
  }
  return sets;
}

void Packed8ThreeBlocks::fill(int fromIndex, int toIndex, int64_t val)
{
  constexpr char block1 = static_cast<char>(
      static_cast<int64_t>(static_cast<uint64_t>(val) >> 16));
  constexpr char block2 = static_cast<char>(
      static_cast<int64_t>(static_cast<uint64_t>(val) >> 8));
  constexpr char block3 = static_cast<char>(val);
  for (int i = fromIndex * 3, end = toIndex * 3; i < end; i += 3) {
    blocks[i] = block1;
    blocks[i + 1] = block2;
    blocks[i + 2] = block3;
  }
}

void Packed8ThreeBlocks::clear() { Arrays::fill(blocks, static_cast<char>(0)); }

int64_t Packed8ThreeBlocks::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
             2 * RamUsageEstimator::NUM_BYTES_INT +
             RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(blocks);
}

wstring Packed8ThreeBlocks::toString()
{
  return getClass().getSimpleName() + L"(bitsPerValue=" +
         to_wstring(bitsPerValue) + L",size=" + to_wstring(size()) +
         L",blocks=" + blocks.size() + L")";
}
} // namespace org::apache::lucene::util::packed