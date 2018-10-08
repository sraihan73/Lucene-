#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::geo
{
class Polygon;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class TopFieldDocs;
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

using Polygon = org::apache::lucene::geo::Polygon;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitudeCeil;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitudeCeil;

/**
 * An indexed location field.
 * <p>
 * Finding all documents within a range at search time is
 * efficient.  Multiple values for the same field in one document
 * is allowed.
 * <p>
 * This field defines static factory methods for common operations:
 * <ul>
 *   <li>{@link #newBoxQuery newBoxQuery()} for matching points within a
 * bounding box. <li>{@link #newDistanceQuery newDistanceQuery()} for matching
 * points within a specified distance. <li>{@link #newPolygonQuery
 * newPolygonQuery()} for matching points within an arbitrary polygon.
 *   <li>{@link #nearest nearest()} for finding the k-nearest neighbors by
 * distance.
 * </ul>
 * <p>
 * If you also need per-document operations such as sort by distance, add a
 * separate {@link LatLonDocValuesField} instance. If you also need to store the
 * value, you should add a separate {@link StoredField} instance. <p>
 * <b>WARNING</b>: Values are indexed with some loss of precision from the
 * original {@code double} values (4.190951585769653E-8 for the latitude
 * component and 8.381903171539307E-8 for longitude).
 * @see PointValues
 * @see LatLonDocValuesField
 */
// TODO ^^^ that is very sandy and hurts the API, usage, and tests tremendously,
// because what the user passes to the field is not actually what gets indexed.
// Float would be 1E-5 error vs 1E-7, but it might be a better tradeoff? then it
// would be completely transparent to the user and lucene would be "lossless".
class LatLonPoint : public Field
{
  GET_CLASS_NAME(LatLonPoint)
  /** LatLonPoint is encoded as integer values so number of bytes is 4 */
public:
  static const int BYTES = Integer::BYTES;
  /**
   * Type for an indexed LatLonPoint
   * <p>
   * Each point stores two dimensions with 4 bytes per dimension.
   */
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
  static LatLonPoint::StaticConstructor staticConstructor;

  /**
   * Change the values of this field
   * @param latitude latitude value: must be within standard +/-90 coordinate
   * bounds.
   * @param longitude longitude value: must be within standard +/-180 coordinate
   * bounds.
   * @throws IllegalArgumentException if latitude or longitude are out of bounds
   */
public:
  virtual void setLocationValue(double latitude, double longitude);

  /**
   * Creates a new LatLonPoint with the specified latitude and longitude
   * @param name field name
   * @param latitude latitude value: must be within standard +/-90 coordinate
   * bounds.
   * @param longitude longitude value: must be within standard +/-180 coordinate
   * bounds.
   * @throws IllegalArgumentException if the field name is null or latitude or
   * longitude are out of bounds
   */
  LatLonPoint(const std::wstring &name, double latitude, double longitude);

  virtual std::wstring toString();

  /** sugar encodes a single point as a byte array */
private:
  static std::deque<char> encode(double latitude, double longitude);

  /** sugar encodes a single point as a byte array, rounding values up */
  static std::deque<char> encodeCeil(double latitude, double longitude);

  /** helper: checks a fieldinfo and throws exception if its definitely not a
   * LatLonPoint */
public:
  static void checkCompatible(std::shared_ptr<FieldInfo> fieldInfo);

  // static methods for generating queries

  /**
   * Create a query for matching a bounding box.
   * <p>
   * The box may cross over the dateline.
   * @param field field name. must not be null.
   * @param minLatitude latitude lower bound: must be within standard +/-90
   * coordinate bounds.
   * @param maxLatitude latitude upper bound: must be within standard +/-90
   * coordinate bounds.
   * @param minLongitude longitude lower bound: must be within standard +/-180
   * coordinate bounds.
   * @param maxLongitude longitude upper bound: must be within standard +/-180
   * coordinate bounds.
   * @return query matching points within this box
   * @throws IllegalArgumentException if {@code field} is null, or the box has
   * invalid coordinates.
   */
  static std::shared_ptr<Query>
  newBoxQuery(const std::wstring &field, double minLatitude, double maxLatitude,
              double minLongitude, double maxLongitude);

private:
  static std::shared_ptr<Query> newBoxInternal(const std::wstring &field,
                                               std::deque<char> &min,
                                               std::deque<char> &max);

private:
  class PointRangeQueryAnonymousInnerClass : public PointRangeQuery
  {
    GET_CLASS_NAME(PointRangeQueryAnonymousInnerClass)
  public:
    PointRangeQueryAnonymousInnerClass(const std::wstring &field,
                                       std::deque<char> &min,
                                       std::deque<char> &max);

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
   * Create a query for matching points within the specified distance of the
   * supplied location.
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
public:
  static std::shared_ptr<Query> newDistanceQuery(const std::wstring &field,
                                                 double latitude,
                                                 double longitude,
                                                 double radiusMeters);

  /**
   * Create a query for matching one or more polygons.
   * @param field field name. must not be null.
   * @param polygons array of polygons. must not be null or empty
   * @return query matching points within this polygon
   * @throws IllegalArgumentException if {@code field} is null, {@code polygons}
   * is null or empty
   * @see Polygon
   */
  static std::shared_ptr<Query> newPolygonQuery(const std::wstring &field,
                                                std::deque<Polygon> &polygons);

  /**
   * Finds the {@code n} nearest indexed points to the provided point, according
   * to Haversine distance. <p> This is functionally equivalent to running
   * {@link MatchAllDocsQuery} with a {@link
   * LatLonDocValuesField#newDistanceSort}, but is far more efficient since it
   * takes advantage of properties the indexed BKD tree.  Currently this only
   * works with {@link Lucene60PointsFormat} (used by the default codec).
   * Multi-valued fields are currently not de-duplicated, so if a document had
   * multiple instances of the specified field that make it into the top n, that
   * document will appear more than once. <p> Documents are ordered by ascending
   * distance from the location. The value returned in {@link FieldDoc} for the
   * hits contains a Double instance with the distance in meters.
   *
   * @param searcher IndexSearcher to find nearest points from.
   * @param field field name. must not be null.
   * @param latitude latitude at the center: must be within standard +/-90
   * coordinate bounds.
   * @param longitude longitude at the center: must be within standard +/-180
   * coordinate bounds.
   * @param n the number of nearest neighbors to retrieve.
   * @return TopFieldDocs containing documents ordered by distance, where the
   * field value for each {@link FieldDoc} is the distance in meters
   * @throws IllegalArgumentException if the underlying PointValues is not a
   * {@code Lucene60PointsReader} (this is a current limitation), or if {@code
   * field} or {@code searcher} is null, or if {@code latitude}, {@code
   * longitude} or {@code n} are out-of-bounds
   * @throws IOException if an IOException occurs while finding the points.
   */
  // TODO: what about multi-valued documents? what happens?
  static std::shared_ptr<TopFieldDocs>
  nearest(std::shared_ptr<IndexSearcher> searcher, const std::wstring &field,
          double latitude, double longitude, int n) ;

protected:
  std::shared_ptr<LatLonPoint> shared_from_this()
  {
    return std::static_pointer_cast<LatLonPoint>(Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
