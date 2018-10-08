using namespace std;

#include "SimpleBoolFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BoolDocValues =
    org::apache::lucene::queries::function::docvalues::BoolDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

SimpleBoolFunction::SimpleBoolFunction(shared_ptr<ValueSource> source)
    : source(source)
{
}

shared_ptr<BoolDocValues> SimpleBoolFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> *const vals =
      source->getValues(context, readerContext);
  return make_shared<BoolDocValuesAnonymousInnerClass>(shared_from_this(),
                                                       vals);
}

SimpleBoolFunction::BoolDocValuesAnonymousInnerClass::
    BoolDocValuesAnonymousInnerClass(
        shared_ptr<SimpleBoolFunction> outerInstance,
        shared_ptr<FunctionValues> vals)
    : org::apache::lucene::queries::function::docvalues::BoolDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->vals = vals;
}

bool SimpleBoolFunction::BoolDocValuesAnonymousInnerClass::boolVal(
    int doc) 
{
  return outerInstance->func(doc, vals);
}

wstring SimpleBoolFunction::BoolDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return outerInstance->name() + StringHelper::toString(L'(') +
         vals->toString(doc) + StringHelper::toString(L')');
}

wstring SimpleBoolFunction::description()
{
  return name() + StringHelper::toString(L'(') + source->description() +
         StringHelper::toString(L')');
}

int SimpleBoolFunction::hashCode()
{
  return source->hashCode() + name().hashCode();
}

bool SimpleBoolFunction::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<SimpleBoolFunction> other =
      any_cast<std::shared_ptr<SimpleBoolFunction>>(o);
  return this->source->equals(other->source);
}

void SimpleBoolFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  source->createWeight(context, searcher);
}
} // namespace org::apache::lucene::queries::function::valuesource