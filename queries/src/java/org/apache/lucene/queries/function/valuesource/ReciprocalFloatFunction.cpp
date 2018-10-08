using namespace std;

#include "ReciprocalFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

ReciprocalFloatFunction::ReciprocalFloatFunction(shared_ptr<ValueSource> source,
                                                 float m, float a, float b)
{
  this->source = source;
  this->m = m;
  this->a = a;
  this->b = b;
}

shared_ptr<FunctionValues> ReciprocalFloatFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> *const vals =
      source->getValues(context, readerContext);
  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this(),
                                                        vals);
}

ReciprocalFloatFunction::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<ReciprocalFloatFunction> outerInstance,
        shared_ptr<FunctionValues> vals)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->vals = vals;
}

float ReciprocalFloatFunction::FloatDocValuesAnonymousInnerClass::floatVal(
    int doc) 
{
  return outerInstance->a /
         (outerInstance->m * vals->floatVal(doc) + outerInstance->b);
}

bool ReciprocalFloatFunction::FloatDocValuesAnonymousInnerClass::exists(
    int doc) 
{
  return vals->exists(doc);
}

wstring ReciprocalFloatFunction::FloatDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Float::toString(outerInstance->a) +
         L"/("
         // C++ TODO: There is no native C++ equivalent to 'toString':
         + to_wstring(outerInstance->m) + L"*float(" + vals->toString(doc) +
         StringHelper::toString(L')') + StringHelper::toString(L'+') +
         to_wstring(outerInstance->b) + StringHelper::toString(L')');
}

void ReciprocalFloatFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  source->createWeight(context, searcher);
}

wstring ReciprocalFloatFunction::description()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Float::toString(a) + L"/(" + to_wstring(m) + L"*float(" +
         source->description() + L")" + L"+" + to_wstring(b) +
         StringHelper::toString(L')');
}

int ReciprocalFloatFunction::hashCode()
{
  int h = Float::floatToIntBits(a) + Float::floatToIntBits(m);
  h ^= (h << 13) | (static_cast<int>(static_cast<unsigned int>(h) >> 20));
  return h + (Float::floatToIntBits(b)) + source->hashCode();
}

bool ReciprocalFloatFunction::equals(any o)
{
  if (ReciprocalFloatFunction::typeid != o.type()) {
    return false;
  }
  shared_ptr<ReciprocalFloatFunction> other =
      any_cast<std::shared_ptr<ReciprocalFloatFunction>>(o);
  return this->m == other->m && this->a == other->a && this->b == other->b &&
         this->source->equals(other->source);
}
} // namespace org::apache::lucene::queries::function::valuesource