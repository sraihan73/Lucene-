using namespace std;

#include "SortedSetFieldSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using DocTermsIndexDocValues =
    org::apache::lucene::queries::function::docvalues::DocTermsIndexDocValues;
using SortField = org::apache::lucene::search::SortField;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;
using SortedSetSortField = org::apache::lucene::search::SortedSetSortField;

SortedSetFieldSource::SortedSetFieldSource(const wstring &field)
    : SortedSetFieldSource(field, SortedSetSelector::Type::MIN)
{
}

SortedSetFieldSource::SortedSetFieldSource(const wstring &field,
                                           SortedSetSelector::Type selector)
    : FieldCacheSource(field), selector(selector)
{
}

shared_ptr<SortField> SortedSetFieldSource::getSortField(bool reverse)
{
  return make_shared<SortedSetSortField>(this->field, reverse, this->selector);
}

shared_ptr<FunctionValues> SortedSetFieldSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<SortedSetDocValues> sortedSet =
      DocValues::getSortedSet(readerContext->reader(), field);
  shared_ptr<SortedDocValues> view =
      SortedSetSelector::wrap(sortedSet, selector);
  return make_shared<DocTermsIndexDocValuesAnonymousInnerClass>(
      shared_from_this(), this->field, view);
}

SortedSetFieldSource::DocTermsIndexDocValuesAnonymousInnerClass::
    DocTermsIndexDocValuesAnonymousInnerClass(
        shared_ptr<SortedSetFieldSource> outerInstance, const wstring &field,
        shared_ptr<SortedDocValues> view)
    : org::apache::lucene::queries::function::docvalues::DocTermsIndexDocValues(
          field, outerInstance, view)
{
  this->outerInstance = outerInstance;
}

wstring SortedSetFieldSource::DocTermsIndexDocValuesAnonymousInnerClass::toTerm(
    const wstring &readableValue)
{
  return readableValue;
}

any SortedSetFieldSource::DocTermsIndexDocValuesAnonymousInnerClass::objectVal(
    int doc) 
{
  return strVal(doc);
}

wstring SortedSetFieldSource::description()
{
  return L"sortedset(" + field + L",selector=" + selector + L')';
}

int SortedSetFieldSource::hashCode()
{
  constexpr int prime = 31;
  int result = FieldCacheSource::hashCode();
  result = prime * result + ((selector == nullptr) ? 0 : selector.hashCode());
  return result;
}

bool SortedSetFieldSource::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (!FieldCacheSource::equals(obj)) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<SortedSetFieldSource> other =
      any_cast<std::shared_ptr<SortedSetFieldSource>>(obj);
  if (selector != other->selector) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::queries::function::valuesource