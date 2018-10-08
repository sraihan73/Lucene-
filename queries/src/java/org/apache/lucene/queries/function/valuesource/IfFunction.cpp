using namespace std;

#include "IfFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

IfFunction::IfFunction(shared_ptr<ValueSource> ifSource,
                       shared_ptr<ValueSource> trueSource,
                       shared_ptr<ValueSource> falseSource)
    : ifSource(ifSource), trueSource(trueSource), falseSource(falseSource)
{
}

shared_ptr<FunctionValues> IfFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> *const ifVals =
      ifSource->getValues(context, readerContext);
  shared_ptr<FunctionValues> *const trueVals =
      trueSource->getValues(context, readerContext);
  shared_ptr<FunctionValues> *const falseVals =
      falseSource->getValues(context, readerContext);

  return make_shared<FunctionValuesAnonymousInnerClass>(
      shared_from_this(), ifVals, trueVals, falseVals);
}

IfFunction::FunctionValuesAnonymousInnerClass::
    FunctionValuesAnonymousInnerClass(shared_ptr<IfFunction> outerInstance,
                                      shared_ptr<FunctionValues> ifVals,
                                      shared_ptr<FunctionValues> trueVals,
                                      shared_ptr<FunctionValues> falseVals)
{
  this->outerInstance = outerInstance;
  this->ifVals = ifVals;
  this->trueVals = trueVals;
  this->falseVals = falseVals;
}

char IfFunction::FunctionValuesAnonymousInnerClass::byteVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->byteVal(doc)
                              : falseVals->byteVal(doc);
}

short IfFunction::FunctionValuesAnonymousInnerClass::shortVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->shortVal(doc)
                              : falseVals->shortVal(doc);
}

float IfFunction::FunctionValuesAnonymousInnerClass::floatVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->floatVal(doc)
                              : falseVals->floatVal(doc);
}

int IfFunction::FunctionValuesAnonymousInnerClass::intVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->intVal(doc) : falseVals->intVal(doc);
}

int64_t IfFunction::FunctionValuesAnonymousInnerClass::longVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->longVal(doc)
                              : falseVals->longVal(doc);
}

double IfFunction::FunctionValuesAnonymousInnerClass::doubleVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->doubleVal(doc)
                              : falseVals->doubleVal(doc);
}

wstring IfFunction::FunctionValuesAnonymousInnerClass::strVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->strVal(doc) : falseVals->strVal(doc);
}

bool IfFunction::FunctionValuesAnonymousInnerClass::boolVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->boolVal(doc)
                              : falseVals->boolVal(doc);
}

bool IfFunction::FunctionValuesAnonymousInnerClass::bytesVal(
    int doc, shared_ptr<BytesRefBuilder> target) 
{
  return ifVals->boolVal(doc) ? trueVals->bytesVal(doc, target)
                              : falseVals->bytesVal(doc, target);
}

any IfFunction::FunctionValuesAnonymousInnerClass::objectVal(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->objectVal(doc)
                              : falseVals->objectVal(doc);
}

bool IfFunction::FunctionValuesAnonymousInnerClass::exists(int doc) throw(
    IOException)
{
  return ifVals->boolVal(doc) ? trueVals->exists(doc) : falseVals->exists(doc);
}

shared_ptr<FunctionValues::ValueFiller>
IfFunction::FunctionValuesAnonymousInnerClass::getValueFiller()
{
  // TODO: we need types of trueSource / falseSource to handle this
  // for now, use float.
  return outerInstance->super->getValueFiller();
}

wstring IfFunction::FunctionValuesAnonymousInnerClass::toString(int doc) throw(
    IOException)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"if(" + ifVals->toString(doc) + StringHelper::toString(L',') +
         trueVals->toString(doc) + StringHelper::toString(L',') +
         falseVals->toString(doc) + StringHelper::toString(L')');
}

wstring IfFunction::description()
{
  return L"if(" + ifSource->description() + StringHelper::toString(L',') +
         trueSource->description() + StringHelper::toString(L',') +
         falseSource + L')';
}

int IfFunction::hashCode()
{
  int h = ifSource->hashCode();
  h = h * 31 + trueSource->hashCode();
  h = h * 31 + falseSource->hashCode();
  return h;
}

bool IfFunction::equals(any o)
{
  if (!(std::dynamic_pointer_cast<IfFunction>(o) != nullptr)) {
    return false;
  }
  shared_ptr<IfFunction> other = any_cast<std::shared_ptr<IfFunction>>(o);
  return ifSource->equals(other->ifSource) &&
         trueSource->equals(other->trueSource) &&
         falseSource->equals(other->falseSource);
}

void IfFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  ifSource->createWeight(context, searcher);
  trueSource->createWeight(context, searcher);
  falseSource->createWeight(context, searcher);
}
} // namespace org::apache::lucene::queries::function::valuesource