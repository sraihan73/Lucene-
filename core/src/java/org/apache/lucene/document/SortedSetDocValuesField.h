#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
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
namespace org::apache::lucene::document
{

using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * <p>
 * Field that stores
 * a set of per-document {@link BytesRef} values, indexed for
 * faceting,grouping,joining.  Here's an example usage:
 *
 * <pre class="prettyprint">
 *   document.add(new SortedSetDocValuesField(name, new BytesRef("hello")));
 *   document.add(new SortedSetDocValuesField(name, new BytesRef("world")));
 * </pre>
 *
 * <p>
 * If you also need to store the value, you should add a
 * separate {@link StoredField} instance.
 *
 * <p>
 * Each value can be at most 32766 bytes long.
 * */

class SortedSetDocValuesField : public Field
{
  GET_CLASS_NAME(SortedSetDocValuesField)

  /**
   * Type for sorted bytes DocValues
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
  static SortedSetDocValuesField::StaticConstructor staticConstructor;

  /**
   * Create a new sorted DocValues field.
   * @param name field name
   * @param bytes binary content
   * @throws IllegalArgumentException if the field name is null
   */
public:
  SortedSetDocValuesField(const std::wstring &name,
                          std::shared_ptr<BytesRef> bytes);

  /**
   * Create a range query that matches all documents whose value is between
   * {@code lowerValue} and {@code upperValue}.
   * <p>This query also works with fields that have indexed
   * {@link SortedDocValuesField}s.
   * <p><b>NOTE</b>: Such queries cannot efficiently advance to the next match,
   * which makes them slow if they are not ANDed with a selective query. As a
   * consequence, they are best used wrapped in an {@link
   * IndexOrDocValuesQuery}, alongside a range query that executes on points,
   * such as
   * {@link BinaryPoint#newRangeQuery}.
   */
  static std::shared_ptr<Query>
  newSlowRangeQuery(const std::wstring &field,
                    std::shared_ptr<BytesRef> lowerValue,
                    std::shared_ptr<BytesRef> upperValue, bool lowerInclusive,
                    bool upperInclusive);

private:
  class SortedSetDocValuesRangeQueryAnonymousInnerClass
      : public SortedSetDocValuesRangeQuery
  {
    GET_CLASS_NAME(SortedSetDocValuesRangeQueryAnonymousInnerClass)
  private:
    std::wstring field;

  public:
    SortedSetDocValuesRangeQueryAnonymousInnerClass(
        const std::wstring &field, std::shared_ptr<BytesRef> lowerValue,
        std::shared_ptr<BytesRef> upperValue, bool lowerInclusive,
        bool upperInclusive);

    std::shared_ptr<SortedSetDocValues>
    getValues(std::shared_ptr<LeafReader> reader,
              const std::wstring &field)  override;

  protected:
    std::shared_ptr<SortedSetDocValuesRangeQueryAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          SortedSetDocValuesRangeQueryAnonymousInnerClass>(
          SortedSetDocValuesRangeQuery::shared_from_this());
    }
  };

  /**
   * Create a query for matching an exact {@link BytesRef} value.
   * <p>This query also works with fields that have indexed
   * {@link SortedDocValuesField}s.
   * <p><b>NOTE</b>: Such queries cannot efficiently advance to the next match,
   * which makes them slow if they are not ANDed with a selective query. As a
   * consequence, they are best used wrapped in an {@link
   * IndexOrDocValuesQuery}, alongside a range query that executes on points,
   * such as
   * {@link BinaryPoint#newExactQuery}.
   */
public:
  static std::shared_ptr<Query>
  newSlowExactQuery(const std::wstring &field, std::shared_ptr<BytesRef> value);

protected:
  std::shared_ptr<SortedSetDocValuesField> shared_from_this()
  {
    return std::static_pointer_cast<SortedSetDocValuesField>(
        Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
