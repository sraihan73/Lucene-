using namespace std;

#include "AssertingDocValuesFormat.h"

namespace org::apache::lucene::codecs::asserting
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using AssertingLeafReader = org::apache::lucene::index::AssertingLeafReader;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

AssertingDocValuesFormat::AssertingDocValuesFormat()
    : org::apache::lucene::codecs::DocValuesFormat(L"Asserting")
{
}

shared_ptr<DocValuesConsumer> AssertingDocValuesFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<DocValuesConsumer> consumer = in_->fieldsConsumer(state);
  assert(consumer != nullptr);
  return make_shared<AssertingDocValuesConsumer>(consumer,
                                                 state->segmentInfo->maxDoc());
}

shared_ptr<DocValuesProducer> AssertingDocValuesFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  assert(state->fieldInfos->hasDocValues());
  shared_ptr<DocValuesProducer> producer = in_->fieldsProducer(state);
  assert(producer != nullptr);
  return make_shared<AssertingDocValuesProducer>(producer,
                                                 state->segmentInfo->maxDoc());
}

AssertingDocValuesFormat::AssertingDocValuesConsumer::
    AssertingDocValuesConsumer(shared_ptr<DocValuesConsumer> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc)
{
}

void AssertingDocValuesFormat::AssertingDocValuesConsumer::addNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  shared_ptr<NumericDocValues> values = valuesProducer->getNumeric(field);

  int docID;
  int lastDocID = -1;
  while ((docID = values->nextDoc()) != NO_MORE_DOCS) {
    assert(docID >= 0 && docID < maxDoc);
    assert(docID > lastDocID);
    lastDocID = docID;
    int64_t value = values->longValue();
  }

  in_->addNumericField(field, valuesProducer);
}

void AssertingDocValuesFormat::AssertingDocValuesConsumer::addBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  shared_ptr<BinaryDocValues> values = valuesProducer->getBinary(field);

  int docID;
  int lastDocID = -1;
  while ((docID = values->nextDoc()) != NO_MORE_DOCS) {
    assert(docID >= 0 && docID < maxDoc);
    assert(docID > lastDocID);
    lastDocID = docID;
    shared_ptr<BytesRef> value = values->binaryValue();
    assert(value->isValid());
  }

  in_->addBinaryField(field, valuesProducer);
}

void AssertingDocValuesFormat::AssertingDocValuesConsumer::addSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  shared_ptr<SortedDocValues> values = valuesProducer->getSorted(field);

  int valueCount = values->getValueCount();
  assert(valueCount <= maxDoc);
  shared_ptr<BytesRef> lastValue = nullptr;
  for (int ord = 0; ord < valueCount; ord++) {
    shared_ptr<BytesRef> b = values->lookupOrd(ord);
    assert(b != nullptr);
    assert(b->isValid());
    if (ord > 0) {
      assert(b->compareTo(lastValue) > 0);
    }
    lastValue = BytesRef::deepCopyOf(b);
  }

  shared_ptr<FixedBitSet> seenOrds = make_shared<FixedBitSet>(valueCount);

  int docID;
  int lastDocID = -1;
  while ((docID = values->nextDoc()) != NO_MORE_DOCS) {
    assert(docID >= 0 && docID < maxDoc);
    assert(docID > lastDocID);
    lastDocID = docID;
    int ord = values->ordValue();
    assert(ord >= 0 && ord < valueCount);
    seenOrds->set(ord);
  }

  assert(seenOrds->cardinality() == valueCount);
  in_->addSortedField(field, valuesProducer);
}

void AssertingDocValuesFormat::AssertingDocValuesConsumer::
    addSortedNumericField(
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> valuesProducer) 
{
  shared_ptr<SortedNumericDocValues> values =
      valuesProducer->getSortedNumeric(field);

  int64_t valueCount = 0;
  int lastDocID = -1;
  while (true) {
    int docID = values->nextDoc();
    if (docID == NO_MORE_DOCS) {
      break;
    }
    assert(values->docID() > lastDocID);
    lastDocID = values->docID();
    int count = values->docValueCount();
    assert(count > 0);
    valueCount += count;
    int64_t previous = numeric_limits<int64_t>::min();
    for (int i = 0; i < count; i++) {
      int64_t nextValue = values->nextValue();
      assert(nextValue >= previous);
      previous = nextValue;
    }
  }
  in_->addSortedNumericField(field, valuesProducer);
}

void AssertingDocValuesFormat::AssertingDocValuesConsumer::addSortedSetField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  shared_ptr<SortedSetDocValues> values = valuesProducer->getSortedSet(field);

  int64_t valueCount = values->getValueCount();
  shared_ptr<BytesRef> lastValue = nullptr;
  for (int64_t i = 0; i < valueCount; i++) {
    shared_ptr<BytesRef> b = values->lookupOrd(i);
    assert(b != nullptr);
    assert(b->isValid());
    if (i > 0) {
      assert(b->compareTo(lastValue) > 0);
    }
    lastValue = BytesRef::deepCopyOf(b);
  }

  int docCount = 0;
  shared_ptr<LongBitSet> seenOrds = make_shared<LongBitSet>(valueCount);
  while (true) {
    int docID = values->nextDoc();
    if (docID == NO_MORE_DOCS) {
      break;
    }
    docCount++;

    int64_t lastOrd = -1;
    while (true) {
      int64_t ord = values->nextOrd();
      if (ord == SortedSetDocValues::NO_MORE_ORDS) {
        break;
      }
      assert((ord >= 0 && ord < valueCount, L"ord=" + to_wstring(ord) +
                                                L" is not in bounds 0 .." +
                                                to_wstring(valueCount - 1)));
      assert((ord > lastOrd,
              L"ord=" + to_wstring(ord) + L",lastOrd=" + to_wstring(lastOrd)));
      seenOrds->set(ord);
      lastOrd = ord;
    }
  }

  assert(seenOrds->cardinality() == valueCount);
  in_->addSortedSetField(field, valuesProducer);
}

AssertingDocValuesFormat::AssertingDocValuesConsumer::
    ~AssertingDocValuesConsumer()
{
  delete in_;
  delete in_; // close again
}

AssertingDocValuesFormat::AssertingDocValuesProducer::
    AssertingDocValuesProducer(shared_ptr<DocValuesProducer> in_, int maxDoc)
    : in_(in_), maxDoc(maxDoc)
{
  // do a few simple checks on init
  assert(toString() != L"");
  assert(ramBytesUsed() >= 0);
  assert(getChildResources() != nullptr);
}

shared_ptr<NumericDocValues>
AssertingDocValuesFormat::AssertingDocValuesProducer::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  assert(field->getDocValuesType() == DocValuesType::NUMERIC);
  shared_ptr<NumericDocValues> values = in_->getNumeric(field);
  assert(values != nullptr);
  return make_shared<AssertingLeafReader::AssertingNumericDocValues>(values,
                                                                     maxDoc);
}

shared_ptr<BinaryDocValues>
AssertingDocValuesFormat::AssertingDocValuesProducer::getBinary(
    shared_ptr<FieldInfo> field) 
{
  assert(field->getDocValuesType() == DocValuesType::BINARY);
  shared_ptr<BinaryDocValues> values = in_->getBinary(field);
  assert(values != nullptr);
  return make_shared<AssertingLeafReader::AssertingBinaryDocValues>(values,
                                                                    maxDoc);
}

shared_ptr<SortedDocValues>
AssertingDocValuesFormat::AssertingDocValuesProducer::getSorted(
    shared_ptr<FieldInfo> field) 
{
  assert(field->getDocValuesType() == DocValuesType::SORTED);
  shared_ptr<SortedDocValues> values = in_->getSorted(field);
  assert(values != nullptr);
  return make_shared<AssertingLeafReader::AssertingSortedDocValues>(values,
                                                                    maxDoc);
}

shared_ptr<SortedNumericDocValues>
AssertingDocValuesFormat::AssertingDocValuesProducer::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  assert(field->getDocValuesType() == DocValuesType::SORTED_NUMERIC);
  shared_ptr<SortedNumericDocValues> values = in_->getSortedNumeric(field);
  assert(values != nullptr);
  return make_shared<AssertingLeafReader::AssertingSortedNumericDocValues>(
      values, maxDoc);
}

shared_ptr<SortedSetDocValues>
AssertingDocValuesFormat::AssertingDocValuesProducer::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  assert(field->getDocValuesType() == DocValuesType::SORTED_SET);
  shared_ptr<SortedSetDocValues> values = in_->getSortedSet(field);
  assert(values != nullptr);
  return make_shared<AssertingLeafReader::AssertingSortedSetDocValues>(values,
                                                                       maxDoc);
}

AssertingDocValuesFormat::AssertingDocValuesProducer::
    ~AssertingDocValuesProducer()
{
  delete in_;
  delete in_; // close again
}

int64_t AssertingDocValuesFormat::AssertingDocValuesProducer::ramBytesUsed()
{
  int64_t v = in_->ramBytesUsed();
  assert(v >= 0);
  return v;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
AssertingDocValuesFormat::AssertingDocValuesProducer::getChildResources()
{
  shared_ptr<deque<std::shared_ptr<Accountable>>> res =
      in_->getChildResources();
  TestUtil::checkReadOnly(res);
  return res;
}

void AssertingDocValuesFormat::AssertingDocValuesProducer::
    checkIntegrity() 
{
  in_->checkIntegrity();
}

shared_ptr<DocValuesProducer>
AssertingDocValuesFormat::AssertingDocValuesProducer::getMergeInstance() throw(
    IOException)
{
  return make_shared<AssertingDocValuesProducer>(in_->getMergeInstance(),
                                                 maxDoc);
}

wstring AssertingDocValuesFormat::AssertingDocValuesProducer::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + in_->toString() + L")";
}
} // namespace org::apache::lucene::codecs::asserting