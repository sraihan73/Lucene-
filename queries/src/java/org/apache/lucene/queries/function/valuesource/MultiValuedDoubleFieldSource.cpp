using namespace std;

#include "MultiValuedDoubleFieldSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using Type = org::apache::lucene::search::SortedNumericSelector::Type;
using SortedNumericSortField =
    org::apache::lucene::search::SortedNumericSortField;

MultiValuedDoubleFieldSource::MultiValuedDoubleFieldSource(
    const wstring &field, SortedNumericSelector::Type selector)
    : DoubleFieldSource(field), selector(selector)
{
  Objects::requireNonNull(
      field, L"Field is required to create a MultiValuedDoubleFieldSource");
  Objects::requireNonNull(selector, L"SortedNumericSelector is required to "
                                    L"create a MultiValuedDoubleFieldSource");
}

shared_ptr<SortField> MultiValuedDoubleFieldSource::getSortField(bool reverse)
{
  return make_shared<SortedNumericSortField>(field, SortField::Type::DOUBLE,
                                             reverse, selector);
}

wstring MultiValuedDoubleFieldSource::description()
{
  return L"double(" + field + StringHelper::toString(L',') + selector.name() +
         L')';
}

shared_ptr<NumericDocValues> MultiValuedDoubleFieldSource::getNumericDocValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<SortedNumericDocValues> sortedDv =
      DocValues::getSortedNumeric(readerContext->reader(), field);
  return SortedNumericSelector::wrap(sortedDv, selector,
                                     SortField::Type::DOUBLE);
}

bool MultiValuedDoubleFieldSource::equals(any o)
{
  if (o.type() != MultiValuedDoubleFieldSource::typeid) {
    return false;
  }
  shared_ptr<MultiValuedDoubleFieldSource> other =
      any_cast<std::shared_ptr<MultiValuedDoubleFieldSource>>(o);
  if (this->selector != other->selector) {
    return false;
  }
  return this->field == other->field;
}

int MultiValuedDoubleFieldSource::hashCode()
{
  int h = DoubleFieldSource::hashCode();
  h += selector.hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function::valuesource