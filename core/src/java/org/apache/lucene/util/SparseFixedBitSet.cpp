using namespace std;

#include "SparseFixedBitSet.h"

namespace org::apache::lucene::util
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

int SparseFixedBitSet::blockCount(int length)
{
  int blockCount = static_cast<int>(static_cast<unsigned int>(length) >> 12);
  if ((blockCount << 12) < length) {
    ++blockCount;
  }
  assert(blockCount << 12) >= length;
  return blockCount;
}

SparseFixedBitSet::SparseFixedBitSet(int length)
    : indices(std::deque<int64_t>(blockCount)),
      bits(std::deque<std::deque<int64_t>>(blockCount)), length(length)
{
  if (length < 1) {
    throw invalid_argument(L"length needs to be >= 1");
  }
  constexpr int blockCount = SparseFixedBitSet::blockCount(length);
  ramBytesUsed_ = BASE_RAM_BYTES_USED +
                  RamUsageEstimator::shallowSizeOf(indices) +
                  RamUsageEstimator::shallowSizeOf(bits);
}

int SparseFixedBitSet::length() { return length_; }

bool SparseFixedBitSet::consistent(int index)
{
  assert((index >= 0 && index < length_,
          L"index=" + to_wstring(index) + L",length=" + to_wstring(length_)));
  return true;
}

int SparseFixedBitSet::cardinality()
{
  int cardinality = 0;
  for (auto bitArray : bits) {
    if (bitArray.size() > 0) {
      for (auto bits : bitArray) {
        cardinality += Long::bitCount(bits);
      }
    }
  }
  return cardinality;
}

int SparseFixedBitSet::approximateCardinality()
{
  // we are assuming that bits are uniformly set and use the linear counting
  // algorithm to estimate the number of bits that are set based on the number
  // of longs that are different from zero
  constexpr int totalLongs =
      static_cast<int>(static_cast<unsigned int>((length_ + 63)) >>
                       6); // total number of longs in the space
  assert(totalLongs >= nonZeroLongCount);
  constexpr int zeroLongs =
      totalLongs - nonZeroLongCount; // number of longs that are zeros
  // No need to guard against division by zero, it will return +Infinity and
  // things will work as expected
  constexpr int64_t estimate =
      round(totalLongs * log(static_cast<double>(totalLongs) / zeroLongs));
  return static_cast<int>(min(length_, estimate));
}

bool SparseFixedBitSet::get(int i)
{
  assert(consistent(i));
  constexpr int i4096 = static_cast<int>(static_cast<unsigned int>(i) >> 12);
  constexpr int64_t index = indices[i4096];
  constexpr int i64 = static_cast<int>(static_cast<unsigned int>(i) >> 6);
  // first check the index, if the i64-th bit is not set, then i is not set
  // note: this relies on the fact that shifts are mod 64 in java
  if ((index & (1LL << i64)) == 0) {
    return false;
  }

  // if it is set, then we count the number of bits that are set on the right
  // of i64, and that gives us the index of the long that stores the bits we
  // are interested in
  constexpr int64_t bits =
      this->bits[i4096][Long::bitCount(index & ((1LL << i64) - 1))];
  return (bits & (1LL << i)) != 0;
}

int SparseFixedBitSet::oversize(int s)
{
  int newSize = s + (static_cast<int>(static_cast<unsigned int>(s) >> 1));
  if (newSize > 50) {
    newSize = 64;
  }
  return newSize;
}

void SparseFixedBitSet::set(int i)
{
  assert(consistent(i));
  constexpr int i4096 = static_cast<int>(static_cast<unsigned int>(i) >> 12);
  constexpr int64_t index = indices[i4096];
  constexpr int i64 = static_cast<int>(static_cast<unsigned int>(i) >> 6);
  if ((index & (1LL << i64)) != 0) {
    // in that case the sub 64-bits block we are interested in already exists,
    // we just need to set a bit in an existing long: the number of ones on
    // the right of i64 gives us the index of the long we need to update
    bits[i4096][Long::bitCount(index & ((1LL << i64) - 1))] |=
        1LL << i; // shifts are mod 64 in java
  } else if (index == 0) {
    // if the index is 0, it means that we just found a block of 4096 bits
    // that has no bit that is set yet. So let's initialize a new block:
    insertBlock(i4096, i64, i);
  } else {
    // in that case we found a block of 4096 bits that has some values, but
    // the sub-block of 64 bits that we are interested in has no value yet,
    // so we need to insert a new long
    insertLong(i4096, i64, i, index);
  }
}

void SparseFixedBitSet::insertBlock(int i4096, int i64, int i)
{
  indices[i4096] = 1LL << i64; // shifts are mod 64 in java
  assert(bits[i4096] == nullptr);
  bits[i4096] = std::deque<int64_t>{1LL << i}; // shifts are mod 64 in java
  ++nonZeroLongCount;
  ramBytesUsed_ += SINGLE_ELEMENT_ARRAY_BYTES_USED;
}

void SparseFixedBitSet::insertLong(int i4096, int i64, int i, int64_t index)
{
  indices[i4096] |= 1LL << i64; // shifts are mod 64 in java
  // we count the number of bits that are set on the right of i64
  // this gives us the index at which to perform the insertion
  constexpr int o = Long::bitCount(index & ((1LL << i64) - 1));
  const std::deque<int64_t> bitArray = bits[i4096];
  if (bitArray[bitArray.size() - 1] == 0) {
    // since we only store non-zero longs, if the last value is 0, it means
    // that we alreay have extra space, make use of it
    System::arraycopy(bitArray, o, bitArray, o + 1, bitArray.size() - o - 1);
    bitArray[o] = 1LL << i;
  } else {
    // we don't have extra space so we need to resize to insert the new long
    constexpr int newSize = oversize(bitArray.size() + 1);
    const std::deque<int64_t> newBitArray = std::deque<int64_t>(newSize);
    System::arraycopy(bitArray, 0, newBitArray, 0, o);
    newBitArray[o] = 1LL << i;
    System::arraycopy(bitArray, o, newBitArray, o + 1, bitArray.size() - o);
    bits[i4096] = newBitArray;
    ramBytesUsed_ += RamUsageEstimator::sizeOf(newBitArray) -
                     RamUsageEstimator::sizeOf(bitArray);
  }
  ++nonZeroLongCount;
}

void SparseFixedBitSet::clear(int i)
{
  assert(consistent(i));
  constexpr int i4096 = static_cast<int>(static_cast<unsigned int>(i) >> 12);
  constexpr int i64 = static_cast<int>(static_cast<unsigned int>(i) >> 6);
  and(i4096, i64, ~(1LL << i));
}

void SparseFixedBitSet::and(int i4096, int i64, int64_t mask)
{
  constexpr int64_t index = indices[i4096];
  if ((index & (1LL << i64)) != 0) {
    // offset of the long bits we are interested in in the array
    constexpr int o = Long::bitCount(index & ((1LL << i64) - 1));
    int64_t bits = this->bits[i4096][o] & mask;
    if (bits == 0) {
      removeLong(i4096, i64, index, o);
    } else {
      this->bits[i4096][o] = bits;
    }
  }
}

void SparseFixedBitSet::removeLong(int i4096, int i64, int64_t index, int o)
{
  index &= ~(1LL << i64);
  indices[i4096] = index;
  if (index == 0) {
    // release memory, there is nothing in this block anymore
    this->bits[i4096].smartpointerreset();
  } else {
    constexpr int length = Long::bitCount(index);
    const std::deque<int64_t> bitArray = bits[i4096];
    System::arraycopy(bitArray, o + 1, bitArray, o, length - o);
    bitArray[length] = 0LL;
  }
  nonZeroLongCount -= 1;
}

void SparseFixedBitSet::clear(int from, int to)
{
  assert(from >= 0);
  assert(to <= length_);
  if (from >= to) {
    return;
  }
  constexpr int firstBlock =
      static_cast<int>(static_cast<unsigned int>(from) >> 12);
  constexpr int lastBlock =
      static_cast<int>(static_cast<unsigned int>((to - 1)) >> 12);
  if (firstBlock == lastBlock) {
    clearWithinBlock(firstBlock, from & MASK_4096, (to - 1) & MASK_4096);
  } else {
    clearWithinBlock(firstBlock, from & MASK_4096, MASK_4096);
    for (int i = firstBlock + 1; i < lastBlock; ++i) {
      nonZeroLongCount -= Long::bitCount(indices[i]);
      indices[i] = 0;
      bits[i].smartpointerreset();
    }
    clearWithinBlock(lastBlock, 0, (to - 1) & MASK_4096);
  }
}

int64_t SparseFixedBitSet::mask(int from, int to)
{
  return ((1LL << (to - from) << 1) - 1) << from;
}

void SparseFixedBitSet::clearWithinBlock(int i4096, int from, int to)
{
  int firstLong = static_cast<int>(static_cast<unsigned int>(from) >> 6);
  int lastLong = static_cast<int>(static_cast<unsigned int>(to) >> 6);

  if (firstLong == lastLong) {
    and(i4096, firstLong, ~mask(from, to));
  } else {
    assert(firstLong < lastLong);
    and(i4096, lastLong, ~mask(0, to));
    for (int i = lastLong - 1; i >= firstLong + 1; --i) {
      and(i4096, i, 0LL);
    }
    and(i4096, firstLong, ~mask(from, 63));
  }
}

int SparseFixedBitSet::firstDoc(int i4096)
{
  int64_t index = 0;
  while (i4096 < indices.size()) {
    index = indices[i4096];
    if (index != 0) {
      constexpr int i64 = Long::numberOfTrailingZeros(index);
      return (i4096 << 12) | (i64 << 6) |
             Long::numberOfTrailingZeros(bits[i4096][0]);
    }
    i4096 += 1;
  }
  return DocIdSetIterator::NO_MORE_DOCS;
}

int SparseFixedBitSet::nextSetBit(int i)
{
  assert(i < length_);
  constexpr int i4096 = static_cast<int>(static_cast<unsigned int>(i) >> 12);
  constexpr int64_t index = indices[i4096];
  const std::deque<int64_t> bitArray = this->bits[i4096];
  int i64 = static_cast<int>(static_cast<unsigned int>(i) >> 6);
  int o = Long::bitCount(index & ((1LL << i64) - 1));
  if ((index & (1LL << i64)) != 0) {
    // There is at least one bit that is set in the current long, check if
    // one of them is after i
    constexpr int64_t bits = static_cast<int64_t>(
        static_cast<uint64_t>(bitArray[o]) >> i); // shifts are mod 64
    if (bits != 0) {
      return i + Long::numberOfTrailingZeros(bits);
    }
    o += 1;
  }
  constexpr int64_t indexBits =
      static_cast<int64_t>(static_cast<uint64_t>(index) >> i64 >>) >
      1;
  if (indexBits == 0) {
    // no more bits are set in the current block of 4096 bits, go to the next
    // one
    return firstDoc(i4096 + 1);
  }
  // there are still set bits
  i64 += 1 + Long::numberOfTrailingZeros(indexBits);
  constexpr int64_t bits = bitArray[o];
  return (i64 << 6) | Long::numberOfTrailingZeros(bits);
}

int SparseFixedBitSet::lastDoc(int i4096)
{
  int64_t index;
  while (i4096 >= 0) {
    index = indices[i4096];
    if (index != 0) {
      constexpr int i64 = 63 - Long::numberOfLeadingZeros(index);
      constexpr int64_t bits = this->bits[i4096][Long::bitCount(index) - 1];
      return (i4096 << 12) | (i64 << 6) |
             (63 - Long::numberOfLeadingZeros(bits));
    }
    i4096 -= 1;
  }
  return -1;
}

int SparseFixedBitSet::prevSetBit(int i)
{
  assert(i >= 0);
  constexpr int i4096 = static_cast<int>(static_cast<unsigned int>(i) >> 12);
  constexpr int64_t index = indices[i4096];
  const std::deque<int64_t> bitArray = this->bits[i4096];
  int i64 = static_cast<int>(static_cast<unsigned int>(i) >> 6);
  constexpr int64_t indexBits = index & ((1LL << i64) - 1);
  constexpr int o = Long::bitCount(indexBits);
  if ((index & (1LL << i64)) != 0) {
    // There is at least one bit that is set in the same long, check if there
    // is one bit that is set that is lower than i
    constexpr int64_t bits = bitArray[o] & ((1LL << i << 1) - 1);
    if (bits != 0) {
      return (i64 << 6) | (63 - Long::numberOfLeadingZeros(bits));
    }
  }
  if (indexBits == 0) {
    // no more bits are set in this block, go find the last bit in the
    // previous block
    return lastDoc(i4096 - 1);
  }
  // go to the previous long
  i64 = 63 - Long::numberOfLeadingZeros(indexBits);
  constexpr int64_t bits = bitArray[o - 1];
  return (i4096 << 12) | (i64 << 6) | (63 - Long::numberOfLeadingZeros(bits));
}

int64_t SparseFixedBitSet::longBits(int64_t index,
                                      std::deque<int64_t> &bits, int i64)
{
  if ((index & (1LL << i64)) == 0) {
    return 0LL;
  } else {
    return bits[Long::bitCount(index & ((1LL << i64) - 1))];
  }
}

void SparseFixedBitSet:: or (int const i4096, int64_t const index,
                             std::deque<int64_t> &bits, int nonZeroLongCount)
{
  assert(Long::bitCount(index) == nonZeroLongCount);
  constexpr int64_t currentIndex = indices[i4096];
  if (currentIndex == 0) {
    // fast path: if we currently have nothing in the block, just copy the data
    // this especially happens all the time if you call OR on an empty set
    indices[i4096] = index;
    this->bits[i4096] = Arrays::copyOf(bits, nonZeroLongCount);
    this->nonZeroLongCount += nonZeroLongCount;
    return;
  }
  const std::deque<int64_t> currentBits = this->bits[i4096];
  const std::deque<int64_t> newBits;
  constexpr int64_t newIndex = currentIndex | index;
  constexpr int requiredCapacity = Long::bitCount(newIndex);
  if (currentBits.size() >= requiredCapacity) {
    newBits = currentBits;
  } else {
    newBits = std::deque<int64_t>(oversize(requiredCapacity));
  }
  // we iterate backwards in order to not override data we might need on the
  // next iteration if the array is reused
  for (int i = Long::numberOfLeadingZeros(newIndex),
           newO = Long::bitCount(newIndex) - 1;
       i < 64;
       i += 1 + Long::numberOfLeadingZeros(newIndex << (i + 1)), newO -= 1) {
    // bitIndex is the index of a bit which is set in newIndex and newO is the
    // number of 1 bits on its right
    constexpr int bitIndex = 63 - i;
    assert(newO == Long::bitCount(newIndex & ((1LL << bitIndex) - 1)));
    newBits[newO] = longBits(currentIndex, currentBits, bitIndex) |
                    longBits(index, bits, bitIndex);
  }
  indices[i4096] = newIndex;
  this->bits[i4096] = newBits;
  this->nonZeroLongCount +=
      nonZeroLongCount - Long::bitCount(currentIndex & index);
}

void SparseFixedBitSet:: or (shared_ptr<SparseFixedBitSet> other)
{
  for (int i = 0; i < other->indices.size(); ++i) {
    constexpr int64_t index = other->indices[i];
    if (index != 0) {
      or (i, index, other->bits[i], Long::bitCount(index));
    }
  }
}

void SparseFixedBitSet::orDense(shared_ptr<DocIdSetIterator> it) throw(
    IOException)
{
  checkUnpositioned(it);
  // The goal here is to try to take advantage of the ordering of documents
  // to build the data-structure more efficiently
  // NOTE: this heavily relies on the fact that shifts are mod 64
  constexpr int firstDoc = it->nextDoc();
  if (firstDoc == DocIdSetIterator::NO_MORE_DOCS) {
    return;
  }
  int i4096 = static_cast<int>(static_cast<unsigned int>(firstDoc) >> 12);
  int i64 = static_cast<int>(static_cast<unsigned int>(firstDoc) >> 6);
  int64_t index = 1LL << i64;
  int64_t currentLong = 1LL << firstDoc;
  // we store at most 64 longs per block so preallocate in order never to have
  // to resize
  std::deque<int64_t> longs(64);
  int numLongs = 0;

  for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = it->nextDoc()) {
    constexpr int doc64 = static_cast<int>(static_cast<unsigned int>(doc) >> 6);
    if (doc64 == i64) {
      // still in the same long, just set the bit
      currentLong |= 1LL << doc;
    } else {
      longs[numLongs++] = currentLong;

      constexpr int doc4096 =
          static_cast<int>(static_cast<unsigned int>(doc) >> 12);
      if (doc4096 == i4096) {
        index |= 1LL << doc64;
      } else {
        // we are on a new block, flush what we buffered
        or (i4096, index, longs, numLongs);
        // and reset state for the new block
        i4096 = doc4096;
        index = 1LL << doc64;
        numLongs = 0;
      }

      // we are on a new long, reset state
      i64 = doc64;
      currentLong = 1LL << doc;
    }
  }

  // flush
  longs[numLongs++] = currentLong;
  or (i4096, index, longs, numLongs);
}

void SparseFixedBitSet:: or (shared_ptr<DocIdSetIterator> it) 
{
  {
    // specialize union with another SparseFixedBitSet
    shared_ptr<SparseFixedBitSet> *const other =
        BitSetIterator::getSparseFixedBitSetOrNull(it);
    if (other != nullptr) {
      checkUnpositioned(it);
      or (other);
      return;
    }
  }

  // We do not specialize the union with a FixedBitSet since FixedBitSets are
  // supposed to be used for dense data and sparse fixed bit sets for sparse
  // data, so a sparse set would likely get upgraded by DocIdSetBuilder before
  // being or'ed with a FixedBitSet

  if (it->cost() < indices.size()) {
    // the default impl is good for sparse iterators
    BitSet:: or (it);
  } else {
    orDense(it);
  }
}

int64_t SparseFixedBitSet::ramBytesUsed() { return ramBytesUsed_; }

wstring SparseFixedBitSet::toString()
{
  return L"SparseFixedBitSet(size=" + to_wstring(length_) + L",cardinality=~" +
         to_wstring(approximateCardinality());
}
} // namespace org::apache::lucene::util