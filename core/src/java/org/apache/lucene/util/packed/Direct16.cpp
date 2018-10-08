using namespace std;

#include "Direct16.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

Direct16::Direct16(int valueCount)
    : PackedInts::MutableImpl(valueCount, 16),
      values(std::deque<short>(valueCount))
{
}

Direct16::Direct16(int packedIntsVersion, shared_ptr<DataInput> in_,
                   int valueCount) 
    : Direct16(valueCount)
{
  for (int i = 0; i < valueCount; ++i) {
    values[i] = in_->readShort();
  }
  // because packed ints have not always been byte-aligned
  constexpr int remaining = static_cast<int>(
      PackedInts::Format::PACKED.byteCount(packedIntsVersion, valueCount, 16) -
      2LL * valueCount);
  for (int i = 0; i < remaining; ++i) {
    in_->readByte();
  }
}

int64_t Direct16::get(int const index) { return values[index] & 0xFFFFLL; }

void Direct16::set(int const index, int64_t const value)
{
  values[index] = static_cast<short>(value);
}

int64_t Direct16::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
             2 * RamUsageEstimator::NUM_BYTES_INT +
             RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(values);
}

void Direct16::clear() { Arrays::fill(values, static_cast<short>(0LL)); }

int Direct16::get(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int gets = min(valueCount - index, len);
  for (int i = index, o = off, end = index + gets; i < end; ++i, ++o) {
    arr[o] = values[i] & 0xFFFFLL;
  }
  return gets;
}

int Direct16::set(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int sets = min(valueCount - index, len);
  for (int i = index, o = off, end = index + sets; i < end; ++i, ++o) {
    values[i] = static_cast<short>(arr[o]);
  }
  return sets;
}

void Direct16::fill(int fromIndex, int toIndex, int64_t val)
{
  assert(val == (val & 0xFFFFLL));
  Arrays::fill(values, fromIndex, toIndex, static_cast<short>(val));
}
} // namespace org::apache::lucene::util::packed