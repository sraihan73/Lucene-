using namespace std;

#include "PointValuesWriter.h"

namespace org::apache::lucene::index
{
using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using Counter = org::apache::lucene::util::Counter;

PointValuesWriter::PointValuesWriter(
    shared_ptr<DocumentsWriterPerThread> docWriter,
    shared_ptr<FieldInfo> fieldInfo)
    : fieldInfo(fieldInfo),
      bytes(make_shared<ByteBlockPool>(docWriter->byteBlockAllocator)),
      iwBytesUsed(docWriter->bytesUsed_),
      packedBytesLength(fieldInfo->getPointDimensionCount() *
                        fieldInfo->getPointNumBytes())
{
  docIDs = std::deque<int>(16);
  iwBytesUsed->addAndGet(16 * Integer::BYTES);
}

void PointValuesWriter::addPackedValue(int docID, shared_ptr<BytesRef> value)
{
  if (value == nullptr) {
    throw invalid_argument(L"field=" + fieldInfo->name +
                           L": point value must not be null");
  }
  if (value->length != packedBytesLength) {
    throw invalid_argument(L"field=" + fieldInfo->name +
                           L": this field's value has length=" +
                           to_wstring(value->length) + L" but should be " +
                           to_wstring(fieldInfo->getPointDimensionCount() *
                                      fieldInfo->getPointNumBytes()));
  }

  if (docIDs.size() == numPoints) {
    docIDs = ArrayUtil::grow(docIDs, numPoints + 1);
    iwBytesUsed->addAndGet((docIDs.size() - numPoints) * Integer::BYTES);
  }
  bytes->append(value);
  docIDs[numPoints] = docID;
  if (docID != lastDocID) {
    numDocs++;
    lastDocID = docID;
  }

  numPoints++;
}

void PointValuesWriter::flush(
    shared_ptr<SegmentWriteState> state, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<PointsWriter> writer) 
{
  shared_ptr<PointValues> points =
      make_shared<MutablePointValuesAnonymousInnerClass>(shared_from_this());

  shared_ptr<PointValues> *const values;
  if (sortMap == nullptr) {
    values = points;
  } else {
    values = make_shared<MutableSortingPointValues>(
        std::static_pointer_cast<MutablePointValues>(points), sortMap);
  }
  shared_ptr<PointsReader> reader =
      make_shared<PointsReaderAnonymousInnerClass>(shared_from_this(), values);
  writer->writeField(fieldInfo, reader);
}

PointValuesWriter::MutablePointValuesAnonymousInnerClass::
    MutablePointValuesAnonymousInnerClass(
        shared_ptr<PointValuesWriter> outerInstance)
{
  this->outerInstance = outerInstance;
  ords = std::deque<int>(outerInstance->numPoints);

  for (int i = 0; i < outerInstance->numPoints; ++i) {
    ords[i] = i;
  }
}

void PointValuesWriter::MutablePointValuesAnonymousInnerClass::intersect(
    shared_ptr<IntersectVisitor> visitor) 
{
  shared_ptr<BytesRef> *const scratch = make_shared<BytesRef>();
  const std::deque<char> packedValue =
      std::deque<char>(outerInstance->packedBytesLength);
  for (int i = 0; i < outerInstance->numPoints; i++) {
    getValue(i, scratch);
    assert(scratch->length == packedValue.size());
    System::arraycopy(scratch->bytes, scratch->offset, packedValue, 0,
                      outerInstance->packedBytesLength);
    visitor->visit(getDocID(i), packedValue);
  }
}

int64_t
PointValuesWriter::MutablePointValuesAnonymousInnerClass::estimatePointCount(
    shared_ptr<IntersectVisitor> visitor)
{
  throw make_shared<UnsupportedOperationException>();
}

std::deque<char>
PointValuesWriter::MutablePointValuesAnonymousInnerClass::getMinPackedValue()
{
  throw make_shared<UnsupportedOperationException>();
}

std::deque<char>
PointValuesWriter::MutablePointValuesAnonymousInnerClass::getMaxPackedValue()
{
  throw make_shared<UnsupportedOperationException>();
}

int PointValuesWriter::MutablePointValuesAnonymousInnerClass::getNumDimensions()
{
  throw make_shared<UnsupportedOperationException>();
}

int PointValuesWriter::MutablePointValuesAnonymousInnerClass::
    getBytesPerDimension()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t PointValuesWriter::MutablePointValuesAnonymousInnerClass::size()
{
  return outerInstance->numPoints;
}

int PointValuesWriter::MutablePointValuesAnonymousInnerClass::getDocCount()
{
  return outerInstance->numDocs;
}

void PointValuesWriter::MutablePointValuesAnonymousInnerClass::swap(int i,
                                                                    int j)
{
  int tmp = ords[i];
  ords[i] = ords[j];
  ords[j] = tmp;
}

int PointValuesWriter::MutablePointValuesAnonymousInnerClass::getDocID(int i)
{
  return outerInstance->docIDs[ords[i]];
}

void PointValuesWriter::MutablePointValuesAnonymousInnerClass::getValue(
    int i, shared_ptr<BytesRef> packedValue)
{
  constexpr int64_t offset =
      static_cast<int64_t>(outerInstance->packedBytesLength) * ords[i];
  packedValue->length = outerInstance->packedBytesLength;
  outerInstance->bytes->setRawBytesRef(packedValue, offset);
}

char PointValuesWriter::MutablePointValuesAnonymousInnerClass::getByteAt(int i,
                                                                         int k)
{
  constexpr int64_t offset =
      static_cast<int64_t>(outerInstance->packedBytesLength) * ords[i] + k;
  return outerInstance->bytes->readByte(offset);
}

PointValuesWriter::PointsReaderAnonymousInnerClass::
    PointsReaderAnonymousInnerClass(
        shared_ptr<PointValuesWriter> outerInstance,
        shared_ptr<org::apache::lucene::index::PointValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

shared_ptr<PointValues>
PointValuesWriter::PointsReaderAnonymousInnerClass::getValues(
    const wstring &fieldName)
{
  if (fieldName == outerInstance->fieldInfo->name == false) {
    throw invalid_argument(L"fieldName must be the same");
  }
  return values;
}

void PointValuesWriter::PointsReaderAnonymousInnerClass::checkIntegrity()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t PointValuesWriter::PointsReaderAnonymousInnerClass::ramBytesUsed()
{
  return 0LL;
}

PointValuesWriter::PointsReaderAnonymousInnerClass::
    ~PointsReaderAnonymousInnerClass()
{
}

PointValuesWriter::MutableSortingPointValues::MutableSortingPointValues(
    shared_ptr<MutablePointValues> in_, shared_ptr<Sorter::DocMap> docMap)
    : in_(in_), docMap(docMap)
{
}

void PointValuesWriter::MutableSortingPointValues::intersect(
    shared_ptr<IntersectVisitor> visitor) 
{
  in_->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
      shared_from_this(), visitor));
}

PointValuesWriter::MutableSortingPointValues::
    IntersectVisitorAnonymousInnerClass::IntersectVisitorAnonymousInnerClass(
        shared_ptr<MutableSortingPointValues> outerInstance,
        shared_ptr<org::apache::lucene::index::PointValues::IntersectVisitor>
            visitor)
{
  this->outerInstance = outerInstance;
  this->visitor = visitor;
}

void PointValuesWriter::MutableSortingPointValues::
    IntersectVisitorAnonymousInnerClass::visit(int docID) 
{
  visitor->visit(outerInstance->docMap->oldToNew(docID));
}

void PointValuesWriter::MutableSortingPointValues::
    IntersectVisitorAnonymousInnerClass::visit(
        int docID, std::deque<char> &packedValue) 
{
  visitor->visit(outerInstance->docMap->oldToNew(docID), packedValue);
}

Relation PointValuesWriter::MutableSortingPointValues::
    IntersectVisitorAnonymousInnerClass::compare(
        std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  return visitor->compare(minPackedValue, maxPackedValue);
}

int64_t PointValuesWriter::MutableSortingPointValues::estimatePointCount(
    shared_ptr<IntersectVisitor> visitor)
{
  return in_->estimatePointCount(visitor);
}

std::deque<char>
PointValuesWriter::MutableSortingPointValues::getMinPackedValue() throw(
    IOException)
{
  return in_->getMinPackedValue();
}

std::deque<char>
PointValuesWriter::MutableSortingPointValues::getMaxPackedValue() throw(
    IOException)
{
  return in_->getMaxPackedValue();
}

int PointValuesWriter::MutableSortingPointValues::getNumDimensions() throw(
    IOException)
{
  return in_->getNumDimensions();
}

int PointValuesWriter::MutableSortingPointValues::getBytesPerDimension() throw(
    IOException)
{
  return in_->getBytesPerDimension();
}

int64_t PointValuesWriter::MutableSortingPointValues::size()
{
  return in_->size();
}

int PointValuesWriter::MutableSortingPointValues::getDocCount()
{
  return in_->getDocCount();
}

void PointValuesWriter::MutableSortingPointValues::getValue(
    int i, shared_ptr<BytesRef> packedValue)
{
  in_->getValue(i, packedValue);
}

char PointValuesWriter::MutableSortingPointValues::getByteAt(int i, int k)
{
  return in_->getByteAt(i, k);
}

int PointValuesWriter::MutableSortingPointValues::getDocID(int i)
{
  return docMap->oldToNew(in_->getDocID(i));
}

void PointValuesWriter::MutableSortingPointValues::swap(int i, int j)
{
  in_->swap(i, j);
}
} // namespace org::apache::lucene::index