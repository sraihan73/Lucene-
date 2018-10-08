using namespace std;

#include "SortedSetDocValuesWriter.h"

namespace org::apache::lucene::index
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;
using SortedSetSortField = org::apache::lucene::search::SortedSetSortField;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using DirectBytesStartArray =
    org::apache::lucene::util::BytesRefHash::DirectBytesStartArray;
using Counter = org::apache::lucene::util::Counter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;

SortedSetDocValuesWriter::SortedSetDocValuesWriter(
    shared_ptr<FieldInfo> fieldInfo, shared_ptr<Counter> iwBytesUsed)
    : hash(make_shared<BytesRefHash>(
          make_shared<ByteBlockPool>(
              make_shared<ByteBlockPool::DirectTrackingAllocator>(iwBytesUsed)),
          BytesRefHash::DEFAULT_CAPACITY,
          make_shared<
              org::apache::lucene::util::BytesRefHash::DirectBytesStartArray>(
              BytesRefHash::DEFAULT_CAPACITY, iwBytesUsed))),
      iwBytesUsed(iwBytesUsed), fieldInfo(fieldInfo)
{
  pending = PackedLongValues::packedBuilder(PackedInts::COMPACT);
  pendingCounts = PackedLongValues::deltaPackedBuilder(PackedInts::COMPACT);
  docsWithField = make_shared<DocsWithFieldSet>();
  bytesUsed = pending->ramBytesUsed() + pendingCounts->ramBytesUsed();
  iwBytesUsed->addAndGet(bytesUsed);
}

void SortedSetDocValuesWriter::addValue(int docID, shared_ptr<BytesRef> value)
{
  assert(docID >= currentDoc);
  if (value == nullptr) {
    throw invalid_argument(L"field \"" + fieldInfo->name +
                           L"\": null value not allowed");
  }
  if (value->length > (ByteBlockPool::BYTE_BLOCK_SIZE - 2)) {
    throw invalid_argument(L"DocValuesField \"" + fieldInfo->name +
                           L"\" is too large, must be <= " +
                           to_wstring(ByteBlockPool::BYTE_BLOCK_SIZE - 2));
  }

  if (docID != currentDoc) {
    finishCurrentDoc();
    currentDoc = docID;
  }

  addOneValue(value);
  updateBytesUsed();
}

void SortedSetDocValuesWriter::finishCurrentDoc()
{
  if (currentDoc == -1) {
    return;
  }
  Arrays::sort(currentValues, 0, currentUpto);
  int lastValue = -1;
  int count = 0;
  for (int i = 0; i < currentUpto; i++) {
    int termID = currentValues[i];
    // if it's not a duplicate
    if (termID != lastValue) {
      pending->add(termID); // record the term id
      count++;
    }
    lastValue = termID;
  }
  // record the number of unique term ids for this doc
  pendingCounts->add(count);
  maxCount = max(maxCount, count);
  currentUpto = 0;
  docsWithField->add(currentDoc);
}

void SortedSetDocValuesWriter::finish(int maxDoc) { finishCurrentDoc(); }

void SortedSetDocValuesWriter::addOneValue(shared_ptr<BytesRef> value)
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

  if (currentUpto == currentValues.size()) {
    currentValues = ArrayUtil::grow(currentValues, currentValues.size() + 1);
    iwBytesUsed->addAndGet((currentValues.size() - currentUpto) *
                           Integer::BYTES);
  }

  currentValues[currentUpto] = termID;
  currentUpto++;
}

void SortedSetDocValuesWriter::updateBytesUsed()
{
  constexpr int64_t newBytesUsed =
      pending->ramBytesUsed() + pendingCounts->ramBytesUsed();
  iwBytesUsed->addAndGet(newBytesUsed - bytesUsed);
  bytesUsed = newBytesUsed;
}

std::deque<std::deque<int64_t>> SortedSetDocValuesWriter::sortDocValues(
    int maxDoc, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<SortedSetDocValues> oldValues) 
{
  std::deque<std::deque<int64_t>> ords(maxDoc);
  int docID;
  while ((docID = oldValues->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    int newDocID = sortMap->oldToNew(docID);
    std::deque<int64_t> docOrds(1);
    int upto = 0;
    while (true) {
      int64_t ord = oldValues->nextOrd();
      if (ord == NO_MORE_ORDS) {
        break;
      }
      if (upto == docOrds.size()) {
        docOrds = ArrayUtil::grow(docOrds);
      }
      docOrds[upto++] = ord;
    }
    ords[newDocID] = Arrays::copyOfRange(docOrds, 0, upto);
  }
  return ords;
}

shared_ptr<Sorter::DocComparator> SortedSetDocValuesWriter::getDocComparator(
    int maxDoc, shared_ptr<SortField> sortField) 
{
  assert(std::dynamic_pointer_cast<SortedSetSortField>(sortField) != nullptr);
  assert(finalOrds == nullptr && finalOrdCounts == nullptr &&
         finalSortedValues.empty() && finalOrdMap.empty());
  int valueCount = hash->size();
  finalOrds = pending->build();
  finalOrdCounts = pendingCounts->build();
  finalSortedValues = hash->sort();
  finalOrdMap = std::deque<int>(valueCount);
  for (int ord = 0; ord < valueCount; ord++) {
    finalOrdMap[finalSortedValues[ord]] = ord;
  }

  shared_ptr<SortedSetSortField> sf =
      std::static_pointer_cast<SortedSetSortField>(sortField);
  shared_ptr<SortedSetDocValues> *const dvs =
      make_shared<BufferedSortedSetDocValues>(finalSortedValues, finalOrdMap,
                                              hash, finalOrds, finalOrdCounts,
                                              maxCount, docsWithField->begin());
  return Sorter::getDocComparator(
      maxDoc, sf,
      [&]() {
        org::apache::lucene::search::SortedSetSelector::wrap(dvs,
                                                             sf->getSelector());
      },
      [&]() { nullptr; });
}

void SortedSetDocValuesWriter::flush(
    shared_ptr<SegmentWriteState> state, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<DocValuesConsumer> dvConsumer) 
{
  constexpr int valueCount = hash->size();
  shared_ptr<PackedLongValues> *const ords;
  shared_ptr<PackedLongValues> *const ordCounts;
  const std::deque<int> sortedValues;
  const std::deque<int> ordMap;

  if (finalOrdCounts == nullptr) {
    ords = pending->build();
    ordCounts = pendingCounts->build();
    sortedValues = hash->sort();
    ordMap = std::deque<int>(valueCount);
    for (int ord = 0; ord < valueCount; ord++) {
      ordMap[sortedValues[ord]] = ord;
    }
  } else {
    ords = finalOrds;
    ordCounts = finalOrdCounts;
    sortedValues = finalSortedValues;
    ordMap = finalOrdMap;
  }

  const std::deque<std::deque<int64_t>> sorted;
  if (sortMap != nullptr) {
    sorted = sortDocValues(state->segmentInfo->maxDoc(), sortMap,
                           make_shared<BufferedSortedSetDocValues>(
                               sortedValues, ordMap, hash, ords, ordCounts,
                               maxCount, docsWithField->begin()));
  } else {
    sorted.clear();
  }
  dvConsumer->addSortedSetField(
      fieldInfo,
      make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
          shared_from_this(), ords, ordCounts, sortedValues, ordMap, sorted));
}

SortedSetDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<SortedSetDocValuesWriter> outerInstance,
        shared_ptr<PackedLongValues> ords,
        shared_ptr<PackedLongValues> ordCounts, deque<int> &sortedValues,
        deque<int> &ordMap, deque<deque<int64_t>> &sorted)
{
  this->outerInstance = outerInstance;
  this->ords = ords;
  this->ordCounts = ordCounts;
  this->sortedValues = sortedValues;
  this->ordMap = ordMap;
  this->sorted = sorted;
}

shared_ptr<SortedSetDocValues>
SortedSetDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    getSortedSet(shared_ptr<FieldInfo> fieldInfoIn)
{
  if (fieldInfoIn != outerInstance->fieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }
  shared_ptr<SortedSetDocValues> *const buf =
      make_shared<BufferedSortedSetDocValues>(
          sortedValues, ordMap, outerInstance->hash, ords, ordCounts,
          outerInstance->maxCount, outerInstance->docsWithField->begin());
  if (sorted.empty()) {
    return buf;
  } else {
    return make_shared<SortingLeafReader::SortingSortedSetDocValues>(buf,
                                                                     sorted);
  }
}

SortedSetDocValuesWriter::BufferedSortedSetDocValues::
    BufferedSortedSetDocValues(std::deque<int> &sortedValues,
                               std::deque<int> &ordMap,
                               shared_ptr<BytesRefHash> hash,
                               shared_ptr<PackedLongValues> ords,
                               shared_ptr<PackedLongValues> ordCounts,
                               int maxCount,
                               shared_ptr<DocIdSetIterator> docsWithField)
    : sortedValues(sortedValues), ordMap(ordMap), hash(hash),
      ordsIter(ords->begin()), ordCountsIter(ordCounts->begin()),
      docsWithField(docsWithField), currentDoc(std::deque<int>(maxCount))
{
}

int SortedSetDocValuesWriter::BufferedSortedSetDocValues::docID()
{
  return docsWithField->docID();
}

int SortedSetDocValuesWriter::BufferedSortedSetDocValues::nextDoc() throw(
    IOException)
{
  int docID = docsWithField->nextDoc();
  if (docID != NO_MORE_DOCS) {
    ordCount = static_cast<int>(ordCountsIter->next());
    assert(ordCount > 0);
    for (int i = 0; i < ordCount; i++) {
      currentDoc[i] = ordMap[Math::toIntExact(ordsIter->next())];
    }
    Arrays::sort(currentDoc, 0, ordCount);
    ordUpto = 0;
  }
  return docID;
}

int64_t SortedSetDocValuesWriter::BufferedSortedSetDocValues::nextOrd()
{
  if (ordUpto == ordCount) {
    return NO_MORE_ORDS;
  } else {
    return currentDoc[ordUpto++];
  }
}

int64_t SortedSetDocValuesWriter::BufferedSortedSetDocValues::cost()
{
  return docsWithField->cost();
}

int SortedSetDocValuesWriter::BufferedSortedSetDocValues::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

bool SortedSetDocValuesWriter::BufferedSortedSetDocValues::advanceExact(
    int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t SortedSetDocValuesWriter::BufferedSortedSetDocValues::getValueCount()
{
  return ordMap.size();
}

shared_ptr<BytesRef>
SortedSetDocValuesWriter::BufferedSortedSetDocValues::lookupOrd(int64_t ord)
{
  assert((ord >= 0 && ord < ordMap.size(), L"ord=" + to_wstring(ord) +
                                               L" is out of bounds 0 .. " +
                                               (ordMap.size() - 1)));
  hash->get(sortedValues[Math::toIntExact(ord)], scratch);
  return scratch;
}

shared_ptr<DocIdSetIterator> SortedSetDocValuesWriter::getDocIdSet()
{
  return docsWithField->begin();
}
} // namespace org::apache::lucene::index