using namespace std;

#include "ScaleFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

ScaleFloatFunction::ScaleFloatFunction(shared_ptr<ValueSource> source,
                                       float min, float max)
    : source(source), min(min), max(max)
{
}

wstring ScaleFloatFunction::description()
{
  return L"scale(" + source->description() + L"," + to_wstring(min) + L"," +
         to_wstring(max) + L")";
}

shared_ptr<ScaleInfo> ScaleFloatFunction::createScaleInfo(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves =
      ReaderUtil::getTopLevelContext(readerContext)->leaves();

  float minVal = numeric_limits<float>::infinity();
  float maxVal = -numeric_limits<float>::infinity();

  for (auto leaf : leaves) {
    int maxDoc = leaf->reader()->maxDoc();
    shared_ptr<FunctionValues> vals = source->getValues(context, leaf);
    for (int i = 0; i < maxDoc; i++) {
      if (!vals->exists(i)) {
        continue;
      }
      float val = vals->floatVal(i);
      if ((Float::floatToRawIntBits(val) & (0xff << 23)) == 0xff << 23) {
        // if the exponent in the float is all ones, then this is +Inf, -Inf or
        // NaN which don't make sense to factor into the scale function
        continue;
      }
      if (val < minVal) {
        minVal = val;
      }
      if (val > maxVal) {
        maxVal = val;
      }
    }
  }

  if (minVal == numeric_limits<float>::infinity()) {
    // must have been an empty index
    minVal = maxVal = 0;
  }

  shared_ptr<ScaleInfo> scaleInfo = make_shared<ScaleInfo>();
  scaleInfo->minVal = minVal;
  scaleInfo->maxVal = maxVal;
  context.emplace(ScaleFloatFunction::this, scaleInfo);
  return scaleInfo;
}

shared_ptr<FunctionValues> ScaleFloatFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{

  shared_ptr<ScaleInfo> scaleInfo =
      std::static_pointer_cast<ScaleInfo>(context[ScaleFloatFunction::this]);
  if (scaleInfo == nullptr) {
    scaleInfo = createScaleInfo(context, readerContext);
  }

  constexpr float scale =
      (scaleInfo->maxVal - scaleInfo->minVal == 0)
          ? 0
          : (max - min) / (scaleInfo->maxVal - scaleInfo->minVal);
  constexpr float minSource = scaleInfo->minVal;
  constexpr float maxSource = scaleInfo->maxVal;

  shared_ptr<FunctionValues> *const vals =
      source->getValues(context, readerContext);

  return make_shared<FloatDocValuesAnonymousInnerClass>(
      shared_from_this(), scale, minSource, maxSource, vals);
}

ScaleFloatFunction::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<ScaleFloatFunction> outerInstance, float scale,
        float minSource, float maxSource, shared_ptr<FunctionValues> vals)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->scale = scale;
  this->minSource = minSource;
  this->maxSource = maxSource;
  this->vals = vals;
}

bool ScaleFloatFunction::FloatDocValuesAnonymousInnerClass::exists(
    int doc) 
{
  return vals->exists(doc);
}

float ScaleFloatFunction::FloatDocValuesAnonymousInnerClass::floatVal(
    int doc) 
{
  return (vals->floatVal(doc) - minSource) * scale + outerInstance->min;
}

wstring ScaleFloatFunction::FloatDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"scale(" + vals->toString(doc) + L",toMin=" +
         to_wstring(outerInstance->min) + L",toMax=" +
         to_wstring(outerInstance->max) + L",fromMin=" + to_wstring(minSource) +
         L",fromMax=" + to_wstring(maxSource) + L")";
}

void ScaleFloatFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  source->createWeight(context, searcher);
}

int ScaleFloatFunction::hashCode()
{
  int h = Float::floatToIntBits(min);
  h = h * 29;
  h += Float::floatToIntBits(max);
  h = h * 29;
  h += source->hashCode();
  return h;
}

bool ScaleFloatFunction::equals(any o)
{
  if (ScaleFloatFunction::typeid != o.type()) {
    return false;
  }
  shared_ptr<ScaleFloatFunction> other =
      any_cast<std::shared_ptr<ScaleFloatFunction>>(o);
  return this->min == other->min && this->max == other->max &&
         this->source->equals(other->source);
}
} // namespace org::apache::lucene::queries::function::valuesource