using namespace std;

#include "EnumFieldSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSourceScorer =
    org::apache::lucene::queries::function::ValueSourceScorer;
using IntDocValues =
    org::apache::lucene::queries::function::docvalues::IntDocValues;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueInt = org::apache::lucene::util::mutable_::MutableValueInt;
const optional<int> EnumFieldSource::DEFAULT_VALUE = -1;

EnumFieldSource::EnumFieldSource(
    const wstring &field, unordered_map<int, wstring> &enumIntToStringMap,
    unordered_map<wstring, int> &enumStringToIntMap)
    : FieldCacheSource(field), enumIntToStringMap(enumIntToStringMap),
      enumStringToIntMap(enumStringToIntMap)
{
}

optional<int> EnumFieldSource::tryParseInt(const wstring &valueStr)
{
  optional<int> intValue = nullopt;
  try {
    intValue = stoi(valueStr);
  } catch (const NumberFormatException &e) {
  }
  return intValue;
}

wstring EnumFieldSource::intValueToStringValue(optional<int> &intVal)
{
  if (!intVal) {
    return L"";
  }

  const wstring enumString = enumIntToStringMap[intVal];
  if (enumString != L"") {
    return enumString;
  }
  // can't find matching enum name - return DEFAULT_VALUE.toString()
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return DEFAULT_VALUE.toString();
}

optional<int> EnumFieldSource::stringValueToIntValue(const wstring &stringVal)
{
  if (stringVal == L"") {
    return nullopt;
  }

  optional<int> intValue;
  const optional<int> enumInt = enumStringToIntMap[stringVal];
  if (enumInt) // enum int found for string
  {
    return enumInt;
  }

  // enum int not found for string
  intValue = tryParseInt(stringVal);
  if (!intValue) // not Integer
  {
    intValue = DEFAULT_VALUE;
  }
  const wstring enumString = enumIntToStringMap[intValue];
  if (enumString != L"") // has matching string
  {
    return intValue;
  }

  return DEFAULT_VALUE;
}

wstring EnumFieldSource::description()
{
  return L"enum(" + field + StringHelper::toString(L')');
}

shared_ptr<FunctionValues> EnumFieldSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<NumericDocValues> *const arr =
      DocValues::getNumeric(readerContext->reader(), field);

  return make_shared<IntDocValuesAnonymousInnerClass>(shared_from_this(),
                                                      readerContext, arr);
}

EnumFieldSource::IntDocValuesAnonymousInnerClass::
    IntDocValuesAnonymousInnerClass(shared_ptr<EnumFieldSource> outerInstance,
                                    shared_ptr<LeafReaderContext> readerContext,
                                    shared_ptr<NumericDocValues> arr)
    : org::apache::lucene::queries::function::docvalues::IntDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->readerContext = readerContext;
  this->arr = arr;
  val = make_shared<MutableValueInt>();
}

int EnumFieldSource::IntDocValuesAnonymousInnerClass::getValueForDoc(
    int doc) 
{
  if (doc < lastDocID) {
    throw make_shared<AssertionError>(
        L"docs were sent out-of-order: lastDocID=" + lastDocID + L" vs doc=" +
        to_wstring(doc));
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

int EnumFieldSource::IntDocValuesAnonymousInnerClass::intVal(int doc) throw(
    IOException)
{
  return getValueForDoc(doc);
}

wstring EnumFieldSource::IntDocValuesAnonymousInnerClass::strVal(int doc) throw(
    IOException)
{
  optional<int> intValue = intVal(doc);
  return outerInstance->intValueToStringValue(intValue);
}

bool EnumFieldSource::IntDocValuesAnonymousInnerClass::exists(int doc) throw(
    IOException)
{
  getValueForDoc(doc);
  return arr->docID() == doc;
}

shared_ptr<ValueSourceScorer>
EnumFieldSource::IntDocValuesAnonymousInnerClass::getRangeScorer(
    shared_ptr<LeafReaderContext> readerContext, const wstring &lowerVal,
    const wstring &upperVal, bool includeLower, bool includeUpper)
{
  optional<int> lower = outerInstance->stringValueToIntValue(lowerVal);
  optional<int> upper = outerInstance->stringValueToIntValue(upperVal);

  // instead of using separate comparison functions, adjust the endpoints.

  if (!lower) {
    lower = numeric_limits<int>::min();
  } else {
    if (!includeLower && lower < numeric_limits<int>::max()) {
      lower++;
    }
  }

  if (!upper) {
    upper = numeric_limits<int>::max();
  } else {
    if (!includeUpper && upper > numeric_limits<int>::min()) {
      upper--;
    }
  }

  constexpr int ll = lower;
  constexpr int uu = upper;

  return make_shared<ValueSourceScorerAnonymousInnerClass>(
      shared_from_this(), readerContext, ll, uu);
}

EnumFieldSource::IntDocValuesAnonymousInnerClass::
    ValueSourceScorerAnonymousInnerClass::ValueSourceScorerAnonymousInnerClass(
        shared_ptr<IntDocValuesAnonymousInnerClass> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, int ll, int uu)
    : org::apache::lucene::queries::function::ValueSourceScorer(readerContext,
                                                                outerInstance)
{
  this->outerInstance = outerInstance;
  this->ll = ll;
  this->uu = uu;
}

bool EnumFieldSource::IntDocValuesAnonymousInnerClass::
    ValueSourceScorerAnonymousInnerClass::matches(int doc) 
{
  if (!exists(doc)) {
    return false;
  }
  int val = intVal(doc);
  return val >= ll && val <= uu;
}

shared_ptr<FunctionValues::ValueFiller>
EnumFieldSource::IntDocValuesAnonymousInnerClass::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

EnumFieldSource::IntDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
        shared_ptr<IntDocValuesAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueInt>();
}

shared_ptr<MutableValue> EnumFieldSource::IntDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void EnumFieldSource::IntDocValuesAnonymousInnerClass::
    ValueFillerAnonymousInnerClass::fillValue(int doc) 
{
  mval->value = intVal(doc);
  mval->exists = outerInstance->arr.docID() == doc;
}

bool EnumFieldSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  if (!FieldCacheSource::equals(o)) {
    return false;
  }

  shared_ptr<EnumFieldSource> that =
      any_cast<std::shared_ptr<EnumFieldSource>>(o);

  if (!enumIntToStringMap.equals(that->enumIntToStringMap)) {
    return false;
  }
  if (!enumStringToIntMap.equals(that->enumStringToIntMap)) {
    return false;
  }

  return true;
}

int EnumFieldSource::hashCode()
{
  int result = FieldCacheSource::hashCode();
  result = 31 * result + enumIntToStringMap.hashCode();
  result = 31 * result + enumStringToIntMap.hashCode();
  return result;
}
} // namespace org::apache::lucene::queries::function::valuesource