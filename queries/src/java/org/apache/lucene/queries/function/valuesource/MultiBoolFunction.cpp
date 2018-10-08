using namespace std;

#include "MultiBoolFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BoolDocValues =
    org::apache::lucene::queries::function::docvalues::BoolDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

MultiBoolFunction::MultiBoolFunction(
    deque<std::shared_ptr<ValueSource>> &sources)
    : sources(sources)
{
}

shared_ptr<BoolDocValues> MultiBoolFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  std::deque<std::shared_ptr<FunctionValues>> vals(sources.size());
  int i = 0;
  for (auto source : sources) {
    vals[i++] = source->getValues(context, readerContext);
  }

  return make_shared<BoolDocValuesAnonymousInnerClass>(shared_from_this(),
                                                       vals);
}

MultiBoolFunction::BoolDocValuesAnonymousInnerClass::
    BoolDocValuesAnonymousInnerClass(
        shared_ptr<MultiBoolFunction> outerInstance,
        deque<std::shared_ptr<FunctionValues>> &vals)
    : org::apache::lucene::queries::function::docvalues::BoolDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->vals = vals;
}

bool MultiBoolFunction::BoolDocValuesAnonymousInnerClass::boolVal(
    int doc) 
{
  return outerInstance->func(doc, vals);
}

wstring MultiBoolFunction::BoolDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(outerInstance->name());
  sb->append(L'(');
  bool first = true;
  for (auto dv : vals) {
    if (first) {
      first = false;
    } else {
      sb->append(L',');
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(dv->toString(doc));
  }
  return sb->toString();
}

wstring MultiBoolFunction::description()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>(name());
  sb->append(L'(');
  bool first = true;
  for (auto source : sources) {
    if (first) {
      first = false;
    } else {
      sb->append(L',');
    }
    sb->append(source->description());
  }
  return sb->toString();
}

int MultiBoolFunction::hashCode()
{
  return sources.hashCode() + name().hashCode();
}

bool MultiBoolFunction::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<MultiBoolFunction> other =
      any_cast<std::shared_ptr<MultiBoolFunction>>(o);
  return this->sources.equals(other->sources);
}

void MultiBoolFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  for (auto source : sources) {
    source->createWeight(context, searcher);
  }
}
} // namespace org::apache::lucene::queries::function::valuesource