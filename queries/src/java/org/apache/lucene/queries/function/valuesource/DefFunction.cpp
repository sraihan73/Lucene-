using namespace std;

#include "DefFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

DefFunction::DefFunction(deque<std::shared_ptr<ValueSource>> &sources)
    : MultiFunction(sources)
{
}

wstring DefFunction::name() { return L"def"; }

shared_ptr<FunctionValues> DefFunction::getValues(
    unordered_map fcontext,
    shared_ptr<LeafReaderContext> readerContext) 
{

  return make_shared<ValuesAnonymousInnerClass>(
      shared_from_this(), valsArr(sources, fcontext, readerContext));
}

DefFunction::ValuesAnonymousInnerClass::ValuesAnonymousInnerClass(
    shared_ptr<DefFunction> outerInstance,
    deque<std::shared_ptr<FunctionValues>> &valsArr)
    : Values(outerInstance, valsArr)
{
  this->outerInstance = outerInstance;
  upto = valsArr.size() - 1;
}

shared_ptr<FunctionValues>
DefFunction::ValuesAnonymousInnerClass::get(int doc) 
{
  for (int i = 0; i < upto; i++) {
    shared_ptr<FunctionValues> vals = valsArr[i];
    if (vals->exists(doc)) {
      return vals;
    }
  }
  return valsArr[upto];
}

char DefFunction::ValuesAnonymousInnerClass::byteVal(int doc) 
{
  return get(doc).byteVal(doc);
}

short DefFunction::ValuesAnonymousInnerClass::shortVal(int doc) throw(
    IOException)
{
  return get(doc).shortVal(doc);
}

float DefFunction::ValuesAnonymousInnerClass::floatVal(int doc) throw(
    IOException)
{
  return get(doc).floatVal(doc);
}

int DefFunction::ValuesAnonymousInnerClass::intVal(int doc) 
{
  return get(doc).intVal(doc);
}

int64_t
DefFunction::ValuesAnonymousInnerClass::longVal(int doc) 
{
  return get(doc).longVal(doc);
}

double
DefFunction::ValuesAnonymousInnerClass::doubleVal(int doc) 
{
  return get(doc).doubleVal(doc);
}

wstring
DefFunction::ValuesAnonymousInnerClass::strVal(int doc) 
{
  return get(doc).strVal(doc);
}

bool DefFunction::ValuesAnonymousInnerClass::boolVal(int doc) 
{
  return get(doc).boolVal(doc);
}

bool DefFunction::ValuesAnonymousInnerClass::bytesVal(
    int doc, shared_ptr<BytesRefBuilder> target) 
{
  return get(doc).bytesVal(doc, target);
}

any DefFunction::ValuesAnonymousInnerClass::objectVal(int doc) throw(
    IOException)
{
  return get(doc).objectVal(doc);
}

bool DefFunction::ValuesAnonymousInnerClass::exists(int doc) 
{
  // return true if any source is exists?
  for (shared_ptr<FunctionValues> vals : valsArr) {
    if (vals->exists(doc)) {
      return true;
    }
  }
  return false;
}

shared_ptr<FunctionValues::ValueFiller>
DefFunction::ValuesAnonymousInnerClass::getValueFiller()
{
  // TODO: need ValueSource.type() to determine correct type
  return outerInstance->super->getValueFiller();
}
} // namespace org::apache::lucene::queries::function::valuesource