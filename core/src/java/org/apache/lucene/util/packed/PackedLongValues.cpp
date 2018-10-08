using namespace std;

#include "PackedLongValues.h"

namespace org::apache::lucene::util::packed
{
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;
using Accountable = org::apache::lucene::util::Accountable;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using LongValues = org::apache::lucene::util::LongValues;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

shared_ptr<PackedLongValues::Builder>
PackedLongValues::packedBuilder(int pageSize, float acceptableOverheadRatio)
{
  return make_shared<PackedLongValues::Builder>(pageSize,
                                                acceptableOverheadRatio);
}

shared_ptr<PackedLongValues::Builder>
PackedLongValues::packedBuilder(float acceptableOverheadRatio)
{
  return packedBuilder(DEFAULT_PAGE_SIZE, acceptableOverheadRatio);
}

shared_ptr<PackedLongValues::Builder>
PackedLongValues::deltaPackedBuilder(int pageSize,
                                     float acceptableOverheadRatio)
{
  return make_shared<DeltaPackedLongValues::Builder>(pageSize,
                                                     acceptableOverheadRatio);
}

shared_ptr<PackedLongValues::Builder>
PackedLongValues::deltaPackedBuilder(float acceptableOverheadRatio)
{
  return deltaPackedBuilder(DEFAULT_PAGE_SIZE, acceptableOverheadRatio);
}

shared_ptr<PackedLongValues::Builder>
PackedLongValues::monotonicBuilder(int pageSize, float acceptableOverheadRatio)
{
  return make_shared<MonotonicLongValues::Builder>(pageSize,
                                                   acceptableOverheadRatio);
}

shared_ptr<PackedLongValues::Builder>
PackedLongValues::monotonicBuilder(float acceptableOverheadRatio)
{
  return monotonicBuilder(DEFAULT_PAGE_SIZE, acceptableOverheadRatio);
}

PackedLongValues::PackedLongValues(
    int pageShift, int pageMask,
    std::deque<std::shared_ptr<PackedInts::Reader>> &values, int64_t size,
    int64_t ramBytesUsed)
    : values(values), pageShift(pageShift), pageMask(pageMask), size(size),
      ramBytesUsed(ramBytesUsed)
{
}

int64_t PackedLongValues::size() { return size_; }

int PackedLongValues::decodeBlock(int block, std::deque<int64_t> &dest)
{
  shared_ptr<PackedInts::Reader> *const vals = values[block];
  constexpr int size = vals->size();
  for (int k = 0; k < size;) {
    k += vals->get(k, dest, k, size - k);
  }
  return size;
}

int64_t PackedLongValues::get(int block, int element)
{
  return values[block]->get(element);
}

int64_t PackedLongValues::get(int64_t index)
{
  assert(index >= 0 && index < size());
  constexpr int block = static_cast<int>(index >> pageShift);
  constexpr int element = static_cast<int>(index & pageMask);
  return get(block, element);
}

int64_t PackedLongValues::ramBytesUsed() { return ramBytesUsed_; }

shared_ptr<Iterator> PackedLongValues::iterator()
{
  return make_shared<Iterator>(shared_from_this());
}

PackedLongValues::Iterator::Iterator(shared_ptr<PackedLongValues> outerInstance)
    : currentValues(std::deque<int64_t>(outerInstance->pageMask + 1)),
      outerInstance(outerInstance)
{
  vOff = pOff = 0;
  fillBlock();
}

void PackedLongValues::Iterator::fillBlock()
{
  if (vOff == outerInstance->values.size()) {
    currentCount = 0;
  } else {
    currentCount = outerInstance->decodeBlock(vOff, currentValues);
    assert(currentCount > 0);
  }
}

bool PackedLongValues::Iterator::hasNext() { return pOff < currentCount; }

int64_t PackedLongValues::Iterator::next()
{
  assert(hasNext());
  int64_t result = currentValues[pOff++];
  if (pOff == currentCount) {
    vOff += 1;
    pOff = 0;
    fillBlock();
  }
  return result;
}

PackedLongValues::Builder::Builder(int pageSize, float acceptableOverheadRatio)
    : pageShift(checkBlockSize(pageSize, MIN_PAGE_SIZE, MAX_PAGE_SIZE)),
      pageMask(pageSize - 1), acceptableOverheadRatio(acceptableOverheadRatio)
{
  values = std::deque<std::shared_ptr<PackedInts::Reader>>(INITIAL_PAGE_COUNT);
  pending = std::deque<int64_t>(pageSize);
  valuesOff = 0;
  pendingOff = 0;
  size_ = 0;
  ramBytesUsed_ = baseRamBytesUsed() + RamUsageEstimator::sizeOf(pending) +
                  RamUsageEstimator::shallowSizeOf(values);
}

shared_ptr<PackedLongValues> PackedLongValues::Builder::build()
{
  finish();
  pending.clear();
  std::deque<std::shared_ptr<PackedInts::Reader>> values =
      Arrays::copyOf(this->values, valuesOff);
  constexpr int64_t ramBytesUsed =
      PackedLongValues::BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(values);
  return make_shared<PackedLongValues>(pageShift, pageMask, values, size_,
                                       ramBytesUsed);
}

int64_t PackedLongValues::Builder::baseRamBytesUsed()
{
  return BASE_RAM_BYTES_USED;
}

int64_t PackedLongValues::Builder::ramBytesUsed() { return ramBytesUsed_; }

int64_t PackedLongValues::Builder::size() { return size_; }

shared_ptr<Builder> PackedLongValues::Builder::add(int64_t l)
{
  if (pending.empty()) {
    throw make_shared<IllegalStateException>(L"Cannot be reused after build()");
  }
  if (pendingOff == pending.size()) {
    // check size
    if (values.size() == valuesOff) {
      constexpr int newLength = ArrayUtil::oversize(valuesOff + 1, 8);
      grow(newLength);
    }
    pack();
  }
  pending[pendingOff++] = l;
  size_ += 1;
  return shared_from_this();
}

void PackedLongValues::Builder::finish()
{
  if (pendingOff > 0) {
    if (values.size() == valuesOff) {
      grow(valuesOff + 1);
    }
    pack();
  }
}

void PackedLongValues::Builder::pack()
{
  pack(pending, pendingOff, valuesOff, acceptableOverheadRatio);
  ramBytesUsed_ += values[valuesOff]->ramBytesUsed();
  valuesOff += 1;
  // reset pending buffer
  pendingOff = 0;
}

void PackedLongValues::Builder::pack(std::deque<int64_t> &values,
                                     int numValues, int block,
                                     float acceptableOverheadRatio)
{
  assert(numValues > 0);
  // compute max delta
  int64_t minValue = values[0];
  int64_t maxValue = values[0];
  for (int i = 1; i < numValues; ++i) {
    minValue = min(minValue, values[i]);
    maxValue = max(maxValue, values[i]);
  }

  // build a new packed reader
  if (minValue == 0 && maxValue == 0) {
    this->values[block] = make_shared<PackedInts::NullReader>(numValues);
  } else {
    constexpr int bitsRequired =
        minValue < 0 ? 64 : PackedInts::bitsRequired(maxValue);
    shared_ptr<PackedInts::Mutable> *const mutable_ = PackedInts::getMutable(
        numValues, bitsRequired, acceptableOverheadRatio);
    for (int i = 0; i < numValues;) {
      i += mutable_->set(i, values, i, numValues - i);
    }
    this->values[block] = mutable_;
  }
}

void PackedLongValues::Builder::grow(int newBlockCount)
{
  ramBytesUsed_ -= RamUsageEstimator::shallowSizeOf(values);
  values = Arrays::copyOf(values, newBlockCount);
  ramBytesUsed_ += RamUsageEstimator::shallowSizeOf(values);
}
} // namespace org::apache::lucene::util::packed