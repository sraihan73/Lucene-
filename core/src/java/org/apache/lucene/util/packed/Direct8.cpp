using namespace std;

#include "Direct8.h"

namespace org::apache::lucene::util::packed
{
using DataInput = org::apache::lucene::store::DataInput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

Direct8::Direct8(int valueCount)
    : PackedInts::MutableImpl(valueCount, 8),
      values(std::deque<char>(valueCount))
{
}

Direct8::Direct8(int packedIntsVersion, shared_ptr<DataInput> in_,
                 int valueCount) 
    : Direct8(valueCount)
{
  in_->readBytes(values, 0, valueCount);
  // because packed ints have not always been byte-aligned
  constexpr int remaining = static_cast<int>(
      PackedInts::Format::PACKED.byteCount(packedIntsVersion, valueCount, 8) -
      1LL * valueCount);
  for (int i = 0; i < remaining; ++i) {
    in_->readByte();
  }
}

int64_t Direct8::get(int const index) { return values[index] & 0xFFLL; }

void Direct8::set(int const index, int64_t const value)
{
  values[index] = static_cast<char>(value);
}

int64_t Direct8::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
             2 * RamUsageEstimator::NUM_BYTES_INT +
             RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(values);
}

void Direct8::clear() { Arrays::fill(values, static_cast<char>(0LL)); }

int Direct8::get(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int gets = min(valueCount - index, len);
  for (int i = index, o = off, end = index + gets; i < end; ++i, ++o) {
    arr[o] = values[i] & 0xFFLL;
  }
  return gets;
}

int Direct8::set(int index, std::deque<int64_t> &arr, int off, int len)
{
  assert((len > 0, L"len must be > 0 (got " + to_wstring(len) + L")"));
  assert(index >= 0 && index < valueCount);
  assert(off + len <= arr.size());

  constexpr int sets = min(valueCount - index, len);
  for (int i = index, o = off, end = index + sets; i < end; ++i, ++o) {
    values[i] = static_cast<char>(arr[o]);
  }
  return sets;
}

void Direct8::fill(int fromIndex, int toIndex, int64_t val)
{
  assert(val == (val & 0xFFLL));
  Arrays::fill(values, fromIndex, toIndex, static_cast<char>(val));
}
} // namespace org::apache::lucene::util::packed