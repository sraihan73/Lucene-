#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
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

using Query = org::apache::lucene::search::Query;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * An indexed {@code double} field for fast range filters.  If you also
 * need to store the value, you should add a separate {@link StoredField}
 * instance. <p> Finding all documents within an N-dimensional shape or range at
 * search time is efficient.  Multiple values for the same field in one document
 * is allowed.
 * <p>
 * This field defines static factory methods for creating common queries:
 * <ul>
 *   <li>{@link #newExactQuery(std::wstring, double)} for matching an exact 1D point.
 *   <li>{@link #newSetQuery(std::wstring, double...)} for matching a set of 1D
 * values. <li>{@link #newRangeQuery(std::wstring, double, double)} for matching a 1D
 * range. <li>{@link #newRangeQuery(std::wstring, double[], double[])} for matching
 * points/ranges in n-dimensional space.
 * </ul>
 * @see PointValues
 */
class DoublePoint final : public Field
{
  GET_CLASS_NAME(DoublePoint)

  /**
   * Return the least double that compares greater than {@code d} consistently
   * with {@link Double#compare}. The only difference with
   * {@link Math#nextUp(double)} is that this method returns {@code +0d} when
   * the argument is {@code -0d}.
   */
public:
  static double nextUp(double d);

  /**
   * Return the greatest double that compares less than {@code d} consistently
   * with {@link Double#compare}. The only difference with
   * {@link Math#nextDown(double)} is that this method returns {@code -0d} when
   * the argument is {@code +0d}.
   */
  static double nextDown(double d);

private:
  static std::shared_ptr<FieldType> getType(int numDims);

public:
  void setDoubleValue(double value) override;

  /** Change the values of this field */
  void setDoubleValues(std::deque<double> &point);

  void setBytesValue(std::shared_ptr<BytesRef> bytes) override;

  std::shared_ptr<Number> numericValue() override;

private:
  static std::shared_ptr<BytesRef> pack(std::deque<double> &point);

  /** Creates a new DoublePoint, indexing the
   *  provided N-dimensional double point.
   *
   *  @param name field name
   *  @param point double[] value
   *  @throws IllegalArgumentException if the field name or value is null.
   */
public:
  DoublePoint(const std::wstring &name, std::deque<double> &point);

  virtual std::wstring toString();

  // public helper methods (e.g. for queries)

  /** Encode single double dimension */
  static void encodeDimension(double value, std::deque<char> &dest,
                              int offset);

  /** Decode single double dimension */
  static double decodeDimension(std::deque<char> &value, int offset);

  // static methods for generating queries

  /**
   * Create a query for matching an exact double value.
   * <p>
   * This is for simple one-dimension points, for multidimensional points use
   * {@link #newRangeQuery(std::wstring, double[], double[])} instead.
   *
   * @param field field name. must not be {@code null}.
   * @param value double value
   * @throws IllegalArgumentException if {@code field} is null.
   * @return a query matching documents with this exact value
   */
  static std::shared_ptr<Query> newExactQuery(const std::wstring &field,
                                              double value);

  /**
   * Create a range query for double values.
   * <p>
   * This is for simple one-dimension ranges, for multidimensional ranges use
   * {@link #newRangeQuery(std::wstring, double[], double[])} instead.
   * <p>
   * You can have half-open ranges (which are in fact &lt;/&le; or &gt;/&ge;
   * queries) by setting {@code lowerValue = Double.NEGATIVE_INFINITY} or {@code
   * upperValue = Double.POSITIVE_INFINITY}. <p> Ranges are inclusive. For
   * exclusive ranges, pass {@link #nextUp(double) nextUp(lowerValue)} or {@link
   * #nextUp(double) nextDown(upperValue)}. <p> Range comparisons are consistent
   * with {@link Double#compareTo(Double)}.
   *
   * @param field field name. must not be {@code null}.
   * @param lowerValue lower portion of the range (inclusive).
   * @param upperValue upper portion of the range (inclusive).
   * @throws IllegalArgumentException if {@code field} is null.
   * @return a query matching documents within this range.
   */
  static std::shared_ptr<Query> newRangeQuery(const std::wstring &field,
                                              double lowerValue,
                                              double upperValue);

  /**
   * Create a range query for n-dimensional double values.
   * <p>
   * You can have half-open ranges (which are in fact &lt;/&le; or &gt;/&ge;
   * queries) by setting {@code lowerValue[i] = Double.NEGATIVE_INFINITY} or
   * {@code upperValue[i] = Double.POSITIVE_INFINITY}. <p> Ranges are inclusive.
   * For exclusive ranges, pass {@code Math#nextUp(lowerValue[i])} or {@code
   * Math.nextDown(upperValue[i])}. <p> Range comparisons are consistent with
   * {@link Double#compareTo(Double)}.
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
  static std::shared_ptr<Query> newRangeQuery(const std::wstring &field,
                                              std::deque<double> &lowerValue,
                                              std::deque<double> &upperValue);

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
                                            std::deque<double> &values);

private:
  class StreamAnonymousInnerClass : public PointInSetQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass)
  private:
    std::deque<double> sortedValues;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass(std::deque<double> &sortedValues,
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

  /**
   * Create a query matching any of the specified 1D values.  This is the points
   * equivalent of {@code TermsQuery}.
   *
   * @param field field name. must not be {@code null}.
   * @param values all values to match
   */
public:
  static std::shared_ptr<Query>
  newSetQuery(const std::wstring &field,
              std::shared_ptr<std::deque<double>> values);

protected:
  std::shared_ptr<DoublePoint> shared_from_this()
  {
    return std::static_pointer_cast<DoublePoint>(Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
