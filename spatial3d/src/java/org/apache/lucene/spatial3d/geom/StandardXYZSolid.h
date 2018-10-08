#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class SidedPlane;
}

namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
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
 * 3D rectangle, bounded on six sides by X,Y,Z limits
 *
 * @lucene.internal
 */
class StandardXYZSolid : public BaseXYZSolid
{
  GET_CLASS_NAME(StandardXYZSolid)

  /** Min-X */
protected:
  const double minX;
  /** Max-X */
  const double maxX;
  /** Min-Y */
  const double minY;
  /** Max-Y */
  const double maxY;
  /** Min-Z */
  const double minZ;
  /** Max-Z */
  const double maxZ;

  /** Whole world? */
  const bool isWholeWorld;
  /** Min-X plane */
  const std::shared_ptr<SidedPlane> minXPlane;
  /** Max-X plane */
  const std::shared_ptr<SidedPlane> maxXPlane;
  /** Min-Y plane */
  const std::shared_ptr<SidedPlane> minYPlane;
  /** Max-Y plane */
  const std::shared_ptr<SidedPlane> maxYPlane;
  /** Min-Z plane */
  const std::shared_ptr<SidedPlane> minZPlane;
  /** Max-Z plane */
  const std::shared_ptr<SidedPlane> maxZPlane;

  /** true if minXPlane intersects globe */
  const bool minXPlaneIntersects;
  /** true if maxXPlane intersects globe */
  const bool maxXPlaneIntersects;
  /** true if minYPlane intersects globe */
  const bool minYPlaneIntersects;
  /** true if maxYPlane intersects globe */
  const bool maxYPlaneIntersects;
  /** true if minZPlane intersects globe */
  const bool minZPlaneIntersects;
  /** true if maxZPlane intersects globe */
  const bool maxZPlaneIntersects;

  /** These are the edge points of the shape, which are defined to be at least
   * one point on each surface area boundary.  In the case of a solid, this
   * includes points which represent the intersection of XYZ bounding planes and
   * the planet, as well as points representing the intersection of single
   * bounding planes with the planet itself.
   */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /** Notable points for minXPlane */
  std::deque<std::shared_ptr<GeoPoint>> const notableMinXPoints;
  /** Notable points for maxXPlane */
  std::deque<std::shared_ptr<GeoPoint>> const notableMaxXPoints;
  /** Notable points for minYPlane */
  std::deque<std::shared_ptr<GeoPoint>> const notableMinYPoints;
  /** Notable points for maxYPlane */
  std::deque<std::shared_ptr<GeoPoint>> const notableMaxYPoints;
  /** Notable points for minZPlane */
  std::deque<std::shared_ptr<GeoPoint>> const notableMinZPoints;
  /** Notable points for maxZPlane */
  std::deque<std::shared_ptr<GeoPoint>> const notableMaxZPoints;

  /**
   * Sole constructor
   *
   *@param planetModel is the planet model.
   *@param minX is the minimum X value.
   *@param maxX is the maximum X value.
   *@param minY is the minimum Y value.
   *@param maxY is the maximum Y value.
   *@param minZ is the minimum Z value.
   *@param maxZ is the maximum Z value.
   */
public:
  StandardXYZSolid(std::shared_ptr<PlanetModel> planetModel, double const minX,
                   double const maxX, double const minY, double const maxY,
                   double const minZ, double const maxZ);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  StandardXYZSolid(std::shared_ptr<PlanetModel> planetModel,
                   std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

protected:
  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

public:
  bool isWithin(double const x, double const y, double const z) override;

  int getRelationship(std::shared_ptr<GeoShape> path) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<StandardXYZSolid> shared_from_this()
  {
    return std::static_pointer_cast<StandardXYZSolid>(
        BaseXYZSolid::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
