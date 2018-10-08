using namespace std;

#include "SortedSetSortField.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;

SortedSetSortField::SortedSetSortField(const wstring &field, bool reverse)
    : SortedSetSortField(field, reverse, SortedSetSelector::Type::MIN)
{
}

SortedSetSortField::SortedSetSortField(const wstring &field, bool reverse,
                                       SortedSetSelector::Type selector)
    : SortField(field, SortField::Type::CUSTOM, reverse), selector(selector)
{
  if (selector == nullptr) {
    throw make_shared<NullPointerException>();
  }
}

SortedSetSelector::Type SortedSetSortField::getSelector() { return selector; }

int SortedSetSortField::hashCode()
{
  return 31 * SortField::hashCode() + selector.hashCode();
}

bool SortedSetSortField::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (!SortField::equals(obj)) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<SortedSetSortField> other =
      any_cast<std::shared_ptr<SortedSetSortField>>(obj);
  if (selector != other->selector) {
    return false;
  }
  return true;
}

wstring SortedSetSortField::toString()
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(wstring(L"<sortedset") + L": \"")
      ->append(getField())
      ->append(L"\">");
  if (getReverse()) {
    buffer->append(L'!');
  }
  if (missingValue != nullptr) {
    buffer->append(L" missingValue=");
    buffer->append(missingValue);
  }
  buffer->append(L" selector=");
  buffer->append(selector);

  return buffer->toString();
}

void SortedSetSortField::setMissingValue(any missingValue)
{
  if (missingValue != STRING_FIRST && missingValue != STRING_LAST) {
    throw invalid_argument(L"For SORTED_SET type, missing value must be either "
                           L"STRING_FIRST or STRING_LAST");
  }
  this->missingValue = missingValue;
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public FieldComparator<?> getComparator(int numHits,
// int sortPos)
shared_ptr < FieldComparator <
    ? >> SortedSetSortField::getComparator(int numHits, int sortPos)
{
  return make_shared<TermOrdValComparatorAnonymousInnerClass>(
      shared_from_this(), numHits, getField(), missingValue == STRING_LAST);
}

SortedSetSortField::TermOrdValComparatorAnonymousInnerClass::
    TermOrdValComparatorAnonymousInnerClass(
        shared_ptr<SortedSetSortField> outerInstance, int numHits,
        const wstring &getField, bool missingValue)
    : FieldComparator::TermOrdValComparator(numHits, getField,
                                            missingValue == STRING_LAST)
{
  this->outerInstance = outerInstance;
}

shared_ptr<SortedDocValues>
SortedSetSortField::TermOrdValComparatorAnonymousInnerClass::getSortedDocValues(
    shared_ptr<LeafReaderContext> context,
    const wstring &field) 
{
  return SortedSetSelector::wrap(
      DocValues::getSortedSet(context->reader(), field),
      outerInstance->selector);
}
} // namespace org::apache::lucene::search