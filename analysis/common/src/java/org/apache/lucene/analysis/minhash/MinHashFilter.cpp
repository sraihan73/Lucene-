using namespace std;

#include "MinHashFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"

namespace org::apache::lucene::analysis::minhash
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
std::deque<std::shared_ptr<LongPair>> const MinHashFilter::cachedIntHashes =
    std::deque<std::shared_ptr<LongPair>>(HASH_CACHE_SIZE);
const wstring MinHashFilter::MIN_HASH_TYPE = L"MIN_HASH";

MinHashFilter::StaticConstructor::StaticConstructor()
{
  for (int i = 0; i < HASH_CACHE_SIZE; i++) {
    cachedIntHashes[i] = make_shared<LongPair>();
    murmurhash3_x64_128(getBytes(i), 0, 4, 0, cachedIntHashes[i]);
  }
}

MinHashFilter::StaticConstructor MinHashFilter::staticConstructor;

std::deque<char> MinHashFilter::getBytes(int i)
{
  std::deque<char> answer(4);
  answer[3] = static_cast<char>(i);
  answer[2] = static_cast<char>(i >> 8);
  answer[1] = static_cast<char>(i >> 16);
  answer[0] = static_cast<char>(i >> 24);
  return answer;
}

MinHashFilter::MinHashFilter(shared_ptr<TokenStream> input, int hashCount,
                             int bucketCount, int hashSetSize,
                             bool withRotation)
    : org::apache::lucene::analysis::TokenFilter(input),
      minHashSets(deque<>(this->hashCount)), withRotation(withRotation)
{
  if (hashCount <= 0) {
    throw invalid_argument(L"hashCount must be greater than zero");
  }
  if (bucketCount <= 0) {
    throw invalid_argument(L"bucketCount must be greater than zero");
  }
  if (hashSetSize <= 0) {
    throw invalid_argument(L"hashSetSize must be greater than zero");
  }
  this->hashCount = hashCount;
  this->bucketCount = bucketCount;
  this->hashSetSize = hashSetSize;
  this->bucketSize = (1LL << 32) / bucketCount;
  if ((1LL << 32) % bucketCount != 0) {
    bucketSize++;
  }
  for (int i = 0; i < this->hashCount; i++) {
    deque<FixedSizeTreeSet<std::shared_ptr<LongPair>>> buckets =
        deque<FixedSizeTreeSet<std::shared_ptr<LongPair>>>(this->bucketCount);
    minHashSets.push_back(buckets);
    for (int j = 0; j < this->bucketCount; j++) {
      shared_ptr<FixedSizeTreeSet<std::shared_ptr<LongPair>>> minSet =
          make_shared<FixedSizeTreeSet<std::shared_ptr<LongPair>>>(
              this->hashSetSize);
      buckets.push_back(minSet);
    }
  }
  doRest();
}

bool MinHashFilter::incrementToken() 
{
  // Pull the underlying stream of tokens
  // Hash each token found
  // Generate the required number of variants of this hash
  // Keep the minimum hash value found so far of each variant

  int positionIncrement = 0;
  if (requiresInitialisation) {
    requiresInitialisation = false;
    bool found = false;
    // First time through so we pull and hash everything
    while (input->incrementToken()) {
      found = true;
      wstring current =
          wstring(termAttribute->buffer(), 0, termAttribute->length());

      for (int i = 0; i < hashCount; i++) {
        std::deque<char> bytes = current.getBytes(L"UTF-16LE");
        shared_ptr<LongPair> hash = make_shared<LongPair>();
        murmurhash3_x64_128(bytes, 0, bytes.size(), 0, hash);
        shared_ptr<LongPair> rehashed = combineOrdered({hash, getIntHash(i)});
        minHashSets[i][static_cast<int>((static_cast<int64_t>(
                                            static_cast<uint64_t>(
                                                rehashed->val2) >>
                                            32)) /
                                        bucketSize)]
            ->add(rehashed);
      }
      endOffset = offsetAttribute->endOffset();
    }
    exhausted = true;
    input->end();
    // We need the end state so an underlying shingle filter can have its state
    // restored correctly.
    endState = captureState();
    if (!found) {
      return false;
    }

    positionIncrement = 1;
    // fix up any wrap around bucket values. ...
    if (withRotation && (hashSetSize == 1)) {
      for (int hashLoop = 0; hashLoop < hashCount; hashLoop++) {
        for (int bucketLoop = 0; bucketLoop < bucketCount; bucketLoop++) {
          if (minHashSets[hashLoop][bucketLoop]->size() == 0) {
            for (int bucketOffset = 1; bucketOffset < bucketCount;
                 bucketOffset++) {
              if (minHashSets[hashLoop]
                             [(bucketLoop + bucketOffset) % bucketCount]
                                 ->size() > 0) {
                shared_ptr<LongPair> replacementHash =
                    minHashSets[hashLoop]
                               [(bucketLoop + bucketOffset) % bucketCount]
                                   ->first();
                minHashSets[hashLoop][bucketLoop]->add(replacementHash);
                break;
              }
            }
          }
        }
      }
    }
  }

  clearAttributes();

  while (hashPosition < hashCount) {
    if (hashPosition == -1) {
      hashPosition++;
    } else {
      while (bucketPosition < bucketCount) {
        if (bucketPosition == -1) {
          bucketPosition++;
        } else {
          shared_ptr<LongPair> hash =
              minHashSets[hashPosition][bucketPosition]->pollFirst();
          if (hash != nullptr) {
            termAttribute->setEmpty();
            if (hashCount > 1) {
              termAttribute->append(int0(hashPosition));
              termAttribute->append(int1(hashPosition));
            }
            int64_t high = hash->val2;
            termAttribute->append(long0(high));
            termAttribute->append(long1(high));
            termAttribute->append(long2(high));
            termAttribute->append(long3(high));
            int64_t low = hash->val1;
            termAttribute->append(long0(low));
            termAttribute->append(long1(low));
            if (hashCount == 1) {
              termAttribute->append(long2(low));
              termAttribute->append(long3(low));
            }
            posIncAttribute->setPositionIncrement(positionIncrement);
            offsetAttribute->setOffset(0, endOffset);
            typeAttribute->setType(MIN_HASH_TYPE);
            posLenAttribute->setPositionLength(1);
            return true;
          } else {
            bucketPosition++;
          }
        }
      }
      bucketPosition = -1;
      hashPosition++;
    }
  }
  return false;
}

shared_ptr<LongPair> MinHashFilter::getIntHash(int i)
{
  if (i < HASH_CACHE_SIZE) {
    return cachedIntHashes[i];
  } else {
    shared_ptr<LongPair> answer = make_shared<LongPair>();
    murmurhash3_x64_128(getBytes(i), 0, 4, 0, answer);
    return answer;
  }
}

void MinHashFilter::end() 
{
  if (!exhausted) {
    input->end();
  }

  restoreState(endState);
}

void MinHashFilter::reset() 
{
  TokenFilter::reset();
  doRest();
}

void MinHashFilter::doRest()
{
  for (int i = 0; i < hashCount; i++) {
    for (int j = 0; j < bucketCount; j++) {
      minHashSets[i][j]->clear();
    }
  }
  endState.reset();
  hashPosition = -1;
  bucketPosition = -1;
  requiresInitialisation = true;
  exhausted = false;
}

wchar_t MinHashFilter::long0(int64_t x)
{
  return static_cast<wchar_t>(x >> 48);
}

wchar_t MinHashFilter::long1(int64_t x)
{
  return static_cast<wchar_t>(x >> 32);
}

wchar_t MinHashFilter::long2(int64_t x)
{
  return static_cast<wchar_t>(x >> 16);
}

wchar_t MinHashFilter::long3(int64_t x) { return static_cast<wchar_t>(x); }

wchar_t MinHashFilter::int0(int x) { return static_cast<wchar_t>(x >> 16); }

wchar_t MinHashFilter::int1(int x) { return static_cast<wchar_t>(x); }

bool MinHashFilter::isLessThanUnsigned(int64_t n1, int64_t n2)
{
  return (n1 < n2) ^ ((n1 < 0) != (n2 < 0));
}

shared_ptr<LongPair> MinHashFilter::combineOrdered(deque<LongPair> &hashCodes)
{
  shared_ptr<LongPair> result = make_shared<LongPair>();
  for (shared_ptr<LongPair> hashCode : hashCodes) {
    result->val1 = result->val1 * 37 + hashCode->val1;
    result->val2 = result->val2 * 37 + hashCode->val2;
  }
  return result;
}

int MinHashFilter::LongPair::compareTo(shared_ptr<LongPair> other)
{
  if (isLessThanUnsigned(val2, other->val2)) {
    return -1;
  } else if (val2 == other->val2) {
    if (isLessThanUnsigned(val1, other->val1)) {
      return -1;
    } else if (val1 == other->val1) {
      return 0;
    } else {
      return 1;
    }
  } else {
    return 1;
  }
}

bool MinHashFilter::LongPair::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<LongPair> longPair = any_cast<std::shared_ptr<LongPair>>(o);

  return val1 == longPair->val1 && val2 == longPair->val2;
}

int MinHashFilter::LongPair::hashCode()
{
  int result = static_cast<int>(
      val1 ^
      (static_cast<int64_t>(static_cast<uint64_t>(val1) >> 32)));
  result = 31 * result +
           static_cast<int>(val2 ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(val2) >> 32)));
  return result;
}

int64_t MinHashFilter::getLongLittleEndian(std::deque<char> &buf, int offset)
{
  return (static_cast<int64_t>(buf[offset + 7]) << 56) |
         ((buf[offset + 6] & 0xffLL) << 48) |
         ((buf[offset + 5] & 0xffLL) << 40) |
         ((buf[offset + 4] & 0xffLL) << 32) |
         ((buf[offset + 3] & 0xffLL) << 24) |
         ((buf[offset + 2] & 0xffLL) << 16) |
         ((buf[offset + 1] & 0xffLL) << 8) |
         ((buf[offset] & 0xffLL)); // no shift needed
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") static void
// murmurhash3_x64_128(byte[] key, int offset, int len, int seed, LongPair out)
void MinHashFilter::murmurhash3_x64_128(std::deque<char> &key, int offset,
                                        int len, int seed,
                                        shared_ptr<LongPair> out)
{
  // The original algorithm does have a 32 bit unsigned seed.
  // We have to mask to match the behavior of the unsigned types and prevent
  // sign extension.
  int64_t h1 = seed & 0x00000000FFFFFFFFLL;
  int64_t h2 = seed & 0x00000000FFFFFFFFLL;

  constexpr int64_t c1 = 0x87c37b91114253d5LL;
  constexpr int64_t c2 = 0x4cf5ad432745937fLL;

  int roundedEnd = offset + (len & 0xFFFFFFF0); // round down to 16 byte block
  for (int i = offset; i < roundedEnd; i += 16) {
    int64_t k1 = getLongLittleEndian(key, i);
    int64_t k2 = getLongLittleEndian(key, i + 8);
    k1 *= c1;
    k1 = Long::rotateLeft(k1, 31);
    k1 *= c2;
    h1 ^= k1;
    h1 = Long::rotateLeft(h1, 27);
    h1 += h2;
    h1 = h1 * 5 + 0x52dce729;
    k2 *= c2;
    k2 = Long::rotateLeft(k2, 33);
    k2 *= c1;
    h2 ^= k2;
    h2 = Long::rotateLeft(h2, 31);
    h2 += h1;
    h2 = h2 * 5 + 0x38495ab5;
  }

  int64_t k1 = 0;
  int64_t k2 = 0;

  switch (len & 15) {
  case 15:
    k2 = (key[roundedEnd + 14] & 0xffLL) << 48;
  case 14:
    k2 |= (key[roundedEnd + 13] & 0xffLL) << 40;
  case 13:
    k2 |= (key[roundedEnd + 12] & 0xffLL) << 32;
  case 12:
    k2 |= (key[roundedEnd + 11] & 0xffLL) << 24;
  case 11:
    k2 |= (key[roundedEnd + 10] & 0xffLL) << 16;
  case 10:
    k2 |= (key[roundedEnd + 9] & 0xffLL) << 8;
  case 9:
    k2 |= (key[roundedEnd + 8] & 0xffLL);
    k2 *= c2;
    k2 = Long::rotateLeft(k2, 33);
    k2 *= c1;
    h2 ^= k2;
  case 8:
    k1 = (static_cast<int64_t>(key[roundedEnd + 7])) << 56;
  case 7:
    k1 |= (key[roundedEnd + 6] & 0xffLL) << 48;
  case 6:
    k1 |= (key[roundedEnd + 5] & 0xffLL) << 40;
  case 5:
    k1 |= (key[roundedEnd + 4] & 0xffLL) << 32;
  case 4:
    k1 |= (key[roundedEnd + 3] & 0xffLL) << 24;
  case 3:
    k1 |= (key[roundedEnd + 2] & 0xffLL) << 16;
  case 2:
    k1 |= (key[roundedEnd + 1] & 0xffLL) << 8;
  case 1:
    k1 |= (key[roundedEnd] & 0xffLL);
    k1 *= c1;
    k1 = Long::rotateLeft(k1, 31);
    k1 *= c2;
    h1 ^= k1;
  }

  // ----------
  // finalization

  h1 ^= len;
  h2 ^= len;

  h1 += h2;
  h2 += h1;

  h1 = fmix64(h1);
  h2 = fmix64(h2);

  h1 += h2;
  h2 += h1;

  out->val1 = h1;
  out->val2 = h2;
}

int64_t MinHashFilter::fmix64(int64_t k)
{
  k ^= static_cast<int64_t>(static_cast<uint64_t>(k) >> 33);
  k *= 0xff51afd7ed558ccdLL;
  k ^= static_cast<int64_t>(static_cast<uint64_t>(k) >> 33);
  k *= 0xc4ceb9fe1a85ec53LL;
  k ^= static_cast<int64_t>(static_cast<uint64_t>(k) >> 33);
  return k;
}
} // namespace org::apache::lucene::analysis::minhash