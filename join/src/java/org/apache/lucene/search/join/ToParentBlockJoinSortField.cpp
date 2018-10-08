using namespace std;

#include "ToParentBlockJoinSortField.h"

namespace org::apache::lucene::search::join
{
using DocValues = org::apache::lucene::index::DocValues;
using FilterNumericDocValues =
    org::apache::lucene::index::FilterNumericDocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using SortField = org::apache::lucene::search::SortField;
using BitSet = org::apache::lucene::util::BitSet;
using NumericUtils = org::apache::lucene::util::NumericUtils;

ToParentBlockJoinSortField::ToParentBlockJoinSortField(
    const wstring &field, Type type, bool reverse,
    shared_ptr<BitSetProducer> parentFilter,
    shared_ptr<BitSetProducer> childFilter)
    : org::apache::lucene::search::SortField(field, type, reverse),
      order(reverse), parentFilter(parentFilter), childFilter(childFilter)
{
  switch (getType()) {
  case STRING:
  case DOUBLE:
  case FLOAT:
  case LONG:
  case INT:
    // ok
    break;
  default:
    throw make_shared<UnsupportedOperationException>(L"Sort type " + type +
                                                     L" is not supported");
  }
}

ToParentBlockJoinSortField::ToParentBlockJoinSortField(
    const wstring &field, Type type, bool reverse, bool order,
    shared_ptr<BitSetProducer> parentFilter,
    shared_ptr<BitSetProducer> childFilter)
    : org::apache::lucene::search::SortField(field, type, reverse),
      order(order), parentFilter(parentFilter), childFilter(childFilter)
{
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public org.apache.lucene.search.FieldComparator<?>
// getComparator(int numHits, int sortPos)
shared_ptr < FieldComparator <
    ? >> ToParentBlockJoinSortField::getComparator(int numHits, int sortPos)
{
  switch (getType()) {
  case STRING:
    return getStringComparator(numHits);
  case DOUBLE:
    return getDoubleComparator(numHits);
  case FLOAT:
    return getFloatComparator(numHits);
  case LONG:
    return getLongComparator(numHits);
  case INT:
    return getIntComparator(numHits);
  default:
    throw make_shared<UnsupportedOperationException>(L"Sort type " + getType() +
                                                     L" is not supported");
  }
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
// getStringComparator(int numHits)
shared_ptr < FieldComparator <
    ? >> ToParentBlockJoinSortField::getStringComparator(int numHits)
{
  return make_shared<TermOrdValComparatorAnonymousInnerClass>(
      shared_from_this(), numHits, getField(), missingValue == STRING_LAST);
}

ToParentBlockJoinSortField::TermOrdValComparatorAnonymousInnerClass::
    TermOrdValComparatorAnonymousInnerClass(
        shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const wstring &getField, bool missingValue)
    : org::apache::lucene::search::FieldComparator::TermOrdValComparator(
          numHits, getField, missingValue == STRING_LAST)
{
  this->outerInstance = outerInstance;
}

shared_ptr<SortedDocValues>
ToParentBlockJoinSortField::TermOrdValComparatorAnonymousInnerClass::
    getSortedDocValues(shared_ptr<LeafReaderContext> context,
                       const wstring &field) 
{
  shared_ptr<SortedSetDocValues> sortedSet =
      DocValues::getSortedSet(context->reader(), field);
  constexpr BlockJoinSelector::Type type = outerInstance->order
                                               ? BlockJoinSelector::Type::MAX
                                               : BlockJoinSelector::Type::MIN;
  shared_ptr<BitSet> *const parents =
      outerInstance->parentFilter->getBitSet(context);
  shared_ptr<BitSet> *const children =
      outerInstance->childFilter->getBitSet(context);
  if (children == nullptr) {
    return DocValues::emptySorted();
  }
  return BlockJoinSelector::wrap(sortedSet, type, parents, children);
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
// getIntComparator(int numHits)
shared_ptr < FieldComparator <
    ? >> ToParentBlockJoinSortField::getIntComparator(int numHits)
{
  return make_shared<IntComparatorAnonymousInnerClass>(
      shared_from_this(), numHits, getField(),
      any_cast<optional<int>>(missingValue));
}

ToParentBlockJoinSortField::IntComparatorAnonymousInnerClass::
    IntComparatorAnonymousInnerClass(
        shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const wstring &getField, optional<int>(optional<int>) missingValue)
    : org::apache::lucene::search::FieldComparator::IntComparator(
          numHits, getField, (optional<int>)missingValue)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
ToParentBlockJoinSortField::IntComparatorAnonymousInnerClass::
    getNumericDocValues(shared_ptr<LeafReaderContext> context,
                        const wstring &field) 
{
  shared_ptr<SortedNumericDocValues> sortedNumeric =
      DocValues::getSortedNumeric(context->reader(), field);
  constexpr BlockJoinSelector::Type type = outerInstance->order
                                               ? BlockJoinSelector::Type::MAX
                                               : BlockJoinSelector::Type::MIN;
  shared_ptr<BitSet> *const parents =
      outerInstance->parentFilter->getBitSet(context);
  shared_ptr<BitSet> *const children =
      outerInstance->childFilter->getBitSet(context);
  if (children == nullptr) {
    return DocValues::emptyNumeric();
  }
  return BlockJoinSelector::wrap(sortedNumeric, type, parents, children);
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
// getLongComparator(int numHits)
shared_ptr < FieldComparator <
    ? >> ToParentBlockJoinSortField::getLongComparator(int numHits)
{
  return make_shared<LongComparatorAnonymousInnerClass>(
      shared_from_this(), numHits, getField(),
      any_cast<optional<int64_t>>(missingValue));
}

ToParentBlockJoinSortField::LongComparatorAnonymousInnerClass::
    LongComparatorAnonymousInnerClass(
        shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const wstring &getField,
        optional<int64_t>(optional<int64_t>) missingValue)
    : org::apache::lucene::search::FieldComparator::LongComparator(
          numHits, getField, (optional<int64_t>)missingValue)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
ToParentBlockJoinSortField::LongComparatorAnonymousInnerClass::
    getNumericDocValues(shared_ptr<LeafReaderContext> context,
                        const wstring &field) 
{
  shared_ptr<SortedNumericDocValues> sortedNumeric =
      DocValues::getSortedNumeric(context->reader(), field);
  constexpr BlockJoinSelector::Type type = outerInstance->order
                                               ? BlockJoinSelector::Type::MAX
                                               : BlockJoinSelector::Type::MIN;
  shared_ptr<BitSet> *const parents =
      outerInstance->parentFilter->getBitSet(context);
  shared_ptr<BitSet> *const children =
      outerInstance->childFilter->getBitSet(context);
  if (children == nullptr) {
    return DocValues::emptyNumeric();
  }
  return BlockJoinSelector::wrap(sortedNumeric, type, parents, children);
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
// getFloatComparator(int numHits)
shared_ptr < FieldComparator <
    ? >> ToParentBlockJoinSortField::getFloatComparator(int numHits)
{
  return make_shared<FloatComparatorAnonymousInnerClass>(
      shared_from_this(), numHits, getField(),
      any_cast<optional<float>>(missingValue));
}

ToParentBlockJoinSortField::FloatComparatorAnonymousInnerClass::
    FloatComparatorAnonymousInnerClass(
        shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const wstring &getField, optional<float>(optional<float>) missingValue)
    : org::apache::lucene::search::FieldComparator::FloatComparator(
          numHits, getField, (optional<float>)missingValue)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
ToParentBlockJoinSortField::FloatComparatorAnonymousInnerClass::
    getNumericDocValues(shared_ptr<LeafReaderContext> context,
                        const wstring &field) 
{
  shared_ptr<SortedNumericDocValues> sortedNumeric =
      DocValues::getSortedNumeric(context->reader(), field);
  constexpr BlockJoinSelector::Type type = outerInstance->order
                                               ? BlockJoinSelector::Type::MAX
                                               : BlockJoinSelector::Type::MIN;
  shared_ptr<BitSet> *const parents =
      outerInstance->parentFilter->getBitSet(context);
  shared_ptr<BitSet> *const children =
      outerInstance->childFilter->getBitSet(context);
  if (children == nullptr) {
    return DocValues::emptyNumeric();
  }
  return make_shared<FilterNumericDocValuesAnonymousInnerClass>(
      shared_from_this(),
      BlockJoinSelector::wrap(sortedNumeric, type, parents, children));
}

ToParentBlockJoinSortField::FloatComparatorAnonymousInnerClass::
    FilterNumericDocValuesAnonymousInnerClass::
        FilterNumericDocValuesAnonymousInnerClass(
            shared_ptr<FloatComparatorAnonymousInnerClass> outerInstance,
            shared_ptr<SortedDocValues> wrap)
    : org::apache::lucene::index::FilterNumericDocValues(wrap)
{
  this->outerInstance = outerInstance;
}

int64_t ToParentBlockJoinSortField::FloatComparatorAnonymousInnerClass::
    FilterNumericDocValuesAnonymousInnerClass::longValue() 
{
  // undo the numericutils sortability
  return NumericUtils::sortableFloatBits(
      static_cast<int>(outerInstance->outerInstance.super.longValue()));
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
// getDoubleComparator(int numHits)
shared_ptr < FieldComparator <
    ? >> ToParentBlockJoinSortField::getDoubleComparator(int numHits)
{
  return make_shared<DoubleComparatorAnonymousInnerClass>(
      shared_from_this(), numHits, getField(),
      any_cast<optional<double>>(missingValue));
}

ToParentBlockJoinSortField::DoubleComparatorAnonymousInnerClass::
    DoubleComparatorAnonymousInnerClass(
        shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const wstring &getField,
        optional<double>(optional<double>) missingValue)
    : org::apache::lucene::search::FieldComparator::DoubleComparator(
          numHits, getField, (optional<double>)missingValue)
{
  this->outerInstance = outerInstance;
}

shared_ptr<NumericDocValues>
ToParentBlockJoinSortField::DoubleComparatorAnonymousInnerClass::
    getNumericDocValues(shared_ptr<LeafReaderContext> context,
                        const wstring &field) 
{
  shared_ptr<SortedNumericDocValues> sortedNumeric =
      DocValues::getSortedNumeric(context->reader(), field);
  constexpr BlockJoinSelector::Type type = outerInstance->order
                                               ? BlockJoinSelector::Type::MAX
                                               : BlockJoinSelector::Type::MIN;
  shared_ptr<BitSet> *const parents =
      outerInstance->parentFilter->getBitSet(context);
  shared_ptr<BitSet> *const children =
      outerInstance->childFilter->getBitSet(context);
  if (children == nullptr) {
    return DocValues::emptyNumeric();
  }
  return make_shared<FilterNumericDocValuesAnonymousInnerClass2>(
      shared_from_this(),
      BlockJoinSelector::wrap(sortedNumeric, type, parents, children));
}

ToParentBlockJoinSortField::DoubleComparatorAnonymousInnerClass::
    FilterNumericDocValuesAnonymousInnerClass2::
        FilterNumericDocValuesAnonymousInnerClass2(
            shared_ptr<DoubleComparatorAnonymousInnerClass> outerInstance,
            shared_ptr<SortedDocValues> wrap)
    : org::apache::lucene::index::FilterNumericDocValues(wrap)
{
  this->outerInstance = outerInstance;
}

int64_t ToParentBlockJoinSortField::DoubleComparatorAnonymousInnerClass::
    FilterNumericDocValuesAnonymousInnerClass2::longValue() 
{
  // undo the numericutils sortability
  return NumericUtils::sortableDoubleBits(
      outerInstance->outerInstance.super.longValue());
}

int ToParentBlockJoinSortField::hashCode()
{
  constexpr int prime = 31;
  int result = SortField::hashCode();
  result =
      prime * result + ((childFilter == nullptr) ? 0 : childFilter->hashCode());
  result = prime * result + (order ? 1231 : 1237);
  result = prime * result +
           ((parentFilter == nullptr) ? 0 : parentFilter->hashCode());
  return result;
}

bool ToParentBlockJoinSortField::equals(any obj)
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
  shared_ptr<ToParentBlockJoinSortField> other =
      any_cast<std::shared_ptr<ToParentBlockJoinSortField>>(obj);
  if (childFilter == nullptr) {
    if (other->childFilter != nullptr) {
      return false;
    }
  } else if (!childFilter->equals(other->childFilter)) {
    return false;
  }
  if (order != other->order) {
    return false;
  }
  if (parentFilter == nullptr) {
    if (other->parentFilter != nullptr) {
      return false;
    }
  } else if (!parentFilter->equals(other->parentFilter)) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::search::join