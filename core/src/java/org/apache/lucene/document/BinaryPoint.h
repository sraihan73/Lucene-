#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class IndexableFieldType;
}
namespace org::apache::lucene::search
{
class Query;
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

using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * An indexed binary field for fast range filters.  If you also
 * need to store the value, you should add a separate {@link StoredField}
 * instance. <p> Finding all documents within an N-dimensional shape or range at
 * search time is efficient.  Multiple values for the same field in one document
 * is allowed.
 * <p>
 * This field defines static factory methods for creating common queries:
 * <ul>
 *   <li>{@link #newExactQuery(std::wstring, byte[])} for matching an exact 1D point.
 *   <li>{@link #newSetQuery(std::wstring, byte[][]) newSetQuery(std::wstring, byte[]...)}
 * for matching a set of 1D values. <li>{@link #newRangeQuery(std::wstring, byte[],
 * byte[])} for matching a 1D range. <li>{@link #newRangeQuery(std::wstring, byte[][],
 * byte[][])} for matching points/ranges in n-dimensional space.
 * </ul>
 * @see PointValues
 */
class BinaryPoint final : public Field
{
  GET_CLASS_NAME(BinaryPoint)

private:
  static std::shared_ptr<FieldType>
  getType(std::deque<std::deque<char>> &point);

  static std::shared_ptr<FieldType> getType(int numDims, int bytesPerDim);

  static std::shared_ptr<BytesRef> pack(std::deque<char> &point);

  /** General purpose API: creates a new BinaryPoint, indexing the
   *  provided N-dimensional binary point.
   *
   *  @param name field name
   *  @param point byte[][] value
   *  @throws IllegalArgumentException if the field name or value is null.
   */
public:
  BinaryPoint(const std::wstring &name, std::deque<char> &point);

  /** Expert API */
  BinaryPoint(const std::wstring &name, std::deque<char> &packedPoint,
              std::shared_ptr<IndexableFieldType> type);

  // static methods for generating queries

  /**
   * Create a query for matching an exact binary value.
   * <p>
   * This is for simple one-dimension points, for multidimensional points use
   * {@link #newRangeQuery(std::wstring, byte[][], byte[][])} instead.
   *
   * @param field field name. must not be {@code null}.
   * @param value binary value
   * @throws IllegalArgumentException if {@code field} is null or {@code value}
   * is null
   * @return a query matching documents with this exact value
   */
  static std::shared_ptr<Query> newExactQuery(const std::wstring &field,
                                              std::deque<char> &value);

  /**
   * Create a range query for binary values.
   * <p>
   * This is for simple one-dimension ranges, for multidimensional ranges use
   * {@link #newRangeQuery(std::wstring, byte[][], byte[][])} instead.
   *
   * @param field field name. must not be {@code null}.
   * @param lowerValue lower portion of the range (inclusive). must not be
   * {@code null}
   * @param upperValue upper portion of the range (inclusive). must not be
   * {@code null}
   * @throws IllegalArgumentException if {@code field} is null, if {@code
   * lowerValue} is null, or if {@code upperValue} is null
   * @return a query matching documents within this range.
   */
  static std::shared_ptr<Query> newRangeQuery(const std::wstring &field,
                                              std::deque<char> &lowerValue,
                                              std::deque<char> &upperValue);

  /**
   * Create a range query for n-dimensional binary values.
   *
   * @param field field name. must not be {@code null}.
   * @param lowerValue lower portion of the range (inclusive). must not be null.
   * @param upperValue upper portion of the range (inclusive). must not be null.
   * @throws IllegalArgumentException if {@code field} is null, if {@code
   * lowerValue} is null, if {@code upperValue} is null, or if {@code
   * lowerValue.length != upperValue.length}
   * @return a query matching documents within this range.
   */
  static std::shared_ptr<Query>
  newRangeQuery(const std::wstring &field,
                std::deque<std::deque<char>> &lowerValue,
                std::deque<std::deque<char>> &upperValue);

private:
  class PointRangeQueryAnonymousInnerClass : public PointRangeQuery
  {
    GET_CLASS_NAME(PointRangeQueryAnonymousInnerClass)
  public:
    PointRangeQueryAnonymousInnerClass(const std::wstring &field,
                                       std::deque<char> &bytes,
                                       std::deque<char> &bytes,
                                       std::shared_ptr<UnknownType> length);

  protected:
    std::wstring toString(int dimension, std::deque<char> &value) override;

  protected:
    std::shared_ptr<PointRangeQueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PointRangeQueryAnonymousInnerClass>(
          org.apache.lucene.search.PointRangeQuery::shared_from_this());
    }
  };

  /**
   * Create a query matching any of the specified 1D values.  This is the points
   * equivalent of {@code TermsQuery}.
   *
   * @param field field name. must not be {@code null}.
   * @param values all values to match
   */
public:
  static std::shared_ptr<Query> newSetQuery(const std::wstring &field,
                                            std::deque<char> &values);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::deque<char>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::deque<char> &a, std::deque<char> &b);
  };

private:
  class StreamAnonymousInnerClass : public PointInSetQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass)
  private:
    std::deque<std::deque<char>> sortedValues;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass(std::deque<std::deque<char>> &sortedValues,
                              std::shared_ptr<BytesRef> encoded);

    int upto = 0;

    std::shared_ptr<BytesRef> next() override;

  protected:
    std::shared_ptr<StreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StreamAnonymousInnerClass>(
          org.apache.lucene.search.PointInSetQuery.Stream::shared_from_this());
    }
  };

protected:
  std::shared_ptr<BinaryPoint> shared_from_this()
  {
    return std::static_pointer_cast<BinaryPoint>(Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
