#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::document
{
class RangeFieldQuery;
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

using QueryType = org::apache::lucene::document::RangeFieldQuery::QueryType;
using Query = org::apache::lucene::search::Query;

/**
 * An indexed InetAddress Range Field
 * <p>
 * This field indexes an {@code InetAddress} range defined as a min/max pairs.
 * It is single dimension only (indexed as two 16 byte paired values). <p>
 * Multiple values are supported.
 *
 * <p>
 * This field defines the following static factory methods for common search
 * operations over Ip Ranges <ul> <li>{@link #newIntersectsQuery
 * newIntersectsQuery()} matches ip ranges that intersect the defined search
 * range. <li>{@link #newWithinQuery newWithinQuery()} matches ip ranges that
 * are within the defined search range. <li>{@link #newContainsQuery
 * newContainsQuery()} matches ip ranges that contain the defined search range.
 *   <li>{@link #newCrossesQuery newCrossesQuery()} matches ip ranges that cross
 * the defined search range
 * </ul>
 */
class InetAddressRange : public Field
{
  GET_CLASS_NAME(InetAddressRange)
  /** The number of bytes per dimension : sync w/ {@code InetAddressPoint} */
public:
  static constexpr int BYTES = InetAddressPoint::BYTES;

private:
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
  static InetAddressRange::StaticConstructor staticConstructor;

  /**
   * Create a new InetAddressRange from min/max value
   * @param name field name. must not be null.
   * @param min range min value; defined as an {@code InetAddress}
   * @param max range max value; defined as an {@code InetAddress}
   */
public:
  InetAddressRange(const std::wstring &name, std::shared_ptr<InetAddress> min,
                   std::shared_ptr<InetAddress> max);

  /**
   * Change (or set) the min/max values of the field.
   * @param min range min value; defined as an {@code InetAddress}
   * @param max range max value; defined as an {@code InetAddress}
   */
  virtual void setRangeValues(std::shared_ptr<InetAddress> min,
                              std::shared_ptr<InetAddress> max);

  /** encode the min/max range into the provided byte array */
private:
  static void encode(std::shared_ptr<InetAddress> min,
                     std::shared_ptr<InetAddress> max,
                     std::deque<char> &bytes);

  /** encode the min/max range and return the byte array */
  static std::deque<char> encode(std::shared_ptr<InetAddress> min,
                                  std::shared_ptr<InetAddress> max);

  /**
   * Create a query for matching indexed ip ranges that {@code INTERSECT} the
   * defined range.
   * @param field field name. must not be null.
   * @param min range min value; provided as an {@code InetAddress}
   * @param max range max value; provided as an {@code InetAddress}
   * @return query for matching intersecting ranges (overlap, within, crosses,
   * or contains)
   * @throws IllegalArgumentException if {@code field} is null, {@code min} or
   * {@code max} is invalid
   */
public:
  static std::shared_ptr<Query>
  newIntersectsQuery(const std::wstring &field,
                     std::shared_ptr<InetAddress> min,
                     std::shared_ptr<InetAddress> max);

  /**
   * Create a query for matching indexed ip ranges that {@code CONTAINS} the
   * defined range.
   * @param field field name. must not be null.
   * @param min range min value; provided as an {@code InetAddress}
   * @param max range max value; provided as an {@code InetAddress}
   * @return query for matching intersecting ranges (overlap, within, crosses,
   * or contains)
   * @throws IllegalArgumentException if {@code field} is null, {@code min} or
   * {@code max} is invalid
   */
  static std::shared_ptr<Query>
  newContainsQuery(const std::wstring &field, std::shared_ptr<InetAddress> min,
                   std::shared_ptr<InetAddress> max);

  /**
   * Create a query for matching indexed ip ranges that are {@code WITHIN} the
   * defined range.
   * @param field field name. must not be null.
   * @param min range min value; provided as an {@code InetAddress}
   * @param max range max value; provided as an {@code InetAddress}
   * @return query for matching intersecting ranges (overlap, within, crosses,
   * or contains)
   * @throws IllegalArgumentException if {@code field} is null, {@code min} or
   * {@code max} is invalid
   */
  static std::shared_ptr<Query>
  newWithinQuery(const std::wstring &field, std::shared_ptr<InetAddress> min,
                 std::shared_ptr<InetAddress> max);

  /**
   * Create a query for matching indexed ip ranges that {@code CROSS} the
   * defined range.
   * @param field field name. must not be null.
   * @param min range min value; provided as an {@code InetAddress}
   * @param max range max value; provided as an {@code InetAddress}
   * @return query for matching intersecting ranges (overlap, within, crosses,
   * or contains)
   * @throws IllegalArgumentException if {@code field} is null, {@code min} or
   * {@code max} is invalid
   */
  static std::shared_ptr<Query>
  newCrossesQuery(const std::wstring &field, std::shared_ptr<InetAddress> min,
                  std::shared_ptr<InetAddress> max);

  /** helper method for creating the desired relational query */
private:
  static std::shared_ptr<Query>
  newRelationQuery(const std::wstring &field, std::shared_ptr<InetAddress> min,
                   std::shared_ptr<InetAddress> max, QueryType relation);

private:
  class RangeFieldQueryAnonymousInnerClass : public RangeFieldQuery
  {
    GET_CLASS_NAME(RangeFieldQueryAnonymousInnerClass)
  public:
    RangeFieldQueryAnonymousInnerClass(const std::wstring &field,
                                       std::deque<char> &encode,
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

  /**
   * Returns the std::wstring representation for the range at the given dimension
   * @param ranges the encoded ranges, never null
   * @param dimension the dimension of interest (not used for this field)
   * @return The string representation for the range at the provided dimension
   */
private:
  static std::wstring toString(std::deque<char> &ranges, int dimension);

protected:
  std::shared_ptr<InetAddressRange> shared_from_this()
  {
    return std::static_pointer_cast<InetAddressRange>(
        Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
