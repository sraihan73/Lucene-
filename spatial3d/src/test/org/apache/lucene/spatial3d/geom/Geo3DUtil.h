#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::geo
{
class Polygon;
}

namespace org::apache::lucene::spatial3d::geom
{
class GeoPolygon;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoPath;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoCircle;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoBBox;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoPolygonFactory;
}
namespace org::apache::lucene::spatial3d::geom
{
class PolygonDescription;
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
namespace org::apache::lucene::spatial3d::geom
{

using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using GeoPolygonFactory =
    org::apache::lucene::spatial3d::geom::GeoPolygonFactory;
using GeoPath = org::apache::lucene::spatial3d::geom::GeoPath;
using GeoPolygon = org::apache::lucene::spatial3d::geom::GeoPolygon;
using GeoCircle = org::apache::lucene::spatial3d::geom::GeoCircle;
using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;

using Polygon = org::apache::lucene::geo::Polygon;

class Geo3DUtil : public std::enable_shared_from_this<Geo3DUtil>
{
  GET_CLASS_NAME(Geo3DUtil)

  /** How many radians are in one earth surface meter */
public:
  static const double RADIANS_PER_METER;
  /** How many radians are in one degree */
  static const double RADIANS_PER_DEGREE;
  /** How many degrees in a radian */
  static const double DEGREES_PER_RADIAN;

private:
  static const double MAX_VALUE;
  static constexpr int BITS = 32;
  static const double MUL;

public:
  static const double DECODE;
  static const int MIN_ENCODED_VALUE = encodeValue(-MAX_VALUE);
  static const int MAX_ENCODED_VALUE = encodeValue(MAX_VALUE);

  static int encodeValue(double x);

  static double decodeValue(int x);

  /** Returns smallest double that would encode to int x. */
  // NOTE: keep this package private!!
  static double decodeValueFloor(int x);

  /** Returns a double value >= x such that if you multiply that value by an
   * int, and then
   *  divide it by that int again, you get precisely the same value back */
private:
  static double getNextSafeDouble(double x);

  /** Returns largest double that would encode to int x. */
  // NOTE: keep this package private!!
public:
  static double decodeValueCeil(int x);

  /** Converts degress to radians */
  static double fromDegrees(double const degrees);

  /** Converts radians to degrees */
  static double toDegrees(double const radians);

  /** Converts earth-surface meters to radians */
  static double fromMeters(double const meters);

  /**
   * Convert a set of Polygon objects into a GeoPolygon.
   * @param polygons are the Polygon objects.
   * @return the GeoPolygon.
   */
  static std::shared_ptr<GeoPolygon>
  fromPolygon(std::deque<Polygon> &polygons);

  /**
   * Convert a Polygon object to a large GeoPolygon.
   * @param polygons is the deque of polygons to convert.
   * @return the large GeoPolygon.
   */
  static std::shared_ptr<GeoPolygon>
  fromLargePolygon(std::deque<Polygon> &polygons);

  /**
   * Convert input parameters to a path.
   * @param pathLatitudes latitude values for points of the path: must be within
   * standard +/-90 coordinate bounds.
   * @param pathLongitudes longitude values for points of the path: must be
   * within standard +/-180 coordinate bounds.
   * @param pathWidthMeters width of the path in meters.
   * @return the path.
   */
  static std::shared_ptr<GeoPath> fromPath(std::deque<double> &pathLatitudes,
                                           std::deque<double> &pathLongitudes,
                                           double const pathWidthMeters);

  /**
   * Convert input parameters to a circle.
   * @param latitude latitude at the center: must be within standard +/-90
   * coordinate bounds.
   * @param longitude longitude at the center: must be within standard +/-180
   * coordinate bounds.
   * @param radiusMeters maximum distance from the center in meters: must be
   * non-negative and finite.
   * @return the circle.
   */
  static std::shared_ptr<GeoCircle> fromDistance(double const latitude,
                                                 double const longitude,
                                                 double const radiusMeters);

  /**
   * Convert input parameters to a box.
   * @param minLatitude latitude lower bound: must be within standard +/-90
   * coordinate bounds.
   * @param maxLatitude latitude upper bound: must be within standard +/-90
   * coordinate bounds.
   * @param minLongitude longitude lower bound: must be within standard +/-180
   * coordinate bounds.
   * @param maxLongitude longitude upper bound: must be within standard +/-180
   * coordinate bounds.
   * @return the box.
   */
  static std::shared_ptr<GeoBBox> fromBox(double const minLatitude,
                                          double const maxLatitude,
                                          double const minLongitude,
                                          double const maxLongitude);

  /**
   * Convert a Polygon object into a GeoPolygon.
   * This method uses
   * @param polygon is the Polygon object.
   * @return the GeoPolygon.
   */
private:
  static std::shared_ptr<GeoPolygon>
  fromPolygon(std::shared_ptr<Polygon> polygon);

  /**
   * Convert a deque of polygons to a deque of polygon descriptions.
   * @param polygons is the deque of polygons to convert.
   * @return the deque of polygon descriptions.
   */
  static std::deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>>
  convertToDescription(std::deque<Polygon> &polygons);
};

} // namespace org::apache::lucene::spatial3d::geom
