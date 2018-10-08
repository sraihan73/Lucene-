#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/Bounds.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Membership.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Plane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"

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
 * An object for accumulating XYZ bounds information.
 *
 * @lucene.experimental
 */
class XYZBounds : public std::enable_shared_from_this<XYZBounds>, public Bounds
{
  GET_CLASS_NAME(XYZBounds)

  /** A 'fudge factor', which is added to maximums and subtracted from minimums,
   * in order to compensate for potential error deltas.  This would not be
   * necessary except that our 'bounds' is defined as always equaling or
   * exceeding the boundary of the shape, and we cannot guarantee that without
   * making MINIMUM_RESOLUTION unacceptably large. Also, see LUCENE-7290 for a
   * description of how geometry can magnify the bounds delta.
   */
private:
  static const double FUDGE_FACTOR;

  /** Minimum x */
  std::optional<double> minX = std::nullopt;
  /** Maximum x */
  std::optional<double> maxX = std::nullopt;
  /** Minimum y */
  std::optional<double> minY = std::nullopt;
  /** Maximum y */
  std::optional<double> maxY = std::nullopt;
  /** Minimum z */
  std::optional<double> minZ = std::nullopt;
  /** Maximum z */
  std::optional<double> maxZ = std::nullopt;

  /** Set to true if no longitude bounds can be stated */
  // C++ NOTE: Fields cannot have the same name as methods:
  bool noLongitudeBound_ = false;
  /** Set to true if no top latitude bound can be stated */
  // C++ NOTE: Fields cannot have the same name as methods:
  bool noTopLatitudeBound_ = false;
  /** Set to true if no bottom latitude bound can be stated */
  // C++ NOTE: Fields cannot have the same name as methods:
  bool noBottomLatitudeBound_ = false;

  /** Construct an empty bounds object */
public:
  XYZBounds();

  // Accessor methods

  /** Return the minimum X value.
   *@return minimum X value.
   */
  virtual std::optional<double> getMinimumX();

  /** Return the maximum X value.
   *@return maximum X value.
   */
  virtual std::optional<double> getMaximumX();

  /** Return the minimum Y value.
   *@return minimum Y value.
   */
  virtual std::optional<double> getMinimumY();

  /** Return the maximum Y value.
   *@return maximum Y value.
   */
  virtual std::optional<double> getMaximumY();

  /** Return the minimum Z value.
   *@return minimum Z value.
   */
  virtual std::optional<double> getMinimumZ();

  /** Return the maximum Z value.
   *@return maximum Z value.
   */
  virtual std::optional<double> getMaximumZ();

  /** Return true if minX is as small as the planet model allows.
   *@return true if minX has reached its bound.
   */
  virtual bool isSmallestMinX(std::shared_ptr<PlanetModel> planetModel);

  /** Return true if maxX is as large as the planet model allows.
   *@return true if maxX has reached its bound.
   */
  virtual bool isLargestMaxX(std::shared_ptr<PlanetModel> planetModel);

  /** Return true if minY is as small as the planet model allows.
   *@return true if minY has reached its bound.
   */
  virtual bool isSmallestMinY(std::shared_ptr<PlanetModel> planetModel);

  /** Return true if maxY is as large as the planet model allows.
   *@return true if maxY has reached its bound.
   */
  virtual bool isLargestMaxY(std::shared_ptr<PlanetModel> planetModel);

  /** Return true if minZ is as small as the planet model allows.
   *@return true if minZ has reached its bound.
   */
  virtual bool isSmallestMinZ(std::shared_ptr<PlanetModel> planetModel);

  /** Return true if maxZ is as large as the planet model allows.
   *@return true if maxZ has reached its bound.
   */
  virtual bool isLargestMaxZ(std::shared_ptr<PlanetModel> planetModel);

  // Modification methods

  std::shared_ptr<Bounds> addPlane(std::shared_ptr<PlanetModel> planetModel,
                                   std::shared_ptr<Plane> plane,
                                   std::deque<Membership> &bounds) override;

  /** Add a horizontal plane to the bounds description.
   * This method should EITHER use the supplied latitude, OR use the supplied
   * plane, depending on what is most efficient.
   *@param planetModel is the planet model.
   *@param latitude is the latitude.
   *@param horizontalPlane is the plane.
   *@param bounds are the constraints on the plane.
   *@return updated Bounds object.
   */
  std::shared_ptr<Bounds>
  addHorizontalPlane(std::shared_ptr<PlanetModel> planetModel,
                     double const latitude,
                     std::shared_ptr<Plane> horizontalPlane,
                     std::deque<Membership> &bounds) override;

  /** Add a vertical plane to the bounds description.
   * This method should EITHER use the supplied longitude, OR use the supplied
   * plane, depending on what is most efficient.
   *@param planetModel is the planet model.
   *@param longitude is the longitude.
   *@param verticalPlane is the plane.
   *@param bounds are the constraints on the plane.
   *@return updated Bounds object.
   */
  std::shared_ptr<Bounds>
  addVerticalPlane(std::shared_ptr<PlanetModel> planetModel,
                   double const longitude, std::shared_ptr<Plane> verticalPlane,
                   std::deque<Membership> &bounds) override;

  std::shared_ptr<Bounds> addXValue(std::shared_ptr<GeoPoint> point) override;

  /** Add a specific X value.
   * @param x is the value to add.
   * @return the bounds object.
   */
  virtual std::shared_ptr<Bounds> addXValue(double const x);

  std::shared_ptr<Bounds> addYValue(std::shared_ptr<GeoPoint> point) override;

  /** Add a specific Y value.
   * @param y is the value to add.
   * @return the bounds object.
   */
  virtual std::shared_ptr<Bounds> addYValue(double const y);

  std::shared_ptr<Bounds> addZValue(std::shared_ptr<GeoPoint> point) override;

  /** Add a specific Z value.
   * @param z is the value to add.
   * @return the bounds object.
   */
  virtual std::shared_ptr<Bounds> addZValue(double const z);

  std::shared_ptr<Bounds>
  addIntersection(std::shared_ptr<PlanetModel> planetModel,
                  std::shared_ptr<Plane> plane1, std::shared_ptr<Plane> plane2,
                  std::deque<Membership> &bounds) override;

  std::shared_ptr<Bounds> addPoint(std::shared_ptr<GeoPoint> point) override;

  std::shared_ptr<Bounds> isWide() override;

  std::shared_ptr<Bounds> noLongitudeBound() override;

  std::shared_ptr<Bounds> noTopLatitudeBound() override;

  std::shared_ptr<Bounds> noBottomLatitudeBound() override;

  std::shared_ptr<Bounds>
  noBound(std::shared_ptr<PlanetModel> planetModel) override;

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
