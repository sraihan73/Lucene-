#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Vector.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/Plane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoShape.h"

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
 * Base class of a family of 3D rectangles, bounded on six sides by X,Y,Z limits
 *
 * @lucene.internal
 */
class BaseXYZSolid : public BasePlanetObject, public XYZSolid
{
  GET_CLASS_NAME(BaseXYZSolid)

  /** Unit deque in x */
protected:
  static const std::shared_ptr<Vector> xUnitVector;
  /** Unit deque in y */
  static const std::shared_ptr<Vector> yUnitVector;
  /** Unit deque in z */
  static const std::shared_ptr<Vector> zUnitVector;

  /** Vertical plane normal to x unit deque passing through origin */
  static const std::shared_ptr<Plane> xVerticalPlane;
  /** Vertical plane normal to y unit deque passing through origin */
  static const std::shared_ptr<Plane> yVerticalPlane;

  /** Empty point deque */
  static std::deque<std::shared_ptr<GeoPoint>> const EMPTY_POINTS;

  /**
   * Base solid constructor.
   *@param planetModel is the planet model.
   */
public:
  BaseXYZSolid(std::shared_ptr<PlanetModel> planetModel);

  /** Construct a single array from a number of individual arrays.
   * @param pointArrays is the array of point arrays.
   * @return the single unified array.
   */
protected:
  static std::deque<std::shared_ptr<GeoPoint>>
  glueTogether(std::deque<GeoPoint> &pointArrays);

public:
  bool isWithin(std::shared_ptr<Vector> point) override;

  bool isWithin(double const x, double const y, double const z) = 0;
  override

      // Signals for relationship of edge points to shape

      /** All edgepoints inside shape */
      protected : static constexpr int ALL_INSIDE = 0;
  /** Some edgepoints inside shape */
  static constexpr int SOME_INSIDE = 1;
  /** No edgepoints inside shape */
  static constexpr int NONE_INSIDE = 2;
  /** No edgepoints at all (means a shape that is the whole world) */
  static constexpr int NO_EDGEPOINTS = 3;

  /** Determine the relationship between this area and the provided
   * shape's edgepoints.
   *@param path is the shape.
   *@return the relationship.
   */
  virtual int isShapeInsideArea(std::shared_ptr<GeoShape> path);

  /** Determine the relationship between a shape and this area's
   * edgepoints.
   *@param path is the shape.
   *@return the relationship.
   */
  virtual int isAreaInsideShape(std::shared_ptr<GeoShape> path);

  /** Get the edge points for this shape.
   *@return the edge points.
   */
  virtual std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() = 0;

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<BaseXYZSolid> shared_from_this()
  {
    return std::static_pointer_cast<BaseXYZSolid>(
        BasePlanetObject::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
