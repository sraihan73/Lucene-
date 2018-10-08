using namespace std;

#include "ComparisonBoolFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using BoolDocValues =
    org::apache::lucene::queries::function::docvalues::BoolDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

ComparisonBoolFunction::ComparisonBoolFunction(shared_ptr<ValueSource> lhs,
                                               shared_ptr<ValueSource> rhs,
                                               const wstring &name)
    : lhs(lhs), rhs(rhs), name(name)
{
}

wstring ComparisonBoolFunction::name() { return this->name_; }

shared_ptr<FunctionValues> ComparisonBoolFunction::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> *const lhsVal =
      this->lhs->getValues(context, readerContext);
  shared_ptr<FunctionValues> *const rhsVal =
      this->rhs->getValues(context, readerContext);
  const wstring compLabel = this->name();

  return make_shared<BoolDocValuesAnonymousInnerClass>(
      shared_from_this(), lhsVal, rhsVal, compLabel);
}

ComparisonBoolFunction::BoolDocValuesAnonymousInnerClass::
    BoolDocValuesAnonymousInnerClass(
        shared_ptr<ComparisonBoolFunction> outerInstance,
        shared_ptr<FunctionValues> lhsVal, shared_ptr<FunctionValues> rhsVal,
        const wstring &compLabel)
    : org::apache::lucene::queries::function::docvalues::BoolDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->lhsVal = lhsVal;
  this->rhsVal = rhsVal;
  this->compLabel = compLabel;
}

bool ComparisonBoolFunction::BoolDocValuesAnonymousInnerClass::boolVal(
    int doc) 
{
  return outerInstance->compare(doc, lhsVal, rhsVal);
}

wstring ComparisonBoolFunction::BoolDocValuesAnonymousInnerClass::toString(
    int doc) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return compLabel + L"(" + lhsVal->toString(doc) + L"," +
         rhsVal->toString(doc) + L")";
}

bool ComparisonBoolFunction::BoolDocValuesAnonymousInnerClass::exists(
    int doc) 
{
  return lhsVal->exists(doc) && rhsVal->exists(doc);
}

bool ComparisonBoolFunction::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<ComparisonBoolFunction> other =
      any_cast<std::shared_ptr<ComparisonBoolFunction>>(o);
  return name() == other->name() && lhs->equals(other->lhs) &&
         rhs->equals(other->rhs);
}

int ComparisonBoolFunction::hashCode()
{
  int h = this->getClass().hashCode();
  h = h * 31 + this->name().hashCode();
  h = h * 31 + lhs->hashCode();
  h = h * 31 + rhs->hashCode();
  return h;
}

wstring ComparisonBoolFunction::description()
{
  return name() + L"(" + lhs->description() + L"," + rhs->description() + L")";
}

void ComparisonBoolFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  lhs->createWeight(context, searcher);
  rhs->createWeight(context, searcher);
}
} // namespace org::apache::lucene::queries::function::valuesource