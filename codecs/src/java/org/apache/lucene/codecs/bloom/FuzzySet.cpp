using namespace std;

#include "FuzzySet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/DataOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "HashFunction.h"
#include "MurmurHash2.h"

namespace org::apache::lucene::codecs::bloom
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

shared_ptr<HashFunction> FuzzySet::hashFunctionForVersion(int version)
{
  if (version < VERSION_START) {
    throw invalid_argument(L"Version " + to_wstring(version) +
                           L" is too old, expected at least " +
                           to_wstring(VERSION_START));
  } else if (version > VERSION_CURRENT) {
    throw invalid_argument(L"Version " + to_wstring(version) +
                           L" is too new, expected at most " +
                           to_wstring(VERSION_CURRENT));
  }
  return MurmurHash2::INSTANCE;
}

std::deque<int> const FuzzySet::usableBitSetSizes;

FuzzySet::StaticConstructor::StaticConstructor()
{
  usableBitSetSizes = std::deque<int>(30);
  int mask = 1;
  int size = mask;
  for (int i = 0; i < usableBitSetSizes.size(); i++) {
    size = (size << 1) | mask;
    usableBitSetSizes[i] = size;
  }
}

FuzzySet::StaticConstructor FuzzySet::staticConstructor;

int FuzzySet::getNearestSetSize(int maxNumberOfBits)
{
  int result = usableBitSetSizes[0];
  for (int i = 0; i < usableBitSetSizes.size(); i++) {
    if (usableBitSetSizes[i] <= maxNumberOfBits) {
      result = usableBitSetSizes[i];
    }
  }
  return result;
}

int FuzzySet::getNearestSetSize(int maxNumberOfValuesExpected,
                                float desiredSaturation)
{
  // Iterate around the various scales of bitset from smallest to largest
  // looking for the first that satisfies value volumes at the chosen saturation
  // level
  for (int i = 0; i < usableBitSetSizes.size(); i++) {
    int numSetBitsAtDesiredSaturation =
        static_cast<int>(usableBitSetSizes[i] * desiredSaturation);
    int estimatedNumUniqueValues =
        getEstimatedNumberUniqueValuesAllowingForCollisions(
            usableBitSetSizes[i], numSetBitsAtDesiredSaturation);
    if (estimatedNumUniqueValues > maxNumberOfValuesExpected) {
      return usableBitSetSizes[i];
    }
  }
  return -1;
}

shared_ptr<FuzzySet> FuzzySet::createSetBasedOnMaxMemory(int maxNumBytes)
{
  int setSize = getNearestSetSize(maxNumBytes);
  return make_shared<FuzzySet>(make_shared<FixedBitSet>(setSize + 1), setSize,
                               hashFunctionForVersion(VERSION_CURRENT));
}

shared_ptr<FuzzySet>
FuzzySet::createSetBasedOnQuality(int maxNumUniqueValues,
                                  float desiredMaxSaturation)
{
  int setSize = getNearestSetSize(maxNumUniqueValues, desiredMaxSaturation);
  return make_shared<FuzzySet>(make_shared<FixedBitSet>(setSize + 1), setSize,
                               hashFunctionForVersion(VERSION_CURRENT));
}

// C++ TODO: No base class can be determined:
FuzzySet::FuzzySet(shared_ptr<FixedBitSet> filter, int bloomSize,
                   shared_ptr<HashFunction> hashFunction)
{
  this->filter = filter;
  this->bloomSize = bloomSize;
  this->hashFunction = hashFunction;
}

FuzzySet::ContainsResult FuzzySet::contains(shared_ptr<BytesRef> value)
{
  int hash = hashFunction->hash(value);
  if (hash < 0) {
    hash = hash * -1;
  }
  return mayContainValue(hash);
}

void FuzzySet::serialize(shared_ptr<DataOutput> out) 
{
  out->writeInt(VERSION_CURRENT);
  out->writeInt(bloomSize);
  std::deque<int64_t> bits = filter->getBits();
  out->writeInt(bits.size());
  for (int i = 0; i < bits.size(); i++) {
    // Can't used VLong encoding because cant cope with negative numbers
    // output by FixedBitSet
    out->writeLong(bits[i]);
  }
}

shared_ptr<FuzzySet>
FuzzySet::deserialize(shared_ptr<DataInput> in_) 
{
  int version = in_->readInt();
  if (version == VERSION_SPI) {
    in_->readString();
  }
  shared_ptr<HashFunction> *const hashFunction =
      hashFunctionForVersion(version);
  int bloomSize = in_->readInt();
  int numLongs = in_->readInt();
  std::deque<int64_t> longs(numLongs);
  for (int i = 0; i < numLongs; i++) {
    longs[i] = in_->readLong();
  }
  shared_ptr<FixedBitSet> bits = make_shared<FixedBitSet>(longs, bloomSize + 1);
  return make_shared<FuzzySet>(bits, bloomSize, hashFunction);
}

FuzzySet::ContainsResult FuzzySet::mayContainValue(int positiveHash)
{
  assert(positiveHash >= 0);
  // Bloom sizes are always base 2 and so can be ANDed for a fast modulo
  int pos = positiveHash & bloomSize;
  if (filter->get(pos)) {
    // This term may be recorded in this index (but could be a collision)
    return ContainsResult::MAYBE;
  }
  // definitely NOT in this segment
  return ContainsResult::NO;
}

void FuzzySet::addValue(shared_ptr<BytesRef> value) 
{
  int hash = hashFunction->hash(value);
  if (hash < 0) {
    hash = hash * -1;
  }
  // Bitmasking using bloomSize is effectively a modulo operation.
  int bloomPos = hash & bloomSize;
  filter->set(bloomPos);
}

shared_ptr<FuzzySet> FuzzySet::downsize(float targetMaxSaturation)
{
  int numBitsSet = filter->cardinality();
  shared_ptr<FixedBitSet> rightSizedBitSet = filter;
  int rightSizedBitSetSize = bloomSize;
  // Hopefully find a smaller size bitset into which we can project accumulated
  // values while maintaining desired saturation level
  for (int i = 0; i < usableBitSetSizes.size(); i++) {
    int candidateBitsetSize = usableBitSetSizes[i];
    float candidateSaturation = static_cast<float>(numBitsSet) /
                                static_cast<float>(candidateBitsetSize);
    if (candidateSaturation <= targetMaxSaturation) {
      rightSizedBitSetSize = candidateBitsetSize;
      break;
    }
  }
  // Re-project the numbers to a smaller space if necessary
  if (rightSizedBitSetSize < bloomSize) {
    // Reset the choice of bitset to the smaller version
    rightSizedBitSet = make_shared<FixedBitSet>(rightSizedBitSetSize + 1);
    // Map across the bits from the large set to the smaller one
    int bitIndex = 0;
    do {
      bitIndex = filter->nextSetBit(bitIndex);
      if (bitIndex != DocIdSetIterator::NO_MORE_DOCS) {
        // Project the larger number into a smaller one effectively
        // modulo-ing by using the target bitset size as a mask
        int downSizedBitIndex = bitIndex & rightSizedBitSetSize;
        rightSizedBitSet->set(downSizedBitIndex);
        bitIndex++;
      }
    } while ((bitIndex >= 0) && (bitIndex <= bloomSize));
  } else {
    return nullptr;
  }
  return make_shared<FuzzySet>(rightSizedBitSet, rightSizedBitSetSize,
                               hashFunction);
}

int FuzzySet::getEstimatedUniqueValues()
{
  return getEstimatedNumberUniqueValuesAllowingForCollisions(
      bloomSize, filter->cardinality());
}

int FuzzySet::getEstimatedNumberUniqueValuesAllowingForCollisions(
    int setSize, int numRecordedBits)
{
  double setSizeAsDouble = setSize;
  double numRecordedBitsAsDouble = numRecordedBits;
  double saturation = numRecordedBitsAsDouble / setSizeAsDouble;
  double logInverseSaturation = log(1 - saturation) * -1;
  return static_cast<int>(setSizeAsDouble * logInverseSaturation);
}

float FuzzySet::getSaturation()
{
  int numBitsSet = filter->cardinality();
  return static_cast<float>(numBitsSet) / static_cast<float>(bloomSize);
}

int64_t FuzzySet::ramBytesUsed()
{
  return RamUsageEstimator::sizeOf(filter->getBits());
}

wstring FuzzySet::toString()
{
  return getClass().getSimpleName() + L"(hash=" + hashFunction + L")";
}
} // namespace org::apache::lucene::codecs::bloom