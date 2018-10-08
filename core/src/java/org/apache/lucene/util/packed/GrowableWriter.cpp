using namespace std;

#include "GrowableWriter.h"

namespace org::apache::lucene::util::packed
{
using DataOutput = org::apache::lucene::store::DataOutput;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

GrowableWriter::GrowableWriter(int startBitsPerValue, int valueCount,
                               float acceptableOverheadRatio)
    : acceptableOverheadRatio(acceptableOverheadRatio)
{
  current = PackedInts::getMutable(valueCount, startBitsPerValue,
                                   this->acceptableOverheadRatio);
  currentMask = mask(current->getBitsPerValue());
}

int64_t GrowableWriter::mask(int bitsPerValue)
{
  return bitsPerValue == 64 ? ~0LL : PackedInts::maxValue(bitsPerValue);
}

int64_t GrowableWriter::get(int index) { return current->get(index); }

int GrowableWriter::size() { return current->size(); }

int GrowableWriter::getBitsPerValue() { return current->getBitsPerValue(); }

shared_ptr<PackedInts::Mutable> GrowableWriter::getMutable() { return current; }

void GrowableWriter::ensureCapacity(int64_t value)
{
  if ((value & currentMask) == value) {
    return;
  }
  constexpr int bitsRequired = PackedInts::unsignedBitsRequired(value);
  assert(bitsRequired > current->getBitsPerValue());
  constexpr int valueCount = size();
  shared_ptr<PackedInts::Mutable> next =
      PackedInts::getMutable(valueCount, bitsRequired, acceptableOverheadRatio);
  PackedInts::copy(current, 0, next, 0, valueCount,
                   PackedInts::DEFAULT_BUFFER_SIZE);
  current = next;
  currentMask = mask(current->getBitsPerValue());
}

void GrowableWriter::set(int index, int64_t value)
{
  ensureCapacity(value);
  current->set(index, value);
}

void GrowableWriter::clear() { current->clear(); }

shared_ptr<GrowableWriter> GrowableWriter::resize(int newSize)
{
  shared_ptr<GrowableWriter> next = make_shared<GrowableWriter>(
      getBitsPerValue(), newSize, acceptableOverheadRatio);
  constexpr int limit = min(size(), newSize);
  PackedInts::copy(current, 0, next, 0, limit, PackedInts::DEFAULT_BUFFER_SIZE);
  return next;
}

int GrowableWriter::get(int index, std::deque<int64_t> &arr, int off,
                        int len)
{
  return current->get(index, arr, off, len);
}

int GrowableWriter::set(int index, std::deque<int64_t> &arr, int off,
                        int len)
{
  int64_t max = 0;
  for (int i = off, end = off + len; i < end; ++i) {
    // bitwise or is nice because either all values are positive and the
    // or-ed result will require as many bits per value as the max of the
    // values, or one of them is negative and the result will be negative,
    // forcing GrowableWriter to use 64 bits per value
    max |= arr[i];
  }
  ensureCapacity(max);
  return current->set(index, arr, off, len);
}

void GrowableWriter::fill(int fromIndex, int toIndex, int64_t val)
{
  ensureCapacity(val);
  current->fill(fromIndex, toIndex, val);
}

int64_t GrowableWriter::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
             RamUsageEstimator::NUM_BYTES_OBJECT_REF + Long::BYTES +
             Float::BYTES) +
         current->ramBytesUsed();
}

void GrowableWriter::save(shared_ptr<DataOutput> out) 
{
  current->save(out);
}
} // namespace org::apache::lucene::util::packed