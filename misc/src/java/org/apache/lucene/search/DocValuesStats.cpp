using namespace std;

#include "DocValuesStats.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;

DocValuesStats<T>::LongDocValuesStats::LongDocValuesStats(const wstring &field)
    : NumericDocValuesStats<long>(field, numeric_limits<long>::max(),
                                  numeric_limits<long>::min())
{
}

void DocValuesStats<T>::LongDocValuesStats::doAccumulate(int count) throw(
    IOException)
{
  int64_t val = ndv->longValue();
  if (val > outerInstance->max_) {
    outerInstance->max_ = val;
  }
  if (val < outerInstance->min_) {
    outerInstance->min_ = val;
  }
  sum_ += val;
  double oldMean = mean_;
  mean_ += (val - mean_) / count;
  variance_ += (val - mean_) * (val - oldMean);
}

optional<int64_t> DocValuesStats<T>::LongDocValuesStats::sum()
{
  return sum_;
}

DocValuesStats<T>::DoubleDocValuesStats::DoubleDocValuesStats(
    const wstring &field)
    : NumericDocValuesStats<double>(field, numeric_limits<double>::max(),
                                    Double::MIN_VALUE)
{
}

void DocValuesStats<T>::DoubleDocValuesStats::doAccumulate(int count) throw(
    IOException)
{
  double val = Double::longBitsToDouble(ndv->longValue());
  if (Double::compare(val, outerInstance->max_) > 0) {
    outerInstance->max_ = val;
  }
  if (Double::compare(val, outerInstance->min_) < 0) {
    outerInstance->min_ = val;
  }
  sum_ += val;
  double oldMean = mean_;
  mean_ += (val - mean_) / count;
  variance_ += (val - mean_) * (val - oldMean);
}

optional<double> DocValuesStats<T>::DoubleDocValuesStats::sum() { return sum_; }

DocValuesStats<T>::SortedLongDocValuesStats::SortedLongDocValuesStats(
    const wstring &field)
    : SortedNumericDocValuesStats<long>(field, numeric_limits<long>::max(),
                                        numeric_limits<long>::min())
{
}

void DocValuesStats<T>::SortedLongDocValuesStats::doAccumulate(int count) throw(
    IOException)
{
  int numValues = sndv->docValueCount();
  while (numValues-- > 0) {
    int64_t val = sndv->nextValue();
    if (val > outerInstance->max_) {
      outerInstance->max_ = val;
    }
    if (val < outerInstance->min_) {
      outerInstance->min_ = val;
    }
    sum_ += val;
    double oldMean = mean_;
    // for correct "running average computation", increase valuesCount with each
    // value, rather than once before the loop stats.
    ++valuesCount_;
    mean_ += (val - mean_) / valuesCount_;
    variance_ += (val - mean_) * (val - oldMean);
  }
}

optional<int64_t> DocValuesStats<T>::SortedLongDocValuesStats::sum()
{
  return sum_;
}

DocValuesStats<T>::SortedDoubleDocValuesStats::SortedDoubleDocValuesStats(
    const wstring &field)
    : SortedNumericDocValuesStats<double>(field, numeric_limits<double>::max(),
                                          Double::MIN_VALUE)
{
}

void DocValuesStats<T>::SortedDoubleDocValuesStats::doAccumulate(
    int count) 
{
  int numValues = sndv->docValueCount();
  while (numValues-- > 0) {
    double val = Double::longBitsToDouble(sndv->nextValue());
    if (Double::compare(val, outerInstance->max_) > 0) {
      outerInstance->max_ = val;
    }
    if (Double::compare(val, outerInstance->min_) < 0) {
      outerInstance->min_ = val;
    }
    sum_ += val;
    double oldMean = mean_;
    // for correct "running average computation", increase valuesCount with each
    // value, rather than once before the loop stats.
    ++valuesCount_;
    mean_ += (val - mean_) / valuesCount_;
    variance_ += (val - mean_) * (val - oldMean);
  }
}

optional<double> DocValuesStats<T>::SortedDoubleDocValuesStats::sum()
{
  return sum_;
}

DocValuesStats<T>::SortedDocValuesStats::SortedDocValuesStats(
    const wstring &field)
    : DocValuesStats<org::apache::lucene::util::BytesRef>(field, nullptr,
                                                          nullptr)
{
}

bool DocValuesStats<T>::SortedDocValuesStats::init(
    shared_ptr<LeafReaderContext> context) 
{
  sdv = context->reader()->getSortedDocValues(field_);
  return sdv != nullptr;
}

bool DocValuesStats<T>::SortedDocValuesStats::hasValue(int doc) throw(
    IOException)
{
  return sdv->advanceExact(doc);
}

void DocValuesStats<T>::SortedDocValuesStats::doAccumulate(int count) throw(
    IOException)
{
  shared_ptr<BytesRef> val = sdv->binaryValue();
  if (max_ == nullptr || val->compareTo(max_) > 0) {
    max_ = copyFrom(val, max_);
  }
  if (min_ == nullptr || val->compareTo(min_) < 0) {
    min_ = copyFrom(val, min_);
  }
}

DocValuesStats<T>::SortedSetDocValuesStats::SortedSetDocValuesStats(
    const wstring &field)
    : DocValuesStats<org::apache::lucene::util::BytesRef>(field, nullptr,
                                                          nullptr)
{
}

bool DocValuesStats<T>::SortedSetDocValuesStats::init(
    shared_ptr<LeafReaderContext> context) 
{
  ssdv = context->reader()->getSortedSetDocValues(field_);
  return ssdv != nullptr;
}

bool DocValuesStats<T>::SortedSetDocValuesStats::hasValue(int doc) throw(
    IOException)
{
  return ssdv->advanceExact(doc);
}

void DocValuesStats<T>::SortedSetDocValuesStats::doAccumulate(int count) throw(
    IOException)
{
  int64_t ord;
  while ((ord = ssdv->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
    shared_ptr<BytesRef> val = ssdv->lookupOrd(ord);
    if (max_ == nullptr || val->compareTo(max_) > 0) {
      max_ = copyFrom(val, max_);
    }
    if (min_ == nullptr || val->compareTo(min_) < 0) {
      min_ = copyFrom(val, min_);
    }
  }
}
} // namespace org::apache::lucene::search