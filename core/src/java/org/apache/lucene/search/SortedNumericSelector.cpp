using namespace std;

#include "SortedNumericSelector.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using FilterNumericDocValues =
    org::apache::lucene::index::FilterNumericDocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using NumericUtils = org::apache::lucene::util::NumericUtils;

shared_ptr<NumericDocValues>
SortedNumericSelector::wrap(shared_ptr<SortedNumericDocValues> sortedNumeric,
                            Type selector, SortField::Type numericType)
{
  if (numericType != SortField::Type::INT &&
      numericType != SortField::Type::LONG &&
      numericType != SortField::Type::FLOAT &&
      numericType != SortField::Type::DOUBLE) {
    throw invalid_argument(L"numericType must be a numeric type");
  }
  shared_ptr<NumericDocValues> *const view;
  shared_ptr<NumericDocValues> singleton =
      DocValues::unwrapSingleton(sortedNumeric);
  if (singleton != nullptr) {
    // it's actually single-valued in practice, but indexed as multi-valued,
    // so just sort on the underlying single-valued dv directly.
    // regardless of selector type, this optimization is safe!
    view = singleton;
  } else {
    switch (selector) {
    case org::apache::lucene::search::SortedNumericSelector::Type::MIN:
      view = make_shared<MinValue>(sortedNumeric);
      break;
    case org::apache::lucene::search::SortedNumericSelector::Type::MAX:
      view = make_shared<MaxValue>(sortedNumeric);
      break;
    default:
      throw make_shared<AssertionError>();
    }
  }
  // undo the numericutils sortability
  switch (numericType) {
  case org::apache::lucene::search::SortField::Type::FLOAT:
    return make_shared<FilterNumericDocValuesAnonymousInnerClass>();
  case org::apache::lucene::search::SortField::Type::DOUBLE:
    return make_shared<FilterNumericDocValuesAnonymousInnerClass2>();
  default:
    return view;
  }
}

SortedNumericSelector::FilterNumericDocValuesAnonymousInnerClass::
    FilterNumericDocValuesAnonymousInnerClass()
    : org::apache::lucene::index::FilterNumericDocValues(view)
{
}

int64_t SortedNumericSelector::FilterNumericDocValuesAnonymousInnerClass::
    longValue() 
{
  return NumericUtils::sortableFloatBits(static_cast<int>(in_::longValue()));
}

SortedNumericSelector::FilterNumericDocValuesAnonymousInnerClass2::
    FilterNumericDocValuesAnonymousInnerClass2()
    : org::apache::lucene::index::FilterNumericDocValues(view)
{
}

int64_t SortedNumericSelector::FilterNumericDocValuesAnonymousInnerClass2::
    longValue() 
{
  return NumericUtils::sortableDoubleBits(in_::longValue());
}

SortedNumericSelector::MinValue::MinValue(
    shared_ptr<SortedNumericDocValues> in_)
    : in_(in_)
{
}

int SortedNumericSelector::MinValue::docID() { return in_->docID(); }

int SortedNumericSelector::MinValue::nextDoc() 
{
  int docID = in_->nextDoc();
  if (docID != NO_MORE_DOCS) {
    value = in_->nextValue();
  }
  return docID;
}

int SortedNumericSelector::MinValue::advance(int target) 
{
  int docID = in_->advance(target);
  if (docID != NO_MORE_DOCS) {
    value = in_->nextValue();
  }
  return docID;
}

bool SortedNumericSelector::MinValue::advanceExact(int target) throw(
    IOException)
{
  if (in_->advanceExact(target)) {
    value = in_->nextValue();
    return true;
  }
  return false;
}

int64_t SortedNumericSelector::MinValue::cost() { return in_->cost(); }

int64_t SortedNumericSelector::MinValue::longValue() { return value; }

SortedNumericSelector::MaxValue::MaxValue(
    shared_ptr<SortedNumericDocValues> in_)
    : in_(in_)
{
}

int SortedNumericSelector::MaxValue::docID() { return in_->docID(); }

void SortedNumericSelector::MaxValue::setValue() 
{
  int count = in_->docValueCount();
  for (int i = 0; i < count; i++) {
    value = in_->nextValue();
  }
}

int SortedNumericSelector::MaxValue::nextDoc() 
{
  int docID = in_->nextDoc();
  if (docID != NO_MORE_DOCS) {
    setValue();
  }
  return docID;
}

int SortedNumericSelector::MaxValue::advance(int target) 
{
  int docID = in_->advance(target);
  if (docID != NO_MORE_DOCS) {
    setValue();
  }
  return docID;
}

bool SortedNumericSelector::MaxValue::advanceExact(int target) throw(
    IOException)
{
  if (in_->advanceExact(target)) {
    setValue();
    return true;
  }
  return false;
}

int64_t SortedNumericSelector::MaxValue::cost() { return in_->cost(); }

int64_t SortedNumericSelector::MaxValue::longValue() { return value; }
} // namespace org::apache::lucene::search