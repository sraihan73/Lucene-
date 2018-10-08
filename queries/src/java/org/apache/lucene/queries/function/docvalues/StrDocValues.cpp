using namespace std;

#include "StrDocValues.h"

namespace org::apache::lucene::queries::function::docvalues
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueStr = org::apache::lucene::util::mutable_::MutableValueStr;

StrDocValues::StrDocValues(shared_ptr<ValueSource> vs) : vs(vs) {}

any StrDocValues::objectVal(int doc) 
{
  return exists(doc) ? strVal(doc) : nullptr;
}

bool StrDocValues::boolVal(int doc)  { return exists(doc); }

wstring StrDocValues::toString(int doc) 
{
  return vs->description() + L"='" + strVal(doc) + L"'";
}

shared_ptr<FunctionValues::ValueFiller> StrDocValues::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

StrDocValues::ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
    shared_ptr<StrDocValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueStr>();
}

shared_ptr<MutableValue>
StrDocValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void StrDocValues::ValueFillerAnonymousInnerClass::fillValue(int doc) throw(
    IOException)
{
  mval->exists = outerInstance->bytesVal(doc, mval::value);
}
} // namespace org::apache::lucene::queries::function::docvalues