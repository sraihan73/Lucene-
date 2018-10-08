using namespace std;

#include "MonotonicLongValues.h"

namespace org::apache::lucene::util::packed
{
//    import static
//    org.apache.lucene.util.packed.MonotonicBlockPackedReader.expected;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using Reader = org::apache::lucene::util::packed::PackedInts::Reader;

MonotonicLongValues::MonotonicLongValues(
    int pageShift, int pageMask, std::deque<std::shared_ptr<Reader>> &values,
    std::deque<int64_t> &mins, std::deque<float> &averages, int64_t size,
    int64_t ramBytesUsed)
    : DeltaPackedLongValues(pageShift, pageMask, values, mins, size,
                            ramBytesUsed),
      averages(averages)
{
  assert(values.size() == averages.size());
}

int64_t MonotonicLongValues::get(int block, int element)
{
  return expected(mins[block], averages[block], element) +
         values[block]->get(element);
}

int MonotonicLongValues::decodeBlock(int block, std::deque<int64_t> &dest)
{
  constexpr int count = DeltaPackedLongValues::decodeBlock(block, dest);
  constexpr float average = averages[block];
  for (int i = 0; i < count; ++i) {
    dest[i] += expected(0, average, i);
  }
  return count;
}

MonotonicLongValues::Builder::Builder(int pageSize,
                                      float acceptableOverheadRatio)
    : DeltaPackedLongValues::Builder(pageSize, acceptableOverheadRatio)
{
  averages = std::deque<float>(outerInstance->values->size());
  ramBytesUsed += RamUsageEstimator::sizeOf(averages);
}

int64_t MonotonicLongValues::Builder::baseRamBytesUsed()
{
  return BASE_RAM_BYTES_USED;
}

shared_ptr<MonotonicLongValues> MonotonicLongValues::Builder::build()
{
  finish();
  pending = nullptr;
  std::deque<std::shared_ptr<PackedInts::Reader>> values =
      Arrays::copyOf(this->values, valuesOff);
  const std::deque<int64_t> mins = Arrays::copyOf(this->mins, valuesOff);
  const std::deque<float> averages = Arrays::copyOf(this->averages, valuesOff);
  constexpr int64_t ramBytesUsed = MonotonicLongValues::BASE_RAM_BYTES_USED +
                                     RamUsageEstimator::sizeOf(values) +
                                     RamUsageEstimator::sizeOf(mins) +
                                     RamUsageEstimator::sizeOf(averages);
  return make_shared<MonotonicLongValues>(outerInstance->pageShift,
                                          outerInstance->pageMask, values, mins,
                                          averages, size, ramBytesUsed);
}

void MonotonicLongValues::Builder::pack(std::deque<int64_t> &values,
                                        int numValues, int block,
                                        float acceptableOverheadRatio)
{
  constexpr float average =
      numValues == 1 ? 0
                     : static_cast<float>(values[numValues - 1] - values[0]) /
                           (numValues - 1);
  for (int i = 0; i < numValues; ++i) {
    values[i] -= expected(0, average, i);
  }
  DeltaPackedLongValues::Builder::pack(values, numValues, block,
                                       acceptableOverheadRatio);
  averages[block] = average;
}

void MonotonicLongValues::Builder::grow(int newBlockCount)
{
  DeltaPackedLongValues::Builder::grow(newBlockCount);
  ramBytesUsed -= RamUsageEstimator::sizeOf(averages);
  averages = Arrays::copyOf(averages, newBlockCount);
  ramBytesUsed += RamUsageEstimator::sizeOf(averages);
}
} // namespace org::apache::lucene::util::packed