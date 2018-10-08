using namespace std;

#include "DeltaPackedLongValues.h"

namespace org::apache::lucene::util::packed
{
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using Reader = org::apache::lucene::util::packed::PackedInts::Reader;

DeltaPackedLongValues::DeltaPackedLongValues(
    int pageShift, int pageMask, std::deque<std::shared_ptr<Reader>> &values,
    std::deque<int64_t> &mins, int64_t size, int64_t ramBytesUsed)
    : PackedLongValues(pageShift, pageMask, values, size, ramBytesUsed),
      mins(mins)
{
  assert(values.size() == mins.size());
}

int64_t DeltaPackedLongValues::get(int block, int element)
{
  return mins[block] + values[block]->get(element);
}

int DeltaPackedLongValues::decodeBlock(int block, std::deque<int64_t> &dest)
{
  constexpr int count = PackedLongValues::decodeBlock(block, dest);
  constexpr int64_t min = mins[block];
  for (int i = 0; i < count; ++i) {
    dest[i] += min;
  }
  return count;
}

DeltaPackedLongValues::Builder::Builder(int pageSize,
                                        float acceptableOverheadRatio)
    : PackedLongValues::Builder(pageSize, acceptableOverheadRatio)
{
  mins = std::deque<int64_t>(outerInstance->values->size());
  ramBytesUsed += RamUsageEstimator::sizeOf(mins);
}

int64_t DeltaPackedLongValues::Builder::baseRamBytesUsed()
{
  return BASE_RAM_BYTES_USED;
}

shared_ptr<DeltaPackedLongValues> DeltaPackedLongValues::Builder::build()
{
  finish();
  pending = nullptr;
  std::deque<std::shared_ptr<PackedInts::Reader>> values =
      Arrays::copyOf(this->values, valuesOff);
  const std::deque<int64_t> mins = Arrays::copyOf(this->mins, valuesOff);
  constexpr int64_t ramBytesUsed =
      DeltaPackedLongValues::BASE_RAM_BYTES_USED +
      RamUsageEstimator::sizeOf(values) + RamUsageEstimator::sizeOf(mins);
  return make_shared<DeltaPackedLongValues>(outerInstance->pageShift,
                                            outerInstance->pageMask, values,
                                            mins, size, ramBytesUsed);
}

void DeltaPackedLongValues::Builder::pack(std::deque<int64_t> &values,
                                          int numValues, int block,
                                          float acceptableOverheadRatio)
{
  int64_t min = values[0];
  for (int i = 1; i < numValues; ++i) {
    min = min(min, values[i]);
  }
  for (int i = 0; i < numValues; ++i) {
    values[i] -= min;
  }
  PackedLongValues::Builder::pack(values, numValues, block,
                                  acceptableOverheadRatio);
  mins[block] = min;
}

void DeltaPackedLongValues::Builder::grow(int newBlockCount)
{
  PackedLongValues::Builder::grow(newBlockCount);
  ramBytesUsed -= RamUsageEstimator::sizeOf(mins);
  mins = Arrays::copyOf(mins, newBlockCount);
  ramBytesUsed += RamUsageEstimator::sizeOf(mins);
}
} // namespace org::apache::lucene::util::packed