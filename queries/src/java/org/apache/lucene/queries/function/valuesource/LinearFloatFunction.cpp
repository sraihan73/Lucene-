using namespace std;

#include "LinearFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

LinearFloatFunction::LinearFloatFunction(shared_ptr<ValueSource> source,
                                         float slope, float intercept)
    : source(source), slope(slope), intercept(intercept)
{
}

wstring LinearFloatFunction::description()
{
  return to_wstring(slope) + L"*float(" + source->description() + L")+" +
         to_wstring(intercept);
}

shared_ptr<FunctionValues> LinearFloatFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> *const vals =
      source->getValues(context, readerContext);
  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this(),
                                                        vals);
}

LinearFloatFunction::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<LinearFloatFunction> outerInstance,
        shared_ptr<FunctionValues> vals)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->vals = vals;
}

float LinearFloatFunction::FloatDocValuesAnonymousInnerClass::floatVal(
    int doc) 
{
  return vals->floatVal(doc) * outerInstance->slope + outerInstance->intercept;
}

bool LinearFloatFunction::FloatDocValuesAnonymousInnerClass::exists(
    int doc) 
{
  return vals->exists(doc);
}

wstring LinearFloatFunction::FloatDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return to_wstring(outerInstance->slope) + L"*float(" + vals->toString(doc) +
         L")+" + to_wstring(outerInstance->intercept);
}

void LinearFloatFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  source->createWeight(context, searcher);
}

int LinearFloatFunction::hashCode()
{
  int h = Float::floatToIntBits(slope);
  h = (static_cast<int>(static_cast<unsigned int>(h) >> 2)) | (h << 30);
  h += Float::floatToIntBits(intercept);
  h ^= (h << 14) | (static_cast<int>(static_cast<unsigned int>(h) >> 19));
  return h + source->hashCode();
}

bool LinearFloatFunction::equals(any o)
{
  if (LinearFloatFunction::typeid != o.type()) {
    return false;
  }
  shared_ptr<LinearFloatFunction> other =
      any_cast<std::shared_ptr<LinearFloatFunction>>(o);
  return this->slope == other->slope && this->intercept == other->intercept &&
         this->source->equals(other->source);
}
} // namespace org::apache::lucene::queries::function::valuesource