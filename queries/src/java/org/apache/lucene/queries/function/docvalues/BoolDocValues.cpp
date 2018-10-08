using namespace std;

#include "BoolDocValues.h"

namespace org::apache::lucene::queries::function::docvalues
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueBool = org::apache::lucene::util::mutable_::MutableValueBool;

BoolDocValues::BoolDocValues(shared_ptr<ValueSource> vs) : vs(vs) {}

char BoolDocValues::byteVal(int doc) 
{
  return boolVal(doc) ? static_cast<char>(1) : static_cast<char>(0);
}

short BoolDocValues::shortVal(int doc) 
{
  return boolVal(doc) ? static_cast<short>(1) : static_cast<short>(0);
}

float BoolDocValues::floatVal(int doc) 
{
  return boolVal(doc) ? static_cast<float>(1) : static_cast<float>(0);
}

int BoolDocValues::intVal(int doc) 
{
  return boolVal(doc) ? 1 : 0;
}

int64_t BoolDocValues::longVal(int doc) 
{
  return boolVal(doc) ? static_cast<int64_t>(1) : static_cast<int64_t>(0);
}

double BoolDocValues::doubleVal(int doc) 
{
  return boolVal(doc) ? static_cast<double>(1) : static_cast<double>(0);
}

wstring BoolDocValues::strVal(int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Boolean::toString(boolVal(doc));
}

any BoolDocValues::objectVal(int doc) 
{
  return exists(doc) ? boolVal(doc) : nullptr;
}

wstring BoolDocValues::toString(int doc) 
{
  return vs->description() + StringHelper::toString(L'=') + strVal(doc);
}

shared_ptr<FunctionValues::ValueFiller> BoolDocValues::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

BoolDocValues::ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
    shared_ptr<BoolDocValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueBool>();
}

shared_ptr<MutableValue>
BoolDocValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void BoolDocValues::ValueFillerAnonymousInnerClass::fillValue(int doc) throw(
    IOException)
{
  mval->value = outerInstance->boolVal(doc);
  mval->exists = outerInstance->exists(doc);
}
} // namespace org::apache::lucene::queries::function::docvalues