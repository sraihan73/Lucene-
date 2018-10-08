#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Holds statistics for a DocValues field. */
template <typename T>
class DocValuesStats : public std::enable_shared_from_this<DocValuesStats>
{
  GET_CLASS_NAME(DocValuesStats)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int missing_ = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int count_ = 0;

protected:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::wstring field_;

  // C++ NOTE: Fields cannot have the same name as methods:
  T min_;
  // C++ NOTE: Fields cannot have the same name as methods:
  T max_;

  DocValuesStats(const std::wstring &field, T initialMin, T initialMax)
      : field(field)
  {
    this->min_ = initialMin;
    this->max_ = initialMax;
  }

  /**
   * Called after {@link #accumulate(int)} was processed and verified that the
   * document has a value for the field. Implementations should update the
   * statistics based on the value of the current document.
   *
   * @param count
   *          the updated number of documents with value for this field.
   */
  virtual void doAccumulate(int count) = 0;

  /**
   * Initializes this object with the given reader context. Returns whether
   * stats can be computed for this segment (i.e. it does have the requested
   * DocValues field).
   */
  virtual bool init(std::shared_ptr<LeafReaderContext> context) = 0;

  /** Returns whether the given document has a value for the requested DocValues
   * field. */
  virtual bool hasValue(int doc) = 0;

public:
  void accumulate(int doc) 
  {
    if (hasValue(doc)) {
      ++count_;
      doAccumulate(count_);
    } else {
      ++missing_;
    }
  }

  void addMissing() { ++missing_; }

  /** The field for which these stats were computed. */
  std::wstring field() { return field_; }

  /** The number of documents which have a value of the field. */
  int count() { return count_; }

  /** The number of documents which do not have a value of the field. */
  int missing() { return missing_; }

  /** The minimum value of the field. Undefined when {@link #count()} is zero.
   */
  T min() { return min_; }

  /** The maximum value of the field. Undefined when {@link #count()} is zero.
   */
  T max() { return max_; }

  /** Holds statistics for a numeric DocValues field. */
public:
  template <typename T>
  class NumericDocValuesStats : public DocValuesStats<T>
  {
    GET_CLASS_NAME(NumericDocValuesStats)
    static_assert(std::is_base_of<Number, T>::value,
                  L"T must inherit from Number");

  protected:
    // C++ NOTE: Fields cannot have the same name as methods:
    double mean_ = 0.0;
    // C++ NOTE: Fields cannot have the same name as methods:
    double variance_ = 0.0;

    std::shared_ptr<NumericDocValues> ndv;

    NumericDocValuesStats(const std::wstring &field, T initialMin, T initialMax)
        : DocValuesStats<T>(field, initialMin, initialMax)
    {
    }

    bool init(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override final
    {
      ndv = context->reader()->getNumericDocValues(outerInstance->field_);
      return ndv != nullptr;
    }

    bool hasValue(int doc)  override final
    {
      return ndv->advanceExact(doc);
    }

    /** The mean of all values of the field. */
  public:
    double mean() { return mean_; }

    /** Returns the variance of all values of the field. */
    double variance()
    {
      int count = outerInstance->this->count();
      return count > 0 ? variance_ / count : 0;
    }

    /** Returns the stdev of all values of the field. */
    double stdev() { return std::sqrt(variance()); }

    /** Returns the sum of values of the field. Note that if the values are
     * large, the {@code sum} might overflow. */
    virtual T sum() = 0;

  protected:
    std::shared_ptr<NumericDocValuesStats> shared_from_this()
    {
      return std::static_pointer_cast<NumericDocValuesStats>(
          DocValuesStats<T>::shared_from_this());
    }
  };

  /** Holds DocValues statistics for a numeric field storing {@code long}
   * values. */
public:
  class LongDocValuesStats final : public NumericDocValuesStats<int64_t>
  {
    GET_CLASS_NAME(LongDocValuesStats)

    // To avoid boxing 'long' to 'Long' while the sum is computed, declare it as
    // private variable.
  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t sum_ = 0;

  public:
    LongDocValuesStats(const std::wstring &field);

  protected:
    void doAccumulate(int count)  override;

  public:
    std::optional<int64_t> sum() override;

  protected:
    std::shared_ptr<LongDocValuesStats> shared_from_this()
    {
      return std::static_pointer_cast<LongDocValuesStats>(
          NumericDocValuesStats<long>::shared_from_this());
    }
  };

  /** Holds DocValues statistics for a numeric field storing {@code double}
   * values. */
public:
  class DoubleDocValuesStats final : public NumericDocValuesStats<double>
  {
    GET_CLASS_NAME(DoubleDocValuesStats)

    // To avoid boxing 'double' to 'Double' while the sum is computed, declare
    // it as private variable.
  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    double sum_ = 0;

  public:
    DoubleDocValuesStats(const std::wstring &field);

  protected:
    void doAccumulate(int count)  override;

  public:
    std::optional<double> sum() override;

  protected:
    std::shared_ptr<DoubleDocValuesStats> shared_from_this()
    {
      return std::static_pointer_cast<DoubleDocValuesStats>(
          NumericDocValuesStats<double>::shared_from_this());
    }
  };

  /** Holds statistics for a sorted-numeric DocValues field. */
public:
  template <typename T>
  class SortedNumericDocValuesStats : public DocValuesStats<T>
  {
    GET_CLASS_NAME(SortedNumericDocValuesStats)
    static_assert(std::is_base_of<Number, T>::value,
                  L"T must inherit from Number");

  protected:
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t valuesCount_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    double mean_ = 0.0;
    // C++ NOTE: Fields cannot have the same name as methods:
    double variance_ = 0.0;

    std::shared_ptr<SortedNumericDocValues> sndv;

    SortedNumericDocValuesStats(const std::wstring &field, T initialMin,
                                T initialMax)
        : DocValuesStats<T>(field, initialMin, initialMax)
    {
    }

    bool init(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override final
    {
      sndv =
          context->reader()->getSortedNumericDocValues(outerInstance->field_);
      return sndv != nullptr;
    }

    bool hasValue(int doc)  override final
    {
      return sndv->advanceExact(doc);
    }

    /** The mean of all values of the field. */
  public:
    double mean() { return mean_; }

    /** Returns the variance of all values of the field. */
    double variance()
    {
      int count = outerInstance->this->count();
      return count > 0 ? variance_ / count : 0;
    }

    /** Returns the stdev of all values of the field. */
    double stdev() { return std::sqrt(variance()); }

    /** Returns the total number of values for this field. */
    int64_t valuesCount() { return valuesCount_; }

    /** Returns the sum of values of the field. Note that if the values are
     * large, the {@code sum} might overflow. */
    virtual T sum() = 0;

  protected:
    std::shared_ptr<SortedNumericDocValuesStats> shared_from_this()
    {
      return std::static_pointer_cast<SortedNumericDocValuesStats>(
          DocValuesStats<T>::shared_from_this());
    }
  };

  /** Holds DocValues statistics for a sorted-numeric field storing {@code long}
   * values. */
public:
  class SortedLongDocValuesStats final
      : public SortedNumericDocValuesStats<int64_t>
  {
    GET_CLASS_NAME(SortedLongDocValuesStats)

    // To avoid boxing 'long' to 'Long' while the sum is computed, declare it as
    // private variable.
  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t sum_ = 0;

  public:
    SortedLongDocValuesStats(const std::wstring &field);

  protected:
    void doAccumulate(int count)  override;

  public:
    std::optional<int64_t> sum() override;

  protected:
    std::shared_ptr<SortedLongDocValuesStats> shared_from_this()
    {
      return std::static_pointer_cast<SortedLongDocValuesStats>(
          SortedNumericDocValuesStats<long>::shared_from_this());
    }
  };

  /** Holds DocValues statistics for a sorted-numeric field storing {@code
   * double} values. */
public:
  class SortedDoubleDocValuesStats final
      : public SortedNumericDocValuesStats<double>
  {
    GET_CLASS_NAME(SortedDoubleDocValuesStats)

    // To avoid boxing 'double' to 'Double' while the sum is computed, declare
    // it as private variable.
  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    double sum_ = 0;

  public:
    SortedDoubleDocValuesStats(const std::wstring &field);

  protected:
    void doAccumulate(int count)  override;

  public:
    std::optional<double> sum() override;

  protected:
    std::shared_ptr<SortedDoubleDocValuesStats> shared_from_this()
    {
      return std::static_pointer_cast<SortedDoubleDocValuesStats>(
          SortedNumericDocValuesStats<double>::shared_from_this());
    }
  };

private:
  static std::shared_ptr<BytesRef> copyFrom(std::shared_ptr<BytesRef> src,
                                            std::shared_ptr<BytesRef> dest)
  {
    if (dest == nullptr) {
      return BytesRef::deepCopyOf(src);
    }

    dest->bytes = ArrayUtil::grow(dest->bytes, src->length);
    System::arraycopy(src->bytes, src->offset, dest->bytes, 0, src->length);
    dest->offset = 0;
    dest->length = src->length;
    return dest;
  }

  /** Holds statistics for a sorted DocValues field. */
public:
  class SortedDocValuesStats : public DocValuesStats<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(SortedDocValuesStats)

  protected:
    std::shared_ptr<SortedDocValues> sdv;

    SortedDocValuesStats(const std::wstring &field);

    bool init(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override final;

    bool hasValue(int doc)  override final;

    void doAccumulate(int count)  override;

  protected:
    std::shared_ptr<SortedDocValuesStats> shared_from_this()
    {
      return std::static_pointer_cast<SortedDocValuesStats>(
          DocValuesStats<org.apache.lucene.util.BytesRef>::shared_from_this());
    }
  };

  /** Holds statistics for a sorted-set DocValues field. */
public:
  class SortedSetDocValuesStats
      : public DocValuesStats<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(SortedSetDocValuesStats)

  protected:
    std::shared_ptr<SortedSetDocValues> ssdv;

    SortedSetDocValuesStats(const std::wstring &field);

    bool init(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override final;

    bool hasValue(int doc)  override final;

    void doAccumulate(int count)  override;

  protected:
    std::shared_ptr<SortedSetDocValuesStats> shared_from_this()
    {
      return std::static_pointer_cast<SortedSetDocValuesStats>(
          DocValuesStats<org.apache.lucene.util.BytesRef>::shared_from_this());
    }
  };
};
} // #include  "core/src/java/org/apache/lucene/search/
