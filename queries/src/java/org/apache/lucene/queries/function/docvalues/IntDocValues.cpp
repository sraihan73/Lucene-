using namespace std;

#include "IntDocValues.h"

namespace org::apache::lucene::queries::function::docvalues
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using ValueSourceScorer =
    org::apache::lucene::queries::function::ValueSourceScorer;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueInt = org::apache::lucene::util::mutable_::MutableValueInt;

IntDocValues::IntDocValues(shared_ptr<ValueSource> vs) : vs(vs) {}

char IntDocValues::byteVal(int doc) 
{
  return static_cast<char>(intVal(doc));
}

short IntDocValues::shortVal(int doc) 
{
  return static_cast<short>(intVal(doc));
}

float IntDocValues::floatVal(int doc) 
{
  return static_cast<float>(intVal(doc));
}

int64_t IntDocValues::longVal(int doc) 
{
  return static_cast<int64_t>(intVal(doc));
}

double IntDocValues::doubleVal(int doc) 
{
  return static_cast<double>(intVal(doc));
}

wstring IntDocValues::strVal(int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Integer::toString(intVal(doc));
}

any IntDocValues::objectVal(int doc) 
{
  return exists(doc) ? intVal(doc) : nullptr;
}

wstring IntDocValues::toString(int doc) 
{
  return vs->description() + StringHelper::toString(L'=') + strVal(doc);
}

shared_ptr<ValueSourceScorer>
IntDocValues::getRangeScorer(shared_ptr<LeafReaderContext> readerContext,
                             const wstring &lowerVal, const wstring &upperVal,
                             bool includeLower, bool includeUpper)
{
  int lower, upper;

  // instead of using separate comparison functions, adjust the endpoints.

  if (lowerVal == L"") {
    lower = numeric_limits<int>::min();
  } else {
    lower = stoi(lowerVal);
    if (!includeLower && lower < numeric_limits<int>::max()) {
      lower++;
    }
  }

  if (upperVal == L"") {
    upper = numeric_limits<int>::max();
  } else {
    upper = stoi(upperVal);
    if (!includeUpper && upper > numeric_limits<int>::min()) {
      upper--;
    }
  }

  constexpr int ll = lower;
  constexpr int uu = upper;

  return make_shared<ValueSourceScorerAnonymousInnerClass>(
      shared_from_this(), readerContext, ll, uu);
}

IntDocValues::ValueSourceScorerAnonymousInnerClass::
    ValueSourceScorerAnonymousInnerClass(
        shared_ptr<IntDocValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, int ll, int uu)
    : org::apache::lucene::queries::function::ValueSourceScorer(readerContext,
                                                                outerInstance)
{
  this->outerInstance = outerInstance;
  this->ll = ll;
  this->uu = uu;
}

bool IntDocValues::ValueSourceScorerAnonymousInnerClass::matches(int doc) throw(
    IOException)
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  int val = outerInstance->intVal(doc);
  return val >= ll && val <= uu;
}

shared_ptr<FunctionValues::ValueFiller> IntDocValues::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

IntDocValues::ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
    shared_ptr<IntDocValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueInt>();
}

shared_ptr<MutableValue>
IntDocValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void IntDocValues::ValueFillerAnonymousInnerClass::fillValue(int doc) throw(
    IOException)
{
  mval->value = outerInstance->intVal(doc);
  mval->exists = outerInstance->exists(doc);
}
} // namespace org::apache::lucene::queries::function::docvalues