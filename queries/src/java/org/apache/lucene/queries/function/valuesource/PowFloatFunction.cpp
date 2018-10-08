using namespace std;

#include "PowFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;

PowFloatFunction::PowFloatFunction(shared_ptr<ValueSource> a,
                                   shared_ptr<ValueSource> b)
    : DualFloatFunction(a, b)
{
}

wstring PowFloatFunction::name() { return L"pow"; }

float PowFloatFunction::func(
    int doc, shared_ptr<FunctionValues> aVals,
    shared_ptr<FunctionValues> bVals) 
{
  return static_cast<float>(pow(aVals->floatVal(doc), bVals->floatVal(doc)));
}
} // namespace org::apache::lucene::queries::function::valuesource