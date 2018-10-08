#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/SidedPlane.h"

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
 * 3D rectangle, bounded on six sides by X,Y,Z limits, degenerate in Y
 *
 * @lucene.internal
 */
class XdYZSolid : public BaseXYZSolid
{
  GET_CLASS_NAME(XdYZSolid)

  /** Min-X */
protected:
  const double minX;
  /** Max-X */
  const double maxX;
  /** Y */
  const double Y;
  /** Min-Z */
  const double minZ;
  /** Max-Z */
  const double maxZ;

  /** Min-X plane */
  const std::shared_ptr<SidedPlane> minXPlane;
  /** Max-X plane */
  const std::shared_ptr<SidedPlane> maxXPlane;
  /** Y plane */
  const std::shared_ptr<Plane> yPlane;
  /** Min-Z plane */
  const std::shared_ptr<SidedPlane> minZPlane;
  /** Max-Z plane */
  const std::shared_ptr<SidedPlane> maxZPlane;

  /** These are the edge points of the shape, which are defined to be at least
   * one point on each surface area boundary.  In the case of a solid, this
   * includes points which represent the intersection of XYZ bounding planes and
   * the planet, as well as points representing the intersection of single
   * bounding planes with the planet itself.
   */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /** Notable points for YPlane */
  std::deque<std::shared_ptr<GeoPoint>> const notableYPoints;

  /**
   * Sole constructor
   *
   *@param planetModel is the planet model.
   *@param minX is the minimum X value.
   *@param maxX is the maximum X value.
   *@param Y is the Y value.
   *@param minZ is the minimum Z value.
   *@param maxZ is the maximum Z value.
   */
public:
  XdYZSolid(std::shared_ptr<PlanetModel> planetModel, double const minX,
            double const maxX, double const Y, double const minZ,
            double const maxZ);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  XdYZSolid(std::shared_ptr<PlanetModel> planetModel,
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
  std::shared_ptr<XdYZSolid> shared_from_this()
  {
    return std::static_pointer_cast<XdYZSolid>(
        BaseXYZSolid::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
