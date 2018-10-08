#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"

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
namespace org::apache::lucene::document
{

using Query = org::apache::lucene::search::Query;

/**
 * <p>
 * Field that stores a per-document <code>long</code> value for scoring,
 * sorting or value retrieval. Here's an example usage:
 *
 * <pre class="prettyprint">
 *   document.add(new NumericDocValuesField(name, 22L));
 * </pre>
 *
 * <p>
 * If you also need to store the value, you should add a
 * separate {@link StoredField} instance.
 * */

class NumericDocValuesField : public Field
{
  GET_CLASS_NAME(NumericDocValuesField)

  /**
   * Type for numeric DocValues.
   */
public:
  static const std::shared_ptr<FieldType> TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static NumericDocValuesField::StaticConstructor staticConstructor;

  /**
   * Creates a new DocValues field with the specified 64-bit long value
   * @param name field name
   * @param value 64-bit long value
   * @throws IllegalArgumentException if the field name is null
   */
public:
  NumericDocValuesField(const std::wstring &name, int64_t value);

  /**
   * Creates a new DocValues field with the specified 64-bit long value
   * @param name field name
   * @param value 64-bit long value or <code>null</code> if the existing fields
   * value should be removed on update
   * @throws IllegalArgumentException if the field name is null
   */
  NumericDocValuesField(const std::wstring &name,
                        std::optional<int64_t> &value);

  /**
   * Create a range query that matches all documents whose value is between
   * {@code lowerValue} and {@code upperValue} included.
   * <p>
   * You can have half-open ranges (which are in fact &lt;/&le; or &gt;/&ge;
   * queries) by setting {@code lowerValue = Long.MIN_VALUE} or {@code
   * upperValue = Long.MAX_VALUE}. <p> Ranges are inclusive. For exclusive
   * ranges, pass {@code Math.addExact(lowerValue, 1)} or {@code
   * Math.addExact(upperValue, -1)}. <p><b>NOTE</b>: Such queries cannot
   * efficiently advance to the next match, which makes them slow if they are
   * not ANDed with a selective query. As a consequence, they are best used
   * wrapped in an {@link IndexOrDocValuesQuery}, alongside a range query that
   * executes on points, such as
   * {@link LongPoint#newRangeQuery}.
   */
  static std::shared_ptr<Query> newSlowRangeQuery(const std::wstring &field,
                                                  int64_t lowerValue,
                                                  int64_t upperValue);

private:
  class SortedNumericDocValuesRangeQueryAnonymousInnerClass
      : public SortedNumericDocValuesRangeQuery
  {
    GET_CLASS_NAME(SortedNumericDocValuesRangeQueryAnonymousInnerClass)
  private:
    std::wstring field;

  public:
    SortedNumericDocValuesRangeQueryAnonymousInnerClass(
        const std::wstring &field, int64_t lowerValue, int64_t upperValue);

    std::shared_ptr<SortedNumericDocValues>
    getValues(std::shared_ptr<LeafReader> reader,
              const std::wstring &field)  override;

  protected:
    std::shared_ptr<SortedNumericDocValuesRangeQueryAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedNumericDocValuesRangeQueryAnonymousInnerClass>(
          SortedNumericDocValuesRangeQuery::shared_from_this());
    }
  };

  /**
   * Create a query for matching an exact long value.
   * <p><b>NOTE</b>: Such queries cannot efficiently advance to the next match,
   * which makes them slow if they are not ANDed with a selective query. As a
   * consequence, they are best used wrapped in an {@link
   * IndexOrDocValuesQuery}, alongside a range query that executes on points,
   * such as
   * {@link LongPoint#newExactQuery}.
   */
public:
  static std::shared_ptr<Query> newSlowExactQuery(const std::wstring &field,
                                                  int64_t value);

protected:
  std::shared_ptr<NumericDocValuesField> shared_from_this()
  {
    return std::static_pointer_cast<NumericDocValuesField>(
        Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
