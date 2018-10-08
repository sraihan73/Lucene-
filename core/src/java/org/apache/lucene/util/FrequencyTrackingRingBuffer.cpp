using namespace std;

#include "FrequencyTrackingRingBuffer.h"

namespace org::apache::lucene::util
{

FrequencyTrackingRingBuffer::FrequencyTrackingRingBuffer(int maxSize,
                                                         int sentinel)
    : maxSize(maxSize), buffer(std::deque<int>(maxSize)),
      frequencies(make_shared<IntBag>(maxSize))
{
  if (maxSize < 2) {
    throw invalid_argument(L"maxSize must be at least 2");
  }
  position = 0;

  Arrays::fill(buffer, sentinel);
  for (int i = 0; i < maxSize; ++i) {
    frequencies->add(sentinel);
  }
  assert(frequencies->frequency(sentinel) == maxSize);
}

int64_t FrequencyTrackingRingBuffer::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + frequencies->ramBytesUsed() +
         RamUsageEstimator::sizeOf(buffer);
}

void FrequencyTrackingRingBuffer::add(int i)
{
  // remove the previous value
  constexpr int removed = buffer[position];
  constexpr bool removedFromBag = frequencies->remove(removed);
  assert(removedFromBag);
  // add the new value
  buffer[position] = i;
  frequencies->add(i);
  // increment the position
  position += 1;
  if (position == maxSize) {
    position = 0;
  }
}

int FrequencyTrackingRingBuffer::frequency(int key)
{
  return frequencies->frequency(key);
}

unordered_map<int, int> FrequencyTrackingRingBuffer::asFrequencyMap()
{
  return frequencies->asMap();
}

FrequencyTrackingRingBuffer::IntBag::IntBag(int maxSize)
    : keys(std::deque<int>(capacity)), freqs(std::deque<int>(capacity)),
      mask(capacity - 1)
{
  // load factor of 2/3
  int capacity = max(2, maxSize * 3 / 2);
  // round up to the next power of two
  capacity = Integer::highestOneBit(capacity - 1) << 1;
  assert(capacity > maxSize);
}

int64_t FrequencyTrackingRingBuffer::IntBag::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(keys) +
         RamUsageEstimator::sizeOf(freqs);
}

int FrequencyTrackingRingBuffer::IntBag::frequency(int key)
{
  for (int slot = key & mask;; slot = (slot + 1) & mask) {
    if (keys[slot] == key) {
      return freqs[slot];
    } else if (freqs[slot] == 0) {
      return 0;
    }
  }
}

int FrequencyTrackingRingBuffer::IntBag::add(int key)
{
  for (int slot = key & mask;; slot = (slot + 1) & mask) {
    if (freqs[slot] == 0) {
      keys[slot] = key;
      return freqs[slot] = 1;
    } else if (keys[slot] == key) {
      return ++freqs[slot];
    }
  }
}

bool FrequencyTrackingRingBuffer::IntBag::remove(int key)
{
  for (int slot = key & mask;; slot = (slot + 1) & mask) {
    if (freqs[slot] == 0) {
      // no such key in the bag
      return false;
    } else if (keys[slot] == key) {
      constexpr int newFreq = --freqs[slot];
      if (newFreq == 0) { // removed
        relocateAdjacentKeys(slot);
      }
      return true;
    }
  }
}

void FrequencyTrackingRingBuffer::IntBag::relocateAdjacentKeys(int freeSlot)
{
  for (int slot = (freeSlot + 1) & mask;; slot = (slot + 1) & mask) {
    constexpr int freq = freqs[slot];
    if (freq == 0) {
      // end of the collision chain, we're done
      break;
    }
    constexpr int key = keys[slot];
    // the slot where <code>key</code> should be if there were no collisions
    constexpr int expectedSlot = key & mask;
    // if the free slot is between the expected slot and the slot where the
    // key is, then we can relocate there
    if (between(expectedSlot, slot, freeSlot)) {
      keys[freeSlot] = key;
      freqs[freeSlot] = freq;
      // slot is the new free slot
      freqs[slot] = 0;
      freeSlot = slot;
    }
  }
}

bool FrequencyTrackingRingBuffer::IntBag::between(int chainStart, int chainEnd,
                                                  int slot)
{
  if (chainStart <= chainEnd) {
    return chainStart <= slot && slot <= chainEnd;
  } else {
    // the chain is across the end of the array
    return slot >= chainStart || slot <= chainEnd;
  }
}

unordered_map<int, int> FrequencyTrackingRingBuffer::IntBag::asMap()
{
  unordered_map<int, int> map_obj = unordered_map<int, int>();
  for (int i = 0; i < keys.size(); ++i) {
    if (freqs[i] > 0) {
      map_obj.emplace(keys[i], freqs[i]);
    }
  }
  return map_obj;
}
} // namespace org::apache::lucene::util