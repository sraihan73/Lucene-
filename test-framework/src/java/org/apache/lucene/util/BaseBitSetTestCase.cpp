using namespace std;

#include "BaseBitSetTestCase.h"

namespace org::apache::lucene::util
{
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using org::junit::Ignore;

BaseBitSetTestCase<T>::JavaUtilBitSet::JavaUtilBitSet(
    shared_ptr<java::util::BitSet> bitSet, int numBits)
    : bitSet(bitSet), numBits(numBits)
{
}

void BaseBitSetTestCase<T>::JavaUtilBitSet::clear(int index)
{
  bitSet->clear(index);
}

bool BaseBitSetTestCase<T>::JavaUtilBitSet::get(int index)
{
  return bitSet->get(index);
}

int BaseBitSetTestCase<T>::JavaUtilBitSet::length() { return numBits; }

int64_t BaseBitSetTestCase<T>::JavaUtilBitSet::ramBytesUsed() { return -1; }

shared_ptr<deque<std::shared_ptr<Accountable>>>
BaseBitSetTestCase<T>::JavaUtilBitSet::getChildResources()
{
  return Collections::emptyList();
}

void BaseBitSetTestCase<T>::JavaUtilBitSet::set(int i) { bitSet->set(i); }

void BaseBitSetTestCase<T>::JavaUtilBitSet::clear(int startIndex, int endIndex)
{
  if (startIndex >= endIndex) {
    return;
  }
  bitSet->clear(startIndex, endIndex);
}

int BaseBitSetTestCase<T>::JavaUtilBitSet::cardinality()
{
  return bitSet->cardinality();
}

int BaseBitSetTestCase<T>::JavaUtilBitSet::prevSetBit(int index)
{
  return bitSet->previousSetBit(index);
}

int BaseBitSetTestCase<T>::JavaUtilBitSet::nextSetBit(int i)
{
  int next = bitSet->nextSetBit(i);
  if (next == -1) {
    next = DocIdSetIterator::NO_MORE_DOCS;
  }
  return next;
}
} // namespace org::apache::lucene::util