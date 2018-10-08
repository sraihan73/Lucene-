using namespace std;

#include "DoubleFieldSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using DoubleDocValues =
    org::apache::lucene::queries::function::docvalues::DoubleDocValues;
using Type = org::apache::lucene::search::SortField::Type;
using SortField = org::apache::lucene::search::SortField;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueDouble =
    org::apache::lucene::util::mutable_::MutableValueDouble;

DoubleFieldSource::DoubleFieldSource(const wstring &field)
    : FieldCacheSource(field)
{
}

wstring DoubleFieldSource::description()
{
  return L"double(" + field + StringHelper::toString(L')');
}

shared_ptr<SortField> DoubleFieldSource::getSortField(bool reverse)
{
  return make_shared<SortField>(field, Type::DOUBLE, reverse);
}

shared_ptr<FunctionValues> DoubleFieldSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{

  shared_ptr<NumericDocValues> *const values =
      getNumericDocValues(context, readerContext);

  return make_shared<DoubleDocValuesAnonymousInnerClass>(shared_from_this(),
                                                         values);
}

DoubleFieldSource::DoubleDocValuesAnonymousInnerClass::
    DoubleDocValuesAnonymousInnerClass(
        shared_ptr<DoubleFieldSource> outerInstance,
        shared_ptr<NumericDocValues> values)
    : org::apache::lucene::queries::function::docvalues::DoubleDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

double DoubleFieldSource::DoubleDocValuesAnonymousInnerClass::getValueForDoc(
    int doc) 
{
  if (doc < lastDocID) {
    throw invalid_argument(L"docs were sent out-of-order: lastDocID=" +
                           lastDocID + L" vs docID=" + to_wstring(doc));
  }
  lastDocID = doc;
  int curDocID = values->docID();
  if (doc > curDocID) {
    curDocID = values->advance(doc);
  }
  if (doc == curDocID) {
    return Double::longBitsToDouble(values->longValue());
  } else {
    return 0.0;
  }
}

double DoubleFieldSource::DoubleDocValuesAnonymousInnerClass::doubleVal(
    int doc) 
{
  return getValueForDoc(doc);
}

bool DoubleFieldSource::DoubleDocValuesAnonymousInnerClass::exists(
    int doc) 
{
  getValueForDoc(doc);
  return doc == values->docID();
}

shared_ptr<FunctionValues::ValueFiller>
DoubleFieldSource::DoubleDocValuesAnonymousInnerClass::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

DoubleFieldSource::DoubleDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
        shared_ptr<DoubleDocValuesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueDouble>();
}

shared_ptr<MutableValue> DoubleFieldSource::DoubleDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void DoubleFieldSource::DoubleDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::fillValue(int doc) 
{
  mval->value = getValueForDoc(doc);
  mval->exists = exists(doc);
}

shared_ptr<NumericDocValues> DoubleFieldSource::getNumericDocValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return DocValues::getNumeric(readerContext->reader(), field);
}

bool DoubleFieldSource::equals(any o)
{
  if (o.type() != DoubleFieldSource::typeid) {
    return false;
  }
  shared_ptr<DoubleFieldSource> other =
      any_cast<std::shared_ptr<DoubleFieldSource>>(o);
  return FieldCacheSource::equals(other);
}

int DoubleFieldSource::hashCode()
{
  int h = Double::typeid->hashCode();
  h += FieldCacheSource::hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function::valuesource