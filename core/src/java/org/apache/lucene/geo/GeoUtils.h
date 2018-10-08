#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class PointValues;
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
namespace org::apache::lucene::geo
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.TO_RADIANS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.cos;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.haversinMeters;

using PointValues = org::apache::lucene::index::PointValues;
using Relation = org::apache::lucene::index::PointValues::Relation;

/**
 * Basic reusable geo-spatial utility methods
 *
 * @lucene.experimental
 */
class GeoUtils final : public std::enable_shared_from_this<GeoUtils>
{
  GET_CLASS_NAME(GeoUtils)
  /** Minimum longitude value. */
public:
  static constexpr double MIN_LON_INCL = -180.0;

  /** Maximum longitude value. */
  static constexpr double MAX_LON_INCL = 180.0;

  /** Minimum latitude value. */
  static constexpr double MIN_LAT_INCL = -90.0;

  /** Maximum latitude value. */
  static constexpr double MAX_LAT_INCL = 90.0;

  /** min longitude value in radians */
  static const double MIN_LON_RADIANS;
  /** min latitude value in radians */
  static const double MIN_LAT_RADIANS;
  /** max longitude value in radians */
  static const double MAX_LON_RADIANS;
  /** max latitude value in radians */
  static const double MAX_LAT_RADIANS;

  // WGS84 earth-ellipsoid parameters
  /** mean earth axis in meters */
  // see http://earth-info.nga.mil/GandG/publications/tr8350.2/wgs84fin.pdf
  static constexpr double EARTH_MEAN_RADIUS_METERS = 6'371'008.7714;

  // No instance:
private:
  GeoUtils();

  /** validates latitude value is within standard +/-90 coordinate bounds */
public:
  static void checkLatitude(double latitude);

  /** validates longitude value is within standard +/-180 coordinate bounds */
  static void checkLongitude(double longitude);

  // some sloppyish stuff, do we really need this to be done in a sloppy way?
  // unless it is performance sensitive, we should try to remove.
private:
  static const double PIO2;

  /**
   * Returns the trigonometric sine of an angle converted as a cos operation.
   * <p>
   * Note that this is not quite right... e.g. sin(0) != 0
   * <p>
   * Special cases:
   * <ul>
   *  <li>If the argument is {@code NaN} or an infinity, then the result is
   * {@code NaN}.
   * </ul>
   * @param a an angle, in radians.
   * @return the sine of the argument.
   * @see Math#sin(double)
   */
  // TODO: deprecate/remove this? at least its no longer public.
public:
  static double sloppySin(double a);

  /**
   * binary search to find the exact sortKey needed to match the specified
   * radius any sort key lte this is a query match.
   */
  static double distanceQuerySortKey(double radius);

  /**
   * Compute the relation between the provided box and distance query.
   * This only works for boxes that do not cross the dateline.
   */
  static PointValues::Relation relate(double minLat, double maxLat,
                                      double minLon, double maxLon, double lat,
                                      double lon, double distanceSortKey,
                                      double axisLat);

  /** Return whether all points of {@code [minLon,maxLon]} are within 90 degrees
   * of {@code lon}. */
  static bool within90LonDegrees(double lon, double minLon, double maxLon);
};

} // namespace org::apache::lucene::geo
