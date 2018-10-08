using namespace std;

#include "ConstValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;

ConstValueSource::ConstValueSource(float constant)
    : constant(constant), dv(constant)
{
}

wstring ConstValueSource::description()
{
  return L"const(" + to_wstring(constant) + L")";
}

shared_ptr<FunctionValues> ConstValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this());
}

ConstValueSource::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<ConstValueSource> outerInstance)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
}

float ConstValueSource::FloatDocValuesAnonymousInnerClass::floatVal(int doc)
{
  return outerInstance->constant;
}

int ConstValueSource::FloatDocValuesAnonymousInnerClass::intVal(int doc)
{
  return static_cast<int>(outerInstance->constant);
}

int64_t ConstValueSource::FloatDocValuesAnonymousInnerClass::longVal(int doc)
{
  return static_cast<int64_t>(outerInstance->constant);
}

double ConstValueSource::FloatDocValuesAnonymousInnerClass::doubleVal(int doc)
{
  return outerInstance->dv;
}

wstring ConstValueSource::FloatDocValuesAnonymousInnerClass::toString(int doc)
{
  return outerInstance->description();
}

any ConstValueSource::FloatDocValuesAnonymousInnerClass::objectVal(int doc)
{
  return outerInstance->constant;
}

bool ConstValueSource::FloatDocValuesAnonymousInnerClass::boolVal(int doc)
{
  return outerInstance->constant != 0.0f;
}

int ConstValueSource::hashCode()
{
  return Float::floatToIntBits(constant) * 31;
}

bool ConstValueSource::equals(any o)
{
  if (!(std::dynamic_pointer_cast<ConstValueSource>(o) != nullptr)) {
    return false;
  }
  shared_ptr<ConstValueSource> other =
      any_cast<std::shared_ptr<ConstValueSource>>(o);
  return this->constant == other->constant;
}

int ConstValueSource::getInt() { return static_cast<int>(constant); }

int64_t ConstValueSource::getLong()
{
  return static_cast<int64_t>(constant);
}

float ConstValueSource::getFloat() { return constant; }

double ConstValueSource::getDouble() { return dv; }

shared_ptr<Number> ConstValueSource::getNumber() { return constant; }

bool ConstValueSource::getBool() { return constant != 0.0f; }
} // namespace org::apache::lucene::queries::function::valuesource