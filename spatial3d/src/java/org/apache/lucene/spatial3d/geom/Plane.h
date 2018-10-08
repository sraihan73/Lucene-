#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "exceptionhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}

namespace org::apache::lucene::spatial3d::geom
{
class Membership;
}
namespace org::apache::lucene::spatial3d::geom
{
class Vector;
}
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}
namespace org::apache::lucene::spatial3d::geom
{
class Bounds;
}
namespace org::apache::lucene::spatial3d::geom
{
class XYZBounds;
}
namespace org::apache::lucene::spatial3d::geom
{
class LatLonBounds;
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
 * We know about three kinds of planes.  First kind: general plain through two
 * points and origin Second kind: horizontal plane at specified height.  Third
 * kind: vertical plane with specified x and y value, through origin.
 *
 * @lucene.experimental
 */
class Plane : public Vector
{
  GET_CLASS_NAME(Plane)
  /** An array with no points in it */
public:
  static std::deque<std::shared_ptr<GeoPoint>> const NO_POINTS;
  /** An array with no bounds in it */
  static std::deque<std::shared_ptr<Membership>> const NO_BOUNDS;
  /** A vertical plane normal to the Y axis */
  static const std::shared_ptr<Plane> normalYPlane;
  /** A vertical plane normal to the X axis */
  static const std::shared_ptr<Plane> normalXPlane;
  /** A vertical plane normal to the Z axis */
  static const std::shared_ptr<Plane> normalZPlane;

  /** Ax + By + Cz + D = 0 */
  const double D;

  /**
   * Construct a plane with all four coefficients defined.
   *@param A is A
   *@param B is B
   *@param C is C
   *@param D is D
   */
  Plane(double const A, double const B, double const C, double const D);

  /**
   * Construct a plane through two points and origin.
   *
   * @param A is the first point (origin based).
   * @param BX is the second point X (origin based).
   * @param BY is the second point Y (origin based).
   * @param BZ is the second point Z (origin based).
   */
  Plane(std::shared_ptr<Vector> A, double const BX, double const BY,
        double const BZ);

  /**
   * Construct a plane through two points and origin.
   *
   * @param A is the first point (origin based).
   * @param B is the second point (origin based).
   */
  Plane(std::shared_ptr<Vector> A, std::shared_ptr<Vector> B);

  /**
   * Construct a horizontal plane at a specified Z.
   *
   * @param planetModel is the planet model.
   * @param sinLat is the sin(latitude).
   */
  Plane(std::shared_ptr<PlanetModel> planetModel, double const sinLat);

  /**
   * Construct a vertical plane through a specified
   * x, y and origin.
   *
   * @param x is the specified x value.
   * @param y is the specified y value.
   */
  Plane(double const x, double const y);

  /**
   * Construct a plane with a specific deque, and D offset
   * from origin.
   * @param v is the normal deque.
   * @param D is the D offset from the origin.
   */
  Plane(std::shared_ptr<Vector> v, double const D);

  /** Construct a plane that is parallel to the one provided, but which is just
   * barely numerically distinguishable from it, in the direction desired.
   * @param basePlane is the starting plane.
   * @param above is set to true if the desired plane is in the positive
   * direction from the base plane, or false in the negative direction.
   */
  Plane(std::shared_ptr<Plane> basePlane, bool const above);

  /** Construct the most accurate normalized plane through an x-y point and
   * including the Z axis. If none of the points can determine the plane, return
   * null.
   * @param planePoints is a set of points to choose from.  The best one for
   * constructing the most precise plane is picked.
   * @return the plane
   */
  static std::shared_ptr<Plane>
  constructNormalizedZPlane(std::deque<Vector> &planePoints);

  /** Construct the most accurate normalized plane through an x-z point and
   * including the Y axis. If none of the points can determine the plane, return
   * null.
   * @param planePoints is a set of points to choose from.  The best one for
   * constructing the most precise plane is picked.
   * @return the plane
   */
  static std::shared_ptr<Plane>
  constructNormalizedYPlane(std::deque<Vector> &planePoints);

  /** Construct the most accurate normalized plane through an y-z point and
   * including the X axis. If none of the points can determine the plane, return
   * null.
   * @param planePoints is a set of points to choose from.  The best one for
   * constructing the most precise plane is picked.
   * @return the plane
   */
  static std::shared_ptr<Plane>
  constructNormalizedXPlane(std::deque<Vector> &planePoints);

  /** Construct a normalized plane through an x-y point and including the Z
   * axis. If the x-y point is at (0,0), return null.
   * @param x is the x value.
   * @param y is the y value.
   * @return a plane passing through the Z axis and (x,y,0).
   */
  static std::shared_ptr<Plane> constructNormalizedZPlane(double const x,
                                                          double const y);

  /** Construct a normalized plane through an x-z point and parallel to the Y
   * axis. If the x-z point is at (0,0), return null.
   * @param x is the x value.
   * @param z is the z value.
   * @param DValue is the offset from the origin for the plane.
   * @return a plane parallel to the Y axis and perpendicular to the x and z
   * values given.
   */
  static std::shared_ptr<Plane> constructNormalizedYPlane(double const x,
                                                          double const z,
                                                          double const DValue);

  /** Construct a normalized plane through a y-z point and parallel to the X
   * axis. If the y-z point is at (0,0), return null.
   * @param y is the y value.
   * @param z is the z value.
   * @param DValue is the offset from the origin for the plane.
   * @return a plane parallel to the X axis and perpendicular to the y and z
   * values given.
   */
  static std::shared_ptr<Plane> constructNormalizedXPlane(double const y,
                                                          double const z,
                                                          double const DValue);

  /**
   * Evaluate the plane equation for a given point, as represented
   * by a deque.
   *
   * @param v is the deque.
   * @return the result of the evaluation.
   */
  virtual double evaluate(std::shared_ptr<Vector> v);

  /**
   * Evaluate the plane equation for a given point, as represented
   * by a deque.
   * @param x is the x value.
   * @param y is the y value.
   * @param z is the z value.
   * @return the result of the evaluation.
   */
  virtual double evaluate(double const x, double const y, double const z);

  /**
   * Evaluate the plane equation for a given point, as represented
   * by a deque.
   *
   * @param v is the deque.
   * @return true if the result is on the plane.
   */
  virtual bool evaluateIsZero(std::shared_ptr<Vector> v);

  /**
   * Evaluate the plane equation for a given point, as represented
   * by a deque.
   *
   * @param x is the x value.
   * @param y is the y value.
   * @param z is the z value.
   * @return true if the result is on the plane.
   */
  virtual bool evaluateIsZero(double const x, double const y, double const z);

  /**
   * Build a normalized plane, so that the deque is normalized.
   *
   * @return the normalized plane object, or null if the plane is indeterminate.
   */
  std::shared_ptr<Plane> normalize() override;

  /** Compute arc distance from plane to a deque expressed with a {@link
   * GeoPoint}.
   *  @see #arcDistance(PlanetModel, double, double, double, Membership...) */
  virtual double arcDistance(std::shared_ptr<PlanetModel> planetModel,
                             std::shared_ptr<GeoPoint> v,
                             std::deque<Membership> &bounds);

  /**
   * Compute arc distance from plane to a deque.
   * @param planetModel is the planet model.
   * @param x is the x deque value.
   * @param y is the y deque value.
   * @param z is the z deque value.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the arc distance.
   */
  virtual double arcDistance(std::shared_ptr<PlanetModel> planetModel,
                             double const x, double const y, double const z,
                             std::deque<Membership> &bounds);

  /**
   * Compute normal distance from plane to a deque.
   * @param v is the deque.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the normal distance.
   */
  virtual double normalDistance(std::shared_ptr<Vector> v,
                                std::deque<Membership> &bounds);

  /**
   * Compute normal distance from plane to a deque.
   * @param x is the deque x.
   * @param y is the deque y.
   * @param z is the deque z.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the normal distance.
   */
  virtual double normalDistance(double const x, double const y, double const z,
                                std::deque<Membership> &bounds);

  /**
   * Compute normal distance squared from plane to a deque.
   * @param v is the deque.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the normal distance squared.
   */
  virtual double normalDistanceSquared(std::shared_ptr<Vector> v,
                                       std::deque<Membership> &bounds);

  /**
   * Compute normal distance squared from plane to a deque.
   * @param x is the deque x.
   * @param y is the deque y.
   * @param z is the deque z.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the normal distance squared.
   */
  virtual double normalDistanceSquared(double const x, double const y,
                                       double const z,
                                       std::deque<Membership> &bounds);

  /**
   * Compute linear distance from plane to a deque.  This is defined
   * as the distance from the given point to the nearest intersection of
   * this plane with the planet surface.
   * @param planetModel is the planet model.
   * @param v is the point.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the linear distance.
   */
  virtual double linearDistance(std::shared_ptr<PlanetModel> planetModel,
                                std::shared_ptr<GeoPoint> v,
                                std::deque<Membership> &bounds);

  /**
   * Compute linear distance from plane to a deque.  This is defined
   * as the distance from the given point to the nearest intersection of
   * this plane with the planet surface.
   * @param planetModel is the planet model.
   * @param x is the deque x.
   * @param y is the deque y.
   * @param z is the deque z.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the linear distance.
   */
  virtual double linearDistance(std::shared_ptr<PlanetModel> planetModel,
                                double const x, double const y, double const z,
                                std::deque<Membership> &bounds);

  /**
   * Compute linear distance squared from plane to a deque.  This is defined
   * as the distance from the given point to the nearest intersection of
   * this plane with the planet surface.
   * @param planetModel is the planet model.
   * @param v is the point.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the linear distance squared.
   */
  virtual double linearDistanceSquared(std::shared_ptr<PlanetModel> planetModel,
                                       std::shared_ptr<GeoPoint> v,
                                       std::deque<Membership> &bounds);

  /**
   * Compute linear distance squared from plane to a deque.  This is defined
   * as the distance from the given point to the nearest intersection of
   * this plane with the planet surface.
   * @param planetModel is the planet model.
   * @param x is the deque x.
   * @param y is the deque y.
   * @param z is the deque z.
   * @param bounds are the bounds which constrain the intersection point.
   * @return the linear distance squared.
   */
  virtual double linearDistanceSquared(std::shared_ptr<PlanetModel> planetModel,
                                       double const x, double const y,
                                       double const z,
                                       std::deque<Membership> &bounds);

  /**
   * Find points on the boundary of the intersection of a plane and the unit
   * sphere, given a starting point, and ending point, and a deque of proportions
   * of the arc (e.g. 0.25, 0.5, 0.75). The angle between the starting point and
   * ending point is assumed to be less than pi.
   * @param start is the start point.
   * @param end is the end point.
   * @param proportions is an array of fractional proportions measured between
   * start and end.
   * @return an array of points corresponding to the proportions passed in.
   */
  virtual std::deque<std::shared_ptr<GeoPoint>>
  interpolate(std::shared_ptr<GeoPoint> start, std::shared_ptr<GeoPoint> end,
              std::deque<double> &proportions);

  /**
   * Modify a point to produce a deque in translated/rotated space.
   * @param start is the start point.
   * @param transX is the translation x value.
   * @param transY is the translation y value.
   * @param transZ is the translation z value.
   * @param sinRA is the sine of the ascension angle.
   * @param cosRA is the cosine of the ascension angle.
   * @param sinHA is the sine of the height angle.
   * @param cosHA is the cosine of the height angle.
   * @return the modified point.
   */
protected:
  static std::shared_ptr<Vector>
  modify(std::shared_ptr<GeoPoint> start, double const transX,
         double const transY, double const transZ, double const sinRA,
         double const cosRA, double const sinHA, double const cosHA);

  /**
   * Reverse modify a point to produce a GeoPoint in normal space.
   * @param point is the translated point.
   * @param transX is the translation x value.
   * @param transY is the translation y value.
   * @param transZ is the translation z value.
   * @param sinRA is the sine of the ascension angle.
   * @param cosRA is the cosine of the ascension angle.
   * @param sinHA is the sine of the height angle.
   * @param cosHA is the cosine of the height angle.
   * @return the original point.
   */
  static std::shared_ptr<GeoPoint>
  reverseModify(std::shared_ptr<Vector> point, double const transX,
                double const transY, double const transZ, double const sinRA,
                double const cosRA, double const sinHA, double const cosHA);

  /**
   * Find the intersection points between two planes, given a set of bounds.
   * @param planetModel is the planet model.
   * @param q is the plane to intersect with.
   * @param bounds are the bounds to consider to determine legal intersection
   * points.
   * @return the set of legal intersection points, or null if the planes are
   * numerically identical.
   */
public:
  virtual std::deque<std::shared_ptr<GeoPoint>>
  findIntersections(std::shared_ptr<PlanetModel> planetModel,
                    std::shared_ptr<Plane> q, std::deque<Membership> &bounds);

  /**
   * Find the points between two planes, where one plane crosses the other,
   * given a set of bounds. Crossing is not just intersection; the planes cannot
   * touch at just one point on the ellipsoid, but must cross at two.
   *
   * @param planetModel is the planet model.
   * @param q is the plane to intersect with.
   * @param bounds are the bounds to consider to determine legal intersection
   * points.
   * @return the set of legal crossing points, or null if the planes are
   * numerically identical.
   */
  virtual std::deque<std::shared_ptr<GeoPoint>>
  findCrossings(std::shared_ptr<PlanetModel> planetModel,
                std::shared_ptr<Plane> q, std::deque<Membership> &bounds);

  /**
   * Checks if three points are coplanar in any of the three planes they can
   * describe. The planes are all assumed to go through the origin.
   *
   * @param A The first point.
   * @param B The second point.
   * @param C The third point
   * @return true if provided points are coplanar in any of the three planes
   * they can describe.
   */
  static bool arePointsCoplanar(std::shared_ptr<GeoPoint> A,
                                std::shared_ptr<GeoPoint> B,
                                std::shared_ptr<GeoPoint> C);

  /**
   * Find the intersection points between two planes, given a set of bounds.
   *
   * @param planetModel is the planet model to use in finding points.
   * @param q          is the plane to intersect with.
   * @param bounds     is the set of bounds.
   * @param moreBounds is another set of bounds.
   * @return the intersection point(s) on the unit sphere, if there are any.
   */
protected:
  virtual std::deque<std::shared_ptr<GeoPoint>>
  findIntersections(std::shared_ptr<PlanetModel> planetModel,
                    std::shared_ptr<Plane> q,
                    std::deque<std::shared_ptr<Membership>> &bounds,
                    std::deque<std::shared_ptr<Membership>> &moreBounds);

  /**
   * Find the points between two planes, where one plane crosses the other,
   * given a set of bounds. Crossing is not just intersection; the planes cannot
   * touch at just one point on the ellipsoid, but must cross at two.
   *
   * @param planetModel is the planet model to use in finding points.
   * @param q          is the plane to intersect with.
   * @param bounds     is the set of bounds.
   * @param moreBounds is another set of bounds.
   * @return the intersection point(s) on the ellipsoid, if there are any.
   */
  virtual std::deque<std::shared_ptr<GeoPoint>>
  findCrossings(std::shared_ptr<PlanetModel> planetModel,
                std::shared_ptr<Plane> q,
                std::deque<std::shared_ptr<Membership>> &bounds,
                std::deque<std::shared_ptr<Membership>> &moreBounds);

  /**
   * Record intersection points for planes with error bounds.
   * This method calls the Bounds object with every intersection point it can
   * find that matches the criteria. Each plane is considered to have two sides,
   * one that is D + MINIMUM_RESOLUTION, and one that is D - MINIMUM_RESOLUTION.
   * Both are examined and intersection points determined.
   */
  virtual void findIntersectionBounds(std::shared_ptr<PlanetModel> planetModel,
                                      std::shared_ptr<Bounds> boundsInfo,
                                      std::shared_ptr<Plane> q,
                                      std::deque<Membership> &bounds);

private:
  static void recordLineBounds(std::shared_ptr<PlanetModel> planetModel,
                               std::shared_ptr<Bounds> boundsInfo,
                               double const lineVectorX,
                               double const lineVectorY,
                               double const lineVectorZ, double const x0,
                               double const y0, double const z0,
                               std::deque<Membership> &bounds);

  /*
  protected void verifyPoint(final PlanetModel planetModel, final GeoPoint
  point, final Plane q) { if (!evaluateIsZero(point)) throw new
  RuntimeException("Intersection point not on original plane; point="+point+",
  plane="+this); if (!q.evaluateIsZero(point)) throw new
  RuntimeException("Intersection point not on intersected plane;
  point="+point+", plane="+q); if (Math.abs(point.x * point.x *
  planetModel.inverseASquared + point.y * point.y * planetModel.inverseBSquared
  + point.z * point.z * planetModel.inverseCSquared - 1.0) >=
  MINIMUM_RESOLUTION) throw new RuntimeException("Intersection point not on
  ellipsoid; point="+point);
  }
  */

  /**
   * Accumulate (x,y,z) bounds information for this plane, intersected with
   * another and the world. Updates min/max information using intersection
   * points found.  These include the error envelope for the planes (D +/-
   * MINIMUM_RESOLUTION).
   * @param planetModel is the planet model to use in determining bounds.
   * @param boundsInfo is the xyz info to update with additional bounding
   * information.
   * @param p is the other plane.
   * @param bounds     are the surfaces delineating what's inside the shape.
   */
public:
  virtual void recordBounds(std::shared_ptr<PlanetModel> planetModel,
                            std::shared_ptr<XYZBounds> boundsInfo,
                            std::shared_ptr<Plane> p,
                            std::deque<Membership> &bounds);

  /**
   * Accumulate (x,y,z) bounds information for this plane, intersected with the
   * unit sphere. Updates min/max information, using max/min points found within
   * the specified bounds.
   *
   * @param planetModel is the planet model to use in determining bounds.
   * @param boundsInfo is the xyz info to update with additional bounding
   * information.
   * @param bounds     are the surfaces delineating what's inside the shape.
   */
  virtual void recordBounds(std::shared_ptr<PlanetModel> planetModel,
                            std::shared_ptr<XYZBounds> boundsInfo,
                            std::deque<Membership> &bounds);

  /**
   * Accumulate bounds information for this plane, intersected with another
   * plane and the world. Updates both latitude and longitude information, using
   * max/min points found within the specified bounds.  Also takes into account
   * the error envelope for all planes being intersected.
   *
   * @param planetModel is the planet model to use in determining bounds.
   * @param boundsInfo is the lat/lon info to update with additional bounding
   * information.
   * @param p is the other plane.
   * @param bounds     are the surfaces delineating what's inside the shape.
   */
  virtual void recordBounds(std::shared_ptr<PlanetModel> planetModel,
                            std::shared_ptr<LatLonBounds> boundsInfo,
                            std::shared_ptr<Plane> p,
                            std::deque<Membership> &bounds);

  /**
   * Accumulate bounds information for this plane, intersected with the unit
   * sphere. Updates both latitude and longitude information, using max/min
   * points found within the specified bounds.
   *
   * @param planetModel is the planet model to use in determining bounds.
   * @param boundsInfo is the lat/lon info to update with additional bounding
   * information.
   * @param bounds     are the surfaces delineating what's inside the shape.
   */
  virtual void recordBounds(std::shared_ptr<PlanetModel> planetModel,
                            std::shared_ptr<LatLonBounds> boundsInfo,
                            std::deque<Membership> &bounds);

  /** Add a point to boundsInfo if within a specifically bounded area.
   * @param boundsInfo is the object to be modified.
   * @param bounds is the area that the point must be within.
   * @param point is the point.
   */
private:
  static void addPoint(std::shared_ptr<Bounds> boundsInfo,
                       std::deque<std::shared_ptr<Membership>> &bounds,
                       std::shared_ptr<GeoPoint> point);

  /**
   * Determine whether the plane intersects another plane within the
   * bounds provided.
   *
   * @param planetModel is the planet model to use in determining intersec