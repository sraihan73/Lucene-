using namespace std;

#include "BinaryDocValuesWriter.h"

namespace org::apache::lucene::index
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using Counter = org::apache::lucene::util::Counter;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using PagedBytes = org::apache::lucene::util::PagedBytes;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

BinaryDocValuesWriter::BinaryDocValuesWriter(shared_ptr<FieldInfo> fieldInfo,
                                             shared_ptr<Counter> iwBytesUsed)
    : bytes(make_shared<PagedBytes>(BLOCK_BITS)),
      bytesOut(bytes->getDataOutput()), iwBytesUsed(iwBytesUsed),
      lengths(PackedLongValues::deltaPackedBuilder(PackedInts::COMPACT)),
      fieldInfo(fieldInfo)
{
  this->docsWithField = make_shared<DocsWithFieldSet>();
  this->bytesUsed = lengths->ramBytesUsed() + docsWithField->ramBytesUsed();
  iwBytesUsed->addAndGet(bytesUsed);
}

void BinaryDocValuesWriter::addValue(int docID, shared_ptr<BytesRef> value)
{
  if (docID <= lastDocID) {
    throw invalid_argument(L"DocValuesField \"" + fieldInfo->name +
                           L"\" appears more than once in this document (only "
                           L"one value is allowed per field)");
  }
  if (value == nullptr) {
    throw invalid_argument(L"field=\"" + fieldInfo->name +
                           L"\": null value not allowed");
  }
  if (value->length > MAX_LENGTH) {
    throw invalid_argument(L"DocValuesField \"" + fieldInfo->name +
                           L"\" is too large, must be <= " +
                           to_wstring(MAX_LENGTH));
  }

  maxLength = max(value->length, maxLength);
  lengths->add(value->length);
  try {
    bytesOut->writeBytes(value->bytes, value->offset, value->length);
  } catch (const IOException &ioe) {
    // Should never happen!
    throw runtime_error(ioe);
  }
  docsWithField->add(docID);
  updateBytesUsed();

  lastDocID = docID;
}

void BinaryDocValuesWriter::updateBytesUsed()
{
  constexpr int64_t newBytesUsed = lengths->ramBytesUsed() +
                                     bytes->ramBytesUsed() +
                                     docsWithField->ramBytesUsed();
  iwBytesUsed->addAndGet(newBytesUsed - bytesUsed);
  bytesUsed = newBytesUsed;
}

void BinaryDocValuesWriter::finish(int maxDoc) {}

shared_ptr<SortingLeafReader::CachedBinaryDVs>
BinaryDocValuesWriter::sortDocValues(
    int maxDoc, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<BinaryDocValues> oldValues) 
{
  shared_ptr<FixedBitSet> docsWithField = make_shared<FixedBitSet>(maxDoc);
  std::deque<std::shared_ptr<BytesRef>> values(maxDoc);
  while (true) {
    int docID = oldValues->nextDoc();
    if (docID == DocIdSetIterator::NO_MORE_DOCS) {
      break;
    }
    int newDocID = sortMap->oldToNew(docID);
    docsWithField->set(newDocID);
    values[newDocID] = BytesRef::deepCopyOf(oldValues->binaryValue());
  }
  return make_shared<SortingLeafReader::CachedBinaryDVs>(values, docsWithField);
}

shared_ptr<Sorter::DocComparator> BinaryDocValuesWriter::getDocComparator(
    int numDoc, shared_ptr<SortField> sortField) 
{
  throw invalid_argument(L"It is forbidden to sort on a binary field");
}

void BinaryDocValuesWriter::flush(
    shared_ptr<SegmentWriteState> state, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<DocValuesConsumer> dvConsumer) 
{
  bytes->freeze(false);
  shared_ptr<PackedLongValues> *const lengths = this->lengths->build();
  shared_ptr<SortingLeafReader::CachedBinaryDVs> *const sorted;
  if (sortMap != nullptr) {
    sorted = sortDocValues(
        state->segmentInfo->maxDoc(), sortMap,
        make_shared<BufferedBinaryDocValues>(
            lengths, maxLength, bytes->getDataInput(), docsWithField->begin()));
  } else {
    sorted.reset();
  }
  dvConsumer->addBinaryField(
      fieldInfo, make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                     shared_from_this(), lengths, sorted));
}

BinaryDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<BinaryDocValuesWriter> outerInstance,
        shared_ptr<PackedLongValues> lengths,
        shared_ptr<
            org::apache::lucene::index::SortingLeafReader::CachedBinaryDVs>
            sorted)
{
  this->outerInstance = outerInstance;
  this->lengths = lengths;
  this->sorted = sorted;
}

shared_ptr<BinaryDocValues>
BinaryDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::getBinary(
    shared_ptr<FieldInfo> fieldInfoIn)
{
  if (fieldInfoIn != outerInstance->fieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }
  if (sorted == nullptr) {
    return make_shared<BufferedBinaryDocValues>(
        lengths, outerInstance->maxLength, outerInstance->bytes->getDataInput(),
        outerInstance->docsWithField->begin());
  } else {
    return make_shared<SortingLeafReader::SortingBinaryDocValues>(sorted);
  }
}

BinaryDocValuesWriter::BufferedBinaryDocValues::BufferedBinaryDocValues(
    shared_ptr<PackedLongValues> lengths, int maxLength,
    shared_ptr<DataInput> bytesIterator,
    shared_ptr<DocIdSetIterator> docsWithFields)
    : value(make_shared<BytesRefBuilder>()), lengthsIterator(lengths->begin()),
      docsWithField(docsWithFields), bytesIterator(bytesIterator)
{
  this->value->grow(maxLength);
}

int BinaryDocValuesWriter::BufferedBinaryDocValues::docID()
{
  return docsWithField->docID();
}

int BinaryDocValuesWriter::BufferedBinaryDocValues::nextDoc() 
{
  int docID = docsWithField->nextDoc();
  if (docID != NO_MORE_DOCS) {
    int length = Math::toIntExact(lengthsIterator->next());
    value->setLength(length);
    bytesIterator->readBytes(value->bytes(), 0, length);
  }
  return docID;
}

int BinaryDocValuesWriter::BufferedBinaryDocValues::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

bool BinaryDocValuesWriter::BufferedBinaryDocValues::advanceExact(
    int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t BinaryDocValuesWriter::BufferedBinaryDocValues::cost()
{
  return docsWithField->cost();
}

shared_ptr<BytesRef>
BinaryDocValuesWriter::BufferedBinaryDocValues::binaryValue()
{
  return value->get();
}

shared_ptr<DocIdSetIterator> BinaryDocValuesWriter::getDocIdSet()
{
  return docsWithField->begin();
}
} // namespace org::apache::lucene::index