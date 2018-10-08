using namespace std;

#include "NumericDocValuesWriter.h"

namespace org::apache::lucene::index
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using Counter = org::apache::lucene::util::Counter;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

NumericDocValuesWriter::NumericDocValuesWriter(shared_ptr<FieldInfo> fieldInfo,
                                               shared_ptr<Counter> iwBytesUsed)
    : iwBytesUsed(iwBytesUsed), fieldInfo(fieldInfo)
{
  pending = PackedLongValues::deltaPackedBuilder(PackedInts::COMPACT);
  docsWithField = make_shared<DocsWithFieldSet>();
  bytesUsed = pending->ramBytesUsed() + docsWithField->ramBytesUsed();
  iwBytesUsed->addAndGet(bytesUsed);
}

void NumericDocValuesWriter::addValue(int docID, int64_t value)
{
  if (docID <= lastDocID) {
    throw invalid_argument(L"DocValuesField \"" + fieldInfo->name +
                           L"\" appears more than once in this document (only "
                           L"one value is allowed per field)");
  }

  pending->add(value);
  docsWithField->add(docID);

  updateBytesUsed();

  lastDocID = docID;
}

void NumericDocValuesWriter::updateBytesUsed()
{
  constexpr int64_t newBytesUsed =
      pending->ramBytesUsed() + docsWithField->ramBytesUsed();
  iwBytesUsed->addAndGet(newBytesUsed - bytesUsed);
  bytesUsed = newBytesUsed;
}

void NumericDocValuesWriter::finish(int maxDoc) {}

shared_ptr<Sorter::DocComparator> NumericDocValuesWriter::getDocComparator(
    int maxDoc, shared_ptr<SortField> sortField) 
{
  assert(finalValues == nullptr);
  finalValues = pending->build();
  shared_ptr<BufferedNumericDocValues> *const docValues =
      make_shared<BufferedNumericDocValues>(finalValues,
                                            docsWithField->begin());
  return Sorter::getDocComparator(maxDoc, sortField, [&]() { nullptr; },
                                  [&]() { docValues; });
}

shared_ptr<DocIdSetIterator> NumericDocValuesWriter::getDocIdSet()
{
  return docsWithField->begin();
}

shared_ptr<SortingLeafReader::CachedNumericDVs>
NumericDocValuesWriter::sortDocValues(
    int maxDoc, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<NumericDocValues> oldDocValues) 
{
  shared_ptr<FixedBitSet> docsWithField = make_shared<FixedBitSet>(maxDoc);
  std::deque<int64_t> values(maxDoc);
  while (true) {
    int docID = oldDocValues->nextDoc();
    if (docID == DocIdSetIterator::NO_MORE_DOCS) {
      break;
    }
    int newDocID = sortMap->oldToNew(docID);
    docsWithField->set(newDocID);
    values[newDocID] = oldDocValues->longValue();
  }
  return make_shared<SortingLeafReader::CachedNumericDVs>(values,
                                                          docsWithField);
}

void NumericDocValuesWriter::flush(
    shared_ptr<SegmentWriteState> state, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<DocValuesConsumer> dvConsumer) 
{
  shared_ptr<PackedLongValues> *const values;
  if (finalValues == nullptr) {
    values = pending->build();
  } else {
    values = finalValues;
  }

  shared_ptr<SortingLeafReader::CachedNumericDVs> *const sorted;
  if (sortMap != nullptr) {
    shared_ptr<NumericDocValues> oldValues =
        make_shared<BufferedNumericDocValues>(values, docsWithField->begin());
    sorted = sortDocValues(state->segmentInfo->maxDoc(), sortMap, oldValues);
  } else {
    sorted.reset();
  }

  dvConsumer->addNumericField(
      fieldInfo, make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                     shared_from_this(), values, sorted));
}

NumericDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<NumericDocValuesWriter> outerInstance,
        shared_ptr<PackedLongValues> values,
        shared_ptr<
            org::apache::lucene::index::SortingLeafReader::CachedNumericDVs>
            sorted)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->sorted = sorted;
}

shared_ptr<NumericDocValues>
NumericDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::getNumeric(
    shared_ptr<FieldInfo> fieldInfo)
{
  if (fieldInfo != outerInstance->fieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }
  if (sorted == nullptr) {
    return make_shared<BufferedNumericDocValues>(
        values, outerInstance->docsWithField->begin());
  } else {
    return make_shared<SortingLeafReader::SortingNumericDocValues>(sorted);
  }
}

NumericDocValuesWriter::BufferedNumericDocValues::BufferedNumericDocValues(
    shared_ptr<PackedLongValues> values,
    shared_ptr<DocIdSetIterator> docsWithFields)
    : iter(values->begin()), docsWithField(docsWithFields)
{
}

int NumericDocValuesWriter::BufferedNumericDocValues::docID()
{
  return docsWithField->docID();
}

int NumericDocValuesWriter::BufferedNumericDocValues::nextDoc() throw(
    IOException)
{
  int docID = docsWithField->nextDoc();
  if (docID != NO_MORE_DOCS) {
    value = iter->next();
  }
  return docID;
}

int NumericDocValuesWriter::BufferedNumericDocValues::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

bool NumericDocValuesWriter::BufferedNumericDocValues::advanceExact(
    int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t NumericDocValuesWriter::BufferedNumericDocValues::cost()
{
  return docsWithField->cost();
}

int64_t NumericDocValuesWriter::BufferedNumericDocValues::longValue()
{
  return value;
}
} // namespace org::apache::lucene::index