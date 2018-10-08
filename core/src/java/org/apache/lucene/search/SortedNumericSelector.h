#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class NumericDocValues;
}

namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::search
{
class SortField;
}

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

using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;

/**
 * Selects a value from the document's deque to use as the representative value
 * <p>
 * This provides a NumericDocValues view over the SortedNumeric, for use with
 * sorting, expressions, function queries, etc.
 */
class SortedNumericSelector
    : public std::enable_shared_from_this<SortedNumericSelector>
{
  GET_CLASS_NAME(SortedNumericSelector)

  /**
   * Type of selection to perform.
   */
public:
  enum class Type {
    GET_CLASS_NAME(Type)
    /**
     * Selects the minimum value in the set
     */
    MIN,
    /**
     * Selects the maximum value in the set
     */
    MAX,
    // TODO: we could do MEDIAN in constant time (at most 2 lookups)
  };

  /**
   * Wraps a multi-valued SortedNumericDocValues as a single-valued view, using
   * the specified selector and numericType.
   */
public:
  static std::shared_ptr<NumericDocValues>
  wrap(std::shared_ptr<SortedNumericDocValues> sortedNumeric, Type selector,
       SortField::Type numericType);

private:
  class FilterNumericDocValuesAnonymousInnerClass
      : public FilterNumericDocValues
  {
    GET_CLASS_NAME(FilterNumericDocValuesAnonymousInnerClass)
  public:
    FilterNumericDocValuesAnonymousInnerClass();

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

private:
  class FilterNumericDocValuesAnonymousInnerClass2
      : public FilterNumericDocValues
  {
    GET_CLASS_NAME(FilterNumericDocValuesAnonymousInnerClass2)
  public:
    FilterNumericDocValuesAnonymousInnerClass2();

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

  /** Wraps a SortedNumericDocValues and returns the first value (min) */
public:
  class MinValue : public NumericDocValues
  {
    GET_CLASS_NAME(MinValue)
  public:
    const std::shared_ptr<SortedNumericDocValues> in_;

  private:
    int64_t value = 0;

  public:
    MinValue(std::shared_ptr<SortedNumericDocValues> in_);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int64_t longValue() override;

  protected:
    std::shared_ptr<MinValue> shared_from_this()
    {
      return std::static_pointer_cast<MinValue>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };

  /** Wraps a SortedNumericDocValues and returns the last value (max) */
public:
  class MaxValue : public NumericDocValues
  {
    GET_CLASS_NAME(MaxValue)
  public:
    const std::shared_ptr<SortedNumericDocValues> in_;

  private:
    int64_t value = 0;

  public:
    MaxValue(std::shared_ptr<SortedNumericDocValues> in_);

    int docID() override;

  private:
    void setValue() ;

  public:
    int nextDoc()  override;

    int advance(int target)  override;

    bool advanceExact(int target)  override;

    int64_t cost() override;

    int64_t longValue() override;

  protected:
    std::shared_ptr<MaxValue> shared_from_this()
    {
      return std::static_pointer_cast<MaxValue>(
          org.apache.lucene.index.NumericDocValues::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::search
