using namespace std;

#include "MultiFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

MultiFunction::MultiFunction(deque<std::shared_ptr<ValueSource>> &sources)
    : sources(sources)
{
}

wstring MultiFunction::description() { return description(name(), sources); }

bool MultiFunction::allExists(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &values) 
{
  for (auto v : values) {
    if (!v->exists(doc)) {
      return false;
    }
  }
  return true;
}

bool MultiFunction::anyExists(
    int doc,
    std::deque<std::shared_ptr<FunctionValues>> &values) 
{
  for (auto v : values) {
    if (v->exists(doc)) {
      return true;
    }
  }
  return false;
}

bool MultiFunction::allExists(
    int doc, shared_ptr<FunctionValues> values1,
    shared_ptr<FunctionValues> values2) 
{
  return values1->exists(doc) && values2->exists(doc);
}

bool MultiFunction::anyExists(
    int doc, shared_ptr<FunctionValues> values1,
    shared_ptr<FunctionValues> values2) 
{
  return values1->exists(doc) || values2->exists(doc);
}

wstring
MultiFunction::description(const wstring &name,
                           deque<std::shared_ptr<ValueSource>> &sources)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(name)->append(L'(');
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

std::deque<std::shared_ptr<FunctionValues>> MultiFunction::valsArr(
    deque<std::shared_ptr<ValueSource>> &sources, unordered_map fcontext,
    shared_ptr<LeafReaderContext> readerContext) 
{
  std::deque<std::shared_ptr<FunctionValues>> valsArr(sources.size());
  int i = 0;
  for (auto source : sources) {
    valsArr[i++] = source->getValues(fcontext, readerContext);
  }
  return valsArr;
}

MultiFunction::Values::Values(
    shared_ptr<MultiFunction> outerInstance,
    std::deque<std::shared_ptr<FunctionValues>> &valsArr)
    : valsArr(valsArr), outerInstance(outerInstance)
{
}

wstring MultiFunction::Values::toString(int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return MultiFunction::toString(outerInstance->name(), valsArr, doc);
}

shared_ptr<FunctionValues::ValueFiller> MultiFunction::Values::getValueFiller()
{
  // TODO: need ValueSource.type() to determine correct type
  return FunctionValues::getValueFiller();
}

wstring
MultiFunction::toString(const wstring &name,
                        std::deque<std::shared_ptr<FunctionValues>> &valsArr,
                        int doc) 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(name)->append(L'(');
  bool firstTime = true;
  for (auto vals : valsArr) {
    if (firstTime) {
      firstTime = false;
    } else {
      sb->append(L',');
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(vals->toString(doc));
  }
  sb->append(L')');
  return sb->toString();
}

void MultiFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  for (auto source : sources) {
    source->createWeight(context, searcher);
  }
}

int MultiFunction::hashCode() { return sources.hashCode() + name().hashCode(); }

bool MultiFunction::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<MultiFunction> other = any_cast<std::shared_ptr<MultiFunction>>(o);
  return this->sources.equals(other->sources);
}
} // namespace org::apache::lucene::queries::function::valuesource