using namespace std;

#include "SortedDocValuesWriter.h"

namespace org::apache::lucene::index
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using DirectBytesStartArray =
    org::apache::lucene::util::BytesRefHash::DirectBytesStartArray;
using Counter = org::apache::lucene::util::Counter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;

SortedDocValuesWriter::SortedDocValuesWriter(shared_ptr<FieldInfo> fieldInfo,
                                             shared_ptr<Counter> iwBytesUsed)
    : hash(make_shared<BytesRefHash>(
          make_shared<ByteBlockPool>(
              make_shared<ByteBlockPool::DirectTrackingAllocator>(iwBytesUsed)),
          BytesRefHash::DEFAULT_CAPACITY,
          make_shared<
              org::apache::lucene::util::BytesRefHash::DirectBytesStartArray>(
              BytesRefHash::DEFAULT_CAPACITY, iwBytesUsed))),
      iwBytesUsed(iwBytesUsed), fieldInfo(fieldInfo)
{
  pending = PackedLongValues::deltaPackedBuilder(PackedInts::COMPACT);
  docsWithField = make_shared<DocsWithFieldSet>();
  bytesUsed = pending->ramBytesUsed() + docsWithField->ramBytesUsed();
  iwBytesUsed->addAndGet(bytesUsed);
}

void SortedDocValuesWriter::addValue(int docID, shared_ptr<BytesRef> value)
{
  if (docID <= lastDocID) {
    throw invalid_argument(L"DocValuesField \"" + fieldInfo->name +
                           L"\" appears more than once in this document (only "
                           L"one value is allowed per field)");
  }
  if (value == nullptr) {
    throw invalid_argument(L"field \"" + fieldInfo->name +
                           L"\": null value not allowed");
  }
  if (value->length > (ByteBlockPool::BYTE_BLOCK_SIZE - 2)) {
    throw invalid_argument(L"DocValuesField \"" + fieldInfo->name +
                           L"\" is too large, must be <= " +
                           to_wstring(ByteBlockPool::BYTE_BLOCK_SIZE - 2));
  }

  addOneValue(value);
  docsWithField->add(docID);

  lastDocID = docID;
}

void SortedDocValuesWriter::finish(int maxDoc) { updateBytesUsed(); }

void SortedDocValuesWriter::addOneValue(shared_ptr<BytesRef> value)
{
  int termID = hash->add(value);
  if (termID < 0) {
    termID = -termID - 1;
  } else {
    // reserve additional space for each unique value:
    // 1. when indexing, when hash is 50% full, rehash() suddenly needs 2*size
    // ints.
    //    TODO: can this same OOM happen in THPF?
    // 2. when flushing, we need 1 int per value (slot in the ordMap).
    iwBytesUsed->addAndGet(2 * Integer::BYTES);
  }

  pending->add(termID);
  updateBytesUsed();
}

void SortedDocValuesWriter::updateBytesUsed()
{
  constexpr int64_t newBytesUsed =
      pending->ramBytesUsed() + docsWithField->ramBytesUsed();
  iwBytesUsed->addAndGet(newBytesUsed - bytesUsed);
  bytesUsed = newBytesUsed;
}

shared_ptr<Sorter::DocComparator> SortedDocValuesWriter::getDocComparator(
    int maxDoc, shared_ptr<SortField> sortField) 
{
  assert(sortField->getType().equals(SortField::Type::STRING));
  assert(finalSortedValues.empty() && finalOrdMap.empty() &&
         finalOrds == nullptr);
  int valueCount = hash->size();
  finalSortedValues = hash->sort();
  finalOrds = pending->build();
  finalOrdMap = std::deque<int>(valueCount);
  for (int ord = 0; ord < valueCount; ord++) {
    finalOrdMap[finalSortedValues[ord]] = ord;
  }
  shared_ptr<SortedDocValues> *const docValues =
      make_shared<BufferedSortedDocValues>(hash, valueCount, finalOrds,
                                           finalSortedValues, finalOrdMap,
                                           docsWithField->begin());
  return Sorter::getDocComparator(maxDoc, sortField, [&]() { docValues; },
                                  [&]() { nullptr; });
}

std::deque<int> SortedDocValuesWriter::sortDocValues(
    int maxDoc, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<SortedDocValues> oldValues) 
{
  std::deque<int> ords(maxDoc);
  Arrays::fill(ords, -1);
  int docID;
  while ((docID = oldValues->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    int newDocID = sortMap->oldToNew(docID);
    ords[newDocID] = oldValues->ordValue();
  }
  return ords;
}

void SortedDocValuesWriter::flush(
    shared_ptr<SegmentWriteState> state, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<DocValuesConsumer> dvConsumer) 
{
  constexpr int valueCount = hash->size();
  shared_ptr<PackedLongValues> *const ords;
  const std::deque<int> sortedValues;
  const std::deque<int> ordMap;
  if (finalOrds == nullptr) {
    sortedValues = hash->sort();
    ords = pending->build();
    ordMap = std::deque<int>(valueCount);
    for (int ord = 0; ord < valueCount; ord++) {
      ordMap[sortedValues[ord]] = ord;
    }
  } else {
    sortedValues = finalSortedValues;
    ords = finalOrds;
    ordMap = finalOrdMap;
  }

  const std::deque<int> sorted;
  if (sortMap != nullptr) {
    sorted = sortDocValues(state->segmentInfo->maxDoc(), sortMap,
                           make_shared<BufferedSortedDocValues>(
                               hash, valueCount, ords, sortedValues, ordMap,
                               docsWithField->begin()));
  } else {
    sorted.clear();
  }
  dvConsumer->addSortedField(
      fieldInfo,
      make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
          shared_from_this(), valueCount, ords, sortedValues, ordMap, sorted));
}

SortedDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<SortedDocValuesWriter> outerInstance, int valueCount,
        shared_ptr<PackedLongValues> ords, deque<int> &sortedValues,
        deque<int> &ordMap, deque<int> &sorted)
{
  this->outerInstance = outerInstance;
  this->valueCount = valueCount;
  this->ords = ords;
  this->sortedValues = sortedValues;
  this->ordMap = ordMap;
  this->sorted = sorted;
}

shared_ptr<SortedDocValues>
SortedDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::getSorted(
    shared_ptr<FieldInfo> fieldInfoIn)
{
  if (fieldInfoIn != outerInstance->fieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }
  shared_ptr<SortedDocValues> *const buf = make_shared<BufferedSortedDocValues>(
      outerInstance->hash, valueCount, ords, sortedValues, ordMap,
      outerInstance->docsWithField->begin());
  if (sorted.empty()) {
    return buf;
  }
  return make_shared<SortingLeafReader::SortingSortedDocValues>(buf, sorted);
}

SortedDocValuesWriter::BufferedSortedDocValues::BufferedSortedDocValues(
    shared_ptr<BytesRefHash> hash, int valueCount,
    shared_ptr<PackedLongValues> docToOrd, std::deque<int> &sortedValues,
    std::deque<int> &ordMap, shared_ptr<DocIdSetIterator> docsWithField)
    : hash(hash), sortedValues(sortedValues), ordMap(ordMap),
      valueCount(valueCount), iter(docToOrd->begin()),
      docsWithField(docsWithField)
{
}

int SortedDocValuesWriter::BufferedSortedDocValues::docID()
{
  return docsWithField->docID();
}

int SortedDocValuesWriter::BufferedSortedDocValues::nextDoc() 
{
  int docID = docsWithField->nextDoc();
  if (docID != NO_MORE_DOCS) {
    ord = Math::toIntExact(iter->next());
    ord = ordMap[ord];
  }
  return docID;
}

int SortedDocValuesWriter::BufferedSortedDocValues::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

bool SortedDocValuesWriter::BufferedSortedDocValues::advanceExact(
    int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t SortedDocValuesWriter::BufferedSortedDocValues::cost()
{
  return docsWithField->cost();
}

int SortedDocValuesWriter::BufferedSortedDocValues::ordValue() { return ord; }

shared_ptr<BytesRef>
SortedDocValuesWriter::BufferedSortedDocValues::lookupOrd(int ord)
{
  assert(ord >= 0 && ord < sortedValues.size());
  assert(sortedValues[ord] >= 0 && sortedValues[ord] < sortedValues.size());
  hash->get(sortedValues[ord], scratch);
  return scratch;
}

int SortedDocValuesWriter::BufferedSortedDocValues::getValueCount()
{
  return valueCount;
}

shared_ptr<DocIdSetIterator> SortedDocValuesWriter::getDocIdSet()
{
  return docsWithField->begin();
}
} // namespace org::apache::lucene::index