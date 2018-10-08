using namespace std;

#include "LongFieldSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocValues = org::apache::lucene::index::DocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using LongDocValues =
    org::apache::lucene::queries::function::docvalues::LongDocValues;
using SortField = org::apache::lucene::search::SortField;
using Type = org::apache::lucene::search::SortField::Type;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueLong = org::apache::lucene::util::mutable_::MutableValueLong;

LongFieldSource::LongFieldSource(const wstring &field) : FieldCacheSource(field)
{
}

wstring LongFieldSource::description()
{
  return L"long(" + field + StringHelper::toString(L')');
}

int64_t LongFieldSource::externalToLong(const wstring &extVal)
{
  return StringHelper::fromString<int64_t>(extVal);
}

any LongFieldSource::longToObject(int64_t val) { return val; }

wstring LongFieldSource::longToString(int64_t val)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return longToObject(val).toString();
}

shared_ptr<SortField> LongFieldSource::getSortField(bool reverse)
{
  return make_shared<SortField>(field, SortField::Type::LONG, reverse);
}

shared_ptr<FunctionValues> LongFieldSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{

  shared_ptr<NumericDocValues> *const arr =
      getNumericDocValues(context, readerContext);

  return make_shared<LongDocValuesAnonymousInnerClass>(shared_from_this(), arr);
}

LongFieldSource::LongDocValuesAnonymousInnerClass::
    LongDocValuesAnonymousInnerClass(shared_ptr<LongFieldSource> outerInstance,
                                     shared_ptr<NumericDocValues> arr)
    : org::apache::lucene::queries::function::docvalues::LongDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->arr = arr;
}

int64_t LongFieldSource::LongDocValuesAnonymousInnerClass::getValueForDoc(
    int doc) 
{
  if (doc < lastDocID) {
    throw invalid_argument(L"docs were sent out-of-order: lastDocID=" +
                           lastDocID + L" vs docID=" + to_wstring(doc));
  }
  lastDocID = doc;
  int curDocID = arr->docID();
  if (doc > curDocID) {
    curDocID = arr->advance(doc);
  }
  if (doc == curDocID) {
    return arr->longValue();
  } else {
    return 0;
  }
}

int64_t LongFieldSource::LongDocValuesAnonymousInnerClass::longVal(
    int doc) 
{
  return getValueForDoc(doc);
}

bool LongFieldSource::LongDocValuesAnonymousInnerClass::exists(int doc) throw(
    IOException)
{
  getValueForDoc(doc);
  return arr->docID() == doc;
}

any LongFieldSource::LongDocValuesAnonymousInnerClass::objectVal(int doc) throw(
    IOException)
{
  int64_t value = getValueForDoc(doc);
  if (arr->docID() == doc) {
    return outerInstance->longToObject(value);
  } else {
    return nullptr;
  }
}

wstring LongFieldSource::LongDocValuesAnonymousInnerClass::strVal(
    int doc) 
{
  int64_t value = getValueForDoc(doc);
  if (arr->docID() == doc) {
    return outerInstance->longToString(value);
  } else {
    return L"";
  }
}

int64_t LongFieldSource::LongDocValuesAnonymousInnerClass::externalToLong(
    const wstring &extVal)
{
  return outerInstance->externalToLong(extVal);
}

shared_ptr<FunctionValues::ValueFiller>
LongFieldSource::LongDocValuesAnonymousInnerClass::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

LongFieldSource::LongDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
        shared_ptr<LongDocValuesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = outerInstance->outerInstance->newMutableValueLong();
}

shared_ptr<MutableValue> LongFieldSource::LongDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void LongFieldSource::LongDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::fillValue(int doc) 
{
  mval->value = getValueForDoc(doc);
  mval->exists = outerInstance->arr.docID() == doc;
}

shared_ptr<NumericDocValues> LongFieldSource::getNumericDocValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return DocValues::getNumeric(readerContext->reader(), field);
}

shared_ptr<MutableValueLong> LongFieldSource::newMutableValueLong()
{
  return make_shared<MutableValueLong>();
}

bool LongFieldSource::equals(any o)
{
  if (o.type() != this->getClass()) {
    return false;
  }
  shared_ptr<LongFieldSource> other =
      any_cast<std::shared_ptr<LongFieldSource>>(o);
  return FieldCacheSource::equals(other);
}

int LongFieldSource::hashCode()
{
  int h = getClass().hashCode();
  h += FieldCacheSource::hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function::valuesource