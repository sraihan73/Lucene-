using namespace std;

#include "MaxFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;

MaxFloatFunction::MaxFloatFunction(
    std::deque<std::shared_ptr<ValueSource>> &sources)
    : MultiFloatFunction(sources)
{
}

wstring MaxFloatFunction::name() { return L"max"; }

float MaxFloatFunction::func(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &valsArr) 
{
  if (!exists(doc, valsArr)) {
    return 0.0f;
  }

  float val = -numeric_limits<float>::infinity();
  for (auto vals : valsArr) {
    if (vals->exists(doc)) {
      val = max(vals->floatVal(doc), val);
    }
  }
  return val;
}

bool MaxFloatFunction::exists(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &valsArr) 
{
  return MultiFunction::anyExists(doc, valsArr);
}
} // namespace org::apache::lucene::queries::function::valuesource