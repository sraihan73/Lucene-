using namespace std;

#include "DoubleDocValues.h"

namespace org::apache::lucene::queries::function::docvalues
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using ValueSourceScorer =
    org::apache::lucene::queries::function::ValueSourceScorer;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueDouble =
    org::apache::lucene::util::mutable_::MutableValueDouble;

DoubleDocValues::DoubleDocValues(shared_ptr<ValueSource> vs) : vs(vs) {}

char DoubleDocValues::byteVal(int doc) 
{
  return static_cast<char>(doubleVal(doc));
}

short DoubleDocValues::shortVal(int doc) 
{
  return static_cast<short>(doubleVal(doc));
}

float DoubleDocValues::floatVal(int doc) 
{
  return static_cast<float>(doubleVal(doc));
}

int DoubleDocValues::intVal(int doc) 
{
  return static_cast<int>(doubleVal(doc));
}

int64_t DoubleDocValues::longVal(int doc) 
{
  return static_cast<int64_t>(doubleVal(doc));
}

bool DoubleDocValues::boolVal(int doc) 
{
  return doubleVal(doc) != 0;
}

wstring DoubleDocValues::strVal(int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Double::toString(doubleVal(doc));
}

any DoubleDocValues::objectVal(int doc) 
{
  return exists(doc) ? doubleVal(doc) : nullptr;
}

wstring DoubleDocValues::toString(int doc) 
{
  return vs->description() + StringHelper::toString(L'=') + strVal(doc);
}

shared_ptr<ValueSourceScorer> DoubleDocValues::getRangeScorer(
    shared_ptr<LeafReaderContext> readerContext, const wstring &lowerVal,
    const wstring &upperVal, bool includeLower, bool includeUpper)
{
  double lower, upper;

  if (lowerVal == L"") {
    lower = -numeric_limits<double>::infinity();
  } else {
    lower = stod(lowerVal);
  }

  if (upperVal == L"") {
    upper = numeric_limits<double>::infinity();
  } else {
    upper = stod(upperVal);
  }

  constexpr double l = lower;
  constexpr double u = upper;

  if (includeLower && includeUpper) {
    return make_shared<ValueSourceScorerAnonymousInnerClass>(
        shared_from_this(), readerContext, l, u);
  } else if (includeLower && !includeUpper) {
    return make_shared<ValueSourceScorerAnonymousInnerClass2>(
        shared_from_this(), readerContext, l, u);
  } else if (!includeLower && includeUpper) {
    return make_shared<ValueSourceScorerAnonymousInnerClass3>(
        shared_from_this(), readerContext, l, u);
  } else {
    return make_shared<ValueSourceScorerAnonymousInnerClass4>(
        shared_from_this(), readerContext, l, u);
  }
}

DoubleDocValues::ValueSourceScorerAnonymousInnerClass::
    ValueSourceScorerAnonymousInnerClass(
        shared_ptr<DoubleDocValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, double l, double u)
    : org::apache::lucene::queries::function::ValueSourceScorer(readerContext,
                                                                outerInstance)
{
  this->outerInstance = outerInstance;
  this->l = l;
  this->u = u;
}

bool DoubleDocValues::ValueSourceScorerAnonymousInnerClass::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  double docVal = outerInstance->doubleVal(doc);
  return docVal >= l && docVal <= u;
}

DoubleDocValues::ValueSourceScorerAnonymousInnerClass2::
    ValueSourceScorerAnonymousInnerClass2(
        shared_ptr<DoubleDocValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, double l, double u)
    : org::apache::lucene::queries::function::ValueSourceScorer(readerContext,
                                                                outerInstance)
{
  this->outerInstance = outerInstance;
  this->l = l;
  this->u = u;
}

bool DoubleDocValues::ValueSourceScorerAnonymousInnerClass2::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  double docVal = outerInstance->doubleVal(doc);
  return docVal >= l && docVal < u;
}

DoubleDocValues::ValueSourceScorerAnonymousInnerClass3::
    ValueSourceScorerAnonymousInnerClass3(
        shared_ptr<DoubleDocValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, double l, double u)
    : org::apache::lucene::queries::function::ValueSourceScorer(readerContext,
                                                                outerInstance)
{
  this->outerInstance = outerInstance;
  this->l = l;
  this->u = u;
}

bool DoubleDocValues::ValueSourceScorerAnonymousInnerClass3::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  double docVal = outerInstance->doubleVal(doc);
  return docVal > l && docVal <= u;
}

DoubleDocValues::ValueSourceScorerAnonymousInnerClass4::
    ValueSourceScorerAnonymousInnerClass4(
        shared_ptr<DoubleDocValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, double l, double u)
    : org::apache::lucene::queries::function::ValueSourceScorer(readerContext,
                                                                outerInstance)
{
  this->outerInstance = outerInstance;
  this->l = l;
  this->u = u;
}

bool DoubleDocValues::ValueSourceScorerAnonymousInnerClass4::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  double docVal = outerInstance->doubleVal(doc);
  return docVal > l && docVal < u;
}

shared_ptr<FunctionValues::ValueFiller> DoubleDocValues::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

DoubleDocValues::ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
    shared_ptr<DoubleDocValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueDouble>();
}

shared_ptr<MutableValue>
DoubleDocValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void DoubleDocValues::ValueFillerAnonymousInnerClass::fillValue(int doc) throw(
    IOException)
{
  mval->value = outerInstance->doubleVal(doc);
  mval->exists = outerInstance->exists(doc);
}
} // namespace org::apache::lucene::queries::function::docvalues