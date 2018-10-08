#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <limits>
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
 * An indexed {@code half-float} field for fast range filters. If you also
 * need to store the value, you should add a separate {@link StoredField}
 * instance. If you need doc values, you can store them in a {@link
 * NumericDocValuesField} and use {@link #halfFloatToSortableShort} and
 * {@link #sortableShortToHalfFloat} for encoding/decoding.
 * <p>
 * The API takes floats, but they will be encoded to half-floats before being
 * indexed. In case the provided floats cannot be represented accurately as a
 * half float, they will be rounded to the closest value that can be
 * represented as a half float. In case of tie, values will be rounded to the
 * value that has a zero as its least significant bit.
 * <p>
 * Finding all documents within an N-dimensional at search time is
 * efficient.  Multiple values for the same field in one document
 * is allowed.
 * <p>
 * This field defines static factory methods for creating common queries:
 * <ul>
 *   <li>{@link #newExactQuery(std::wstring, float)} for matching an exact 1D point.
 *   <li>{@link #newSetQuery(std::wstring, float...)} for matching a set of 1D values.
 *   <li>{@link #newRangeQuery(std::wstring, float, float)} for matching a 1D range.
 *   <li>{@link #newRangeQuery(std::wstring, float[], float[])} for matching
 * points/ranges in n-dimensional space.
 * </ul>
 * @see PointValues
 */
class HalfFloatPoint final : public Field
{
  GET_CLASS_NAME(HalfFloatPoint)

  /** The number of bytes used to represent a half-float value. */
public:
  static constexpr int BYTES = 2;

  /**
   * Return the first half float which is immediately greater than {@code v}.
   * If the argument is {@link Float#NaN} then the return value is
   * {@link Float#NaN}. If the argument is {@link Float#POSITIVE_INFINITY}
   * then the return value is {@link Float#POSITIVE_INFINITY}.
   */
  static float nextUp(float v);

  /**
   * Return the first half float which is immediately smaller than {@code v}.
   * If the argument is {@link Float#NaN} then the return value is
   * {@link Float#NaN}. If the argument is {@link Float#NEGATIVE_INFINITY}
   * then the return value is {@link Float#NEGATIVE_INFINITY}.
   */
  static float nextDown(float v);

  /** Convert a half-float to a short value that maintains ordering. */
  static short halfFloatToSortableShort(float v);

  /** Convert short bits to a half-float value that maintains ordering. */
  static float sortableShortToHalfFloat(short bits);

private:
  static short sortableShortBits(short s);

public:
  static short halfFloatToShortBits(float v);

  // divide by 2^shift and round to the closest int
  // round to even in case of tie
  static int roundShift(int i, int shift);

  static float shortBitsToHalfFloat(short s);

  static void shortToSortableBytes(short value, std::deque<char> &result,
                                   int offset);

  static short sortableBytesToShort(std::deque<char> &encoded, int offset);

private:
  static std::shared_ptr<FieldType> getType(int numDims);

public:
  void setFloatValue(float value) override;

  /** Change the values of this field */
  void setFloatValues(std::deque<float> &point);

  void setBytesValue(std::shared_ptr<BytesRef> bytes) override;

  std::shared_ptr<Number> numericValue() override;

private:
  static std::shared_ptr<BytesRef> pack(std::deque<float> &point);

  /** Creates a new FloatPoint, indexing the
   *  provided N-dimensional float point.
   *
   *  @param name field name
   *  @param point float[] value
   *  @throws IllegalArgumentException if the field name or value is null.
   */
public:
  HalfFloatPoint(const std::wstring &name, std::deque<float> &point);

  virtual std::wstring toString();

  // public helper methods (e.g. for queries)

  /** Encode single float dimension */
  static void encodeDimension(float value, std::deque<char> &dest, int offset);

  /** Decode single float dimension */
  static float decodeDimension(std::deque<char> &value, int offset);

  // static methods for generating queries

  /**
   * Create a query for matching an exact half-float value. It will be rounded
   * to the closest half-float if {@code value} cannot be represented accurately
   * as a half-float.
   * <p>
   * This is for simple one-dimension points, for multidimensional points use
   * {@link #newRangeQuery(std::wstring, float[], float[])} instead.
   *
   * @param field field name. must not be {@code null}.
   * @param value half-float value
   * @throws IllegalArgumentException if {@code field} is null.
   * @return a query matching documents with this exact value
   */
  static std::shared_ptr<Query> newExactQuery(const std::wstring &field,
                                              float value);

  /**
   * Create a range query for half-float values. Bounds will be rounded to the
   * closest half-float if they cannot be represented accurately as a
   * half-float.
   * <p>
   * This is for simple one-dimension ranges, for multidimensional ranges use
   * {@link #newRangeQuery(std::wstring, float[], float[])} instead.
   * <p>
   * You can have half-open ranges (which are in fact &lt;/&le; or &gt;/&ge;
   * queries) by setting {@code lowerValue = Float.NEGATIVE_INFINITY} or {@code
   * upperValue = Float.POSITIVE_INFINITY}. <p> Ranges are inclusive. For
   * exclusive ranges, pass {@code nextUp(lowerValue)} or {@code
   * nextDown(upperValue)}. <p> Range comparisons are consistent with {@link
   * Float#compareTo(Float)}.
   *
   * @param field field name. must not be {@code null}.
   * @param lowerValue lower portion of the range (inclusive).
   * @param upperValue upper portion of the range (inclusive).
   * @throws IllegalArgumentException if {@code field} is null.
   * @return a query matching documents within this range.
   */
  static std::shared_ptr<Query>
  newRangeQuery(const std::wstring &field, float lowerValue, float upperValue);

  /**
   * Create a range query for n-dimensional half-float values. Bounds will be
   * rounded to the closest half-float if they cannot be represented accurately
   * as a half-float.
   * <p>
   * You can have half-open ranges (which are in fact &lt;/&le; or &gt;/&ge;
   * queries) by setting {@code lowerValue[i] = Float.NEGATIVE_INFINITY} or
   * {@code upperValue[i] = Float.POSITIVE_INFINITY}. <p> Ranges are inclusive.
   * For exclusive ranges, pass {@code nextUp(lowerValue[i])} or {@code
   * nextDown(upperValue[i])}. <p> Range comparisons are consistent with {@link
   * Float#compareTo(Float)}.
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
                                              std::deque<float> &lowerValue,
                                              std::deque<float> &upperValue);

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
   * Create a query matching any of the specified 1D values.
   * This is the points equivalent of {@code TermsQuery}.
   * Values will be rounded to the closest half-float if they
   * cannot be represented accurately as a half-float.
   *
   * @param field field name. must not be {@code null}.
   * @param values all values to match
   */
public:
  static std::shared_ptr<Query> newSetQuery(const std::wstring &field,
                                            std::deque<float> &values);

private:
  class StreamAnonymousInnerClass : public PointInSetQuery::Stream
  {
    GET_CLASS_NAME(StreamAnonymousInnerClass)
  private:
    std::deque<float> sortedValues;
    std::shared_ptr<BytesRef> encoded;

  public:
    StreamAnonymousInnerClass(std::deque<float> &sortedValues,
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
              std::shared_ptr<std::deque<float>> values);

protected:
  std::shared_ptr<HalfFloatPoint> shared_from_this()
  {
    return std::static_pointer_cast<HalfFloatPoint>(Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
