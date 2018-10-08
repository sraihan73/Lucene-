#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/search/SortField.h"
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

using FieldInfo = org::apache::lucene::index::FieldInfo;
using Query = org::apache::lucene::search::Query;
using SortField = org::apache::lucene::search::SortField;

/**
 * An per-document location field.
 * <p>
 * Sorting by distance is efficient. Multiple values for the same field in one
 * document is allowed. <p> This field defines static factory methods for common
 * operations: <ul> <li>{@link #newDistanceSort newDistanceSort()} for ordering
 * documents by distance from a specified location.
 * </ul>
 * <p>
 * If you also need query operations, you should add a separate {@link
 * LatLonPoint} instance. If you also need to store the value, you should add a
 * separate {@link StoredField} instance. <p> <b>WARNING</b>: Values are indexed
 * with some loss of precision from the original {@code double} values
 * (4.190951585769653E-8 for the latitude component and 8.381903171539307E-8 for
 * longitude).
 * @see LatLonPoint
 */
class LatLonDocValuesField : public Field
{
  GET_CLASS_NAME(LatLonDocValuesField)

  /**
   * Type for a LatLonDocValuesField
   * <p>
   * Each value stores a 64-bit long where the upper 32 bits are the encoded
   * latitude, and the lower 32 bits are the encoded longitude.
   * @see org.apache.lucene.geo.GeoEncodingUtils#decodeLatitude(int)
   * @see org.apache.lucene.geo.GeoEncodingUtils#decodeLongitude(int)
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
  static LatLonDocValuesField::StaticConstructor staticConstructor;

  /**
   * Creates a new LatLonDocValuesField with the specified latitude and
   * longitude
   * @param name field name
   * @param latitude latitude value: must be within standard +/-90 coordinate
   * bounds.
   * @param longitude longitude value: must be within standard +/-180 coordinate
   * bounds.
   * @throws IllegalArgumentException if the field name is null or latitude or
   * longitude are out of bounds
   */
public:
  LatLonDocValuesField(const std::wstring &name, double latitude,
                       double longitude);

  /**
   * Change the values of this field
   * @param latitude latitude value: must be within standard +/-90 coordinate
   * bounds.
   * @param longitude longitude value: must be within standard +/-180 coordinate
   * bounds.
   * @throws IllegalArgumentException if latitude or longitude are out of bounds
   */
  virtual void setLocationValue(double latitude, double longitude);

  /** helper: checks a fieldinfo and throws exception if its definitely not a
   * LatLonDocValuesField */
  static void checkCompatible(std::shared_ptr<FieldInfo> fieldInfo);

  virtual std::wstring toString();

  /**
   * Creates a SortField for sorting by distance from a location.
   * <p>
   * This sort orders documents by ascending distance from the location. The
   * value returned in {@link FieldDoc} for the hits contains a Double instance
   * with the distance in meters. <p> If a document is missing the field, then
   * by default it is treated as having {@link Double#POSITIVE_INFINITY}
   * distance (missing values sort last). <p> If a document contains multiple
   * values for the field, the <i>closest</i> distance to the location is used.
   *
   * @param field field name. must not be null.
   * @param latitude latitude at the center: must be within standard +/-90
   * coordinate bounds.
   * @param longitude longitude at the center: must be within standard +/-180
   * coordinate bounds.
   * @return SortField ordering documents by distance
   * @throws IllegalArgumentException if {@code field} is null or location has
   * invalid coordinates.
   */
  static std::shared_ptr<SortField>
  newDistanceSort(const std::wstring &field, double latitude, double longitude);

  /**
   * Create a query for matching a bounding box using doc values.
   * This query is usually slow as it does not use an index structure and needs
   * to verify documents one-by-one in order to know whether they match. It is
   * best used wrapped in an {@link IndexOrDocValuesQuery} alongside a
   * {@link LatLonPoint#newBoxQuery}.
   */
  static std::shared_ptr<Query>
  newSlowBoxQuery(const std::wstring &field, double minLatitude,
                  double maxLatitude, double minLongitude, double maxLongitude);

  /**
   * Create a query for matching points within the specified distance of the
   * supplied location. This query is usually slow as it does not use an index
   * structure and needs to verify documents one-by-one in order to know whether
   * they match. It is best used wrapped in an {@link IndexOrDocValuesQuery}
   * alongside a
   * {@link LatLonPoint#newDistanceQuery}.
   * @param field field name. must not be null.
   * @param latitude latitude at the center: must be within standard +/-90
   * coordinate bounds.
   * @param longitude longitude at the center: must be within standard +/-180
   * coordinate bounds.
   * @param radiusMeters maximum distance from the center in meters: must be
   * non-negative and finite.
   * @return query matching points within this distance
   * @throws IllegalArgumentException if {@code field} is null, location has
   * invalid coordinates, or radius is invalid.
   */
  static std::shared_ptr<Query> newSlowDistanceQuery(const std::wstring &field,
                                                     double latitude,
                                                     double longitude,
                                                     double radiusMeters);

protected:
  std::shared_ptr<LatLonDocValuesField> shared_from_this()
  {
    return std::static_pointer_cast<LatLonDocValuesField>(
        Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
