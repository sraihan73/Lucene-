#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "stringbuilder.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::geo
{
class Polygon;
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
//    import static Math.PI;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static Math.max;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static Math.min;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.checkLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.checkLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MAX_LAT_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MIN_LAT_INCL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MAX_LAT_RADIANS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MAX_LON_RADIANS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MIN_LAT_RADIANS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.MIN_LON_RADIANS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.EARTH_MEAN_RADIUS_METERS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoUtils.sloppySin;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.TO_DEGREES;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.asin;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.cos;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.toDegrees;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.SloppyMath.toRadians;

/** Represents a lat/lon rectangle. */
class Rectangle : public std::enable_shared_from_this<Rectangle>
{
  GET_CLASS_NAME(Rectangle)
  /** maximum longitude value (in degrees) */
public:
  const double minLat;
  /** minimum longitude value (in degrees) */
  const double minLon;
  /** maximum latitude value (in degrees) */
  const double maxLat;
  /** minimum latitude value (in degrees) */
  const double maxLon;

  /**
   * Constructs a bounding box by first validating the provided latitude and
   * longitude coordinates
   */
  Rectangle(double minLat, double maxLat, double minLon, double maxLon);

  virtual std::wstring toString();

  /** Returns true if this bounding box crosses the dateline */
  virtual bool crossesDateline();

  /** Compute Bounding Box for a circle using WGS-84 parameters */
  static std::shared_ptr<Rectangle>
  fromPointDistance(double const centerLat, double const centerLon,
                    double const radiusMeters);

  /** maximum error from {@link #axisLat(double, double)}. logic must be
   * prepared to handle this */
  static const double AXISLAT_ERROR;

  /**
   * Calculate the latitude of a circle's intersections with its bbox meridians.
   * <p>
   * <b>NOTE:</b> the returned value will be +/- {@link #AXISLAT_ERROR} of the
   * actual value.
   * @param centerLat The latitude of the circle center
   * @param radiusMeters The radius of the circle in meters
   * @return A latitude
   */
  static double axisLat(double centerLat, double radiusMeters);

  /** Returns the bounding box over an array of polygons */
  static std::shared_ptr<Rectangle>
  fromPolygon(std::deque<std::shared_ptr<Polygon>> &polygons);

  virtual bool equals(std::any o);

  virtual int hashCode();
};

} // namespace org::apache::lucene::geo
