using namespace std;

#include "FloatFieldSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using Type = org::apache::lucene::search::SortField::Type;
using SortField = org::apache::lucene::search::SortField;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueFloat =
    org::apache::lucene::util::mutable_::MutableValueFloat;

FloatFieldSource::FloatFieldSource(const wstring &field)
    : FieldCacheSource(field)
{
}

wstring FloatFieldSource::description()
{
  return L"float(" + field + StringHelper::toString(L')');
}

shared_ptr<SortField> FloatFieldSource::getSortField(bool reverse)
{
  return make_shared<SortField>(field, Type::FLOAT, reverse);
}

shared_ptr<FunctionValues> FloatFieldSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{

  shared_ptr<NumericDocValues> *const arr =
      getNumericDocValues(context, readerContext);

  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this(),
                                                        arr);
}

FloatFieldSource::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<FloatFieldSource> outerInstance,
        shared_ptr<NumericDocValues> arr)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->arr = arr;
}

float FloatFieldSource::FloatDocValuesAnonymousInnerClass::getValueForDoc(
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
    return Float::intBitsToFloat(static_cast<int>(arr->longValue()));
  } else {
    return 0.0f;
  }
}

float FloatFieldSource::FloatDocValuesAnonymousInnerClass::floatVal(
    int doc) 
{
  return getValueForDoc(doc);
}

bool FloatFieldSource::FloatDocValuesAnonymousInnerClass::exists(int doc) throw(
    IOException)
{
  getValueForDoc(doc);
  return arr->docID() == doc;
}

shared_ptr<FunctionValues::ValueFiller>
FloatFieldSource::FloatDocValuesAnonymousInnerClass::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

FloatFieldSource::FloatDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
        shared_ptr<FloatDocValuesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueFloat>();
}

shared_ptr<MutableValue> FloatFieldSource::FloatDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void FloatFieldSource::FloatDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::fillValue(int doc) 
{
  mval->value = floatVal(doc);
  mval->exists = outerInstance->arr.docID() == doc;
}

shared_ptr<NumericDocValues> FloatFieldSource::getNumericDocValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return DocValues::getNumeric(readerContext->reader(), field);
}

bool FloatFieldSource::equals(any o)
{
  if (o.type() != FloatFieldSource::typeid) {
    return false;
  }
  shared_ptr<FloatFieldSource> other =
      any_cast<std::shared_ptr<FloatFieldSource>>(o);
  return FieldCacheSource::equals(other);
}

int FloatFieldSource::hashCode()
{
  int h = Float::typeid->hashCode();
  h += FieldCacheSource::hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function::valuesource