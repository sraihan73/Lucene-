using namespace std;

#include "MultiFloatFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

MultiFloatFunction::MultiFloatFunction(
    std::deque<std::shared_ptr<ValueSource>> &sources)
    : sources(sources)
{
}

bool MultiFloatFunction::exists(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &valsArr) 
{
  return MultiFunction::allExists(doc, valsArr);
}

wstring MultiFloatFunction::description()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(name())->append(L'(');
  bool firstTime = true;
  for (auto source : sources) {
    if (firstTime) {
      firstTime = false;
    } else {
      sb->append(L',');
    }
    sb->append(source);
  }
  sb->append(L')');
  return sb->toString();
}

shared_ptr<FunctionValues> MultiFloatFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  std::deque<std::shared_ptr<FunctionValues>> valsArr(sources.size());
  for (int i = 0; i < sources.size(); i++) {
    valsArr[i] = sources[i]->getValues(context, readerContext);
  }

  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this(),
                                                        valsArr);
}

MultiFloatFunction::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(
        shared_ptr<MultiFloatFunction> outerInstance,
        deque<std::shared_ptr<FunctionValues>> &valsArr)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->valsArr = valsArr;
}

float MultiFloatFunction::FloatDocValuesAnonymousInnerClass::floatVal(
    int doc) 
{
  return outerInstance->func(doc, valsArr);
}

bool MultiFloatFunction::FloatDocValuesAnonymousInnerClass::exists(
    int doc) 
{
  return outerInstance->exists(doc, valsArr);
}

wstring MultiFloatFunction::FloatDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return MultiFunction::toString(outerInstance->name(), valsArr, doc);
}

void MultiFloatFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  for (auto source : sources) {
    source->createWeight(context, searcher);
  }
}

int MultiFloatFunction::hashCode()
{
  return Arrays::hashCode(sources) + name().hashCode();
}

bool MultiFloatFunction::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<MultiFloatFunction> other =
      any_cast<std::shared_ptr<MultiFloatFunction>>(o);
  return this->name() == other->name() &&
         Arrays::equals(this->sources, other->sources);
}
} // namespace org::apache::lucene::queries::function::valuesource