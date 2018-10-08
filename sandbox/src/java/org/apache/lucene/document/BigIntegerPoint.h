#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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
 * An indexed 128-bit {@code int64_t} field.
 * <p>
 * Finding all documents within an N-dimensional shape or range at search time
 * is efficient.  Multiple values for the same field in one document is allowed.
 * <p>
 * This field defines static factory methods for creating common queries:
 * <ul>
 *   <li>{@link #newExactQuery(std::wstring, int64_t)} for matching an exact 1D
 * point. <li>{@link #newSetQuery(std::wstring, int64_t...)} for matching a set of
 * 1D values. <li>{@link #newRangeQuery(std::wstring, int64_t, int64_t)} for
 * matching a 1D range. <li>{@link #newRangeQuery(std::wstring, int64_t[],
 * int64_t[])} for matching points/ranges in n-dimensional space.
 * </ul>
 * @see PointValues
 */
class BigIntegerPoint : public Field
{
  GET_CLASS_NAME(BigIntegerPoint)

  /** The number of bytes per dimension: 128 bits. */
public:
  static constexpr int BYTES = 16;

  /** A constant holding the minimum value a BigIntegerPoint can have,
   * -2<sup>127</sup>. */
  static const std::shared_ptr<int64_t> MIN_VALUE;

  /** A constant holding the maximum value a BigIntegerPoint can have,
   * 2<sup>127</sup>-1. */
  static const std::shared_ptr<int64_t> MAX_VALUE;

private:
  static std::shared_ptr<FieldType> getType(int numDims);

  /** Change the values of this field */
public:
  virtual void setBigIntegerValues(std::deque<int64_t> &point);

  void setBytesValue(std::shared_ptr<BytesRef> bytes) override;

  std::shared_ptr<Number> numericValue() override;

private:
  static std::shared_ptr<BytesRef> pack(std::deque<int64_t> &point);

  /** Creates a new BigIntegerPoint, indexing the
   *  provided N-dimensional big integer point.
   *
   *  @param name field name
   *  @param point int64_t[] value
   *  @throws IllegalArgumentException if the field name or value is null.
   */
public:
  BigIntegerPoint(const std::wstring &name, std::deque<int64_t> &point);

  virtual std::wstring toString();

  // public helper methods (e.g. for queries)

  /** Encode single int64_t dimension */
  static void encodeDimension(std::shared_ptr<int64_t> value,
                              std::deque<char> &dest, int offset);

  /** Decode single int64_t dimension */
  static std::shared_ptr<int64_t> decodeDimension(std::deque<char> &value,
                                                     int offset);

  // static methods for generating queries

  /**
   * Create a query for matching an exact big integer value.
   * <p>
   * This is for simple one-dimension points, for multidimensional points use
   * {@link #newRangeQuery(std::wstring, int64_t[], int64_t[])} instead.
   *
   * @param field field name. must not be {@code null}.
   * @param value exact value. must not be {@code null}.
   * @throws IllegalArgumentException if {@code field} is null or {@code value}
   * is null.
   * @return a query matching documents with this exact value
   */
  static std::shared_ptr<Query>
  newExactQuery(const std::wstring &field, std::shared_ptr<int64_t> value);

  /**
   * Create a range query for big integer values.
   * <p>
   * This is for simple one-dimension ranges, for multidimensional ranges use
   * {@link #newRangeQuery(std::wstring, int64_t[], int64_t[])} instead.
   * <p>
   * You can have half-open ranges (which are in fact &lt;/&le; or &gt;/&ge;
   * queries) by setting {@code lowerValue = BigIntegerPoint.MIN_VALUE} or
   * {@code upperValue = BigIntegerPoint.MAX_VALUE}. <p> Ranges are inclusive.
   * For exclusive ranges, pass {@code lowerValue.add(int64_t.ONE)} or {@code
   * upperValue.subtract(int64_t.ONE)}
   *
   * @param field field name. must not be {@code null}.
   * @param lowerValue lower portion of the range (inclusive). must not be
   * {@code null}.
   * @param upperValue upper portion of the range (inclusive). must not be
   * {@code null}.
   * @throws IllegalArgumentException if {@code field} is null, {@code
   * lowerValue} is null, or {@code upperValue} is null.
   * @return a query matching documents within this range.
   */
  static std::shared_ptr<Query>
  newRangeQuery(const std::wstring &field,
                std::shared_ptr<int64_t> lowerValue,
                std::shared_ptr<int64_t> upperValue);

  /**
   * Create a range query for n-dimensional big integer values.
   * <p>
   * You can have half-open ranges (which are in fact &lt;/&le; or &gt;/&ge;
   * queries) by setting {@code lowerValue[i] = BigIntegerPoint.MIN_VALUE} or
   * {@code upperValue[i] = BigIntegerPoint.MAX_VALUE}. <p> Ranges are
   * inclusive. For exclusive ranges, pass {@code
   * lowerValue[i].add(int64_t.ONE)} or {@code
   * upperValue[i].subtract(int64_t.ONE)}
   *
   * @param field field name. must not be {@code null}.
   * @param lowerValue lower portion of the range (inclusive). must not be
   * {@code null}.
   * @param upperValue upper portion of the range (inclusive). must not be
   * {@code null}.
   * @throws IllegalArgumentException if {@code field} is null, if {@code
   * lowerValue} is null, if {@code upperValue} is null, or if {@code
   * lowerValue.length != upperValue.length}
   * @return a query matching documents within this range.
   */
  static std::shared_ptr<Query>
  newRangeQuery(const std::wstring &field,
                std::deque<std::shared_ptr<int64_t>> &lowerValue,
                std::deque<std::shared_ptr<int64_t>> &upperValue);

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
                                            std::deque<int64_t> &values);

private:
  class StreamAnonymousInnerClass : public PointInSetQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<int64_t>> sortedValues;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass(
        std::deque<std::shared_ptr<int64_t>> &sortedValues,
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
  std::shared_ptr<BigIntegerPoint> shared_from_this()
  {
    return std::static_pointer_cast<BigIntegerPoint>(Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
