using namespace std;

#include "NormValuesWriter.h"

namespace org::apache::lucene::index
{
using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Counter = org::apache::lucene::util::Counter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

NormValuesWriter::NormValuesWriter(shared_ptr<FieldInfo> fieldInfo,
                                   shared_ptr<Counter> iwBytesUsed)
    : iwBytesUsed(iwBytesUsed), fieldInfo(fieldInfo)
{
  docsWithField = make_shared<DocsWithFieldSet>();
  pending = PackedLongValues::deltaPackedBuilder(PackedInts::COMPACT);
  bytesUsed = pending->ramBytesUsed() + docsWithField->ramBytesUsed();
  iwBytesUsed->addAndGet(bytesUsed);
}

void NormValuesWriter::addValue(int docID, int64_t value)
{
  if (docID <= lastDocID) {
    throw invalid_argument(L"Norm for \"" + fieldInfo->name +
                           L"\" appears more than once in this document (only "
                           L"one value is allowed per field)");
  }

  pending->add(value);
  docsWithField->add(docID);

  updateBytesUsed();

  lastDocID = docID;
}

void NormValuesWriter::updateBytesUsed()
{
  constexpr int64_t newBytesUsed = pending->ramBytesUsed();
  iwBytesUsed->addAndGet(newBytesUsed - bytesUsed);
  bytesUsed = newBytesUsed;
}

void NormValuesWriter::finish(int maxDoc) {}

void NormValuesWriter::flush(
    shared_ptr<SegmentWriteState> state, shared_ptr<Sorter::DocMap> sortMap,
    shared_ptr<NormsConsumer> normsConsumer) 
{
  shared_ptr<PackedLongValues> *const values = pending->build();
  shared_ptr<SortingLeafReader::CachedNumericDVs> *const sorted;
  if (sortMap != nullptr) {
    sorted = NumericDocValuesWriter::sortDocValues(
        state->segmentInfo->maxDoc(), sortMap,
        make_shared<BufferedNorms>(values, docsWithField->begin()));
  } else {
    sorted.reset();
  }
  normsConsumer->addNormsField(fieldInfo,
                               make_shared<NormsProducerAnonymousInnerClass>(
                                   shared_from_this(), values, sorted));
}

NormValuesWriter::NormsProducerAnonymousInnerClass::
    NormsProducerAnonymousInnerClass(
        shared_ptr<NormValuesWriter> outerInstance,
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
NormValuesWriter::NormsProducerAnonymousInnerClass::getNorms(
    shared_ptr<FieldInfo> fieldInfo2)
{
  if (outerInstance->fieldInfo != outerInstance->fieldInfo) {
    throw invalid_argument(L"wrong fieldInfo");
  }
  if (sorted == nullptr) {
    return make_shared<BufferedNorms>(values,
                                      outerInstance->docsWithField->begin());
  } else {
    return make_shared<SortingLeafReader::SortingNumericDocValues>(sorted);
  }
}

void NormValuesWriter::NormsProducerAnonymousInnerClass::checkIntegrity() {}

NormValuesWriter::NormsProducerAnonymousInnerClass::
    ~NormsProducerAnonymousInnerClass()
{
}

int64_t NormValuesWriter::NormsProducerAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}

NormValuesWriter::BufferedNorms::BufferedNorms(
    shared_ptr<PackedLongValues> values,
    shared_ptr<DocIdSetIterator> docsWithFields)
    : iter(values->begin()), docsWithField(docsWithFields)
{
}

int NormValuesWriter::BufferedNorms::docID() { return docsWithField->docID(); }

int NormValuesWriter::BufferedNorms::nextDoc() 
{
  int docID = docsWithField->nextDoc();
  if (docID != NO_MORE_DOCS) {
    value = iter->next();
  }
  return docID;
}

int NormValuesWriter::BufferedNorms::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

bool NormValuesWriter::BufferedNorms::advanceExact(int target) throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t NormValuesWriter::BufferedNorms::cost()
{
  return docsWithField->cost();
}

int64_t NormValuesWriter::BufferedNorms::longValue() { return value; }
} // namespace org::apache::lucene::index