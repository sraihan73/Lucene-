#pragma once
#include "stringhelper.h"
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/geo/DistancePredicate.h"

#include  "core/src/java/org/apache/lucene/geo/PolygonPredicate.h"
#include  "core/src/java/org/apache/lucene/geo/Polygon.h"
#include  "core/src/java/org/apache/lucene/geo/Polygon2D.h"
#include  "core/src/java/org/apache/lucene/geo/Grid.h"
#include  "core/src/java/org/apache/lucene/geo/Rectangle.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"

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
namespace org::apache::lucene::geo
{

using Relation = org::apache::lucene::index::PointValues::Relation;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MAX_LAT_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MAX_LON_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MIN_LON_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MIN_LAT_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.checkLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.checkLongitude;

/**
 * reusable geopoint encoding methods
 *
 * @lucene.experimental
 */
class GeoEncodingUtils final
    : public std::enable_shared_from_this<GeoEncodingUtils>
{
  GET_CLASS_NAME(GeoEncodingUtils)
  /** number of bits used for quantizing latitude and longitude values */
public:
  static constexpr short BITS = 32;

private:
  static const double LAT_SCALE;
  static const double LAT_DECODE;
  static const double LON_SCALE;
  static const double LON_DECODE;

  // No instance:
  GeoEncodingUtils();

  /**
   * Quantizes double (64 bit) latitude into 32 bits (rounding down: in the
   * direction of -90)
   * @param latitude latitude value: must be within standard +/-90 coordinate
   * bounds.
   * @return encoded value as a 32-bit {@code int}
   * @throws IllegalArgumentException if latitude is out of bounds
   */
public:
  static int encodeLatitude(double latitude);

  /**
   * Quantizes double (64 bit) latitude into 32 bits (rounding up: in the
   * direction of +90)
   * @param latitude latitude value: must be within standard +/-90 coordinate
   * bounds.
   * @return encoded value as a 32-bit {@code int}
   * @throws IllegalArgumentException if latitude is out of bounds
   */
  static int encodeLatitudeCeil(double latitude);

  /**
   * Quantizes double (64 bit) longitude into 32 bits (rounding down: in the
   * direction of -180)
   * @param longitude longitude value: must be within standard +/-180 coordinate
   * bounds.
   * @return encoded value as a 32-bit {@code int}
   * @throws IllegalArgumentException if longitude is out of bounds
   */
  static int encodeLongitude(double longitude);

  /**
   * Quantizes double (64 bit) longitude into 32 bits (rounding up: in the
   * direction of +180)
   * @param longitude longitude value: must be within standard +/-180 coordinate
   * bounds.
   * @return encoded value as a 32-bit {@code int}
   * @throws IllegalArgumentException if longitude is out of bounds
   */
  static int encodeLongitudeCeil(double longitude);

  /**
   * Turns quantized value from {@link #encodeLatitude} back into a double.
   * @param encoded encoded value: 32-bit quantized value.
   * @return decoded latitude value.
   */
  static double decodeLatitude(int encoded);

  /**
   * Turns quantized value from byte array back into a double.
   * @param src byte array containing 4 bytes to decode at {@code offset}
   * @param offset offset into {@code src} to decode from.
   * @return decoded latitude value.
   */
  static double decodeLatitude(std::deque<char> &src, int offset);

  /**
   * Turns quantized value from {@link #encodeLongitude} back into a double.
   * @param encoded encoded value: 32-bit quantized value.
   * @return decoded longitude value.
   */
  static double decodeLongitude(int encoded);

  /**
   * Turns quantized value from byte array back into a double.
   * @param src byte array containing 4 bytes to decode at {@code offset}
   * @param offset offset into {@code src} to decode from.
   * @return decoded longitude value.
   */
  static double decodeLongitude(std::deque<char> &src, int offset);

  /** Create a predicate that checks whether points are within a distance of a
   * given point. It works by computing the bounding box around the circle that
   * is defined by the given points/distance and splitting it into between 1024
   * and 4096 smaller boxes (4096*0.75^2=2304 on average). Then for each sub
   * box, it computes the relation between this box and the distance query.
   * Finally at search time, it first computes the sub box that the point
   * belongs to, most of the time, no distance computation will need to be
   * performed since all points from the sub box will either be in or out of the
   * circle.
   *  @lucene.internal */
  static std::shared_ptr<DistancePredicate>
  createDistancePredicate(double lat, double lon, double radiusMeters);

  /** Create a predicate that checks whether points are within a polygon.
   *  It works the same way as {@link #createDistancePredicate}.
   *  @lucene.internal */
  static std::shared_ptr<PolygonPredicate>
  createPolygonPredicate(std::deque<std::shared_ptr<Polygon>> &polygons,
                         std::shared_ptr<Polygon2D> tree);

private:
  static std::shared_ptr<Grid>
  createSubBoxes(std::shared_ptr<Rectangle> boundingBox,
                 std::function<Relation(Rectangle *)> &boxToRelation);

  /** Compute the minimum shift value so that
   * {@code (b>>>shift)-(a>>>shift)} is less that {@code ARITY}. */
  static int computeShift(int64_t a, int64_t b);

private:
  class Grid : public std::enable_shared_from_this<Grid>
  {
    GET_CLASS_NAME(Grid)
  public:
    static constexpr int ARITY = 64;

    const int latShift, lonShift;
    const int latBase, lonBase;
    const int maxLatDelta, maxLonDelta;
    std::deque<char> const relations;

  private:
    Grid(int latShift, int lonShift, int latBase, int lonBase, int maxLatDelta,
         int maxLonDelta, std::deque<char> &relations);
  };

  /** A predicate that checks whether a given point is within a distance of
   * another point. */
public:
  class DistancePredicate : public Grid
  {
    GET_CLASS_NAME(DistancePredicate)

  private:
    const double lat, lon;
    const double distanceKey;

    DistancePredicate(int latShift, int lonShift, int latBase, int lonBase,
                      int maxLatDelta, int maxLonDelta,
                      std::deque<char> &relations, double lat, double lon,
                      double distanceKey);

    /** Check whether the given point is within a distance of another point.
     *  NOTE: this operates directly on the encoded representation of points. */
  public:
    virtual bool test(int lat, int lon);

  protected:
    std::shared_ptr<DistancePredicate> shared_from_this()
    {
      return std::static_pointer_cast<DistancePredicate>(
          Grid::shared_from_this());
    }
  };

  /** A predicate that checks whether a given point is within a polygon. */
public:
  class PolygonPredicate : public Grid
  {
    GET_CLASS_NAME(PolygonPredicate)

  private:
    const std::shared_ptr<Polygon2D> tree;

    PolygonPredicate(int latShift, int lonShift, int latBase, int lonBase,
                     int maxLatDelta, int maxLonDelta,
                     std::deque<char> &relations,
                     std::shared_ptr<Polygon2D> tree);

    /** Check whether the given point is within the considered polygon.
     *  NOTE: this operates directly on the encoded representation of points. */
  public:
    virtual bool test(int lat, int lon);

  protected:
    std::shared_ptr<PolygonPredicate> shared_from_this()
    {
      return std::static_pointer_cast<PolygonPredicate>(
          Grid::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/geo/
