using namespace std;

#include "MultiValuedIntFieldSource.h"

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

MultiValuedIntFieldSource::MultiValuedIntFieldSource(
    const wstring &field, SortedNumericSelector::Type selector)
    : IntFieldSource(field), selector(selector)
{
  Objects::requireNonNull(
      field, L"Field is required to create a MultiValuedIntFieldSource");
  Objects::requireNonNull(selector, L"SortedNumericSelector is required to "
                                    L"create a MultiValuedIntFieldSource");
}

shared_ptr<SortField> MultiValuedIntFieldSource::getSortField(bool reverse)
{
  return make_shared<SortedNumericSortField>(field, SortField::Type::INT,
                                             reverse, selector);
}

wstring MultiValuedIntFieldSource::description()
{
  return L"int(" + field + StringHelper::toString(L',') + selector.name() +
         L')';
}

shared_ptr<NumericDocValues> MultiValuedIntFieldSource::getNumericDocValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<SortedNumericDocValues> sortedDv =
      DocValues::getSortedNumeric(readerContext->reader(), field);
  return SortedNumericSelector::wrap(sortedDv, selector, SortField::Type::INT);
}

bool MultiValuedIntFieldSource::equals(any o)
{
  if (o.type() != MultiValuedIntFieldSource::typeid) {
    return false;
  }
  shared_ptr<MultiValuedIntFieldSource> other =
      any_cast<std::shared_ptr<MultiValuedIntFieldSource>>(o);
  if (this->selector != other->selector) {
    return false;
  }
  return this->field == other->field;
}

int MultiValuedIntFieldSource::hashCode()
{
  int h = IntFieldSource::hashCode();
  h += selector.hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function::valuesource