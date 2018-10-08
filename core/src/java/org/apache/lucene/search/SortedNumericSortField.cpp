using namespace std;

#include "SortedNumericSortField.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;

SortedNumericSortField::SortedNumericSortField(const wstring &field,
                                               SortField::Type type)
    : SortedNumericSortField(field, type, false)
{
}

SortedNumericSortField::SortedNumericSortField(const wstring &field,
                                               SortField::Type type,
                                               bool reverse)
    : SortedNumericSortField(field, type, reverse,
                             SortedNumericSelector::Type::MIN)
{
}

SortedNumericSortField::SortedNumericSortField(
    const wstring &field, SortField::Type type, bool reverse,
    SortedNumericSelector::Type selector)
    : SortField(field, SortField::Type::CUSTOM, reverse), selector(selector),
      type(type)
{
  if (selector == nullptr) {
    throw make_shared<NullPointerException>();
  }
  if (type == nullptr) {
    throw make_shared<NullPointerException>();
  }
}

SortField::Type SortedNumericSortField::getNumericType() { return type; }

SortedNumericSelector::Type SortedNumericSortField::getSelector()
{
  return selector;
}

int SortedNumericSortField::hashCode()
{
  constexpr int prime = 31;
  int result = SortField::hashCode();
  result = prime * result + selector.hashCode();
  result = prime * result + type.hashCode();
  return result;
}

bool SortedNumericSortField::equals(any obj)
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
  shared_ptr<SortedNumericSortField> other =
      any_cast<std::shared_ptr<SortedNumericSortField>>(obj);
  if (selector != other->selector) {
    return false;
  }
  if (type != other->type) {
    return false;
  }
  return true;
}

wstring SortedNumericSortField::toString()
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(wstring(L"<sortednumeric") + L": \"")
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
  buffer->append(L" type=");
  buffer->append(type);

  return buffer->toString();
}

void SortedNumericSortField::setMissingValue(any missingValue)
{
  this->missingValue = missingValue;
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public FieldComparator<?> getComparator(int numHits,
// int sortPos)
shared_ptr < FieldComparator <
    ? >> SortedNumericSortField::getComparator(int numHits, int sortPos)
{
  switch (type) {
  case org::apache::lucene::search::SortField::Type::INT:
    return make_shared<IntComparatorAnonymousInnerClass>(
        shared_from_this(), numHits, getField(),
        any_cast<optional<int>>(missingValue));
  case org::apache::lucene::search::SortField::Type::FLOAT:
    return make_shared<FloatComparatorAnonymousInnerClass>(
        shared_from_this(), numHits, getField(),
        any_cast<optional<float>>(missingValue));
  case org::apache::lucene::search::SortField::Type::LONG:
    return make_shared<LongComparatorAnonymousInnerClass>(
        shared_from_this(), numHits, getField(),
        any_cast<optional<int64_t>>(missingValue));
  case org::apache::lucene::search::SortField::Type::DOUBLE:
    return make_shared<DoubleComparatorAnonymousInnerClass>(
        shared_from_this(), numHits, getField(),
        any_cast<optional<double>>(missingValue));
  default:
    throw make_shared<AssertionError>();
  }
}

SortedNumericSortField::IntComparatorAnonymousInnerClass::
    IntComparatorAnonymousInnerClass(
        shared_ptr<SortedNumericSortField> outerInstance, int numHits,
        const wstring &getField, optional<int>(optional<int>) missingValue)
    : FieldComparator::IntComparator(numHits, getField,
                                     (optional<int>)missingValue)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
SortedNumericSortField::IntComparatorAnonymousInnerClass::getNumericDocValues(
    shared_ptr<LeafReaderContext> context,
    const wstring &field) 
{
  return SortedNumericSelector::wrap(
      DocValues::getSortedNumeric(context->reader(), field),
      outerInstance->selector, outerInstance->type);
}

SortedNumericSortField::FloatComparatorAnonymousInnerClass::
    FloatComparatorAnonymousInnerClass(
        shared_ptr<SortedNumericSortField> outerInstance, int numHits,
        const wstring &getField, optional<float>(optional<float>) missingValue)
    : FieldComparator::FloatComparator(numHits, getField,
                                       (optional<float>)missingValue)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
SortedNumericSortField::FloatComparatorAnonymousInnerClass::getNumericDocValues(
    shared_ptr<LeafReaderContext> context,
    const wstring &field) 
{
  return SortedNumericSelector::wrap(
      DocValues::getSortedNumeric(context->reader(), field),
      outerInstance->selector, outerInstance->type);
}

SortedNumericSortField::LongComparatorAnonymousInnerClass::
    LongComparatorAnonymousInnerClass(
        shared_ptr<SortedNumericSortField> outerInstance, int numHits,
        const wstring &getField,
        optional<int64_t>(optional<int64_t>) missingValue)
    : FieldComparator::LongComparator(numHits, getField,
                                      (optional<int64_t>)missingValue)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
SortedNumericSortField::LongComparatorAnonymousInnerClass::getNumericDocValues(
    shared_ptr<LeafReaderContext> context,
    const wstring &field) 
{
  return SortedNumericSelector::wrap(
      DocValues::getSortedNumeric(context->reader(), field),
      outerInstance->selector, outerInstance->type);
}

SortedNumericSortField::DoubleComparatorAnonymousInnerClass::
    DoubleComparatorAnonymousInnerClass(
        shared_ptr<SortedNumericSortField> outerInstance, int numHits,
        const wstring &getField,
        optional<double>(optional<double>) missingValue)
    : FieldComparator::DoubleComparator(numHits, getField,
                                        (optional<double>)missingValue)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
SortedNumericSortField::DoubleComparatorAnonymousInnerClass::
    getNumericDocValues(shared_ptr<LeafReaderContext> context,
                        const wstring &field) 
{
  return SortedNumericSelector::wrap(
      DocValues::getSortedNumeric(context->reader(), field),
      outerInstance->selector, outerInstance->type);
}
} // namespace org::apache::lucene::search