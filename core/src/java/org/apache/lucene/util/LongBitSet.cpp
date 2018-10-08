using namespace std;

#include "LongBitSet.h"

namespace org::apache::lucene::util
{

shared_ptr<LongBitSet> LongBitSet::ensureCapacity(shared_ptr<LongBitSet> bits,
                                                  int64_t numBits)
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
    return make_shared<LongBitSet>(arr, static_cast<int64_t>(arr.size())
                                            << 6);
  }
}

int LongBitSet::bits2words(int64_t numBits)
{
  if (numBits < 0 || numBits > MAX_NUM_BITS) {
    throw invalid_argument(L"numBits must be 0 .. " + to_wstring(MAX_NUM_BITS) +
                           L"; got: " + to_wstring(numBits));
  }
  return static_cast<int>((numBits - 1) >> 6) +
         1; // I.e.: get the word-offset of the last bit and add one (make sure
            // to use >> so 0 returns 0!)
}

LongBitSet::LongBitSet(int64_t numBits)
    : bits(std::deque<int64_t>(bits2words(numBits))), numBits(numBits),
      numWords(bits.size())
{
}

LongBitSet::LongBitSet(std::deque<int64_t> &storedBits, int64_t numBits)
    : bits(storedBits), numBits(numBits), numWords(bits2words(numBits))
{
  if (numWords > storedBits.size()) {
    throw invalid_argument(L"The given long array is too small  to hold " +
                           to_wstring(numBits) + L" bits");
  }

  assert(verifyGhostBitsClear());
}

bool LongBitSet::verifyGhostBitsClear()
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

int64_t LongBitSet::length() { return numBits; }

std::deque<int64_t> LongBitSet::getBits() { return bits; }

int64_t LongBitSet::cardinality()
{
  // Depends on the ghost bits being clear!
  return BitUtil::pop_array(bits, 0, numWords);
}

bool LongBitSet::get(int64_t index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int i = static_cast<int>(index >> 6); // div 64
  // signed shift will keep a negative index and force an
  // array-index-out-of-bounds-exception, removing the need for an explicit
  // check.
  int64_t bitmask = 1LL << index;
  return (bits[i] & bitmask) != 0;
}

void LongBitSet::set(int64_t index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L" numBits=" + to_wstring(numBits)));
  int wordNum = static_cast<int>(index >> 6); // div 64
  int64_t bitmask = 1LL << index;
  bits[wordNum] |= bitmask;
}

bool LongBitSet::getAndSet(int64_t index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int wordNum = static_cast<int>(index >> 6); // div 64
  int64_t bitmask = 1LL << index;
  bool val = (bits[wordNum] & bitmask) != 0;
  bits[wordNum] |= bitmask;
  return val;
}

void LongBitSet::clear(int64_t index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int wordNum = static_cast<int>(index >> 6);
  int64_t bitmask = 1LL << index;
  bits[wordNum] &= ~bitmask;
}

bool LongBitSet::getAndClear(int64_t index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int wordNum = static_cast<int>(index >> 6); // div 64
  int64_t bitmask = 1LL << index;
  bool val = (bits[wordNum] & bitmask) != 0;
  bits[wordNum] &= ~bitmask;
  return val;
}

int64_t LongBitSet::nextSetBit(int64_t index)
{
  // Depends on the ghost bits being clear!
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L", numBits=" + to_wstring(numBits)));
  int i = static_cast<int>(index >> 6);
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

  return -1;
}

int64_t LongBitSet::prevSetBit(int64_t index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L" numBits=" + to_wstring(numBits)));
  int i = static_cast<int>(index >> 6);
  constexpr int subIndex =
      static_cast<int>(index & 0x3f); // index within the word
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

void LongBitSet:: or (shared_ptr<LongBitSet> other)
{
  assert((other->numWords <= numWords, L"numWords=" + to_wstring(numWords) +
                                           L", other.numWords=" +
                                           to_wstring(other->numWords)));
  int pos = min(numWords, other->numWords);
  while (--pos >= 0) {
    bits[pos] |= other->bits[pos];
  }
}

void LongBitSet:: xor
    (shared_ptr<LongBitSet> other) {
      assert((other->numWords <= numWords, L"numWords=" + to_wstring(numWords) +
                                               L", other.numWords=" +
                                               to_wstring(other->numWords)));
      int pos = min(numWords, other->numWords);
      while (--pos >= 0) {
        bits[pos] ^= other->bits[pos];
      }
    }

    bool LongBitSet::intersects(shared_ptr<LongBitSet> other)
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

void LongBitSet::and(shared_ptr<LongBitSet> other)
{
  int pos = min(numWords, other->numWords);
  while (--pos >= 0) {
    bits[pos] &= other->bits[pos];
  }
  if (numWords > other->numWords) {
    Arrays::fill(bits, other->numWords, numWords, 0LL);
  }
}

void LongBitSet::andNot(shared_ptr<LongBitSet> other)
{
  int pos = min(numWords, other->numWords);
  while (--pos >= 0) {
    bits[pos] &= ~other->bits[pos];
  }
}

bool LongBitSet::scanIsEmpty()
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

void LongBitSet::flip(int64_t startIndex, int64_t endIndex)
{
  assert(startIndex >= 0 && startIndex < numBits);
  assert(endIndex >= 0 && endIndex <= numBits);
  if (endIndex <= startIndex) {
    return;
  }

  int startWord = static_cast<int>(startIndex >> 6);
  int endWord = static_cast<int>((endIndex - 1) >> 6);

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

void LongBitSet::flip(int64_t index)
{
  assert((index >= 0 && index < numBits,
          L"index=" + to_wstring(index) + L" numBits=" + to_wstring(numBits)));
  int wordNum = static_cast<int>(index >> 6); // div 64
  int64_t bitmask = 1LL << index;           // mod 64 is implicit
  bits[wordNum] ^= bitmask;
}

void LongBitSet::set(int64_t startIndex, int64_t endIndex)
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

  int startWord = static_cast<int>(startIndex >> 6);
  int endWord = static_cast<int>((endIndex - 1) >> 6);

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

void LongBitSet::clear(int64_t startIndex, int64_t endIndex)
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

  int startWord = static_cast<int>(startIndex >> 6);
  int endWord = static_cast<int>((endIndex - 1) >> 6);

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

shared_ptr<LongBitSet> LongBitSet::clone()
{
  std::deque<int64_t> bits(this->bits.size());
  System::arraycopy(this->bits, 0, bits, 0, numWords);
  return make_shared<LongBitSet>(bits, numBits);
}

bool LongBitSet::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<LongBitSet>(o) != nullptr)) {
    return false;
  }
  shared_ptr<LongBitSet> other = any_cast<std::shared_ptr<LongBitSet>>(o);
  if (numBits != other->numBits) {
    return false;
  }
  // Depends on the ghost bits being clear!
  return Arrays::equals(bits, other->bits);
}

int LongBitSet::hashCode()
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
} // namespace org::apache::lucene::util