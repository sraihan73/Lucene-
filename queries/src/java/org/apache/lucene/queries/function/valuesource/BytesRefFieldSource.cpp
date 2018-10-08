using namespace std;

#include "BytesRefFieldSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using DocTermsIndexDocValues =
    org::apache::lucene::queries::function::docvalues::DocTermsIndexDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueStr = org::apache::lucene::util::mutable_::MutableValueStr;

BytesRefFieldSource::BytesRefFieldSource(const wstring &field)
    : FieldCacheSource(field)
{
}

shared_ptr<FunctionValues> BytesRefFieldSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FieldInfo> *const fieldInfo =
      readerContext->reader()->getFieldInfos()->fieldInfo(field);

  // To be sorted or not to be sorted, that is the question
  // TODO: do it cleaner?
  if (fieldInfo != nullptr &&
      fieldInfo->getDocValuesType() == DocValuesType::BINARY) {
    shared_ptr<BinaryDocValues> *const binaryValues =
        DocValues::getBinary(readerContext->reader(), field);
    return make_shared<FunctionValuesAnonymousInnerClass>(shared_from_this(),
                                                          binaryValues);
  } else {
    return make_shared<DocTermsIndexDocValuesAnonymousInnerClass>(
        shared_from_this(), readerContext, field);
  }
}

BytesRefFieldSource::FunctionValuesAnonymousInnerClass::
    FunctionValuesAnonymousInnerClass(
        shared_ptr<BytesRefFieldSource> outerInstance,
        shared_ptr<BinaryDocValues> binaryValues)
{
  this->outerInstance = outerInstance;
  this->binaryValues = binaryValues;
  lastDocID = -1;
}

shared_ptr<BytesRef>
BytesRefFieldSource::FunctionValuesAnonymousInnerClass::getValueForDoc(
    int doc) 
{
  if (doc < lastDocID) {
    throw invalid_argument(L"docs were sent out-of-order: lastDocID=" +
                           lastDocID + L" vs docID=" + to_wstring(doc));
  }
  lastDocID = doc;
  int curDocID = binaryValues->docID();
  if (doc > curDocID) {
    curDocID = binaryValues->advance(doc);
  }
  if (doc == curDocID) {
    return binaryValues->binaryValue();
  } else {
    return nullptr;
  }
}

bool BytesRefFieldSource::FunctionValuesAnonymousInnerClass::exists(
    int doc) 
{
  return getValueForDoc(doc) != nullptr;
}

bool BytesRefFieldSource::FunctionValuesAnonymousInnerClass::bytesVal(
    int doc, shared_ptr<BytesRefBuilder> target) 
{
  shared_ptr<BytesRef> value = getValueForDoc(doc);
  if (value == nullptr || value->length == 0) {
    return false;
  } else {
    target->copyBytes(value);
    return true;
  }
}

wstring BytesRefFieldSource::FunctionValuesAnonymousInnerClass::strVal(
    int doc) 
{
  shared_ptr<BytesRefBuilder> *const bytes = make_shared<BytesRefBuilder>();
  return bytesVal(doc, bytes) ? bytes->get().utf8ToString() : L"";
}

any BytesRefFieldSource::FunctionValuesAnonymousInnerClass::objectVal(
    int doc) 
{
  return strVal(doc);
}

wstring BytesRefFieldSource::FunctionValuesAnonymousInnerClass::toString(
    int doc) 
{
  return outerInstance->description() + StringHelper::toString(L'=') +
         strVal(doc);
}

shared_ptr<FunctionValues::ValueFiller>
BytesRefFieldSource::FunctionValuesAnonymousInnerClass::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

BytesRefFieldSource::FunctionValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
        shared_ptr<FunctionValuesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueStr>();
}

shared_ptr<MutableValue>
BytesRefFieldSource::FunctionValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void BytesRefFieldSource::FunctionValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::fillValue(int doc) 
{
  shared_ptr<BytesRef> value = getValueForDoc(doc);
  mval->exists = value != nullptr;
  mval::value->clear();
  if (value != nullptr) {
    mval::value::copyBytes(value);
  }
}

BytesRefFieldSource::DocTermsIndexDocValuesAnonymousInnerClass::
    DocTermsIndexDocValuesAnonymousInnerClass(
        shared_ptr<BytesRefFieldSource> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, const wstring &field)
    : org::apache::lucene::queries::function::docvalues::DocTermsIndexDocValues(
          outerInstance, readerContext, field)
{
  this->outerInstance = outerInstance;
}

wstring BytesRefFieldSource::DocTermsIndexDocValuesAnonymousInnerClass::toTerm(
    const wstring &readableValue)
{
  return readableValue;
}

any BytesRefFieldSource::DocTermsIndexDocValuesAnonymousInnerClass::objectVal(
    int doc) 
{
  return strVal(doc);
}

wstring
BytesRefFieldSource::DocTermsIndexDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  return outerInstance->description() + StringHelper::toString(L'=') +
         strVal(doc);
}
} // namespace org::apache::lucene::queries::function::valuesource