using namespace std;

#include "VectorValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

VectorValueSource::VectorValueSource(
    deque<std::shared_ptr<ValueSource>> &sources)
    : sources(sources)
{
}

deque<std::shared_ptr<ValueSource>> VectorValueSource::getSources()
{
  return sources;
}

int VectorValueSource::dimension() { return sources.size(); }

wstring VectorValueSource::name() { return L"deque"; }

shared_ptr<FunctionValues> VectorValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  int size = sources.size();

  // special-case x,y and lat,lon since it's so common
  if (size == 2) {
    shared_ptr<FunctionValues> *const x =
        sources[0]->getValues(context, readerContext);
    shared_ptr<FunctionValues> *const y =
        sources[1]->getValues(context, readerContext);
    return make_shared<FunctionValuesAnonymousInnerClass>(shared_from_this(), x,
                                                          y);
  }

  std::deque<std::shared_ptr<FunctionValues>> valsArr(size);
  for (int i = 0; i < size; i++) {
    valsArr[i] = sources[i]->getValues(context, readerContext);
  }

  return make_shared<FunctionValuesAnonymousInnerClass2>(shared_from_this(),
                                                         valsArr);
}

VectorValueSource::FunctionValuesAnonymousInnerClass::
    FunctionValuesAnonymousInnerClass(
        shared_ptr<VectorValueSource> outerInstance,
        shared_ptr<FunctionValues> x, shared_ptr<FunctionValues> y)
{
  this->outerInstance = outerInstance;
  this->x = x;
  this->y = y;
}

void VectorValueSource::FunctionValuesAnonymousInnerClass::byteVal(
    int doc, std::deque<char> &vals) 
{
  vals[0] = x->byteVal(doc);
  vals[1] = y->byteVal(doc);
}

void VectorValueSource::FunctionValuesAnonymousInnerClass::shortVal(
    int doc, std::deque<short> &vals) 
{
  vals[0] = x->shortVal(doc);
  vals[1] = y->shortVal(doc);
}

void VectorValueSource::FunctionValuesAnonymousInnerClass::intVal(
    int doc, std::deque<int> &vals) 
{
  vals[0] = x->intVal(doc);
  vals[1] = y->intVal(doc);
}

void VectorValueSource::FunctionValuesAnonymousInnerClass::longVal(
    int doc, std::deque<int64_t> &vals) 
{
  vals[0] = x->longVal(doc);
  vals[1] = y->longVal(doc);
}

void VectorValueSource::FunctionValuesAnonymousInnerClass::floatVal(
    int doc, std::deque<float> &vals) 
{
  vals[0] = x->floatVal(doc);
  vals[1] = y->floatVal(doc);
}

void VectorValueSource::FunctionValuesAnonymousInnerClass::doubleVal(
    int doc, std::deque<double> &vals) 
{
  vals[0] = x->doubleVal(doc);
  vals[1] = y->doubleVal(doc);
}

void VectorValueSource::FunctionValuesAnonymousInnerClass::strVal(
    int doc, std::deque<wstring> &vals) 
{
  vals[0] = x->strVal(doc);
  vals[1] = y->strVal(doc);
}

wstring VectorValueSource::FunctionValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return outerInstance->name() + L"(" + x->toString(doc) + L"," +
         y->toString(doc) + L")";
}

VectorValueSource::FunctionValuesAnonymousInnerClass2::
    FunctionValuesAnonymousInnerClass2(
        shared_ptr<VectorValueSource> outerInstance,
        deque<std::shared_ptr<FunctionValues>> &valsArr)
{
  this->outerInstance = outerInstance;
  this->valsArr = valsArr;
}

void VectorValueSource::FunctionValuesAnonymousInnerClass2::byteVal(
    int doc, std::deque<char> &vals) 
{
  for (int i = 0; i < valsArr.size(); i++) {
    vals[i] = valsArr[i]->byteVal(doc);
  }
}

void VectorValueSource::FunctionValuesAnonymousInnerClass2::shortVal(
    int doc, std::deque<short> &vals) 
{
  for (int i = 0; i < valsArr.size(); i++) {
    vals[i] = valsArr[i]->shortVal(doc);
  }
}

void VectorValueSource::FunctionValuesAnonymousInnerClass2::floatVal(
    int doc, std::deque<float> &vals) 
{
  for (int i = 0; i < valsArr.size(); i++) {
    vals[i] = valsArr[i]->floatVal(doc);
  }
}

void VectorValueSource::FunctionValuesAnonymousInnerClass2::intVal(
    int doc, std::deque<int> &vals) 
{
  for (int i = 0; i < valsArr.size(); i++) {
    vals[i] = valsArr[i]->intVal(doc);
  }
}

void VectorValueSource::FunctionValuesAnonymousInnerClass2::longVal(
    int doc, std::deque<int64_t> &vals) 
{
  for (int i = 0; i < valsArr.size(); i++) {
    vals[i] = valsArr[i]->longVal(doc);
  }
}

void VectorValueSource::FunctionValuesAnonymousInnerClass2::doubleVal(
    int doc, std::deque<double> &vals) 
{
  for (int i = 0; i < valsArr.size(); i++) {
    vals[i] = valsArr[i]->doubleVal(doc);
  }
}

void VectorValueSource::FunctionValuesAnonymousInnerClass2::strVal(
    int doc, std::deque<wstring> &vals) 
{
  for (int i = 0; i < valsArr.size(); i++) {
    vals[i] = valsArr[i]->strVal(doc);
  }
}

wstring VectorValueSource::FunctionValuesAnonymousInnerClass2::toString(
    int doc) 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(outerInstance->name())->append(L'(');
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

void VectorValueSource::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  for (auto source : sources) {
    source->createWeight(context, searcher);
  }
}

wstring VectorValueSource::description()
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
  sb->append(L")");
  return sb->toString();
}

bool VectorValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<VectorValueSource>(o) != nullptr)) {
    return false;
  }

  shared_ptr<VectorValueSource> that =
      any_cast<std::shared_ptr<VectorValueSource>>(o);

  return sources.equals(that->sources);
}

int VectorValueSource::hashCode() { return sources.hashCode(); }
} // namespace org::apache::lucene::queries::function::valuesource