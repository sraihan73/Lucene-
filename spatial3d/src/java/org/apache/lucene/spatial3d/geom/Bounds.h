#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class Membership;
}

namespace org::apache::lucene::spatial3d::geom
{
class Plane;
}
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
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
 * An interface for accumulating bounds information.
 * The bounds object is initially empty.  Bounding points
 * are then applied by supplying (x,y,z) tuples.  It is also
 * possible to indicate the following edge cases:
 * (1) No longitude bound possible
 * (2) No upper latitude bound possible
 * (3) No lower latitude bound possible
 * When any of these have been applied, further application of
 * points cannot override that decision.
 *
 * @lucene.experimental
 */
class Bounds
{
  GET_CLASS_NAME(Bounds)

  /** Add a general plane to the bounds description.
   *@param planetModel is the planet model.
   *@param plane is the plane.
   *@param bounds are the membership bounds for points along the arc.
   */
public:
  virtual std::shared_ptr<Bounds>
  addPlane(std::shared_ptr<PlanetModel> planetModel,
           std::shared_ptr<Plane> plane, std::deque<Membership> &bounds) = 0;

  /** Add a horizontal plane to the bounds description.
   * This method should EITHER use the supplied latitude, OR use the supplied
   * plane, depending on what is most efficient.
   *@param planetModel is the planet model.
   *@param latitude is the latitude.
   *@param horizontalPlane is the plane.
   *@param bounds are the constraints on the plane.
   *@return updated Bounds object.
   */
  virtual std::shared_ptr<Bounds>
  addHorizontalPlane(std::shared_ptr<PlanetModel> planetModel,
                     double const latitude,
                     std::shared_ptr<Plane> horizontalPlane,
                     std::deque<Membership> &bounds) = 0;

  /** Add a vertical plane to the bounds description.
   * This method should EITHER use the supplied longitude, OR use the supplied
   * plane, depending on what is most efficient.
   *@param planetModel is the planet model.
   *@param longitude is the longitude.
   *@param verticalPlane is the plane.
   *@param bounds are the constraints on the plane.
   *@return updated Bounds object.
   */
  virtual std::shared_ptr<Bounds>
  addVerticalPlane(std::shared_ptr<PlanetModel> planetModel,
                   double const longitude, std::shared_ptr<Plane> verticalPlane,
                   std::deque<Membership> &bounds) = 0;

  /** Add the intersection between two planes to the bounds description.
   * Where the shape has intersecting planes, it is better to use this method
   * than just adding the point, since this method takes each plane's error
   *envelope into account.
   *@param planetModel is the planet model.
   *@param plane1 is the first plane.
   *@param plane2 is the second plane.
   *@param bounds are the membership bounds for the intersection.
   */
  virtual std::shared_ptr<Bounds>
  addIntersection(std::shared_ptr<PlanetModel> planetModel,
                  std::shared_ptr<Plane> plane1, std::shared_ptr<Plane> plane2,
                  std::deque<Membership> &bounds) = 0;

  /** Add a single point.
   *@param point is the point.
   *@return the updated Bounds object.
   */
  virtual std::shared_ptr<Bounds> addPoint(std::shared_ptr<GeoPoint> point) = 0;

  /** Add an X value.
   *@param point is the point to take the x value from.
   *@return the updated object.
   */
  virtual std::shared_ptr<Bounds>
  addXValue(std::shared_ptr<GeoPoint> point) = 0;

  /** Add a Y value.
   *@param point is the point to take the y value from.
   *@return the updated object.
   */
  virtual std::shared_ptr<Bounds>
  addYValue(std::shared_ptr<GeoPoint> point) = 0;

  /** Add a Z value.
   *@param point is the point to take the z value from.
   *@return the updated object.
   */
  virtual std::shared_ptr<Bounds>
  addZValue(std::shared_ptr<GeoPoint> point) = 0;

  /** Signal that the shape exceeds Math.PI in longitude.
   *@return the updated Bounds object.
   */
  virtual std::shared_ptr<Bounds> isWide() = 0;

  /** Signal that there is no longitude bound.
   *@return the updated Bounds object.
   */
  virtual std::shared_ptr<Bounds> noLongitudeBound() = 0;

  /** Signal that there is no top latitude bound.
   *@return the updated Bounds object.
   */
  virtual std::shared_ptr<Bounds> noTopLatitudeBound() = 0;

  /** Signal that there is no bottom latitude bound.
   *@return the updated Bounds object.
   */
  virtual std::shared_ptr<Bounds> noBottomLatitudeBound() = 0;

  /** Signal that there is no bound whatsoever.
   * The bound is limited only by the constraints of the
   * planet.
   *@return the updated Bounds object.,
   */
  virtual std::shared_ptr<Bounds>
  noBound(std::shared_ptr<PlanetModel> planetModel) = 0;
};

} // namespace org::apache::lucene::spatial3d::geom
