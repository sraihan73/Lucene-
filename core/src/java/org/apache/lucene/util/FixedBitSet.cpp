using namespace std;

#include "FixedBitSet.h"

namespace org::apache::lucene::util
{
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

shared_ptr<FixedBitSet>
FixedBitSet::ensureCapacity(shared_ptr<FixedBitSet> bits, int numBits)
{
  if (numBits < bits->numBits) {
    return bits;
  } else {
    // Depends on the ghost bits being clear!
    // (Otherwise, they may become visible in the new instance)
    int numWords = bits2words(numBits);
    std::deque<int64_t> arr = bits->getBits();
    if (numWords >= arr.size()) {
      arr = ArrayUtil::grow(arr, numWords + 1);
    }
    return make_shared<FixedBitSet>(arr, arr.size() << 6);
  }
}

int FixedBitSet::bits2words(int numBits)
{
  return ((numBits - 1) >> 6) +
         1; // I.e.: get the word-offset of the last bit and add one (make sure
            // to use >> so 0 returns 0!)
}

int64_t FixedBitSet::intersectionCount(shared_ptr<FixedBitSet> a,
                                         shared_ptr<FixedBitSet> b)
{
  // Depends on the ghost bits being clear!
  return BitUtil::pop_intersect(a->bits, b->bits, 0,
                                min(a->numWords, b->numWords));
}

int64_t FixedBitSet::unionCount(shared_ptr<FixedBitSet> a,
                                  shared_ptr<FixedBitSet> b)
{
  // Depends on the ghost bits being clear!
  int64_t tot =
      BitUtil::pop_union(a->bits, b->bits, 0, min(a->numWords, b->numWords));
  if (a->numWords < b->numWords) {
    tot += BitUtil::pop_array(b->bits, a->numWords, b->numWords - a->numWords);
  } else if (a->numWords > b->numWords) {
    tot += BitUtil::pop_array(a->bits, b->numWords, a->numWords - b->numWords);
  }
  return tot;
}

int64_t FixedBitSet::andNotCount(shared_ptr<FixedBitSet> a,
                                   shared_ptr<FixedBitSet> b)
{
  // Depends on the ghost bits being clear!
  int64_t tot =
      BitUtil::pop_andnot(a->bits, b->bits, 0, min(a->numWords, b->numWords));
  if (a->numWords > b->numWords) {
    tot += BitUtil::pop_array(a->bits, b->numWords, a->numWords - b->numWords);
  }
  return tot;
}

FixedBitSet::FixedBitSet(int numBits)
    : bits(std::deque<int64_t>(bits2words(numBits))), numBits(numBits),
      numWords(bits.size())
{
}

FixedBitSet::FixedBitSet(std::deque<int64_t> &storedBits, int numBits)
    : bits(storedBits), numBits(numBits), numWords(bits2words(numBits))
{
  if (numWords > storedBits.size()) {
    throw invalid_argument(L"The given long array is too small  to hold " +
                           to_wstring(numBits) + L" bits");
  }

  assert(verifyGhostBitsClear());
}

bool FixedBitSet::verifyGhostBitsClear()
{
  for (int i = numWords; i < bits.size(); i++) {
    if (bits[i] != 0) {
      return false;
    }
  }

  if ((numBits & 0x3f) == 0) {
    return true;
  }

  int64_t mask = -1LL << numBits;

  return (bits[numWords - 1] & mask) == 0;
}

int FixedBitSet::length() { return numBits; }

int64_t FixedBitSet::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(bits);
}

std::deque<int64_t> FixedBitSet::getBits() { return bits; }

int FixedBitSet::cardinality()
{
  // Depends on the ghost bits being clear!
  return static_cast<int>(BitUtil::pop_array(bits, 0, numWords));
}

bool FixedBitSet::get(int index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int i = index >> 6; // div 64
  // signed shift will keep a negative index and force an
  // array-index-out-of-bounds-exception, removing the need for an explicit
  // check.
  int64_t bitmask = 1LL << index;
  return (bits[i] & bitmask) != 0;
}

void FixedBitSet::set(int index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int wordNum = index >> 6; // div 64
  int64_t bitmask = 1LL << index;
  bits[wordNum] |= bitmask;
}

bool FixedBitSet::getAndSet(int index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int wordNum = index >> 6; // div 64
  int64_t bitmask = 1LL << index;
  bool val = (bits[wordNum] & bitmask) != 0;
  bits[wordNum] |= bitmask;
  return val;
}

void FixedBitSet::clear(int index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int wordNum = index >> 6;
  int64_t bitmask = 1LL << index;
  bits[wordNum] &= ~bitmask;
}

bool FixedBitSet::getAndClear(int index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int wordNum = index >> 6; // div 64
  int64_t bitmask = 1LL << index;
  bool val = (bits[wordNum] & bitmask) != 0;
  bits[wordNum] &= ~bitmask;
  return val;
}

int FixedBitSet::nextSetBit(int index)
{
  // Depends on the ghost bits being clear!
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int i = index >> 6;
  int64_t word = bits[i] >> index; // skip all the bits to the right of index

  if (word != 0) {
    return index + Long::numberOfTrailingZeros(word);
  }

  while (++i < numWords) {
    word = bits[i];
    if (word != 0) {
      return (i << 6) + Long::numberOfTrailingZeros(word);
    }
  }

  return DocIdSetIterator::NO_MORE_DOCS;
}

int FixedBitSet::prevSetBit(int index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L" numBits=" + to_wstring(numBits)));
  int i = index >> 6;
  constexpr int subIndex = index & 0x3f; // index within the word
  int64_t word =
      (bits[i] << (63 - subIndex)); // skip all the bits to the left of index

  if (word != 0) {
    return (i << 6) + subIndex -
           Long::numberOfLeadingZeros(word); // See LUCENE-3197
  }

  while (--i >= 0) {
    word = bits[i];
    if (word != 0) {
      return (i << 6) + 63 - Long::numberOfLeadingZeros(word);
    }
  }

  return -1;
}

void FixedBitSet:: or (shared_ptr<DocIdSetIterator> iter) 
{
  if (BitSetIterator::getFixedBitSetOrNull(iter) != nullptr) {
    checkUnpositioned(iter);
    shared_ptr<FixedBitSet> *const bits =
        BitSetIterator::getFixedBitSetOrNull(iter);
    or (bits);
  } else {
    BitSet:: or (iter);
  }
}

void FixedBitSet:: or (shared_ptr<FixedBitSet> other)
{
  or (other->bits, other->numWords);
}

void FixedBitSet:: or
    (std::deque<int64_t> & otherArr, int const otherNumWords)
{
  assert((otherNumWords <= numWords, L"numWords=" + to_wstring(numWords) +
                                         L", otherNumWords=" +
                                         to_wstring(otherNumWords)));
  const std::deque<int64_t> thisArr = this->bits;
  int pos = min(numWords, otherNumWords);
  while (--pos >= 0) {
    thisArr[pos] |= otherArr[pos];
  }
}

void FixedBitSet:: xor
    (shared_ptr<FixedBitSet> other) { xor(other->bits, other->numWords); }

    void FixedBitSet::
        xor (shared_ptr<DocIdSetIterator> iter) 
{
  checkUnpositioned(iter);
  if (BitSetIterator::getFixedBitSetOrNull(iter) != nullptr) {
    shared_ptr<FixedBitSet> *const bits =
        BitSetIterator::getFixedBitSetOrNull(iter);
    xor(bits);
  } else {
    int doc;
    while ((doc = iter->nextDoc()) < numBits) {
      flip(doc);
    }
  }
}

void FixedBitSet:: xor
    (std::deque<int64_t> & otherBits, int otherNumWords) {
      assert((otherNumWords <= numWords, L"numWords=" + to_wstring(numWords) +
                                             L", other.numWords=" +
                                             to_wstring(otherNumWords)));
      const std::deque<int64_t> thisBits = this->bits;
      int pos = min(numWords, otherNumWords);
      while (--pos >= 0) {
        thisBits[pos] ^= otherBits[pos];
      }
    }

    bool FixedBitSet::intersects(shared_ptr<FixedBitSet> other)
{
  // Depends on the ghost bits being clear!
  int pos = min(numWords, other->numWords);
  while (--pos >= 0) {
    if ((bits[pos] & other->bits[pos]) != 0) {
      return true;
    }
  }
  return false;
}

void FixedBitSet::and(shared_ptr<FixedBitSet> other)
{
  and(other->bits, other->numWords);
}

void FixedBitSet::and(std::deque<int64_t> &otherArr, int const otherNumWords)
{
  const std::deque<int64_t> thisArr = this->bits;
  int pos = min(this->numWords, otherNumWords);
  while (--pos >= 0) {
    thisArr[pos] &= otherArr[pos];
  }
  if (this->numWords > otherNumWords) {
    Arrays::fill(thisArr, otherNumWords, this->numWords, 0LL);
  }
}

void FixedBitSet::andNot(shared_ptr<FixedBitSet> other)
{
  andNot(other->bits, other->numWords);
}

void FixedBitSet::andNot(std::deque<int64_t> &otherArr,
                         int const otherNumWords)
{
  const std::deque<int64_t> thisArr = this->bits;
  int pos = min(this->numWords, otherNumWords);
  while (--pos >= 0) {
    thisArr[pos] &= ~otherArr[pos];
  }
}

bool FixedBitSet::scanIsEmpty()
{
  // This 'slow' implementation is still faster than any external one could be
  // (e.g.: (bitSet.length() == 0 || bitSet.nextSetBit(0) == -1))
  // especially for small BitSets
  // Depends on the ghost bits being clear!
  constexpr int count = numWords;

  for (int i = 0; i < count; i++) {
    if (bits[i] != 0) {
      return false;
    }
  }

  return true;
}

void FixedBitSet::flip(int startIndex, int endIndex)
{
  assert(startIndex >= 0 && startIndex < numBits);
  assert(endIndex >= 0 && endIndex <= numBits);
  if (endIndex <= startIndex) {
    return;
  }

  int startWord = startIndex >> 6;
  int endWord = (endIndex - 1) >> 6;

  /*** Grrr, java shifting uses only the lower 6 bits of the count so -1L>>>64
  == -1
   * for that reason, make sure not to use endmask if the bits to flip will
   * be zero in the last word (redefine endWord to be the last changed...)
  long startmask = -1L << (startIndex & 0x3f);     // example: 11111...111000
  long endmask = -1L >>> (64-(endIndex & 0x3f));   // example: 00111...111111
  ***/

  int64_t startmask = -1LL << startIndex;
  int64_t endmask = static_cast<int64_t>(
      static_cast<uint64_t>(-1LL) >>
      -endIndex); // 64-(endIndex&0x3f) is the same as -endIndex since only the
                  // lowest 6 bits are used

  if (startWord == endWord) {
    bits[startWord] ^= (startmask & endmask);
    return;
  }

  bits[startWord] ^= startmask;

  for (int i = startWord + 1; i < endWord; i++) {
    bits[i] = ~bits[i];
  }

  bits[endWord] ^= endmask;
}

void FixedBitSet::flip(int index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L" numBits=" + to_wstring(numBits)));
  int wordNum = index >> 6;         // div 64
  int64_t bitmask = 1LL << index; // mod 64 is implicit
  bits[wordNum] ^= bitmask;
}

void FixedBitSet::set(int startIndex, int endIndex)
{
  assert((startIndex >= 0 && startIndex < numBits,
          L"startIndex=" + to_wstring(startIndex) + L", numBits=" +
              to_wstring(numBits)));
  assert((endIndex >= 0 && endIndex <= numBits,
          L"endIndex=" + to_wstring(endIndex) + L", numBits=" +
              to_wstring(numBits)));
  if (endIndex <= startIndex) {
    return;
  }

  int startWord = startIndex >> 6;
  int endWord = (endIndex - 1) >> 6;

  int64_t startmask = -1LL << startIndex;
  int64_t endmask = static_cast<int64_t>(
      static_cast<uint64_t>(-1LL) >>
      -endIndex); // 64-(endIndex&0x3f) is the same as -endIndex since only the
                  // lowest 6 bits are used

  if (startWord == endWord) {
    bits[startWord] |= (startmask & endmask);
    return;
  }

  bits[startWord] |= startmask;
  Arrays::fill(bits, startWord + 1, endWord, -1LL);
  bits[endWord] |= endmask;
}

void FixedBitSet::clear(int startIndex, int endIndex)
{
  assert((startIndex >= 0 && startIndex < numBits,
          L"startIndex=" + to_wstring(startIndex) + L", numBits=" +
              to_wstring(numBits)));
  assert((endIndex >= 0 && endIndex <= numBits,
          L"endIndex=" + to_wstring(endIndex) + L", numBits=" +
              to_wstring(numBits)));
  if (endIndex <= startIndex) {
    return;
  }

  int startWord = startIndex >> 6;
  int endWord = (endIndex - 1) >> 6;

  int64_t startmask = -1LL << startIndex;
  int64_t endmask = static_cast<int64_t>(
      static_cast<uint64_t>(-1LL) >>
      -endIndex); // 64-(endIndex&0x3f) is the same as -endIndex since only the
                  // lowest 6 bits are used

  // invert masks since we are clearing
  startmask = ~startmask;
  endmask = ~endmask;

  if (startWord == endWord) {
    bits[startWord] &= (startmask | endmask);
    return;
  }

  bits[startWord] &= startmask;
  Arrays::fill(bits, startWord + 1, endWord, 0LL);
  bits[endWord] &= endmask;
}

shared_ptr<FixedBitSet> FixedBitSet::clone()
{
  std::deque<int64_t> bits(this->bits.size());
  System::arraycopy(this->bits, 0, bits, 0, numWords);
  return make_shared<FixedBitSet>(bits, numBits);
}

bool FixedBitSet::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<FixedBitSet>(o) != nullptr)) {
    return false;
  }
  shared_ptr<FixedBitSet> other = any_cast<std::shared_ptr<FixedBitSet>>(o);
  if (numBits != other->numBits) {
    return false;
  }
  // Depends on the ghost bits being clear!
  return Arrays::equals(bits, other->bits);
}

int FixedBitSet::hashCode()
{
  // Depends on the ghost bits being clear!
  int64_t h = 0;
  for (int i = numWords; --i >= 0;) {
    h ^= bits[i];
    h = (h << 1) | (static_cast<int64_t>(static_cast<uint64_t>(h) >>
                                           63)); // rotate left
  }
  // fold leftmost bits into right and add a constant to prevent
  // empty sets from returning 0, which is too common.
  return static_cast<int>((h >> 32) ^ h) + 0x98761234;
}

shared_ptr<FixedBitSet> FixedBitSet::copyOf(shared_ptr<Bits> bits)
{
  if (std::dynamic_pointer_cast<FixedBits>(bits) != nullptr) {
    // restore the original FixedBitSet
    shared_ptr<FixedBits> fixedBits = std::static_pointer_cast<FixedBits>(bits);
    bits = make_shared<FixedBitSet>(fixedBits->bits, fixedBits->length_);
  }

  if (std::dynamic_pointer_cast<FixedBitSet>(bits) != nullptr) {
    return (std::static_pointer_cast<FixedBitSet>(bits))->clone();
  } else {
    int length = bits->length();
    shared_ptr<FixedBitSet> bitSet = make_shared<FixedBitSet>(length);
    bitSet->set(0, length);
    for (int i = 0; i < length; ++i) {
      if (bits->get(i) == false) {
        bitSet->clear(i);
      }
    }
    return bitSet;
  }
}

shared_ptr<Bits> FixedBitSet::asReadOnlyBits()
{
  return make_shared<FixedBits>(bits, numBits);
}
} // namespace org::apache::lucene::util