#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/SortField.h"

#include  "core/src/java/org/apache/lucene/search/SortedNumericSelector.h"
namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
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
namespace org::apache::lucene::search
{

/**
 * SortField for {@link SortedNumericDocValues}.
 * <p>
 * A SortedNumericDocValues contains multiple values for a field, so sorting
 * with this technique "selects" a value as the representative sort value for
 * the document. <p> By default, the minimum value in the deque is selected as
 * the sort value, but this can be customized. <p> Like sorting by string, this
 * also supports sorting missing values as first or last, via {@link
 * #setMissingValue(Object)}.
 * @see SortedNumericSelector
 */
class SortedNumericSortField : public SortField
{
  GET_CLASS_NAME(SortedNumericSortField)

private:
  const SortedNumericSelector::Type selector;
  const SortField::Type type;

  /**
   * Creates a sort, by the minimum value in the set
   * for the document.
   * @param field Name of field to sort by.  Must not be null.
   * @param type Type of values
   */
public:
  SortedNumericSortField(const std::wstring &field, SortField::Type type);

  /**
   * Creates a sort, possibly in reverse, by the minimum value in the set
   * for the document.
   * @param field Name of field to sort by.  Must not be null.
   * @param type Type of values
   * @param reverse True if natural order should be reversed.
   */
  SortedNumericSortField(const std::wstring &field, SortField::Type type,
                         bool reverse);

  /**
   * Creates a sort, possibly in reverse, specifying how the sort value from
   * the document's set is selected.
   * @param field Name of field to sort by.  Must not be null.
   * @param type Type of values
   * @param reverse True if natural order should be reversed.
   * @param selector custom selector type for choosing the sort value from the
   * set.
   */
  SortedNumericSortField(const std::wstring &field, SortField::Type type,
                         bool reverse, SortedNumericSelector::Type selector);

  /** Returns the numeric type in use for this sort */
  virtual SortField::Type getNumericType();

  /** Returns the selector in use for this sort */
  virtual SortedNumericSelector::Type getSelector();

  virtual int hashCode();

  bool equals(std::any obj) override;

  virtual std::wstring toString();

  void setMissingValue(std::any missingValue) override;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: @Override public FieldComparator<?> getComparator(int
  // numHits, int sortPos)
  std::shared_ptr < FieldComparator <
      ? >> getComparator(int numHits, int sortPos) override;

private:
  class IntComparatorAnonymousInnerClass : public FieldComparator::IntComparator
  {
    GET_CLASS_NAME(IntComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<SortedNumericSortField> outerInstance;

  public:
    IntComparatorAnonymousInnerClass(
        std::shared_ptr<SortedNumericSortField> outerInstance, int numHits,
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
          FieldComparator.IntComparator::shared_from_this());
    }
  };

private:
  class FloatComparatorAnonymousInnerClass
      : public FieldComparator::FloatComparator
  {
    GET_CLASS_NAME(FloatComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<SortedNumericSortField> outerInstance;

  public:
    FloatComparatorAnonymousInnerClass(
        std::shared_ptr<SortedNumericSortField> outerInstance, int numHits,
        const std::wstring &getField,
        std::optional<float>(std::optional<float>) missingValue);

  protected:
    std::shared_ptr<NumericDocValues>
    getNumericDocValues(std::shared_ptr<LeafReaderContext> context,
                        const std::wstring &field)  override;

  protected:
    std::shared_ptr<FloatComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FloatComparatorAnonymousInnerClass>(
          FieldComparator.FloatComparator::shared_from_this());
    }
  };

private:
  class LongComparatorAnonymousInnerClass
      : public FieldComparator::LongComparator
  {
    GET_CLASS_NAME(LongComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<SortedNumericSortField> outerInstance;

  public:
    LongComparatorAnonymousInnerClass(
        std::shared_ptr<SortedNumericSortField> outerInstance, int numHits,
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
          FieldComparator.LongComparator::shared_from_this());
    }
  };

private:
  class DoubleComparatorAnonymousInnerClass
      : public FieldComparator::DoubleComparator
  {
    GET_CLASS_NAME(DoubleComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<SortedNumericSortField> outerInstance;

  public:
    DoubleComparatorAnonymousInnerClass(
        std::shared_ptr<SortedNumericSortField> outerInstance, int numHits,
        const std::wstring &getField,
        std::optional<double>(std::optional<double>) missingValue);

  protected:
    std::shared_ptr<NumericDocValues>
    getNumericDocValues(std::shared_ptr<LeafReaderContext> context,
                        const std::wstring &field)  override;

  protected:
    std::shared_ptr<DoubleComparatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleComparatorAnonymousInnerClass>(
          FieldComparator.DoubleComparator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SortedNumericSortField> shared_from_this()
  {
    return std::static_pointer_cast<SortedNumericSortField>(
        SortField::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
