using namespace std;

#include "SentinelIntSet.h"

namespace org::apache::lucene::util
{

SentinelIntSet::SentinelIntSet(int size, int emptyVal) : emptyVal(emptyVal)
{
  int tsize =
      max(org::apache::lucene::util::BitUtil::nextHighestPowerOfTwo(size), 1);
  rehashCount = tsize - (tsize >> 2);
  if (size >= rehashCount) { // should be able to hold "size" w/o re-hashing
    tsize <<= 1;
    rehashCount = tsize - (tsize >> 2);
  }
  keys = std::deque<int>(tsize);
  if (emptyVal != 0) {
    clear();
  }
}

void SentinelIntSet::clear()
{
  Arrays::fill(keys, emptyVal);
  count = 0;
}

int SentinelIntSet::hash(int key) { return key; }

int SentinelIntSet::size() { return count; }

int SentinelIntSet::getSlot(int key)
{
  assert(key != emptyVal);
  int h = hash(key);
  int s = h & (keys.size() - 1);
  if (keys[s] == key || keys[s] == emptyVal) {
    return s;
  }

  int increment = (h >> 7) | 1;
  do {
    s = (s + increment) & (keys.size() - 1);
  } while (keys[s] != key && keys[s] != emptyVal);
  return s;
}

int SentinelIntSet::find(int key)
{
  assert(key != emptyVal);
  int h = hash(key);
  int s = h & (keys.size() - 1);
  if (keys[s] == key) {
    return s;
  }
  if (keys[s] == emptyVal) {
    return -s - 1;
  }

  int increment = (h >> 7) | 1;
  for (;;) {
    s = (s + increment) & (keys.size() - 1);
    if (keys[s] == key) {
      return s;
    }
    if (keys[s] == emptyVal) {
      return -s - 1;
    }
  }
}

bool SentinelIntSet::exists(int key) { return find(key) >= 0; }

int SentinelIntSet::put(int key)
{
  int s = find(key);
  if (s < 0) {
    count++;
    if (count >= rehashCount) {
      rehash();
      s = getSlot(key);
    } else {
      s = -s - 1;
    }
    keys[s] = key;
  }
  return s;
}

void SentinelIntSet::rehash()
{
  int newSize = keys.size() << 1;
  std::deque<int> oldKeys = keys;
  keys = std::deque<int>(newSize);
  if (emptyVal != 0) {
    Arrays::fill(keys, emptyVal);
  }

  for (auto key : oldKeys) {
    if (key == emptyVal) {
      continue;
    }
    int newSlot = getSlot(key);
    keys[newSlot] = key;
  }
  rehashCount = newSize - (newSize >> 2);
}

int64_t SentinelIntSet::ramBytesUsed()
{
  return RamUsageEstimator::alignObjectSize(
             Integer::BYTES * 3 + RamUsageEstimator::NUM_BYTES_OBJECT_REF) +
         RamUsageEstimator::sizeOf(keys);
}
} // namespace org::apache::lucene::util