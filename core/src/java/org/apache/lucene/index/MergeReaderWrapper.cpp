using namespace std;

#include "MergeReaderWrapper.h"

namespace org::apache::lucene::index
{
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Bits = org::apache::lucene::util::Bits;
using FutureObjects = org::apache::lucene::util::FutureObjects;

MergeReaderWrapper::MergeReaderWrapper(shared_ptr<CodecReader> in_) throw(
    IOException)
    : in_(in_)
{

  shared_ptr<FieldsProducer> fields = in_->getPostingsReader();
  if (fields->size() > 0) {
    fields = fields->getMergeInstance();
  }
  this->fields = fields;

  shared_ptr<NormsProducer> norms = in_->getNormsReader();
  if (norms != nullptr) {
    norms = norms->getMergeInstance();
  }
  this->norms = norms;

  shared_ptr<DocValuesProducer> docValues = in_->getDocValuesReader();
  if (docValues != nullptr) {
    docValues = docValues->getMergeInstance();
  }
  this->docValues = docValues;

  shared_ptr<StoredFieldsReader> store = in_->getFieldsReader();
  if (store != nullptr) {
    store = store->getMergeInstance();
  }
  this->store = store;

  shared_ptr<TermVectorsReader> vectors = in_->getTermVectorsReader();
  if (vectors != nullptr) {
    vectors = vectors->getMergeInstance();
  }
  this->vectors = vectors;
}

shared_ptr<Terms>
MergeReaderWrapper::terms(const wstring &field) 
{
  ensureOpen();
  // We could check the FieldInfo IndexOptions but there's no point since
  //   PostingsReader will simply return null for fields that don't exist or
  //   that have no terms index.
  return fields->terms(field);
}

shared_ptr<NumericDocValues>
MergeReaderWrapper::getNumericDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr) {
    // Field does not exist
    return nullptr;
  }
  if (fi->getDocValuesType() != DocValuesType::NUMERIC) {
    // Field was not indexed with doc values
    return nullptr;
  }
  return docValues->getNumeric(fi);
}

shared_ptr<BinaryDocValues>
MergeReaderWrapper::getBinaryDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr) {
    // Field does not exist
    return nullptr;
  }
  if (fi->getDocValuesType() != DocValuesType::BINARY) {
    // Field was not indexed with doc values
    return nullptr;
  }
  return docValues->getBinary(fi);
}

shared_ptr<SortedDocValues>
MergeReaderWrapper::getSortedDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr) {
    // Field does not exist
    return nullptr;
  }
  if (fi->getDocValuesType() != DocValuesType::SORTED) {
    // Field was not indexed with doc values
    return nullptr;
  }
  return docValues->getSorted(fi);
}

shared_ptr<SortedNumericDocValues>
MergeReaderWrapper::getSortedNumericDocValues(const wstring &field) throw(
    IOException)
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr) {
    // Field does not exist
    return nullptr;
  }
  if (fi->getDocValuesType() != DocValuesType::SORTED_NUMERIC) {
    // Field was not indexed with doc values
    return nullptr;
  }
  return docValues->getSortedNumeric(fi);
}

shared_ptr<SortedSetDocValues> MergeReaderWrapper::getSortedSetDocValues(
    const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr) {
    // Field does not exist
    return nullptr;
  }
  if (fi->getDocValuesType() != DocValuesType::SORTED_SET) {
    // Field was not indexed with doc values
    return nullptr;
  }
  return docValues->getSortedSet(fi);
}

shared_ptr<NumericDocValues>
MergeReaderWrapper::getNormValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr || !fi->hasNorms()) {
    // Field does not exist or does not index norms
    return nullptr;
  }
  return norms->getNorms(fi);
}

shared_ptr<FieldInfos> MergeReaderWrapper::getFieldInfos()
{
  return in_->getFieldInfos();
}

shared_ptr<Bits> MergeReaderWrapper::getLiveDocs()
{
  return in_->getLiveDocs();
}

void MergeReaderWrapper::checkIntegrity() 
{
  in_->checkIntegrity();
}

shared_ptr<Fields>
MergeReaderWrapper::getTermVectors(int docID) 
{
  ensureOpen();
  checkBounds(docID);
  if (vectors == nullptr) {
    return nullptr;
  }
  return vectors->get(docID);
}

shared_ptr<PointValues>
MergeReaderWrapper::getPointValues(const wstring &fieldName) 
{
  return in_->getPointValues(fieldName);
}

int MergeReaderWrapper::numDocs() { return in_->numDocs(); }

int MergeReaderWrapper::maxDoc() { return in_->maxDoc(); }

void MergeReaderWrapper::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{
  ensureOpen();
  checkBounds(docID);
  store->visitDocument(docID, visitor);
}

void MergeReaderWrapper::doClose()  { delete in_; }

shared_ptr<CacheHelper> MergeReaderWrapper::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<CacheHelper> MergeReaderWrapper::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

void MergeReaderWrapper::checkBounds(int docID)
{
  FutureObjects::checkIndex(docID, maxDoc());
}

wstring MergeReaderWrapper::toString()
{
  return L"MergeReaderWrapper(" + in_ + L")";
}

shared_ptr<LeafMetaData> MergeReaderWrapper::getMetaData()
{
  return in_->getMetaData();
}
} // namespace org::apache::lucene::index