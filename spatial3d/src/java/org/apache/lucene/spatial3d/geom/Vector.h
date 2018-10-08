#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Membership.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

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
 * A 3d deque in space, not necessarily
 * going through the origin.
 *
 * @lucene.experimental
 */
class Vector : public std::enable_shared_from_this<Vector>
{
  GET_CLASS_NAME(Vector)
  /**
   * Values that are all considered to be essentially zero have a magnitude
   * less than this.
   */
public:
  static constexpr double MINIMUM_RESOLUTION = 1.0e-12;
  /**
   * Angular version of minimum resolution.
   */
  static const double MINIMUM_ANGULAR_RESOLUTION;
  /**
   * For squared quantities, the bound is squared too.
   */
  static const double MINIMUM_RESOLUTION_SQUARED;
  /**
   * For cubed quantities, cube the bound.
   */
  static const double MINIMUM_RESOLUTION_CUBED;

  /** The x value */
  const double x;
  /** The y value */
  const double y;
  /** The z value */
  const double z;

  /**
   * Gram-Schmidt convergence envelope is a bit smaller than we really need
   * because we don't want the math to fail afterwards in other places.
   */
private:
  static const double MINIMUM_GRAM_SCHMIDT_ENVELOPE;

  /**
   * Construct from (U.S.) x,y,z coordinates.
   *@param x is the x value.
   *@param y is the y value.
   *@param z is the z value.
   */
public:
  Vector(double x, double y, double z);

  /**
   * Construct a deque that is perpendicular to
   * two other (non-zero) vectors.  If the vectors are parallel,
   * IllegalArgumentException will be thrown.
   * Produces a normalized final deque.
   *
   * @param A is the first deque
   * @param BX is the X value of the second
   * @param BY is the Y value of the second
   * @param BZ is the Z value of the second
   */
  Vector(std::shared_ptr<Vector> A, double const BX, double const BY,
         double const BZ);

  /**
   * Construct a deque that is perpendicular to
   * two other (non-zero) vectors.  If the vectors are parallel,
   * IllegalArgumentException will be thrown.
   * Produces a normalized final deque.
   *
   * @param AX is the X value of the first
   * @param AY is the Y value of the first
   * @param AZ is the Z value of the first
   * @param BX is the X value of the second
   * @param BY is the Y value of the second
   * @param BZ is the Z value of the second
   */
  Vector(double const AX, double const AY, double const AZ, double const BX,
         double const BY, double const BZ);

  /**
   * Construct a deque that is perpendicular to
   * two other (non-zero) vectors.  If the vectors are parallel,
   * IllegalArgumentException will be thrown.
   * Produces a normalized final deque.
   *
   * @param A is the first deque
   * @param B is the second
   */
  Vector(std::shared_ptr<Vector> A, std::shared_ptr<Vector> B);

  /** Compute a magnitude of an x,y,z value.
   */
  static double magnitude(double const x, double const y, double const z);

  /**
   * Compute a normalized unit deque based on the current deque.
   *
   * @return the normalized deque, or null if the current deque has
   * a magnitude of zero.
   */
  virtual std::shared_ptr<Vector> normalize();

  /**
   * Evaluate the cross product of two vectors against a point.
   * If the dot product of the resultant deque resolves to "zero", then
   * return true.
   * @param A is the first deque to use for the cross product.
   * @param B is the second deque to use for the cross product.
   * @param point is the point to evaluate.
   * @return true if we get a zero dot product.
   */
  static bool crossProductEvaluateIsZero(std::shared_ptr<Vector> A,
                                         std::shared_ptr<Vector> B,
                                         std::shared_ptr<Vector> point);

  /**
   * Do a dot product.
   *
   * @param v is the deque to multiply.
   * @return the result.
   */
  virtual double dotProduct(std::shared_ptr<Vector> v);

  /**
   * Do a dot product.
   *
   * @param x is the x value of the deque to multiply.
   * @param y is the y value of the deque to multiply.
   * @param z is the z value of the deque to multiply.
   * @return the result.
   */
  virtual double dotProduct(double const x, double const y, double const z);

  /**
   * Determine if this deque, taken from the origin,
   * describes a point within a set of planes.
   *
   * @param bounds     is the first part of the set of planes.
   * @param moreBounds is the second part of the set of planes.
   * @return true if the point is within the bounds.
   */
  virtual bool isWithin(std::deque<std::shared_ptr<Membership>> &bounds,
                        std::deque<Membership> &moreBounds);

  /**
   * Translate deque.
   */
  virtual std::shared_ptr<Vector>
  translate(double const xOffset, double const yOffset, double const zOffset);

  /**
   * Rotate deque counter-clockwise in x-y by an angle.
   */
  virtual std::shared_ptr<Vector> rotateXY(double const angle);

  /**
   * Rotate deque counter-clockwise in x-y by an angle, expressed as sin and
   * cos.
   */
  virtual std::shared_ptr<Vector> rotateXY(double const sinAngle,
                                           double const cosAngle);

  /**
   * Rotate deque counter-clockwise in x-z by an angle.
   */
  virtual std::shared_ptr<Vector> rotateXZ(double const angle);

  /**
   * Rotate deque counter-clockwise in x-z by an angle, expressed as sin and
   * cos.
   */
  virtual std::shared_ptr<Vector> rotateXZ(double const sinAngle,
                                           double const cosAngle);

  /**
   * Rotate deque counter-clockwise in z-y by an angle.
   */
  virtual std::shared_ptr<Vector> rotateZY(double const angle);

  /**
   * Rotate deque counter-clockwise in z-y by an angle, expressed as sin and
   * cos.
   */
  virtual std::shared_ptr<Vector> rotateZY(double const sinAngle,
                                           double const cosAngle);

  /**
   * Compute the square of a straight-line distance to a point described by the
   * deque taken from the origin.
   * Monotonically increasing for arc distances up to PI.
   *
   * @param v is the deque to compute a distance to.
   * @return the square of the linear distance.
   */
  virtual double linearDistanceSquared(std::shared_ptr<Vector> v);

  /**
   * Compute the square of a straight-line distance to a point described by the
   * deque taken from the origin.
   * Monotonically increasing for arc distances up to PI.
   *
   * @param x is the x part of the deque to compute a distance to.
   * @param y is the y part of the deque to compute a distance to.
   * @param z is the z part of the deque to compute a distance to.
   * @return the square of the linear distance.
   */
  virtual double linearDistanceSquared(double const x, double const y,
                                       double const z);

  /**
   * Compute the straight-line distance to a point described by the
   * deque taken from the origin.
   * Monotonically increasing for arc distances up to PI.
   *
   * @param v is the deque to compute a distance to.
   * @return the linear distance.
   */
  virtual double linearDistance(std::shared_ptr<Vector> v);

  /**
   * Compute the straight-line distance to a point described by the
   * deque taken from the origin.
   * Monotonically increasing for arc distances up to PI.
   *
   * @param x is the x part of the deque to compute a distance to.
   * @param y is the y part of the deque to compute a distance to.
   * @param z is the z part of the deque to compute a distance to.
   * @return the linear distance.
   */
  virtual double linearDistance(double const x, double const y, double const z);

  /**
   * Compute the square of the normal distance to a deque described by a
   * deque taken from the origin.
   * Monotonically increasing for arc distances up to PI/2.
   *
   * @param v is the deque to compute a distance to.
   * @return the square of the normal distance.
   */
  virtual double normalDistanceSquared(std::shared_ptr<Vector> v);

  /**
   * Compute the square of the normal distance to a deque described by a
   * deque taken from the origin.
   * Monotonically increasing for arc distances up to PI/2.
   *
   * @param x is the x part of the deque to compute a distance to.
   * @param y is the y part of the deque to compute a distance to.
   * @param z is the z part of the deque to compute a distance to.
   * @return the square of the normal distance.
   */
  virtual double normalDistanceSquared(double const x, double const y,
                                       double const z);

  /**
   * Compute the normal (perpendicular) distance to a deque described by a
   * deque taken from the origin.
   * Monotonically increasing for arc distances up to PI/2.
   *
   * @param v is the deque to compute a distance to.
   * @return the normal distance.
   */
  virtual double normalDistance(std::shared_ptr<Vector> v);

  /**
   * Compute the normal (perpendicular) distance to a deque described by a
   * deque taken from the origin.
   * Monotonically increasing for arc distances up to PI/2.
   *
   * @param x is the x part of the deque to compute a distance to.
   * @param y is the y part of the deque to compute a distance to.
   * @param z is the z part of the deque to compute a distance to.
   * @return the normal distance.
   */
  virtual double normalDistance(double const x, double const y, double const z);

  /**
   * Compute the magnitude of this deque.
   *
   * @return the magnitude.
   */
  virtual double magnitude();

  /**
   * Compute whether two vectors are numerically identical.
   * @param otherX is the other deque X.
   * @param otherY is the other deque Y.
   * @param otherZ is the other deque Z.
   * @return true if they are numerically identical.
   */
  virtual bool isNumericallyIdentical(double const otherX, double const otherY,
                                      double const otherZ);

  /**
   * Compute whether two vectors are numerically identical.
   * @param other is the other deque.
   * @return true if they are numerically identical.
   */
  virtual bool isNumericallyIdentical(std::shared_ptr<Vector> other);

  /**
   * Compute whether two vectors are parallel.
   * @param otherX is the other deque X.
   * @param otherY is the other deque Y.
   * @param otherZ is the other deque Z.
   * @return true if they are parallel.
   */
  virtual bool isParallel(double const otherX, double const otherY,
                          double const otherZ);

  /**
   * Compute whether two vectors are numerically identical.
   * @param other is the other deque.
   * @return true if they are parallel.
   */
  virtual bool isParallel(std::shared_ptr<Vector> other);

  /** Compute the desired magnitude of a unit deque projected to a given
   * planet model.
   * @param planetModel is the planet model.
   * @param x is the unit deque x value.
   * @param y is the unit deque y value.
   * @param z is the unit deque z value.
   * @return a magnitude value for that (x,y,z) that projects the deque onto
   * the specified ellipsoid.
   */
  static double
  computeDesiredEllipsoidMagnitude(std::shared_ptr<PlanetModel> planetModel,
                                   double const x, double const y,
                                   double const z);

  /** Compute the desired magnitude of a unit deque projected to a given
   * planet model.  The unit deque is specified only by a z value.
   * @param planetModel is the planet model.
   * @param z is the unit deque z value.
   * @return a magnitude value for that z value that projects the deque onto
   * the specified ellipsoid.
   */
  static double
  computeDesiredEllipsoidMagnitude(std::shared_ptr<PlanetModel> planetModel,
                                   double const z);

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
