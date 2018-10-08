using namespace std;

#include "DoubleConstValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using DoubleDocValues =
    org::apache::lucene::queries::function::docvalues::DoubleDocValues;

DoubleConstValueSource::DoubleConstValueSource(double constant)
    : constant(constant), fv(static_cast<float>(constant)),
      lv(static_cast<int64_t>(constant))
{
}

wstring DoubleConstValueSource::description()
{
  return L"const(" + to_wstring(constant) + L")";
}

shared_ptr<FunctionValues> DoubleConstValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  return make_shared<DoubleDocValuesAnonymousInnerClass>(shared_from_this());
}

DoubleConstValueSource::DoubleDocValuesAnonymousInnerClass::
    DoubleDocValuesAnonymousInnerClass(
        shared_ptr<DoubleConstValueSource> outerInstance)
    : org::apache::lucene::queries::function::docvalues::DoubleDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
}

float DoubleConstValueSource::DoubleDocValuesAnonymousInnerClass::floatVal(
    int doc)
{
  return outerInstance->fv;
}

int DoubleConstValueSource::DoubleDocValuesAnonymousInnerClass::intVal(int doc)
{
  return static_cast<int>(outerInstance->lv);
}

int64_t
DoubleConstValueSource::DoubleDocValuesAnonymousInnerClass::longVal(int doc)
{
  return outerInstance->lv;
}

double
DoubleConstValueSource::DoubleDocValuesAnonymousInnerClass::doubleVal(int doc)
{
  return outerInstance->constant;
}

wstring
DoubleConstValueSource::DoubleDocValuesAnonymousInnerClass::strVal(int doc)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Double::toString(outerInstance->constant);
}

any DoubleConstValueSource::DoubleDocValuesAnonymousInnerClass::objectVal(
    int doc)
{
  return outerInstance->constant;
}

wstring
DoubleConstValueSource::DoubleDocValuesAnonymousInnerClass::toString(int doc)
{
  return outerInstance->description();
}

int DoubleConstValueSource::hashCode()
{
  int64_t bits = Double::doubleToRawLongBits(constant);
  return static_cast<int>(
      bits ^
      (static_cast<int64_t>(static_cast<uint64_t>(bits) >> 32)));
}

bool DoubleConstValueSource::equals(any o)
{
  if (!(std::dynamic_pointer_cast<DoubleConstValueSource>(o) != nullptr)) {
    return false;
  }
  shared_ptr<DoubleConstValueSource> other =
      any_cast<std::shared_ptr<DoubleConstValueSource>>(o);
  return this->constant == other->constant;
}

int DoubleConstValueSource::getInt() { return static_cast<int>(lv); }

int64_t DoubleConstValueSource::getLong() { return lv; }

float DoubleConstValueSource::getFloat() { return fv; }

double DoubleConstValueSource::getDouble() { return constant; }

shared_ptr<Number> DoubleConstValueSource::getNumber() { return constant; }

bool DoubleConstValueSource::getBool() { return constant != 0; }
} // namespace org::apache::lucene::queries::function::valuesource