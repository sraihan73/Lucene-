using namespace std;

#include "SimpleFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;

SimpleFloatFunction::SimpleFloatFunction(shared_ptr<ValueSource> source)
    : SingleFunction(source)
{
}

shared_ptr<FunctionValues> SimpleFloatFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> *const vals =
      source->getValues(context, readerContext);
  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this(),
                                                        vals);
}

SimpleFloatFunction::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<SimpleFloatFunction> outerInstance,
        shared_ptr<FunctionValues> vals)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->vals = vals;
}

float SimpleFloatFunction::FloatDocValuesAnonymousInnerClass::floatVal(
    int doc) 
{
  return outerInstance->func(doc, vals);
}

wstring SimpleFloatFunction::FloatDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return outerInstance->name() + StringHelper::toString(L'(') +
         vals->toString(doc) + StringHelper::toString(L')');
}
} // namespace org::apache::lucene::queries::function::valuesource