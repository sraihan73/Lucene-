using namespace std;

#include "MultiValuedLongFieldSource.h"

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

MultiValuedLongFieldSource::MultiValuedLongFieldSource(
    const wstring &field, SortedNumericSelector::Type selector)
    : LongFieldSource(field), selector(selector)
{
  Objects::requireNonNull(
      field, L"Field is required to create a MultiValuedLongFieldSource");
  Objects::requireNonNull(selector, L"SortedNumericSelector is required to "
                                    L"create a MultiValuedLongFieldSource");
}

shared_ptr<SortField> MultiValuedLongFieldSource::getSortField(bool reverse)
{
  return make_shared<SortedNumericSortField>(field, SortField::Type::LONG,
                                             reverse, selector);
}

wstring MultiValuedLongFieldSource::description()
{
  return L"long(" + field + StringHelper::toString(L',') + selector.name() +
         L')';
}

shared_ptr<NumericDocValues> MultiValuedLongFieldSource::getNumericDocValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<SortedNumericDocValues> sortedDv =
      DocValues::getSortedNumeric(readerContext->reader(), field);
  return SortedNumericSelector::wrap(sortedDv, selector, SortField::Type::LONG);
}

bool MultiValuedLongFieldSource::equals(any o)
{
  if (o.type() != MultiValuedLongFieldSource::typeid) {
    return false;
  }
  shared_ptr<MultiValuedLongFieldSource> other =
      any_cast<std::shared_ptr<MultiValuedLongFieldSource>>(o);
  if (this->selector != other->selector) {
    return false;
  }
  return this->field == other->field;
}

int MultiValuedLongFieldSource::hashCode()
{
  int h = LongFieldSource::hashCode();
  h += selector.hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function::valuesource