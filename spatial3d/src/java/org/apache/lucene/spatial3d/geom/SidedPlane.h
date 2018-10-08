#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Vector.h"

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
 * Combination of a plane, and a sign value indicating what evaluation values
 * are on the correct side of the plane.
 *
 * @lucene.experimental
 */
class SidedPlane : public Plane, public Membership
{
  GET_CLASS_NAME(SidedPlane)
  /** The sign value for evaluation of a point on the correct side of the plane
   */
public:
  const double sigNum;

  /**
   * Construct a SidedPlane identical to an existing one, but reversed.
   *
   * @param sidedPlane is the existing plane.
   */
  SidedPlane(std::shared_ptr<SidedPlane> sidedPlane);

  /**
   * Construct a sided plane from a pair of vectors describing points, and
   * including origin, plus a point p which describes the side.
   *
   * @param pX point X to evaluate
   * @param pY point Y to evaluate
   * @param pZ point Z to evaluate
   * @param A is the first in-plane point
   * @param B is the second in-plane point
   */
  SidedPlane(double const pX, double const pY, double const pZ,
             std::shared_ptr<Vector> A, std::shared_ptr<Vector> B);

  /**
   * Construct a sided plane from a pair of vectors describing points, and
   * including origin, plus a point p which describes the side.
   *
   * @param p point to evaluate
   * @param A is the first in-plane point
   * @param B is the second in-plane point
   */
  SidedPlane(std::shared_ptr<Vector> p, std::shared_ptr<Vector> A,
             std::shared_ptr<Vector> B);

  /**
   * Construct a sided plane from a pair of vectors describing points, and
   * including origin.  Choose the side arbitrarily.
   *
   * @param A is the first in-plane point
   * @param B is the second in-plane point
   */
  SidedPlane(std::shared_ptr<Vector> A, std::shared_ptr<Vector> B);

  /**
   * Construct a sided plane from a pair of vectors describing points, and
   * including origin, plus a point p which describes the side.
   *
   * @param p point to evaluate
   * @param A is the first in-plane point
   * @param BX is the X value of the second in-plane point
   * @param BY is the Y value of the second in-plane point
   * @param BZ is the Z value of the second in-plane point
   */
  SidedPlane(std::shared_ptr<Vector> p, std::shared_ptr<Vector> A,
             double const BX, double const BY, double const BZ);

  /**
   * Construct a sided plane from a pair of vectors describing points, and
   * including origin, plus a point p which describes the side.
   *
   * @param p point to evaluate
   * @param onSide is true if the point is on the correct side of the plane,
   * false otherwise.
   * @param A is the first in-plane point
   * @param B is the second in-plane point
   */
  SidedPlane(std::shared_ptr<Vector> p, bool const onSide,
             std::shared_ptr<Vector> A, std::shared_ptr<Vector> B);

  /**
   * Construct a sided plane from a point and a Z coordinate.
   *
   * @param p      point to evaluate.
   * @param planetModel is the planet model.
   * @param sinLat is the sin of the latitude of the plane.
   */
  SidedPlane(std::shared_ptr<Vector> p,
             std::shared_ptr<PlanetModel> planetModel, double sinLat);

  /**
   * Construct a sided vertical plane from a point and specified x and y
   * coordinates.
   *
   * @param p point to evaluate.
   * @param x is the specified x.
   * @param y is the specified y.
   */
  SidedPlane(std::shared_ptr<Vector> p, double x, double y);

  /**
   * Construct a sided plane with a normal deque and offset.
   *
   * @param p point to evaluate.
   * @param v is the normal deque.
   * @param D is the origin offset for the plan.
   */
  SidedPlane(std::shared_ptr<Vector> p, std::shared_ptr<Vector> v, double D);

  /**
   * Construct a sided plane with a normal deque and offset.
   *
   * @param pX X coord of point to evaluate.
   * @param pY Y coord of point to evaluate.
   * @param pZ Z coord of point to evaluate.
   * @param v is the normal deque.
   * @param D is the origin offset for the plan.
   */
  SidedPlane(double pX, double pY, double pZ, std::shared_ptr<Vector> v,
             double D);

  /** Construct a sided plane from two points and a third normal deque.
   */
  static std::shared_ptr<SidedPlane> constructNormalizedPerpendicularSidedPlane(
      std::shared_ptr<Vector> insidePoint, std::shared_ptr<Vector> normalVector,
      std::shared_ptr<Vector> point1, std::shared_ptr<Vector> point2);

  /** Construct a sided plane from three points.
   */
  static std::shared_ptr<SidedPlane> constructNormalizedThreePointSidedPlane(
      std::shared_ptr<Vector> insidePoint, std::shared_ptr<Vector> point1,
      std::shared_ptr<Vector> point2, std::shared_ptr<Vector> point3);

  bool isWithin(double x, double y, double z) override;

  /**
   * Check whether a point is strictly within a plane.
   * @param v is the point.
   * @return true if within.
   */
  virtual bool strictlyWithin(std::shared_ptr<Vector> v);

  /**
   * Check whether a point is strictly within a plane.
   * @param x is the point x value.
   * @param y is the point y value.
   * @param z is the point z value.
   * @return true if within.
   */
  virtual bool strictlyWithin(double x, double y, double z);

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<SidedPlane> shared_from_this()
  {
    return std::static_pointer_cast<SidedPlane>(Plane::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
