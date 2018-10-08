#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include <optional>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class Bounds;
}

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
 * An object for accumulating latitude/longitude bounds information.
 *
 * @lucene.experimental
 */
class LatLonBounds : public std::enable_shared_from_this<LatLonBounds>,
                     public Bounds
{
  GET_CLASS_NAME(LatLonBounds)

  /** Set to true if no longitude bounds can be stated */
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool noLongitudeBound_ = false;
  /** Set to true if no top latitude bound can be stated */
  // C++ NOTE: Fields cannot have the same name as methods:
  bool noTopLatitudeBound_ = false;
  /** Set to true if no bottom latitude bound can be stated */
  // C++ NOTE: Fields cannot have the same name as methods:
  bool noBottomLatitudeBound_ = false;

  /** If non-null, the minimum latitude bound */
  std::optional<double> minLatitude = std::nullopt;
  /** If non-null, the maximum latitude bound */
  std::optional<double> maxLatitude = std::nullopt;

  // For longitude bounds, this class needs to worry about keeping track of the
  // distinction between left-side bounds and right-side bounds.  Points are
  // always submitted in pairs which have a maximum longitude separation of
  // Math.PI.  It's therefore always possible to determine which point
  // represents a left bound, and which point represents a right bound.
  //
  // The next problem is how to compare two of the same kind of bound, e.g. two
  // left bounds. We need to keep track of the leftmost longitude of the shape,
  // but since this is a circle, this is arbitrary.  What we could try to do
  // instead would be to find a pair of (left,right) bounds such that: (1) all
  // other bounds are within, and (2) the left minus right distance is minimized
  // Unfortunately, there are still shapes that cannot be summarized in this way
  // correctly. For example. consider a spiral that entirely circles the globe;
  // we might arbitrarily choose lat/lon bounds that do not in fact circle the
  // globe.
  //
  // One way to handle the longitude issue correctly is therefore to stipulate
  // that we walk the bounds of the shape in some kind of connected order.  Each
  // point or circle is therefore added in a sequence.  We also need an interior
  // point to make sure we have the right choice of longitude bounds.  But even
  // with this, we still can't always choose whether the actual shape goes right
  // or left.
  //
  // We can make the specification truly general by submitting the following in
  // order: addSide(PlaneSide side, Membership... constraints)
  // ...
  // This is unambiguous, but I still can't see yet how this would help compute
  // the bounds.  The plane solution would in general seem to boil down to the
  // same logic that relies on points along the path to define the shape
  // boundaries.  I guess the one thing that you do know for a bounded edge is
  // that the endpoints are actually connected.  But it is not clear whether
  // relationship helps in any way.
  //
  // In any case, if we specify shapes by a sequence of planes, we should
  // stipulate that multiple sequences are allowed, provided they progressively
  // tile an area of the sphere that is connected and sequential. For example,
  // paths do alternating rectangles and circles, in sequence.  Each sequence
  // member is described by a sequence of planes.  I think it would also be
  // reasonable to insist that the first segment of a shape overlap or adjoin
  // the previous shape.
  //
  // Here's a way to think about it that might help: Traversing every edge
  // should grow the longitude bounds in the direction of the traversal.  So if
  // the traversal is always known to be less than PI in total longitude angle,
  // then it is possible to use the endpoints to determine the unambiguous
  // extension of the envelope. For example, say you are currently at longitude
  // -0.5.  The next point is at longitude PI-0.1.  You could say that the
  // difference in longitude going one way around would be beter than the
  // distance the other way around, and therefore the longitude envelope should
  // be extended accordingly.  But in practice, when an edge goes near a pole
  // and may be inclined as well, the longer longitude change might be the right
  // path, even if the arc length is short.  So this too doesn't work.
  //
  // Given we have a hard time making an exact match, here's the current
  // proposal.  The proposal is a heuristic, based on the idea that most areas
  // are small compared to the circumference of the globe. We keep track of the
  // last point we saw, and take each point as it arrives, and compute its
  // longitude. Then, we have a choice as to which way to expand the envelope:
  // we can expand by going to the left or to the right.  We choose the
  // direction with the least longitude difference.  (If we aren't sure, and can
  // recognize that, we can set "unconstrained in longitude".)

  /** If non-null, the left longitude bound */
  std::optional<double> leftLongitude = std::nullopt;
  /** If non-null, the right longitude bound */
  std::optional<double> rightLongitude = std::nullopt;

  /** Construct an empty bounds object */
public:
  LatLonBounds();

  // Accessor methods

  /** Get maximum latitude, if any.
   *@return maximum latitude or null.
   */
  virtual std::optional<double> getMaxLatitude();

  /** Get minimum latitude, if any.
   *@return minimum latitude or null.
   */
  virtual std::optional<double> getMinLatitude();

  /** Get left longitude, if any.
   *@return left longitude, or null.
   */
  virtual std::optional<double> getLeftLongitude();

  /** Get right longitude, if any.
   *@return right longitude, or null.
   */
  virtual std::optional<double> getRightLongitude();

  // Degenerate case check

  /** Check if there's no longitude bound.
   *@return true if no longitude bound.
   */
  virtual bool checkNoLongitudeBound();

  /** Check if there's no top latitude bound.
   *@return true if no top latitude bound.
   */
  virtual bool checkNoTopLatitudeBound();

  /** Check if there's no bottom latitude bound.
   *@return true if no bottom latitude bound.
   */
  virtual bool checkNoBottomLatitudeBound();

  // Modification methods

  std::shared_ptr<Bounds> addPlane(std::shared_ptr<PlanetModel> planetModel,
                                   std::shared_ptr<Plane> plane,
                                   std::deque<Membership> &bounds) override;

  std::shared_ptr<Bounds>
  addHorizontalPlane(std::shared_ptr<PlanetModel> planetModel,
                     double const latitude,
                     std::shared_ptr<Plane> horizontalPlane,
                     std::deque<Membership> &bounds) override;

  std::shared_ptr<Bounds>
  addVerticalPlane(std::shared_ptr<PlanetModel> planetModel,
                   double const longitude, std::shared_ptr<Plane> verticalPlane,
                   std::deque<Membership> &bounds) override;

  std::shared_ptr<Bounds> isWide() override;

  std::shared_ptr<Bounds> addXValue(std::shared_ptr<GeoPoint> point) override;

  std::shared_ptr<Bounds> addYValue(std::shared_ptr<GeoPoint> point) override;

  std::shared_ptr<Bounds> addZValue(std::shared_ptr<GeoPoint> point) override;

  std::shared_ptr<Bounds>
  addIntersection(std::shared_ptr<PlanetModel> planetModel,
                  std::shared_ptr<Plane> plane1, std::shared_ptr<Plane> plane2,
                  std::deque<Membership> &bounds) override;

  std::shared_ptr<Bounds> addPoint(std::shared_ptr<GeoPoint> point) override;

  std::shared_ptr<Bounds> noLongitudeBound() override;

  std::shared_ptr<Bounds> noTopLatitudeBound() override;

  std::shared_ptr<Bounds> noBottomLatitudeBound() override;

  std::shared_ptr<Bounds>
  noBound(std::shared_ptr<PlanetModel> planetModel) override;

  // Protected methods

  /** Update latitude bound.
   *@param latitude is the latitude.
   */
private:
  void addLatitudeBound(double latitude);

  /** Update longitude bound.
   *@param longitude is the new longitude value.
   */
  void addLongitudeBound(double longitude);
};

} // namespace org::apache::lucene::spatial3d::geom
