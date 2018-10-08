using namespace std;

#include "FloatDocValues.h"

namespace org::apache::lucene::queries::function::docvalues
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;
using MutableValueFloat =
    org::apache::lucene::util::mutable_::MutableValueFloat;

FloatDocValues::FloatDocValues(shared_ptr<ValueSource> vs) : vs(vs) {}

char FloatDocValues::byteVal(int doc) 
{
  return static_cast<char>(floatVal(doc));
}

short FloatDocValues::shortVal(int doc) 
{
  return static_cast<short>(floatVal(doc));
}

int FloatDocValues::intVal(int doc) 
{
  return static_cast<int>(floatVal(doc));
}

int64_t FloatDocValues::longVal(int doc) 
{
  return static_cast<int64_t>(floatVal(doc));
}

bool FloatDocValues::boolVal(int doc) 
{
  return floatVal(doc) != 0.0f;
}

double FloatDocValues::doubleVal(int doc) 
{
  return static_cast<double>(floatVal(doc));
}

wstring FloatDocValues::strVal(int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Float::toString(floatVal(doc));
}

any FloatDocValues::objectVal(int doc) 
{
  return exists(doc) ? floatVal(doc) : nullptr;
}

wstring FloatDocValues::toString(int doc) 
{
  return vs->description() + StringHelper::toString(L'=') + strVal(doc);
}

shared_ptr<FunctionValues::ValueFiller> FloatDocValues::getValueFiller()
{
  return make_shared<ValueFillerAnonymousInnerClass>(shared_from_this());
}

FloatDocValues::ValueFillerAnonymousInnerClass::ValueFillerAnonymousInnerClass(
    shared_ptr<FloatDocValues> outerInstance)
{
  this->outerInstance = outerInstance;
  mval = make_shared<MutableValueFloat>();
}

shared_ptr<MutableValue>
FloatDocValues::ValueFillerAnonymousInnerClass::getValue()
{
  return mval;
}

void FloatDocValues::ValueFillerAnonymousInnerClass::fillValue(int doc) throw(
    IOException)
{
  mval->value = outerInstance->floatVal(doc);
  mval->exists = outerInstance->exists(doc);
}
} // namespace org::apache::lucene::queries::function::docvalues