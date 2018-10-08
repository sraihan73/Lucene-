#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
}

namespace org::apache::lucene::spatial3d::geom
{
class SidedPlane;
}
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
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
class GeoShape;
}
namespace org::apache::lucene::spatial3d::geom
{
class Bounds;
}
namespace org::apache::lucene::spatial3d::geom
{
class DistanceStyle;
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
 * Fast implementation of a polygon representing S2 geometry cell. There are no
 * checks validating that points are convex therefore users must be provide four
 * points in CCW or the logic will fail.
 *
 * @lucene.internal
 */
class GeoS2Shape : public GeoBasePolygon
{
  GET_CLASS_NAME(GeoS2Shape)

  /** The first point */
protected:
  const std::shared_ptr<GeoPoint> point1;
  /** The second point */
  const std::shared_ptr<GeoPoint> point2;
  /** The third point */
  const std::shared_ptr<GeoPoint> point3;
  /** The fourth point */
  const std::shared_ptr<GeoPoint> point4;

  /** The first plane */
  const std::shared_ptr<SidedPlane> plane1;
  /** The second plane */
  const std::shared_ptr<SidedPlane> plane2;
  /** The third plane */
  const std::shared_ptr<SidedPlane> plane3;
  /** The fourth plane */
  const std::shared_ptr<SidedPlane> plane4;

  /** Notable points for the first plane */
  std::deque<std::shared_ptr<GeoPoint>> const plane1Points;
  /** Notable points for second plane */
  std::deque<std::shared_ptr<GeoPoint>> const plane2Points;
  /** Notable points for third plane */
  std::deque<std::shared_ptr<GeoPoint>> const plane3Points;
  /** Notable points for fourth plane */
  std::deque<std::shared_ptr<GeoPoint>> const plane4Points;

  /** Edge point for this S2 cell */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /**
   * It builds from 4 points given in CCW. It must be convex or logic will fail.
   *
   *@param planetModel is the planet model.
   *@param point1  the first point.
   *@param point2  the second point.
   *@param point3  the third point.
   *@param point4  the four point.
   */
public:
  GeoS2Shape(std::shared_ptr<PlanetModel> planetModel,
             std::shared_ptr<GeoPoint> point1, std::shared_ptr<GeoPoint> point2,
             std::shared_ptr<GeoPoint> point3,
             std::shared_ptr<GeoPoint> point4);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoS2Shape(std::shared_ptr<PlanetModel> planetModel,
             std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  bool isWithin(double const x, double const y, double const z) override;

  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

  bool intersects(std::shared_ptr<Plane> p,
                  std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                  std::deque<Membership> &bounds) override;

  bool intersects(std::shared_ptr<GeoShape> geoShape) override;

  void getBounds(std::shared_ptr<Bounds> bounds) override;

  double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle, double x,
                         double y, double z) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<GeoS2Shape> shared_from_this()
  {
    return std::static_pointer_cast<GeoS2Shape>(
        GeoBasePolygon::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
