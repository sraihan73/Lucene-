using namespace std;

#include "CodecReader.h"

namespace org::apache::lucene::index
{
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using FutureObjects = org::apache::lucene::util::FutureObjects;

CodecReader::CodecReader() {}

void CodecReader::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{
  checkBounds(docID);
  getFieldsReader()->visitDocument(docID, visitor);
}

shared_ptr<Fields> CodecReader::getTermVectors(int docID) 
{
  shared_ptr<TermVectorsReader> termVectorsReader = getTermVectorsReader();
  if (termVectorsReader == nullptr) {
    return nullptr;
  }
  checkBounds(docID);
  return termVectorsReader->get(docID);
}

void CodecReader::checkBounds(int docID)
{
  FutureObjects::checkIndex(docID, maxDoc());
}

shared_ptr<Terms> CodecReader::terms(const wstring &field) 
{
  // ensureOpen(); no; getPostingsReader calls this
  // We could check the FieldInfo IndexOptions but there's no point since
  //   PostingsReader will simply return null for fields that don't exist or
  //   that have no terms index.
  return getPostingsReader()->terms(field);
}

shared_ptr<FieldInfo> CodecReader::getDVField(const wstring &field,
                                              DocValuesType type)
{
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr) {
    // Field does not exist
    return nullptr;
  }
  if (fi->getDocValuesType() == DocValuesType::NONE) {
    // Field was not indexed with doc values
    return nullptr;
  }
  if (fi->getDocValuesType() != type) {
    // Field DocValues are different than requested type
    return nullptr;
  }

  return fi;
}

shared_ptr<NumericDocValues>
CodecReader::getNumericDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getDVField(field, DocValuesType::NUMERIC);
  if (fi == nullptr) {
    return nullptr;
  }
  return getDocValuesReader()->getNumeric(fi);
}

shared_ptr<BinaryDocValues>
CodecReader::getBinaryDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getDVField(field, DocValuesType::BINARY);
  if (fi == nullptr) {
    return nullptr;
  }
  return getDocValuesReader()->getBinary(fi);
}

shared_ptr<SortedDocValues>
CodecReader::getSortedDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getDVField(field, DocValuesType::SORTED);
  if (fi == nullptr) {
    return nullptr;
  }
  return getDocValuesReader()->getSorted(fi);
}

shared_ptr<SortedNumericDocValues>
CodecReader::getSortedNumericDocValues(const wstring &field) 
{
  ensureOpen();

  shared_ptr<FieldInfo> fi = getDVField(field, DocValuesType::SORTED_NUMERIC);
  if (fi == nullptr) {
    return nullptr;
  }
  return getDocValuesReader()->getSortedNumeric(fi);
}

shared_ptr<SortedSetDocValues>
CodecReader::getSortedSetDocValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getDVField(field, DocValuesType::SORTED_SET);
  if (fi == nullptr) {
    return nullptr;
  }
  return getDocValuesReader()->getSortedSet(fi);
}

shared_ptr<NumericDocValues>
CodecReader::getNormValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr || fi->hasNorms() == false) {
    // Field does not exist or does not index norms
    return nullptr;
  }

  return getNormsReader()->getNorms(fi);
}

shared_ptr<PointValues>
CodecReader::getPointValues(const wstring &field) 
{
  ensureOpen();
  shared_ptr<FieldInfo> fi = getFieldInfos()->fieldInfo(field);
  if (fi == nullptr || fi->getPointDimensionCount() == 0) {
    // Field does not exist or does not index points
    return nullptr;
  }

  return getPointsReader()->getValues(field);
}

void CodecReader::doClose()  {}

int64_t CodecReader::ramBytesUsed()
{
  ensureOpen();

  // terms/postings
  int64_t ramBytesUsed = getPostingsReader()->ramBytesUsed();

  // norms
  if (getNormsReader() != nullptr) {
    ramBytesUsed += getNormsReader()->ramBytesUsed();
  }

  // docvalues
  if (getDocValuesReader() != nullptr) {
    ramBytesUsed += getDocValuesReader()->ramBytesUsed();
  }

  // stored fields
  if (getFieldsReader() != nullptr) {
    ramBytesUsed += getFieldsReader()->ramBytesUsed();
  }

  // term vectors
  if (getTermVectorsReader() != nullptr) {
    ramBytesUsed += getTermVectorsReader()->ramBytesUsed();
  }

  // points
  if (getPointsReader() != nullptr) {
    ramBytesUsed += getPointsReader()->ramBytesUsed();
  }

  return ramBytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
CodecReader::getChildResources()
{
  ensureOpen();
  const deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>(6);

  // terms/postings
  resources.push_back(
      Accountables::namedAccountable(L"postings", getPostingsReader()));

  // norms
  if (getNormsReader() != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"norms", getNormsReader()));
  }

  // docvalues
  if (getDocValuesReader() != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"docvalues", getDocValuesReader()));
  }

  // stored fields
  if (getFieldsReader() != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"stored fields", getFieldsReader()));
  }

  // term vectors
  if (getTermVectorsReader() != nullptr) {
    resources.push_back(Accountables::namedAccountable(L"term vectors",
                                                       getTermVectorsReader()));
  }

  // points
  if (getPointsReader() != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"points", getPointsReader()));
  }

  return Collections::unmodifiableList(resources);
}

void CodecReader::checkIntegrity() 
{
  ensureOpen();

  // terms/postings
  getPostingsReader()->checkIntegrity();

  // norms
  if (getNormsReader() != nullptr) {
    getNormsReader()->checkIntegrity();
  }

  // docvalues
  if (getDocValuesReader() != nullptr) {
    getDocValuesReader()->checkIntegrity();
  }

  // stored fields
  if (getFieldsReader() != nullptr) {
    getFieldsReader()->checkIntegrity();
  }

  // term vectors
  if (getTermVectorsReader() != nullptr) {
    getTermVectorsReader()->checkIntegrity();
  }

  // points
  if (getPointsReader() != nullptr) {
    getPointsReader()->checkIntegrity();
  }
}
} // namespace org::apache::lucene::index