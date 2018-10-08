using namespace std;

#include "Direct32.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

Direct32::Direct32(int valueCount)
    : PackedInts::MutableImpl(valueCount, 32),
      values(std::deque<int>(valueCount))
{
}

Direct32::Direct32(int packedIntsVersion, shared_ptr<DataInput> in_,
                   int valueCount) 
    : Direct32(valueCount)
{
  for (int i = 0; i < valueCount; ++i) {
    values[i] = in_->readInt();
  }
  // because packed ints have not always been byte-aligned
  constexpr int remaining = static_cast<int>(
      PackedInts::Format::PACKED.byteCount(packedIntsVersion, valueCount, 32) -
      4LL * valueCount);
  for (int i = 0; i < remaining; ++i) {
    in_->readByte();
  }
}

int64_t Direct32::get(int const index)
{
  return values[index] & 0xFFFFFFFFLL;
}

void Direct32::set(int const index, int64_t const value)
{
  values[index] = static_cast<int>(value);
}

int64_t Direct32::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
             2 * RamUsageEstimator::NUM_BYTES_INT +
             RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(values);
}

void Direct32::clear() { Arrays::fill(values, static_cast<int>(0LL)); }

int Direct32::get(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int gets = min(valueCount - index, len);
  for (int i = index, o = off, end = index + gets; i < end; ++i, ++o) {
    arr[o] = values[i] & 0xFFFFFFFFLL;
  }
  return gets;
}

int Direct32::set(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int sets = min(valueCount - index, len);
  for (int i = index, o = off, end = index + sets; i < end; ++i, ++o) {
    values[i] = static_cast<int>(arr[o]);
  }
  return sets;
}

void Direct32::fill(int fromIndex, int toIndex, int64_t val)
{
  assert(val == (val & 0xFFFFFFFFLL));
  Arrays::fill(values, fromIndex, toIndex, static_cast<int>(val));
}
} // namespace org::apache::lucene::util::packed