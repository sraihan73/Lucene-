using namespace std;

#include "RangeMapFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

RangeMapFloatFunction::RangeMapFloatFunction(shared_ptr<ValueSource> source,
                                             float min, float max, float target,
                                             optional<float> &def)
    : RangeMapFloatFunction(source, min, max, new ConstValueSource(target),
                            def == nullptr ? nullptr
                                           : new ConstValueSource(def))
{
}

RangeMapFloatFunction::RangeMapFloatFunction(shared_ptr<ValueSource> source,
                                             float min, float max,
                                             shared_ptr<ValueSource> target,
                                             shared_ptr<ValueSource> def)
    : source(source), min(min), max(max), target(target), defaultVal(def)
{
}

wstring RangeMapFloatFunction::description()
{
  return L"map_obj(" + source->description() + L"," + to_wstring(min) + L"," +
         to_wstring(max) + L"," + target->description() + L"," +
         (defaultVal == nullptr ? L"null" : defaultVal->description()) + L")";
}

shared_ptr<FunctionValues> RangeMapFloatFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> *const vals =
      source->getValues(context, readerContext);
  shared_ptr<FunctionValues> *const targets =
      target->getValues(context, readerContext);
  shared_ptr<FunctionValues> *const defaults =
      (this->defaultVal == nullptr)
          ? nullptr
          : defaultVal->getValues(context, readerContext);
  return make_shared<FloatDocValuesAnonymousInnerClass>(
      shared_from_this(), vals, targets, defaults);
}

RangeMapFloatFunction::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<RangeMapFloatFunction> outerInstance,
        shared_ptr<FunctionValues> vals, shared_ptr<FunctionValues> targets,
        shared_ptr<FunctionValues> defaults)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->vals = vals;
  this->targets = targets;
  this->defaults = defaults;
}

float RangeMapFloatFunction::FloatDocValuesAnonymousInnerClass::floatVal(
    int doc) 
{
  float val = vals->floatVal(doc);
  return (val >= outerInstance->min && val <= outerInstance->max)
             ? targets->floatVal(doc)
             : (outerInstance->defaultVal == nullptr ? val
                                                     : defaults->floatVal(doc));
}

wstring RangeMapFloatFunction::FloatDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"map_obj(" + vals->toString(doc) + L",min=" +
         to_wstring(outerInstance->min) + L",max=" +
         to_wstring(outerInstance->max) + L",target=" + targets->toString(doc) +
         L",defaultVal=" +
         (defaults == nullptr ? L"null" : (defaults->toString(doc))) + L")";
}

void RangeMapFloatFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  source->createWeight(context, searcher);
}

int RangeMapFloatFunction::hashCode()
{
  int h = source->hashCode();
  h ^= (h << 10) | (static_cast<int>(static_cast<unsigned int>(h) >> 23));
  h += Float::floatToIntBits(min);
  h ^= (h << 14) | (static_cast<int>(static_cast<unsigned int>(h) >> 19));
  h += Float::floatToIntBits(max);
  h += target->hashCode();
  if (defaultVal != nullptr) {
    h += defaultVal->hashCode();
  }
  return h;
}

bool RangeMapFloatFunction::equals(any o)
{
  if (RangeMapFloatFunction::typeid != o.type()) {
    return false;
  }
  shared_ptr<RangeMapFloatFunction> other =
      any_cast<std::shared_ptr<RangeMapFloatFunction>>(o);
  return this->min == other->min && this->max == other->max &&
         this->target->equals(other->target) &&
         this->source->equals(other->source) &&
         (this->defaultVal == other->defaultVal ||
          (this->defaultVal != nullptr &&
           this->defaultVal->equals(other->defaultVal)));
}
} // namespace org::apache::lucene::queries::function::valuesource