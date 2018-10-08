using namespace std;

#include "SlowCodecReaderWrapper.h"

namespace org::apache::lucene::index
{
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Bits = org::apache::lucene::util::Bits;

SlowCodecReaderWrapper::SlowCodecReaderWrapper() {}

shared_ptr<CodecReader>
SlowCodecReaderWrapper::wrap(shared_ptr<LeafReader> reader) 
{
  if (std::dynamic_pointer_cast<CodecReader>(reader) != nullptr) {
    return std::static_pointer_cast<CodecReader>(reader);
  } else {
    // simulate it slowly, over the leafReader api:
    reader->checkIntegrity();
    return make_shared<CodecReaderAnonymousInnerClass>(reader);
  }
}

SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::
    CodecReaderAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::LeafReader> reader)
{
  this->reader = reader;
}

shared_ptr<TermVectorsReader>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getTermVectorsReader()
{
  reader->ensureOpen();
  return readerToTermVectorsReader(reader);
}

shared_ptr<StoredFieldsReader>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getFieldsReader()
{
  reader->ensureOpen();
  return readerToStoredFieldsReader(reader);
}

shared_ptr<NormsProducer>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getNormsReader()
{
  reader->ensureOpen();
  return readerToNormsProducer(reader);
}

shared_ptr<DocValuesProducer>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getDocValuesReader()
{
  reader->ensureOpen();
  return readerToDocValuesProducer(reader);
}

shared_ptr<FieldsProducer>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getPostingsReader()
{
  reader->ensureOpen();
  try {
    return readerToFieldsProducer(reader);
  } catch (const IOException &bogus) {
    throw make_shared<AssertionError>(bogus);
  }
}

shared_ptr<FieldInfos>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getFieldInfos()
{
  return reader->getFieldInfos();
}

shared_ptr<PointsReader>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getPointsReader()
{
  return pointValuesToReader(reader);
}

shared_ptr<Bits>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getLiveDocs()
{
  return reader->getLiveDocs();
}

int SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::numDocs()
{
  return reader->numDocs();
}

int SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::maxDoc()
{
  return reader->maxDoc();
}

shared_ptr<CacheHelper>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return reader->getCoreCacheHelper();
}

shared_ptr<CacheHelper>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return reader->getReaderCacheHelper();
}

wstring SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::toString()
{
  return L"SlowCodecReaderWrapper(" + reader + L")";
}

shared_ptr<LeafMetaData>
SlowCodecReaderWrapper::CodecReaderAnonymousInnerClass::getMetaData()
{
  return reader->getMetaData();
}

shared_ptr<PointsReader>
SlowCodecReaderWrapper::pointValuesToReader(shared_ptr<LeafReader> reader)
{
  return make_shared<PointsReaderAnonymousInnerClass>(reader);
}

SlowCodecReaderWrapper::PointsReaderAnonymousInnerClass::
    PointsReaderAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::LeafReader> reader)
{
  this->reader = reader;
}

shared_ptr<PointValues>
SlowCodecReaderWrapper::PointsReaderAnonymousInnerClass::getValues(
    const wstring &field) 
{
  return reader->getPointValues(field);
}

void SlowCodecReaderWrapper::PointsReaderAnonymousInnerClass::
    checkIntegrity() 
{
  // We already checkIntegrity the entire reader up front
}

SlowCodecReaderWrapper::PointsReaderAnonymousInnerClass::
    ~PointsReaderAnonymousInnerClass()
{
}

int64_t
SlowCodecReaderWrapper::PointsReaderAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}

shared_ptr<NormsProducer>
SlowCodecReaderWrapper::readerToNormsProducer(shared_ptr<LeafReader> reader)
{
  return make_shared<NormsProducerAnonymousInnerClass>(reader);
}

SlowCodecReaderWrapper::NormsProducerAnonymousInnerClass::
    NormsProducerAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::LeafReader> reader)
{
  this->reader = reader;
}

shared_ptr<NumericDocValues>
SlowCodecReaderWrapper::NormsProducerAnonymousInnerClass::getNorms(
    shared_ptr<FieldInfo> field) 
{
  return reader->getNormValues(field->name);
}

void SlowCodecReaderWrapper::NormsProducerAnonymousInnerClass::
    checkIntegrity() 
{
  // We already checkIntegrity the entire reader up front
}

SlowCodecReaderWrapper::NormsProducerAnonymousInnerClass::
    ~NormsProducerAnonymousInnerClass()
{
}

int64_t
SlowCodecReaderWrapper::NormsProducerAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}

shared_ptr<DocValuesProducer>
SlowCodecReaderWrapper::readerToDocValuesProducer(shared_ptr<LeafReader> reader)
{
  return make_shared<DocValuesProducerAnonymousInnerClass>(reader);
}

SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::
    DocValuesProducerAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::LeafReader> reader)
{
  this->reader = reader;
}

shared_ptr<NumericDocValues>
SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  return reader->getNumericDocValues(field->name);
}

shared_ptr<BinaryDocValues>
SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::getBinary(
    shared_ptr<FieldInfo> field) 
{
  return reader->getBinaryDocValues(field->name);
}

shared_ptr<SortedDocValues>
SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::getSorted(
    shared_ptr<FieldInfo> field) 
{
  return reader->getSortedDocValues(field->name);
}

shared_ptr<SortedNumericDocValues>
SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  return reader->getSortedNumericDocValues(field->name);
}

shared_ptr<SortedSetDocValues>
SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  return reader->getSortedSetDocValues(field->name);
}

void SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::
    checkIntegrity() 
{
  // We already checkIntegrity the entire reader up front
}

SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::
    ~DocValuesProducerAnonymousInnerClass()
{
}

int64_t
SlowCodecReaderWrapper::DocValuesProducerAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}

shared_ptr<StoredFieldsReader>
SlowCodecReaderWrapper::readerToStoredFieldsReader(
    shared_ptr<LeafReader> reader)
{
  return make_shared<StoredFieldsReaderAnonymousInnerClass>(reader);
}

SlowCodecReaderWrapper::StoredFieldsReaderAnonymousInnerClass::
    StoredFieldsReaderAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::LeafReader> reader)
{
  this->reader = reader;
}

void SlowCodecReaderWrapper::StoredFieldsReaderAnonymousInnerClass::
    visitDocument(int docID,
                  shared_ptr<StoredFieldVisitor> visitor) 
{
  reader->document(docID, visitor);
}

shared_ptr<StoredFieldsReader>
SlowCodecReaderWrapper::StoredFieldsReaderAnonymousInnerClass::clone()
{
  return readerToStoredFieldsReader(reader);
}

void SlowCodecReaderWrapper::StoredFieldsReaderAnonymousInnerClass::
    checkIntegrity() 
{
  // We already checkIntegrity the entire reader up front
}

SlowCodecReaderWrapper::StoredFieldsReaderAnonymousInnerClass::
    ~StoredFieldsReaderAnonymousInnerClass()
{
}

int64_t
SlowCodecReaderWrapper::StoredFieldsReaderAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}

shared_ptr<TermVectorsReader>
SlowCodecReaderWrapper::readerToTermVectorsReader(shared_ptr<LeafReader> reader)
{
  return make_shared<TermVectorsReaderAnonymousInnerClass>(reader);
}

SlowCodecReaderWrapper::TermVectorsReaderAnonymousInnerClass::
    TermVectorsReaderAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::LeafReader> reader)
{
  this->reader = reader;
}

shared_ptr<Fields>
SlowCodecReaderWrapper::TermVectorsReaderAnonymousInnerClass::get(
    int docID) 
{
  return reader->getTermVectors(docID);
}

shared_ptr<TermVectorsReader>
SlowCodecReaderWrapper::TermVectorsReaderAnonymousInnerClass::clone()
{
  return readerToTermVectorsReader(reader);
}

void SlowCodecReaderWrapper::TermVectorsReaderAnonymousInnerClass::
    checkIntegrity() 
{
  // We already checkIntegrity the entire reader up front
}

SlowCodecReaderWrapper::TermVectorsReaderAnonymousInnerClass::
    ~TermVectorsReaderAnonymousInnerClass()
{
}

int64_t
SlowCodecReaderWrapper::TermVectorsReaderAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}

shared_ptr<FieldsProducer> SlowCodecReaderWrapper::readerToFieldsProducer(
    shared_ptr<LeafReader> reader) 
{
  deque<wstring> indexedFields = deque<wstring>();
  for (auto fieldInfo : reader->getFieldInfos()) {
    if (fieldInfo->getIndexOptions() != IndexOptions::NONE) {
      indexedFields.push_back(fieldInfo->name);
    }
  }
  sort(indexedFields.begin(), indexedFields.end());
  return make_shared<FieldsProducerAnonymousInnerClass>(reader, indexedFields);
}

SlowCodecReaderWrapper::FieldsProducerAnonymousInnerClass::
    FieldsProducerAnonymousInnerClass(
        shared_ptr<org::apache::lucene::index::LeafReader> reader,
        deque<wstring> &indexedFields)
{
  this->reader = reader;
  this->indexedFields = indexedFields;
}

shared_ptr<Iterator<wstring>>
SlowCodecReaderWrapper::FieldsProducerAnonymousInnerClass::iterator()
{
  return indexedFields.begin();
}

shared_ptr<Terms>
SlowCodecReaderWrapper::FieldsProducerAnonymousInnerClass::terms(
    const wstring &field) 
{
  return reader->terms(field);
}

int SlowCodecReaderWrapper::FieldsProducerAnonymousInnerClass::size()
{
  return indexedFields.size();
}

void SlowCodecReaderWrapper::FieldsProducerAnonymousInnerClass::
    checkIntegrity() 
{
  // We already checkIntegrity the entire reader up front
}

SlowCodecReaderWrapper::FieldsProducerAnonymousInnerClass::
    ~FieldsProducerAnonymousInnerClass()
{
}

int64_t
SlowCodecReaderWrapper::FieldsProducerAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}
} // namespace org::apache::lucene::index