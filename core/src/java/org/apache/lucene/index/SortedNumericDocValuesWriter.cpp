using namespace std;

#include "SortedNumericDocValuesWriter.h"

namespace org::apache::lucene::index
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using SortedNumericSortField =
    org::apache::lucene::search::SortedNumericSortField;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Counter = org::apache::lucene::util::Counter;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

SortedNumericDocValuesWriter::SortedNumericDocValuesWriter(
    shared_ptr<FieldInfo> fieldInfo, shared_ptr<Counter> iwBytesUsed)
    : iwBytesUsed(iwBytesUsed), fieldInfo(fieldInfo)
{
  pending = PackedLongValues::deltaPackedBuilder(PackedInts::COMPACT);
  pendingCounts = PackedLongValues::deltaPackedBuilder(PackedInts::COMPACT);
  docsWithField = make_shared<DocsWithFieldSet>();
  bytesUsed = pending->ramBytesUsed() + pendingCounts->ramBytesUsed() +
              docsWithField->ramBytesUsed() +
              RamUsageEstimator::sizeOf(currentValues);
  iwBytesUsed->addAndGet(bytesUsed);
}

void SortedNumericDocValuesWriter::addValue(int docID, int64_t value)
{
  assert(docID >= currentDoc);
  if (docID != currentDoc) {
    finishCurrentDoc();
    currentDoc = docID;
  }

  addOneValue(value);
  updateBytesUsed();
}

void SortedNumericDocValuesWriter::finishCurrentDoc()
{
  if (currentDoc == -1) {
    return;
  }
  Arrays::sort(currentValues, 0, currentUpto);
  for (int i = 0; i < currentUpto; i++) {
    pending->add(currentValues[i]);
  }
  // record the number of values for this doc
  pendingCounts->add(currentUpto);
  currentUpto = 0;

  docsWithField->add(currentDoc);
}

void SortedNumericDocValuesWriter::finish(int maxDoc) { finishCurrentDoc(); }

void SortedNumericDocValuesWriter::addOneValue(int64_t value)
{
  if (currentUpto == currentValues.size()) {
    currentValues = ArrayUtil::grow(currentValues, currentValues.size() + 1);
  }

  currentValues[currentUpto] = value;
  currentUpto++;
}

void SortedNumericDocValuesWriter::updateBytesUsed()
{
  constexpr int64_t newBytesUsed =
      pending->ramBytesUsed() + pendingCounts->ramBytesUsed() +
      docsWithField->ramBytesUsed() + RamUsageEstimator::sizeOf(currentValues);
  iwBytesUsed->addAndGet(newBytesUsed - bytesUsed);
  bytesUsed = newBytesUsed;
}

shared_ptr<Sorter::DocComparator>
SortedNumericDocValuesWriter::getDocComparator(
    int maxDoc, shared_ptr<SortField> sortField) 
{
  assert(std::dynamic_pointer_cast<SortedNumericSortField>(sortField) !=
         nullptr);
  assert(finalValues == nullptr && finalValuesCount == nullptr);
  finalValues = pending->build();
  finalValuesCount = pendingCounts->build();
  shared_ptr<SortedNumericDocValues> *const docValues =
      make_shared<BufferedSortedNumericDocValues>(finalValues, finalValuesCount,
                                                  docsWithField->begin());
  shared_ptr<SortedNumericSortField> sf =
      std::static_pointer_cast<SortedNumericSortField>(sortField);
  return Sorter::getDocComparator(
      maxDoc, sf, [&]() { nullptr; },
      [&]() {
        org::apache::lucene::search::SortedNumericSelector::wrap(
            docValues, sf->getSelector(), sf->getNumericType());
      });
}

std::deque<std::deque<int64_t>> SortedNumericDocValuesWriter::sortDocValues(
    int maxDoc, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<SortedNumericDocValues> oldValues) 
{
  std::deque<std::deque<int64_t>> values(maxDoc);
  int docID;
  while ((docID = oldValues->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    int newDocID = sortMap->oldToNew(docID);
    std::deque<int64_t> docValues(oldValues->docValueCount());
    for (int i = 0; i < docValues.size(); i++) {
      docValues[i] = oldValues->nextValue();
    }
    values[newDocID] = docValues;
  }
  return values;
}

void SortedNumericDocValuesWriter::flush(
    shared_ptr<SegmentWriteState> state, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<DocValuesConsumer> dvConsumer) 
{
  shared_ptr<PackedLongValues> *const values;
  shared_ptr<PackedLongValues> *const valueCounts;
  if (finalValues == nullptr) {
    values = pending->build();
    valueCounts = pendingCounts->build();
  } else {
    values = finalValues;
    valueCounts = finalValuesCount;
  }

  const std::deque<std::deque<int64_t>> sorted;
  if (sortMap != nullptr) {
    sorted = sortDocValues(state->segmentInfo->maxDoc(), sortMap,
                           make_shared<BufferedSortedNumericDocValues>(
                               values, valueCounts, docsWithField->begin()));
  } else {
    sorted.clear();
  }

  dvConsumer->addSortedNumericField(
      fieldInfo, make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                     shared_from_this(), values, valueCounts, sorted));
}

SortedNumericDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<SortedNumericDocValuesWriter> outerInstance,
        shared_ptr<PackedLongValues> values,
        shared_ptr<PackedLongValues> valueCounts,
        deque<deque<int64_t>> &sorted)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->valueCounts = valueCounts;
  this->sorted = sorted;
}

shared_ptr<SortedNumericDocValues>
SortedNumericDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    getSortedNumeric(shared_ptr<FieldInfo> fieldInfoIn)
{
  if (fieldInfoIn != outerInstance->fieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }
  shared_ptr<SortedNumericDocValues> *const buf =
      make_shared<BufferedSortedNumericDocValues>(
          values, valueCounts, outerInstance->docsWithField->begin());
  if (sorted.empty()) {
    return buf;
  } else {
    return make_shared<SortingLeafReader::SortingSortedNumericDocValues>(
        buf, sorted);
  }
}

SortedNumericDocValuesWriter::BufferedSortedNumericDocValues::
    BufferedSortedNumericDocValues(shared_ptr<PackedLongValues> values,
                                   shared_ptr<PackedLongValues> valueCounts,
                                   shared_ptr<DocIdSetIterator> docsWithField)
    : valuesIter(values->begin()), valueCountsIter(valueCounts->begin()),
      docsWithField(docsWithField)
{
}

int SortedNumericDocValuesWriter::BufferedSortedNumericDocValues::docID()
{
  return docsWithField->docID();
}

int SortedNumericDocValuesWriter::BufferedSortedNumericDocValues::
    nextDoc() 
{
  for (int i = valueUpto; i < valueCount; ++i) {
    valuesIter->next();
  }

  int docID = docsWithField->nextDoc();
  if (docID != NO_MORE_DOCS) {
    valueCount = Math::toIntExact(valueCountsIter->next());
    valueUpto = 0;
  }
  return docID;
}

int SortedNumericDocValuesWriter::BufferedSortedNumericDocValues::advance(
    int target)
{
  throw make_shared<UnsupportedOperationException>();
}

bool SortedNumericDocValuesWriter::BufferedSortedNumericDocValues::advanceExact(
    int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

int SortedNumericDocValuesWriter::BufferedSortedNumericDocValues::
    docValueCount()
{
  return valueCount;
}

int64_t
SortedNumericDocValuesWriter::BufferedSortedNumericDocValues::nextValue()
{
  if (valueUpto == valueCount) {
    throw make_shared<IllegalStateException>();
  }
  valueUpto++;
  return valuesIter->next();
}

int64_t SortedNumericDocValuesWriter::BufferedSortedNumericDocValues::cost()
{
  return docsWithField->cost();
}

shared_ptr<DocIdSetIterator> SortedNumericDocValuesWriter::getDocIdSet()
{
  return docsWithField->begin();
}
} // namespace org::apache::lucene::index