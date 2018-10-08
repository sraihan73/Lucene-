using namespace std;

#include "ProductFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;

ProductFloatFunction::ProductFloatFunction(
    std::deque<std::shared_ptr<ValueSource>> &sources)
    : MultiFloatFunction(sources)
{
}

wstring ProductFloatFunction::name() { return L"product"; }

float ProductFloatFunction::func(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &valsArr) 
{
  float val = 1.0f;
  for (auto vals : valsArr) {
    val *= vals->floatVal(doc);
  }
  return val;
}
} // namespace org::apache::lucene::queries::function::valuesource