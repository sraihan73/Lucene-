using namespace std;

#include "IntFieldSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocValues = org::apache::lucene::index::DocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using IntDocValues =
    org::apache::lucene::queries::function::docvalues::IntDocValues;
using SortField = org::apache::lucene::search::SortField;
using Type = org::apache::lucene::search::SortField::Type;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueInt = org::apache::lucene::util::mutable_::MutableValueInt;

IntFieldSource::IntFieldSource(const wstring &field) : FieldCacheSource(field)
{
}

wstring IntFieldSource::description()
{
  return L"int(" + field + StringHelper::toString(L')');
}

shared_ptr<SortField> IntFieldSource::getSortField(bool reverse)
{
  return make_shared<SortField>(field, SortField::Type::INT, reverse);
}

shared_ptr<FunctionValues> IntFieldSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{

  shared_ptr<NumericDocValues> *const arr =
      getNumericDocValues(context, readerContext);

  return make_shared<IntDocValuesAnonymousInnerClass>(shared_from_this(), arr);
}

IntFieldSource::IntDocValuesAnonymousInnerClass::
    IntDocValuesAnonymousInnerClass(shared_ptr<IntFieldSource> outerInstance,
                                    shared_ptr<NumericDocValues> arr)
    : org::apache::lucene::queries::function::docvalues::IntDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->arr = arr;
}

int IntFieldSource::IntDocValuesAnonymousInnerClass::getValueForDoc(
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
    return static_cast<int>(arr->longValue());
  } else {
    return 0;
  }
}

int IntFieldSource::IntDocValuesAnonymousInnerClass::intVal(int doc) throw(
    IOException)
{
  return getValueForDoc(doc);
}

wstring IntFieldSource::IntDocValuesAnonymousInnerClass::strVal(int doc) throw(
    IOException)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Integer::toString(intVal(doc));
}

bool IntFieldSource::IntDocValuesAnonymousInnerClass::exists(int doc) throw(
    IOException)
{
  getValueForDoc(doc);
  return arr->docID() == doc;
}

shared_ptr<FunctionValues::ValueFiller>
IntFieldSource::IntDocValuesAnonymousInnerClass::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

IntFieldSource::IntDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
        shared_ptr<IntDocValuesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueInt>();
}

shared_ptr<MutableValue> IntFieldSource::IntDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void IntFieldSource::IntDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::fillValue(int doc) 
{
  mval->value = getValueForDoc(doc);
  mval->exists = outerInstance->arr.docID() == doc;
}

shared_ptr<NumericDocValues> IntFieldSource::getNumericDocValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return DocValues::getNumeric(readerContext->reader(), field);
}

bool IntFieldSource::equals(any o)
{
  if (o.type() != IntFieldSource::typeid) {
    return false;
  }
  shared_ptr<IntFieldSource> other =
      any_cast<std::shared_ptr<IntFieldSource>>(o);
  return FieldCacheSource::equals(other);
}

int IntFieldSource::hashCode()
{
  int h = Integer::typeid->hashCode();
  h += FieldCacheSource::hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function::valuesource