using namespace std;

#include "Direct64.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

Direct64::Direct64(int valueCount)
    : PackedInts::MutableImpl(valueCount, 64),
      values(std::deque<int64_t>(valueCount))
{
}

Direct64::Direct64(int packedIntsVersion, shared_ptr<DataInput> in_,
                   int valueCount) 
    : Direct64(valueCount)
{
  for (int i = 0; i < valueCount; ++i) {
    values[i] = in_->readLong();
  }
}

int64_t Direct64::get(int const index) { return values[index]; }

void Direct64::set(int const index, int64_t const value)
{
  values[index] = (value);
}

int64_t Direct64::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
             2 * RamUsageEstimator::NUM_BYTES_INT +
             RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(values);
}

void Direct64::clear() { Arrays::fill(values, 0LL); }

int Direct64::get(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int gets = min(valueCount - index, len);
  System::arraycopy(values, index, arr, off, gets);
  return gets;
}

int Direct64::set(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int sets = min(valueCount - index, len);
  System::arraycopy(arr, off, values, index, sets);
  return sets;
}

void Direct64::fill(int fromIndex, int toIndex, int64_t val)
{
  Arrays::fill(values, fromIndex, toIndex, val);
}
} // namespace org::apache::lucene::util::packed