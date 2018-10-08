using namespace std;

#include "LongDocValues.h"

namespace org::apache::lucene::queries::function::docvalues
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using ValueSourceScorer =
    org::apache::lucene::queries::function::ValueSourceScorer;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueLong = org::apache::lucene::util::mutable_::MutableValueLong;

LongDocValues::LongDocValues(shared_ptr<ValueSource> vs) : vs(vs) {}

char LongDocValues::byteVal(int doc) 
{
  return static_cast<char>(longVal(doc));
}

short LongDocValues::shortVal(int doc) 
{
  return static_cast<short>(longVal(doc));
}

float LongDocValues::floatVal(int doc) 
{
  return static_cast<float>(longVal(doc));
}

int LongDocValues::intVal(int doc) 
{
  return static_cast<int>(longVal(doc));
}

double LongDocValues::doubleVal(int doc) 
{
  return static_cast<double>(longVal(doc));
}

bool LongDocValues::boolVal(int doc) 
{
  return longVal(doc) != 0;
}

wstring LongDocValues::strVal(int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Long::toString(longVal(doc));
}

any LongDocValues::objectVal(int doc) 
{
  return exists(doc) ? longVal(doc) : nullptr;
}

wstring LongDocValues::toString(int doc) 
{
  return vs->description() + StringHelper::toString(L'=') + strVal(doc);
}

int64_t LongDocValues::externalToLong(const wstring &extVal)
{
  return StringHelper::fromString<int64_t>(extVal);
}

shared_ptr<ValueSourceScorer>
LongDocValues::getRangeScorer(shared_ptr<LeafReaderContext> readerContext,
                              const wstring &lowerVal, const wstring &upperVal,
                              bool includeLower, bool includeUpper)
{
  int64_t lower, upper;

  // instead of using separate comparison functions, adjust the endpoints.

  if (lowerVal == L"") {
    lower = numeric_limits<int64_t>::min();
  } else {
    lower = externalToLong(lowerVal);
    if (!includeLower && lower < numeric_limits<int64_t>::max()) {
      lower++;
    }
  }

  if (upperVal == L"") {
    upper = numeric_limits<int64_t>::max();
  } else {
    upper = externalToLong(upperVal);
    if (!includeUpper && upper > numeric_limits<int64_t>::min()) {
      upper--;
    }
  }

  constexpr int64_t ll = lower;
  constexpr int64_t uu = upper;

  return make_shared<ValueSourceScorerAnonymousInnerClass>(
      shared_from_this(), readerContext, ll, uu);
}

LongDocValues::ValueSourceScorerAnonymousInnerClass::
    ValueSourceScorerAnonymousInnerClass(
        shared_ptr<LongDocValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, int64_t ll, int64_t uu)
    : org::apache::lucene::queries::function::ValueSourceScorer(readerContext,
                                                                outerInstance)
{
  this->outerInstance = outerInstance;
  this->ll = ll;
  this->uu = uu;
}

bool LongDocValues::ValueSourceScorerAnonymousInnerClass::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  int64_t val = outerInstance->longVal(doc);
  return val >= ll && val <= uu;
}

shared_ptr<FunctionValues::ValueFiller> LongDocValues::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

LongDocValues::ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
    shared_ptr<LongDocValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueLong>();
}

shared_ptr<MutableValue>
LongDocValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void LongDocValues::ValueFillerAnonymousInnerClass::fillValue(int doc) throw(
    IOException)
{
  mval->value = outerInstance->longVal(doc);
  mval->exists = outerInstance->exists(doc);
}
} // namespace org::apache::lucene::queries::function::docvalues