using namespace std;

#include "FunctionValues.h"

namespace org::apache::lucene::queries::function
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Explanation = org::apache::lucene::search::Explanation;
using Scorer = org::apache::lucene::search::Scorer;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueFloat =
    org::apache::lucene::util::mutable_::MutableValueFloat;

char FunctionValues::byteVal(int doc) 
{
  throw make_shared<UnsupportedOperationException>();
}

short FunctionValues::shortVal(int doc) 
{
  throw make_shared<UnsupportedOperationException>();
}

float FunctionValues::floatVal(int doc) 
{
  throw make_shared<UnsupportedOperationException>();
}

int FunctionValues::intVal(int doc) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t FunctionValues::longVal(int doc) 
{
  throw make_shared<UnsupportedOperationException>();
}

double FunctionValues::doubleVal(int doc) 
{
  throw make_shared<UnsupportedOperationException>();
}

wstring FunctionValues::strVal(int doc) 
{
  throw make_shared<UnsupportedOperationException>();
}

bool FunctionValues::boolVal(int doc) 
{
  return intVal(doc) != 0;
}

bool FunctionValues::bytesVal(
    int doc, shared_ptr<BytesRefBuilder> target) 
{
  wstring s = strVal(doc);
  if (s == L"") {
    target->clear();
    return false;
  }
  target->copyChars(s);
  return true;
}

any FunctionValues::objectVal(int doc) 
{
  // most FunctionValues are functions, so by default return a Float()
  return floatVal(doc);
}

bool FunctionValues::exists(int doc)  { return true; }

int FunctionValues::ordVal(int doc) 
{
  throw make_shared<UnsupportedOperationException>();
}

int FunctionValues::numOrd()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<ValueFiller> FunctionValues::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

FunctionValues::ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
    shared_ptr<FunctionValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueFloat>();
}

shared_ptr<MutableValue>
FunctionValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void FunctionValues::ValueFillerAnonymousInnerClass::fillValue(int doc) throw(
    IOException)
{
  mval->value = outerInstance->floatVal(doc);
}

void FunctionValues::byteVal(int doc,
                             std::deque<char> &vals) 
{
  throw make_shared<UnsupportedOperationException>();
}

void FunctionValues::shortVal(int doc,
                              std::deque<short> &vals) 
{
  throw make_shared<UnsupportedOperationException>();
}

void FunctionValues::floatVal(int doc,
                              std::deque<float> &vals) 
{
  throw make_shared<UnsupportedOperationException>();
}

void FunctionValues::intVal(int doc, std::deque<int> &vals) 
{
  throw make_shared<UnsupportedOperationException>();
}

void FunctionValues::longVal(int doc,
                             std::deque<int64_t> &vals) 
{
  throw make_shared<UnsupportedOperationException>();
}

void FunctionValues::doubleVal(int doc,
                               std::deque<double> &vals) 
{
  throw make_shared<UnsupportedOperationException>();
}

void FunctionValues::strVal(int doc,
                            std::deque<wstring> &vals) 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Explanation> FunctionValues::explain(int doc) 
{
  return Explanation::match(floatVal(doc), toString(doc));
}

shared_ptr<ValueSourceScorer>
FunctionValues::getScorer(shared_ptr<LeafReaderContext> readerContext)
{
  return make_shared<ValueSourceScorerAnonymousInnerClass>(shared_from_this(),
                                                           readerContext);
}

FunctionValues::ValueSourceScorerAnonymousInnerClass::
    ValueSourceScorerAnonymousInnerClass(
        shared_ptr<FunctionValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext)
    : ValueSourceScorer(readerContext, outerInstance)
{
  this->outerInstance = outerInstance;
}

bool FunctionValues::ValueSourceScorerAnonymousInnerClass::matches(int doc)
{
  return true;
}

shared_ptr<ValueSourceScorer>
FunctionValues::getRangeScorer(shared_ptr<LeafReaderContext> readerContext,
                               const wstring &lowerVal, const wstring &upperVal,
                               bool includeLower,
                               bool includeUpper) 
{
  float lower;
  float upper;

  if (lowerVal == L"") {
    lower = -numeric_limits<float>::infinity();
  } else {
    lower = stof(lowerVal);
  }
  if (upperVal == L"") {
    upper = numeric_limits<float>::infinity();
  } else {
    upper = stof(upperVal);
  }

  constexpr float l = lower;
  constexpr float u = upper;

  if (includeLower && includeUpper) {
    return make_shared<ValueSourceScorerAnonymousInnerClass2>(
        shared_from_this(), readerContext, l, u);
  } else if (includeLower && !includeUpper) {
    return make_shared<ValueSourceScorerAnonymousInnerClass3>(
        shared_from_this(), readerContext, l, u);
  } else if (!includeLower && includeUpper) {
    return make_shared<ValueSourceScorerAnonymousInnerClass4>(
        shared_from_this(), readerContext, l, u);
  } else {
    return make_shared<ValueSourceScorerAnonymousInnerClass5>(
        shared_from_this(), readerContext, l, u);
  }
}

FunctionValues::ValueSourceScorerAnonymousInnerClass2::
    ValueSourceScorerAnonymousInnerClass2(
        shared_ptr<FunctionValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, float l, float u)
    : ValueSourceScorer(readerContext, outerInstance)
{
  this->outerInstance = outerInstance;
  this->l = l;
  this->u = u;
}

bool FunctionValues::ValueSourceScorerAnonymousInnerClass2::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  float docVal = outerInstance->floatVal(doc);
  return docVal >= l && docVal <= u;
}

FunctionValues::ValueSourceScorerAnonymousInnerClass3::
    ValueSourceScorerAnonymousInnerClass3(
        shared_ptr<FunctionValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, float l, float u)
    : ValueSourceScorer(readerContext, outerInstance)
{
  this->outerInstance = outerInstance;
  this->l = l;
  this->u = u;
}

bool FunctionValues::ValueSourceScorerAnonymousInnerClass3::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  float docVal = outerInstance->floatVal(doc);
  return docVal >= l && docVal < u;
}

FunctionValues::ValueSourceScorerAnonymousInnerClass4::
    ValueSourceScorerAnonymousInnerClass4(
        shared_ptr<FunctionValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, float l, float u)
    : ValueSourceScorer(readerContext, outerInstance)
{
  this->outerInstance = outerInstance;
  this->l = l;
  this->u = u;
}

bool FunctionValues::ValueSourceScorerAnonymousInnerClass4::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  float docVal = outerInstance->floatVal(doc);
  return docVal > l && docVal <= u;
}

FunctionValues::ValueSourceScorerAnonymousInnerClass5::
    ValueSourceScorerAnonymousInnerClass5(
        shared_ptr<FunctionValues> outerInstance,
        shared_ptr<LeafReaderContext> readerContext, float l, float u)
    : ValueSourceScorer(readerContext, outerInstance)
{
  this->outerInstance = outerInstance;
  this->l = l;
  this->u = u;
}

bool FunctionValues::ValueSourceScorerAnonymousInnerClass5::matches(
    int doc) 
{
  if (!outerInstance->exists(doc)) {
    return false;
  }
  float docVal = outerInstance->floatVal(doc);
  return docVal > l && docVal < u;
}
} // namespace org::apache::lucene::queries::function