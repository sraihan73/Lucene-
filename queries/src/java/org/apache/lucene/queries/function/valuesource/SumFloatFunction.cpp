using namespace std;

#include "SumFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;

SumFloatFunction::SumFloatFunction(
    std::deque<std::shared_ptr<ValueSource>> &sources)
    : MultiFloatFunction(sources)
{
}

wstring SumFloatFunction::name() { return L"sum"; }

float SumFloatFunction::func(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &valsArr) 
{
  float val = 0.0f;
  for (auto vals : valsArr) {
    val += vals->floatVal(doc);
  }
  return val;
}
} // namespace org::apache::lucene::queries::function::valuesource