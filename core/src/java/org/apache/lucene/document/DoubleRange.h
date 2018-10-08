#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/document/RangeFieldQuery.h"

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

using QueryType = org::apache::lucene::document::RangeFieldQuery::QueryType;
using Query = org::apache::lucene::search::Query;

/**
 * An indexed Double Range field.
 * <p>
 * This field indexes dimensional ranges defined as min/max pairs. It supports
 * up to a maximum of 4 dimensions (indexed as 8 numeric values). With 1
 * dimension representing a single double range, 2 dimensions representing a
 * bounding box, 3 dimensions a bounding cube, and 4 dimensions a tesseract. <p>
 * Multiple values for the same field in one document is supported, and open
 * ended ranges can be defined using
 * {@code Double.NEGATIVE_INFINITY} and {@code Double.POSITIVE_INFINITY}.
 *
 * <p>
 * This field defines the following static factory methods for common search
 * operations over double ranges: <ul> <li>{@link #newIntersectsQuery
 * newIntersectsQuery()} matches ranges that intersect the defined search range.
 *   <li>{@link #newWithinQuery newWithinQuery()} matches ranges that are within
 * the defined search range. <li>{@link #newContainsQuery newContainsQuery()}
 * matches ranges that contain the defined search range.
 * </ul>
 */
class DoubleRange : public Field
{
  GET_CLASS_NAME(DoubleRange)
  /** stores double values so number of bytes is 8 */
public:
  static const int BYTES = Double::BYTES;

  /**
   * Create a new DoubleRange type, from min/max parallel arrays
   *
   * @param name field name. must not be null.
   * @param min range min values; each entry is the min value for the dimension
   * @param max range max values; each entry is the max value for the dimension
   */
  DoubleRange(const std::wstring &name, std::deque<double> &min,
              std::deque<double> &max);

  /** set the field type */
private:
  static std::shared_ptr<FieldType> getType(int dimensions);

  /**
   * Changes the values of the field.
   * @param min array of min values. (accepts {@code Double.NEGATIVE_INFINITY})
   * @param max array of max values. (accepts {@code Double.POSITIVE_INFINITY})
   * @throws IllegalArgumentException if {@code min} or {@code max} is invalid
   */
public:
  virtual void setRangeValues(std::deque<double> &min,
                              std::deque<double> &max);

  /** validate the arguments */
private:
  static void checkArgs(std::deque<double> &min, std::deque<double> &max);

  /**
   * Encodes the min, max ranges into a byte array
   */
  static std::deque<char> encode(std::deque<double> &min,
                                  std::deque<double> &max);

  /**
   * encode the ranges into a sortable byte array ({@code Double.NaN} not
   * allowed) <p> example for 4 dimensions (8 bytes per dimension value): minD1
   * ... minD4 | maxD1 ... maxD4
   */
public:
  static void verifyAndEncode(std::deque<double> &min,
                              std::deque<double> &max,
                              std::deque<char> &bytes);

  /** encode the given value into the byte array at the defined offset */
private:
  static void encode(double val, std::deque<char> &bytes, int offset);

  /**
   * Get the min value for the given dimension
   * @param dimension the dimension, always positive
   * @return the decoded min value
   */
public:
  virtual double getMin(int dimension);

  /**
   * Get the max value for the given dimension
   * @param dimension the dimension, always positive
   * @return the decoded max value
   */
  virtual double getMax(int dimension);

  /** decodes the min value (for the defined dimension) from the encoded input
   * byte array */
  static double decodeMin(std::deque<char> &b, int dimension);

  /** decodes the max value (for the defined dimension) from the encoded input
   * byte array */
  static double decodeMax(std::deque<char> &b, int dimension);

  /**
   * Create a query for matching indexed ranges that intersect the defined
   * range.
   * @param field field name. must not be null.
   * @param min array of min values. (accepts {@code Double.NEGATIVE_INFINITY})
   * @param max array of max values. (accepts {@code Double.POSITIVE_INFINITY})
   * @return query for matching intersecting ranges (overlap, within, or
   * contains)
   * @throws IllegalArgumentException if {@code field} is null, {@code min} or
   * {@code max} is invalid
   */
  static std::shared_ptr<Query> newIntersectsQuery(const std::wstring &field,
                                                   std::deque<double> &min,
                                                   std::deque<double> &max);

  /**
   * Create a query for matching indexed ranges that contain the defined range.
   * @param field field name. must not be null.
   * @param min array of min values. (accepts {@code Double.MIN_VALUE})
   * @param max array of max values. (accepts {@code Double.MAX_VALUE})
   * @return query for matching ranges that contain the defined range
   * @throws IllegalArgumentException if {@code field} is null, {@code min} or
   * {@code max} is invalid
   */
  static std::shared_ptr<Query> newContainsQuery(const std::wstring &field,
                                                 std::deque<double> &min,
                                                 std::deque<double> &max);

  /**
   * Create a query for matching indexed ranges that are within the defined
   * range.
   * @param field field name. must not be null.
   * @param min array of min values. (accepts {@code Double.MIN_VALUE})
   * @param max array of max values. (accepts {@code Double.MAX_VALUE})
   * @return query for matching ranges within the defined range
   * @throws IllegalArgumentException if {@code field} is null, {@code min} or
   * {@code max} is invalid
   */
  static std::shared_ptr<Query> newWithinQuery(const std::wstring &field,
                                               std::deque<double> &min,
                                               std::deque<double> &max);

  /**
   * Create a query for matching indexed ranges that cross the defined range.
   * A CROSSES is defined as any set of ranges that are not disjoint and not
   * wholly contained by the query. Effectively, its the complement of
   * union(WITHIN, DISJOINT).
   * @param field field name. must not be null.
   * @param min array of min values. (accepts {@code Double.MIN_VALUE})
   * @param max array of max values. (accepts {@code Double.MAX_VALUE})
   * @return query for matching ranges within the defined range
   * @throws IllegalArgumentException if {@code field} is null, {@code min} or
   * {@code max} is invalid
   */
  static std::shared_ptr<Query> newCrossesQuery(const std::wstring &field,
                                                std::deque<double> &min,
                                                std::deque<double> &max);

  /** helper method for creating the desired relational query */
private:
  static std::shared_ptr<Query> newRelationQuery(const std::wstring &field,
                                                 std::deque<double> &min,
                                                 std::deque<double> &max,
                                                 QueryType relation);

private:
  class RangeFieldQueryAnonymousInnerClass : public RangeFieldQuery
  {
    GET_CLASS_NAME(RangeFieldQueryAnonymousInnerClass)
  public:
    RangeFieldQueryAnonymousInnerClass(const std::wstring &field,
                                       std::deque<char> &encode,
                                       std::shared_ptr<UnknownType> length,
                                       QueryType relation);

  protected:
    std::wstring toString(std::deque<char> &ranges, int dimension) override;

  protected:
    std::shared_ptr<RangeFieldQueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RangeFieldQueryAnonymousInnerClass>(
          RangeFieldQuery::shared_from_this());
    }
  };

public:
  virtual std::wstring toString();

  /**
   * Returns the std::wstring representation for the range at the given dimension
   * @param ranges the encoded ranges, never null
   * @param dimension the dimension of interest
   * @return The string representation for the range at the provided dimension
   */
private:
  static std::wstring toString(std::deque<char> &ranges, int dimension);

protected:
  std::shared_ptr<DoubleRange> shared_from_this()
  {
    return std::static_pointer_cast<DoubleRange>(Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
