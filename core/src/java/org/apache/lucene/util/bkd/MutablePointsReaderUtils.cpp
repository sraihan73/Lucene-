using namespace std;

#include "MutablePointsReaderUtils.h"

namespace org::apache::lucene::util::bkd
{
using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntroSelector = org::apache::lucene::util::IntroSelector;
using IntroSorter = org::apache::lucene::util::IntroSorter;
using MSBRadixSorter = org::apache::lucene::util::MSBRadixSorter;
using RadixSelector = org::apache::lucene::util::RadixSelector;
using Selector = org::apache::lucene::util::Selector;
using StringHelper = org::apache::lucene::util::StringHelper;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

MutablePointsReaderUtils::MutablePointsReaderUtils() {}

void MutablePointsReaderUtils::sort(int maxDoc, int packedBytesLength,
                                    shared_ptr<MutablePointValues> reader,
                                    int from, int to)
{
  constexpr int bitsPerDocId = PackedInts::bitsRequired(maxDoc - 1);
  make_shared<MSBRadixSorterAnonymousInnerClass>(
      packedBytesLength + (bitsPerDocId + 7) / 8, packedBytesLength, reader,
      bitsPerDocId)
      .sort(from, to);
}

MutablePointsReaderUtils::MSBRadixSorterAnonymousInnerClass::
    MSBRadixSorterAnonymousInnerClass(int packedBytesLength,
                                      int packedBytesLength,
                                      shared_ptr<MutablePointValues> reader,
                                      int bitsPerDocId)
    : org::apache::lucene::util::MSBRadixSorter(packedBytesLength +
                                                (bitsPerDocId + 7) / 8)
{
  this->packedBytesLength = packedBytesLength;
  this->reader = reader;
  this->bitsPerDocId = bitsPerDocId;
}

void MutablePointsReaderUtils::MSBRadixSorterAnonymousInnerClass::swap(int i,
                                                                       int j)
{
  reader->swap(i, j);
}

int MutablePointsReaderUtils::MSBRadixSorterAnonymousInnerClass::byteAt(int i,
                                                                        int k)
{
  if (k < packedBytesLength) {
    return Byte::toUnsignedInt(reader->getByteAt(i, k));
  } else {
    constexpr int shift = bitsPerDocId - ((k - packedBytesLength + 1) << 3);
    return (static_cast<int>(static_cast<unsigned int>(reader->getDocID(i)) >>
                             max(0, shift))) &
           0xff;
  }
}

shared_ptr<org::apache::lucene::util::Sorter>
MutablePointsReaderUtils::MSBRadixSorterAnonymousInnerClass::getFallbackSorter(
    int k)
{
  return make_shared<IntroSorterAnonymousInnerClass>(shared_from_this(), k);
}

MutablePointsReaderUtils::MSBRadixSorterAnonymousInnerClass::
    IntroSorterAnonymousInnerClass::IntroSorterAnonymousInnerClass(
        shared_ptr<MSBRadixSorterAnonymousInnerClass> outerInstance, int k)
{
  this->outerInstance = outerInstance;
  this->k = k;
  pivot = make_shared<BytesRef>();
  scratch = make_shared<BytesRef>();
}

void MutablePointsReaderUtils::MSBRadixSorterAnonymousInnerClass::
    IntroSorterAnonymousInnerClass::swap(int i, int j)
{
  outerInstance->reader.swap(i, j);
}

void MutablePointsReaderUtils::MSBRadixSorterAnonymousInnerClass::
    IntroSorterAnonymousInnerClass::setPivot(int i)
{
  outerInstance->reader.getValue(i, pivot);
  pivotDoc = outerInstance->reader.getDocID(i);
}

int MutablePointsReaderUtils::MSBRadixSorterAnonymousInnerClass::
    IntroSorterAnonymousInnerClass::comparePivot(int j)
{
  if (k < outerInstance->packedBytesLength) {
    outerInstance->reader.getValue(j, scratch);
    int cmp = StringHelper::compare(outerInstance->packedBytesLength - k,
                                    pivot::bytes, pivot::offset + k,
                                    scratch::bytes, scratch::offset + k);
    if (cmp != 0) {
      return cmp;
    }
  }
  return pivotDoc - outerInstance->reader.getDocID(j);
}

void MutablePointsReaderUtils::sortByDim(int sortedDim, int bytesPerDim,
                                         std::deque<int> &commonPrefixLengths,
                                         shared_ptr<MutablePointValues> reader,
                                         int from, int to,
                                         shared_ptr<BytesRef> scratch1,
                                         shared_ptr<BytesRef> scratch2)
{

  // No need for a fancy radix sort here, this is called on the leaves only so
  // there are not many values to sort
  constexpr int offset =
      sortedDim * bytesPerDim + commonPrefixLengths[sortedDim];
  constexpr int numBytesToCompare =
      bytesPerDim - commonPrefixLengths[sortedDim];
  make_shared<IntroSorterAnonymousInnerClass2>(reader, scratch1, scratch2,
                                               offset, numBytesToCompare)
      .sort(from, to);
}

MutablePointsReaderUtils::IntroSorterAnonymousInnerClass2::
    IntroSorterAnonymousInnerClass2(shared_ptr<MutablePointValues> reader,
                                    shared_ptr<BytesRef> scratch1,
                                    shared_ptr<BytesRef> scratch2, int offset,
                                    int numBytesToCompare)
{
  this->reader = reader;
  this->scratch1 = scratch1;
  this->scratch2 = scratch2;
  this->offset = offset;
  this->numBytesToCompare = numBytesToCompare;
}

void MutablePointsReaderUtils::IntroSorterAnonymousInnerClass2::swap(int i,
                                                                     int j)
{
  reader->swap(i, j);
}

void MutablePointsReaderUtils::IntroSorterAnonymousInnerClass2::setPivot(int i)
{
  reader->getValue(i, pivot);
  pivotDoc = reader->getDocID(i);
}

int MutablePointsReaderUtils::IntroSorterAnonymousInnerClass2::comparePivot(
    int j)
{
  reader->getValue(j, scratch2);
  int cmp = StringHelper::compare(numBytesToCompare, pivot::bytes,
                                  pivot::offset + offset, scratch2->bytes,
                                  scratch2->offset + offset);
  if (cmp == 0) {
    cmp = pivotDoc - reader->getDocID(j);
  }
  return cmp;
}

void MutablePointsReaderUtils::partition(int maxDoc, int splitDim,
                                         int bytesPerDim, int commonPrefixLen,
                                         shared_ptr<MutablePointValues> reader,
                                         int from, int to, int mid,
                                         shared_ptr<BytesRef> scratch1,
                                         shared_ptr<BytesRef> scratch2)
{
  constexpr int offset = splitDim * bytesPerDim + commonPrefixLen;
  constexpr int cmpBytes = bytesPerDim - commonPrefixLen;
  constexpr int bitsPerDocId = PackedInts::bitsRequired(maxDoc - 1);
  make_shared<RadixSelectorAnonymousInnerClass>(
      cmpBytes + (bitsPerDocId + 7) / 8, reader, scratch1, scratch2, offset,
      cmpBytes, bitsPerDocId)
      .select(from, to, mid);
}

MutablePointsReaderUtils::RadixSelectorAnonymousInnerClass::
    RadixSelectorAnonymousInnerClass(int cmpBytes,
                                     shared_ptr<MutablePointValues> reader,
                                     shared_ptr<BytesRef> scratch1,
                                     shared_ptr<BytesRef> scratch2, int offset,
                                     int cmpBytes, int bitsPerDocId)
    : org::apache::lucene::util::RadixSelector(cmpBytes +
                                               (bitsPerDocId + 7) / 8)
{
  this->reader = reader;
  this->scratch1 = scratch1;
  this->scratch2 = scratch2;
  this->offset = offset;
  this->cmpBytes = cmpBytes;
  this->bitsPerDocId = bitsPerDocId;
}

shared_ptr<Selector>
MutablePointsReaderUtils::RadixSelectorAnonymousInnerClass::getFallbackSelector(
    int k)
{
  return make_shared<IntroSelectorAnonymousInnerClass>(shared_from_this(), k);
}

MutablePointsReaderUtils::RadixSelectorAnonymousInnerClass::
    IntroSelectorAnonymousInnerClass::IntroSelectorAnonymousInnerClass(
        shared_ptr<RadixSelectorAnonymousInnerClass> outerInstance, int k)
{
  this->outerInstance = outerInstance;
  this->k = k;
  pivot = outerInstance->scratch1;
}

void MutablePointsReaderUtils::RadixSelectorAnonymousInnerClass::
    IntroSelectorAnonymousInnerClass::swap(int i, int j)
{
  outerInstance->reader.swap(i, j);
}

void MutablePointsReaderUtils::RadixSelectorAnonymousInnerClass::
    IntroSelectorAnonymousInnerClass::setPivot(int i)
{
  outerInstance->reader.getValue(i, pivot);
  pivotDoc = outerInstance->reader.getDocID(i);
}

int MutablePointsReaderUtils::RadixSelectorAnonymousInnerClass::
    IntroSelectorAnonymousInnerClass::comparePivot(int j)
{
  if (k < outerInstance->cmpBytes) {
    outerInstance->reader.getValue(j, outerInstance->scratch2);
    int cmp = StringHelper::compare(outerInstance->cmpBytes - k, pivot::bytes,
                                    pivot::offset + outerInstance->offset + k,
                                    outerInstance->scratch2.bytes,
                                    outerInstance->scratch2.offset +
                                        outerInstance->offset + k);
    if (cmp != 0) {
      return cmp;
    }
  }
  return pivotDoc - outerInstance->reader.getDocID(j);
}

void MutablePointsReaderUtils::RadixSelectorAnonymousInnerClass::swap(int i,
                                                                      int j)
{
  reader->swap(i, j);
}

int MutablePointsReaderUtils::RadixSelectorAnonymousInnerClass::byteAt(int i,
                                                                       int k)
{
  if (k < cmpBytes) {
    return Byte::toUnsignedInt(reader->getByteAt(i, offset + k));
  } else {
    constexpr int shift = bitsPerDocId - ((k - cmpBytes + 1) << 3);
    return (static_cast<int>(static_cast<unsigned int>(reader->getDocID(i)) >>
                             max(0, shift))) &
           0xff;
  }
}
} // namespace org::apache::lucene::util::bkd