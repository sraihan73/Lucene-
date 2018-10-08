using namespace std;

#include "DualFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

DualFloatFunction::DualFloatFunction(shared_ptr<ValueSource> a,
                                     shared_ptr<ValueSource> b)
    : a(a), b(b)
{
}

wstring DualFloatFunction::description()
{
  return name() + L"(" + a->description() + L"," + b->description() + L")";
}

shared_ptr<FunctionValues> DualFloatFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> *const aVals =
      a->getValues(context, readerContext);
  shared_ptr<FunctionValues> *const bVals =
      b->getValues(context, readerContext);
  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this(),
                                                        aVals, bVals);
}

DualFloatFunction::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<DualFloatFunction> outerInstance,
        shared_ptr<FunctionValues> aVals, shared_ptr<FunctionValues> bVals)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->aVals = aVals;
  this->bVals = bVals;
}

float DualFloatFunction::FloatDocValuesAnonymousInnerClass::floatVal(
    int doc) 
{
  return outerInstance->func(doc, aVals, bVals);
}

bool DualFloatFunction::FloatDocValuesAnonymousInnerClass::exists(
    int doc) 
{
  return MultiFunction::allExists(doc, aVals, bVals);
}

wstring DualFloatFunction::FloatDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return outerInstance->name() + StringHelper::toString(L'(') +
         aVals->toString(doc) + StringHelper::toString(L',') +
         bVals->toString(doc) + StringHelper::toString(L')');
}

void DualFloatFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  a->createWeight(context, searcher);
  b->createWeight(context, searcher);
}

int DualFloatFunction::hashCode()
{
  int h = a->hashCode();
  h ^= (h << 13) | (static_cast<int>(static_cast<unsigned int>(h) >> 20));
  h += b->hashCode();
  h ^= (h << 23) | (static_cast<int>(static_cast<unsigned int>(h) >> 10));
  h += name().hashCode();
  return h;
}

bool DualFloatFunction::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<DualFloatFunction> other =
      any_cast<std::shared_ptr<DualFloatFunction>>(o);
  return this->a->equals(other->a) && this->b->equals(other->b);
}
} // namespace org::apache::lucene::queries::function::valuesource