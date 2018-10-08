#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <any>
#include <cmath>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}

namespace org::apache::lucene::spatial3d::geom
{
class Vector;
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

/**
 * Holds mathematical constants associated with the model of a planet.
 * @lucene.experimental
 */
class PlanetModel : public std::enable_shared_from_this<PlanetModel>,
                    public SerializableObject
{
  GET_CLASS_NAME(PlanetModel)

  /** Planet model corresponding to sphere. */
public:
  static const std::shared_ptr<PlanetModel> SPHERE;

  /** Mean radius */
  // see http://earth-info.nga.mil/GandG/publications/tr8350.2/wgs84fin.pdf
  static constexpr double WGS84_MEAN = 6371008.7714;
  /** Polar radius */
  static constexpr double WGS84_POLAR = 6356752.314245;
  /** Equatorial radius */
  static constexpr double WGS84_EQUATORIAL = 6378137.0;
  /** Planet model corresponding to WGS84 */
  static const std::shared_ptr<PlanetModel> WGS84;

  // Surface of the planet:
  // x^2/a^2 + y^2/b^2 + z^2/c^2 = 1.0
  // Scaling factors are a,b,c.  geo3d can only support models where a==b, so
  // use ab instead.

  /** The x/y scaling factor */
  const double ab;
  /** The z scaling factor */
  const double c;
  /** The inverse of ab */
  const double inverseAb;
  /** The inverse of c */
  const double inverseC;
  /** The square of the inverse of ab */
  const double inverseAbSquared;
  /** The square of the inverse of c */
  const double inverseCSquared;
  /** The flattening value */
  const double flattening;
  /** The square ratio */
  const double squareRatio;
  /** The scale of the planet */
  const double scale;
  /** The inverse of scale */
  const double inverseScale;

  // We do NOT include radius, because all computations in geo3d are in radians,
  // not meters.

  // Compute north and south pole for planet model, since these are commonly
  // used.

  /** North pole */
  const std::shared_ptr<GeoPoint> NORTH_POLE;
  /** South pole */
  const std::shared_ptr<GeoPoint> SOUTH_POLE;
  /** Min X pole */
  const std::shared_ptr<GeoPoint> MIN_X_POLE;
  /** Max X pole */
  const std::shared_ptr<GeoPoint> MAX_X_POLE;
  /** Min Y pole */
  const std::shared_ptr<GeoPoint> MIN_Y_POLE;
  /** Max Y pole */
  const std::shared_ptr<GeoPoint> MAX_Y_POLE;
  /** Minimum surface distance between poles */
  const double minimumPoleDistance;

  /** Constructor.
   * @param ab is the x/y scaling factor.
   * @param c is the z scaling factor.
   */
  PlanetModel(double const ab, double const c);

  /** Deserialization constructor.
   * @param inputStream is the input stream.
   */
  PlanetModel(std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  /** Does this planet model describe a sphere?
   *@return true if so.
   */
  virtual bool isSphere();

  /** Find the minimum magnitude of all points on the ellipsoid.
   * @return the minimum magnitude for the planet.
   */
  virtual double getMinimumMagnitude();

  /** Find the maximum magnitude of all points on the ellipsoid.
   * @return the maximum magnitude for the planet.
   */
  virtual double getMaximumMagnitude();

  /** Find the minimum x value.
   *@return the minimum X value.
   */
  virtual double getMinimumXValue();

  /** Find the maximum x value.
   *@return the maximum X value.
   */
  virtual double getMaximumXValue();

  /** Find the minimum y value.
   *@return the minimum Y value.
   */
  virtual double getMinimumYValue();

  /** Find the maximum y value.
   *@return the maximum Y value.
   */
  virtual double getMaximumYValue();

  /** Find the minimum z value.
   *@return the minimum Z value.
   */
  virtual double getMinimumZValue();

  /** Find the maximum z value.
   *@return the maximum Z value.
   */
  virtual double getMaximumZValue();

  /** Check if point is on surface.
   * @param v is the point to check.
   * @return true if the point is on the planet surface.
   */
  virtual bool pointOnSurface(std::shared_ptr<Vector> v);

  /** Check if point is on surface.
   * @param x is the x coord.
   * @param y is the y coord.
   * @param z is the z coord.
   */
  virtual bool pointOnSurface(double const x, double const y, double const z);

  /** Check if point is outside surface.
   * @param v is the point to check.
   * @return true if the point is outside the planet surface.
   */
  virtual bool pointOutside(std::shared_ptr<Vector> v);

  /** Check if point is outside surface.
   * @param x is the x coord.
   * @param y is the y coord.
   * @param z is the z coord.
   */
  virtual bool pointOutside(double const x, double const y, double const z);

  /** Compute a GeoPoint that's scaled to actually be on the planet surface.
   * @param deque is the deque.
   * @return the scaled point.
   */
  virtual std::shared_ptr<GeoPoint>
  createSurfacePoint(std::shared_ptr<Vector> deque);

  /** Compute a GeoPoint that's based on (x,y,z) values, but is scaled to
   * actually be on the planet surface.
   * @param x is the x value.
   * @param y is the y value.
   * @param z is the z value.
   * @return the scaled point.
   */
  virtual std::shared_ptr<GeoPoint>
  createSurfacePoint(double const x, double const y, double const z);

  /** Compute a GeoPoint that's a bisection between two other GeoPoints.
   * @param pt1 is the first point.
   * @param pt2 is the second point.
   * @return the bisection point, or null if a unique one cannot be found.
   */
  virtual std::shared_ptr<GeoPoint> bisection(std::shared_ptr<GeoPoint> pt1,
                                              std::shared_ptr<GeoPoint> pt2);

  /** Compute surface distance between two points.
   * @param pt1 is the first point.
   * @param pt2 is the second point.
   * @return the adjusted angle, when multiplied by the mean earth radius,
   * yields a surface distance.  This will differ from GeoPoint.arcDistance()
   * only when the planet model is not a sphere. @see {@link
   * GeoPoint#arcDistance(Vector)}
   */
  virtual double surfaceDistance(std::shared_ptr<GeoPoint> pt1,
                                 std::shared_ptr<GeoPoint> pt2);

  /** Compute new point given original point, a bearing direction, and an
   * adjusted angle (as would be computed by the surfaceDistance() method
   * above).  The original point can be anywhere on the globe.  The bearing
   * direction ranges from 0 (due east at the equator) to pi/2 (due north) to pi
   * (due west at the equator) to 3 pi/4 (due south) to 2 pi.
   * @param from is the starting point.
   * @param dist is the adjusted angle.
   * @param bearing is the direction to proceed.
   * @return the new point, consistent with the bearing direction and distance.
   */
  virtual std::shared_ptr<GeoPoint>
  surfacePointOnBearing(std::shared_ptr<GeoPoint> from, double const dist,
                        double const bearing);

  bool equals(std::any const o) override;

  virtual int hashCode();

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::spatial3d::geom
