#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
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

using Query = org::apache::lucene::search::Query;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

/**
 * An indexed 2-Dimension Bounding Box field for the Geospatial Lat/Lon
 * Coordinate system <p> This field indexes 2-dimension Latitude, Longitude
 * based Geospatial Bounding Boxes. The bounding boxes are defined as
 * {@code minLat, minLon, maxLat, maxLon} where min/max lat,lon pairs using
 * double floating point precision. <p> Multiple values for the same field in
 * one document is supported.
 *
 * <p>
 * This field defines the following static factory methods for common search
 * operations over double ranges: <ul> <li>{@link #newIntersectsQuery
 * newIntersectsQuery()} matches bounding boxes that intersect the defined
 * search bounding box. <li>{@link #newWithinQuery newWithinQuery()} matches
 * bounding boxes that are within the defined search bounding box. <li>{@link
 * #newContainsQuery newContainsQuery()} matches bounding boxes that contain the
 * defined search bounding box. <li>{@link #newCrossesQuery newCrosses()}
 * matches bounding boxes that cross the defined search bounding box.
 * </ul>
 *
 * <p>
 * The following Field limitations and restrictions apply:
 * <ul>
 *   <li>Dateline wrapping is not supported.
 *   <li>Due to an encoding limitation Eastern and Western Hemisphere Bounding
 * Boxes that share the dateline are not supported.
 * </ul>
 */
class LatLonBoundingBox : public Field
{
  GET_CLASS_NAME(LatLonBoundingBox)
  /** uses same encoding as {@link LatLonPoint} so numBytes is the same */
public:
  static const int BYTES = LatLonPoint::BYTES;

  /**
   * Create a new 2D GeoBoundingBoxField representing a 2 dimensional geospatial
   * bounding box
   *
   * @param name field name. must not be null
   * @param minLat minimum latitude value (in degrees); valid in [-90.0 : 90.0]
   * @param minLon minimum longitude value (in degrees); valid in [-180.0 :
   * 180.0]
   * @param maxLat maximum latitude value (in degrees); valid in [minLat : 90.0]
   * @param maxLon maximum longitude value (in degrees); valid in [minLon :
   * 180.0]
   */
  LatLonBoundingBox(const std::wstring &name, double const minLat,
                    double const minLon, double const maxLat,
                    double const maxLon);

  /** set the field type */
  static std::shared_ptr<FieldType> getType(int geoDimensions);

  /**
   * Changes the values of the field
   * @param minLat minimum latitude value (in degrees); valid in [-90.0 : 90.0]
   * @param minLon minimum longitude value (in degrees); valid in [-180.0 :
   * 180.0]
   * @param maxLat maximum latitude value (in degrees); valid in [minLat : 90.0]
   * @param maxLon maximum longitude value (in degrees); valid in [minLon :
   * 180.0]
   * @throws IllegalArgumentException if {@code min} or {@code max} is invalid
   */
  virtual void setRangeValues(double minLat, double minLon, double maxLat,
                              double maxLon);

  /** validate the two-dimension arguments */
  static void checkArgs(double const minLat, double const minLon,
                        double const maxLat, double const maxLon);

  /**
   * Create a new 2d query that finds all indexed 2d GeoBoundingBoxField values
   * that intersect the defined 3d bounding ranges
   * @param field field name. must not be null
   * @param minLat minimum latitude value (in degrees); valid in [-90.0 : 90.0]
   * @param minLon minimum longitude value (in degrees); valid in [-180.0 :
   * 180.0]
   * @param maxLat maximum latitude value (in degrees); valid in [minLat : 90.0]
   * @param maxLon maximum longitude value (in degrees); valid in [minLon :
   * 180.0]
   * @return query for matching intersecting 2d bounding boxes
   */
  static std::shared_ptr<Query> newIntersectsQuery(const std::wstring &field,
                                                   double const minLat,
                                                   double const minLon,
                                                   double const maxLat,
                                                   double const maxLon);

  /**
   * Create a new 2d query that finds all indexed 2d GeoBoundingBoxField values
   * that are within the defined 2d bounding box
   * @param field field name. must not be null
   * @param minLat minimum latitude value (in degrees); valid in [-90.0 : 90.0]
   * @param minLon minimum longitude value (in degrees); valid in [-180.0 :
   * 180.0]
   * @param maxLat maximum latitude value (in degrees); valid in [minLat : 90.0]
   * @param maxLon maximum longitude value (in degrees); valid in [minLon :
   * 180.0]
   * @return query for matching 3d bounding boxes that are within the defined
   * bounding box
   */
  static std::shared_ptr<Query>
  newWithinQuery(const std::wstring &field, double const minLat,
                 double const minLon, double const maxLat, double const maxLon);

  /**
   * Create a new 2d query that finds all indexed 2d GeoBoundingBoxField values
   * that contain the defined 2d bounding box
   * @param field field name. must not be null
   * @param minLat minimum latitude value (in degrees); valid in [-90.0 : 90.0]
   * @param minLon minimum longitude value (in degrees); valid in [-180.0 :
   * 180.0]
   * @param maxLat maximum latitude value (in degrees); valid in [minLat : 90.0]
   * @param maxLon maximum longitude value (in degrees); valid in [minLon :
   * 180.0]
   * @return query for matching 2d bounding boxes that contain the defined
   * bounding box
   */
  static std::shared_ptr<Query> newContainsQuery(const std::wstring &field,
                                                 double const minLat,
                                                 double const minLon,
                                                 double const maxLat,
                                                 double const maxLon);

  /**
   * Create a new 2d query that finds all indexed 2d GeoBoundingBoxField values
   * that cross the defined 3d bounding box
   * @param field field name. must not be null
   * @param minLat minimum latitude value (in degrees); valid in [-90.0 : 90.0]
   * @param minLon minimum longitude value (in degrees); valid in [-180.0 :
   * 180.0]
   * @param maxLat maximum latitude value (in degrees); valid in [minLat : 90.0]
   * @param maxLon maximum longitude value (in degrees); valid in [minLon :
   * 180.0]
   * @return query for matching 2d bounding boxes that cross the defined
   * bounding box
   */
  static std::shared_ptr<Query> newCrossesQuery(const std::wstring &field,
                                                double const minLat,
                                                double const minLon,
                                                double const maxLat,
                                                double const maxLon);

  /** helper method to create a two-dimensional geospatial bounding box query */
private:
  static std::shared_ptr<Query>
  newRangeQuery(const std::wstring &field, double const minLat,
                double const minLon, double const maxLat, double const maxLon,
                RangeFieldQuery::QueryType const queryType);

private:
  class RangeFieldQueryAnonymousInnerClass : public RangeFieldQuery
  {
    GET_CLASS_NAME(RangeFieldQueryAnonymousInnerClass)
  public:
    RangeFieldQueryAnonymousInnerClass(
        const std::wstring &field, std::deque<char> &encode,
        org::apache::lucene::document::RangeFieldQuery::QueryType queryType);

  protected:
    std::wstring toString(std::deque<char> &ranges, int dimension) override;

  protected:
    std::shared_ptr<RangeFieldQueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RangeFieldQueryAnonymousInnerClass>(
          RangeFieldQuery::shared_from_this());
    }
  };

  /** encodes a two-dimensional geo bounding box into a byte array */
public:
  static std::deque<char> encode(double minLat, double minLon, double maxLat,
                                  double maxLon);

  /** encodes a two-dimensional geopoint (lat, lon) into a byte array */
  static void encode(double lat, double lon, std::deque<char> &result,
                     int offset);

  virtual std::wstring toString();

private:
  static std::wstring toString(std::deque<char> &ranges, int dimension);

protected:
  std::shared_ptr<LatLonBoundingBox> shared_from_this()
  {
    return std::static_pointer_cast<LatLonBoundingBox>(
        Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
