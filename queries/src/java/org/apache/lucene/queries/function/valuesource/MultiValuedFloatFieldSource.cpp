using namespace std;

#include "MultiValuedFloatFieldSource.h"

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

MultiValuedFloatFieldSource::MultiValuedFloatFieldSource(
    const wstring &field, SortedNumericSelector::Type selector)
    : FloatFieldSource(field), selector(selector)
{
  Objects::requireNonNull(
      field, L"Field is required to create a MultiValuedFloatFieldSource");
  Objects::requireNonNull(selector, L"SortedNumericSelector is required to "
                                    L"create a MultiValuedFloatFieldSource");
}

shared_ptr<SortField> MultiValuedFloatFieldSource::getSortField(bool reverse)
{
  return make_shared<SortedNumericSortField>(field, SortField::Type::FLOAT,
                                             reverse, selector);
}

wstring MultiValuedFloatFieldSource::description()
{
  return L"float(" + field + StringHelper::toString(L',') + selector.name() +
         L')';
}

shared_ptr<NumericDocValues> MultiValuedFloatFieldSource::getNumericDocValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<SortedNumericDocValues> sortedDv =
      DocValues::getSortedNumeric(readerContext->reader(), field);
  return SortedNumericSelector::wrap(sortedDv, selector,
                                     SortField::Type::FLOAT);
}

bool MultiValuedFloatFieldSource::equals(any o)
{
  if (o.type() != MultiValuedFloatFieldSource::typeid) {
    return false;
  }
  shared_ptr<MultiValuedFloatFieldSource> other =
      any_cast<std::shared_ptr<MultiValuedFloatFieldSource>>(o);
  if (this->selector != other->selector) {
    return false;
  }
  return this->field == other->field;
}

int MultiValuedFloatFieldSource::hashCode()
{
  int h = FloatFieldSource::hashCode();
  h += selector.hashCode();
  return h;
}
} // namespace org::apache::lucene::queries::function::valuesource