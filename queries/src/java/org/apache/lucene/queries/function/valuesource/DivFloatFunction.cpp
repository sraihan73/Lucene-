using namespace std;

#include "DivFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;

DivFloatFunction::DivFloatFunction(shared_ptr<ValueSource> a,
                                   shared_ptr<ValueSource> b)
    : DualFloatFunction(a, b)
{
}

wstring DivFloatFunction::name() { return L"div"; }

float DivFloatFunction::func(
    int doc, shared_ptr<FunctionValues> aVals,
    shared_ptr<FunctionValues> bVals) 
{
  return aVals->floatVal(doc) / bVals->floatVal(doc);
}
} // namespace org::apache::lucene::queries::function::valuesource