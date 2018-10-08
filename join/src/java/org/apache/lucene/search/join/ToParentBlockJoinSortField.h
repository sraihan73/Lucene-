#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/join/BitSetProducer.h"

namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"

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
namespace org::apache::lucene::search::join
{

using FieldComparator = org::apache::lucene::search::FieldComparator;
using SortField = org::apache::lucene::search::SortField;

/**
 * A special sort field that allows sorting parent docs based on nested / child
 * level fields. Based on the sort order it either takes the document with the
 * lowest or highest field value into account.
 *
 * @lucene.experimental
 */
class ToParentBlockJoinSortField : public SortField
{
  GET_CLASS_NAME(ToParentBlockJoinSortField)

private:
  const bool order;
  const std::shared_ptr<BitSetProducer> parentFilter;
  const std::shared_ptr<BitSetProducer> childFilter;

  /**
   * Create ToParentBlockJoinSortField. The parent document ordering is based on
   * child document ordering (reverse).
   *
   * @param field The sort field on the nested / child level.
   * @param type The sort type on the nested / child level.
   * @param reverse Whether natural order should be reversed on the nested /
   * child level.
   * @param parentFilter Filter that identifies the parent documents.
   * @param childFilter Filter that defines which child documents participates
   * in sorting.
   */
public:
  ToParentBlockJoinSortField(const std::wstring &field, Type type, bool reverse,
                             std::shared_ptr<BitSetProducer> parentFilter,
                             std::shared_ptr<BitSetProducer> childFilter);

  /**
   * Create ToParentBlockJoinSortField.
   *
   * @param field The sort field on the nested / child level.
   * @param type The sort type on the nested / child level.
   * @param reverse Whether natural order should be reversed on the nested /
   * child document level.
   * @param order Whether natural order should be reversed on the parent level.
   * @param parentFilter Filter that identifies the parent documents.
   * @param childFilter Filter that defines which child documents participates
   * in sorting.
   */
  ToParentBlockJoinSortField(const std::wstring &field, Type type, bool reverse,
                             bool order,
                             std::shared_ptr<BitSetProducer> parentFilter,
                             std::shared_ptr<BitSetProducer> childFilter);

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: @Override public org.apache.lucene.search.FieldComparator<?>
  // getComparator(int numHits, int sortPos)
  std::shared_ptr < FieldComparator <
      ? >> getComparator(int numHits, int sortPos) override;

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
  // getStringComparator(int numHits)
  std::shared_ptr < FieldComparator < ? >> getStringComparator(int numHits);

private:
  class TermOrdValComparatorAnonymousInnerClass
      : public FieldComparator::TermOrdValComparator
  {
    GET_CLASS_NAME(TermOrdValComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<ToParentBlockJoinSortField> outerInstance;

  public:
    TermOrdValComparatorAnonymousInnerClass(
        std::shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const std::wstring &getField, bool missingValue);

  protected:
    std::shared_ptr<SortedDocValues>
    getSortedDocValues(std::shared_ptr<LeafReaderContext> context,
                       const std::wstring &field)  override;

  protected:
    std::shared_ptr<TermOrdValComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TermOrdValComparatorAnonymousInnerClass>(
          org.apache.lucene.search.FieldComparator
              .TermOrdValComparator::shared_from_this());
    }
  };

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
  // getIntComparator(int numHits)
  std::shared_ptr < FieldComparator < ? >> getIntComparator(int numHits);

private:
  class IntComparatorAnonymousInnerClass : public FieldComparator::IntComparator
  {
    GET_CLASS_NAME(IntComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<ToParentBlockJoinSortField> outerInstance;

  public:
    IntComparatorAnonymousInnerClass(
        std::shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const std::wstring &getField,
        std::optional<int>(std::optional<int>) missingValue);

  protected:
    std::shared_ptr<NumericDocValues>
    getNumericDocValues(std::shared_ptr<LeafReaderContext> context,
                        const std::wstring &field)  override;

  protected:
    std::shared_ptr<IntComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntComparatorAnonymousInnerClass>(
          org.apache.lucene.search.FieldComparator
              .IntComparator::shared_from_this());
    }
  };

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
  // getLongComparator(int numHits)
  std::shared_ptr < FieldComparator < ? >> getLongComparator(int numHits);

private:
  class LongComparatorAnonymousInnerClass
      : public FieldComparator::LongComparator
  {
    GET_CLASS_NAME(LongComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<ToParentBlockJoinSortField> outerInstance;

  public:
    LongComparatorAnonymousInnerClass(
        std::shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const std::wstring &getField,
        std::optional<int64_t>(std::optional<int64_t>) missingValue);

  protected:
    std::shared_ptr<NumericDocValues>
    getNumericDocValues(std::shared_ptr<LeafReaderContext> context,
                        const std::wstring &field)  override;

  protected:
    std::shared_ptr<LongComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongComparatorAnonymousInnerClass>(
          org.apache.lucene.search.FieldComparator
              .LongComparator::shared_from_this());
    }
  };

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
  // getFloatComparator(int numHits)
  std::shared_ptr < FieldComparator < ? >> getFloatComparator(int numHits);

private:
  class FloatComparatorAnonymousInnerClass
      : public FieldComparator::FloatComparator
  {
    GET_CLASS_NAME(FloatComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<ToParentBlockJoinSortField> outerInstance;

  public:
    FloatComparatorAnonymousInnerClass(
        std::shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const std::wstring &getField,
        std::optional<float>(std::optional<float>) missingValue);

  protected:
    std::shared_ptr<NumericDocValues>
    getNumericDocValues(std::shared_ptr<LeafReaderContext> context,
                        const std::wstring &field)  override;

  private:
    class FilterNumericDocValuesAnonymousInnerClass
        : public FilterNumericDocValues
    {
      GET_CLASS_NAME(FilterNumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<FloatComparatorAnonymousInnerClass> outerInstance;

    public:
      FilterNumericDocValuesAnonymousInnerClass(
          std::shared_ptr<FloatComparatorAnonymousInnerClass> outerInstance,
          std::shared_ptr<SortedDocValues> wrap);

      int64_t longValue()  override;

    protected:
      std::shared_ptr<FilterNumericDocValuesAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            FilterNumericDocValuesAnonymousInnerClass>(
            org.apache.lucene.index.FilterNumericDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FloatComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FloatComparatorAnonymousInnerClass>(
          org.apache.lucene.search.FieldComparator
              .FloatComparator::shared_from_this());
    }
  };

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private org.apache.lucene.search.FieldComparator<?>
  // getDoubleComparator(int numHits)
  std::shared_ptr < FieldComparator < ? >> getDoubleComparator(int numHits);

private:
  class DoubleComparatorAnonymousInnerClass
      : public FieldComparator::DoubleComparator
  {
    GET_CLASS_NAME(DoubleComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<ToParentBlockJoinSortField> outerInstance;

  public:
    DoubleComparatorAnonymousInnerClass(
        std::shared_ptr<ToParentBlockJoinSortField> outerInstance, int numHits,
        const std::wstring &getField,
        std::optional<double>(std::optional<double>) missingValue);

  protected:
    std::shared_ptr<NumericDocValues>
    getNumericDocValues(std::shared_ptr<LeafReaderContext> context,
                        const std::wstring &field)  override;

  private:
    class FilterNumericDocValuesAnonymousInnerClass2
        : public FilterNumericDocValues
    {
      GET_CLASS_NAME(FilterNumericDocValuesAnonymousInnerClass2)
    private:
      std::shared_ptr<DoubleComparatorAnonymousInnerClass> outerInstance;

    public:
      FilterNumericDocValuesAnonymousInnerClass2(
          std::shared_ptr<DoubleComparatorAnonymousInnerClass> outerInstance,
          std::shared_ptr<SortedDocValues> wrap);

      int64_t longValue()  override;

    protected:
      std::shared_ptr<FilterNumericDocValuesAnonymousInnerClass2>
      shared_from_this()
      {
        return std::static_pointer_cast<
            FilterNumericDocValuesAnonymousInnerClass2>(
            org.apache.lucene.index.FilterNumericDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DoubleComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleComparatorAnonymousInnerClass>(
          org.apache.lucene.search.FieldComparator
              .DoubleComparator::shared_from_this());
    }
  };

public:
  virtual int hashCode();

  bool equals(std::any obj) override;

protected:
  std::shared_ptr<ToParentBlockJoinSortField> shared_from_this()
  {
    return std::static_pointer_cast<ToParentBlockJoinSortField>(
        org.apache.lucene.search.SortField::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
