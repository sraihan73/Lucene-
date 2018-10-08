#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
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
class CircleSlice;
}
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}
namespace org::apache::lucene::spatial3d::geom
{
class DistanceStyle;
}
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
class GeoShape;
}
namespace org::apache::lucene::spatial3d::geom
{
class SidedPlane;
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
 * Circular area with a center and a radius that represents the surface distance
 * to the center. The circle is divided in sectors where the circle edge is
 * approximated using Vincenty formulae. The higher is the precision the more
 * sectors are needed to describe the shape and therefore a penalty in
 * performance.
 *
 * @lucene.experimental
 */
class GeoExactCircle : public GeoBaseCircle
{
  GET_CLASS_NAME(GeoExactCircle)
  /** Center of circle */
protected:
  const std::shared_ptr<GeoPoint> center;
  /** Radius of circle */
  const double radius;
  /** Actual accuracy */
  const double actualAccuracy;
  /** A point that is on the edge of the circle */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;
  /** Slices of the circle */
  const std::deque<std::shared_ptr<CircleSlice>> circleSlices;

  /** Constructor.
   *@param planetModel is the planet model.
   *@param lat is the center latitude.
   *@param lon is the center longitude.
   *@param radius is the surface radius for the circle.
   *@param accuracy is the allowed error value (linear distance). Maximum
   *accuracy is 1e-12.
   */
public:
  GeoExactCircle(std::shared_ptr<PlanetModel> planetModel, double const lat,
                 double const lon, double const radius, double const accuracy);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoExactCircle(std::shared_ptr<PlanetModel> planetModel,
                 std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  double getRadius() override;

  std::shared_ptr<GeoPoint> getCenter() override;

protected:
  double distance(std::shared_ptr<DistanceStyle> distanceStyle, double const x,
                  double const y, double const z) override;

  void distanceBounds(std::shared_ptr<Bounds> bounds,
                      std::shared_ptr<DistanceStyle> distanceStyle,
                      double const distanceValue) override;

  double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y,
                         double const z) override;

public:
  bool isWithin(double const x, double const y, double const z) override;

  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

  bool intersects(std::shared_ptr<Plane> p,
                  std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                  std::deque<Membership> &bounds) override;

  bool intersects(std::shared_ptr<GeoShape> geoShape) override;

  void getBounds(std::shared_ptr<Bounds> bounds) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

  /** A temporary description of a section of circle.
   */
protected:
  class ApproximationSlice
      : public std::enable_shared_from_this<ApproximationSlice>
  {
    GET_CLASS_NAME(ApproximationSlice)
  public:
    const std::shared_ptr<SidedPlane> plane;
    const std::shared_ptr<GeoPoint> endPoint1;
    const double point1Bearing;
    const std::shared_ptr<GeoPoint> endPoint2;
    const double point2Bearing;
    const std::shared_ptr<GeoPoint> middlePoint;
    const double middlePointBearing;
    const bool mustSplit;

    ApproximationSlice(std::shared_ptr<GeoPoint> center,
                       std::shared_ptr<GeoPoint> endPoint1,
                       double const point1Bearing,
                       std::shared_ptr<GeoPoint> endPoint2,
                       double const point2Bearing,
                       std::shared_ptr<GeoPoint> middlePoint,
                       double const middlePointBearing, bool const mustSplit);

    virtual std::wstring toString();
  };

  /** A  description of a section of circle.
   */
protected:
  class CircleSlice : public std::enable_shared_from_this<CircleSlice>
  {
    GET_CLASS_NAME(CircleSlice)
  public:
    std::deque<std::shared_ptr<GeoPoint>> const notableEdgePoints;
    const std::shared_ptr<SidedPlane> circlePlane;
    const std::shared_ptr<SidedPlane> plane1;
    const std::shared_ptr<SidedPlane> plane2;

    CircleSlice(std::shared_ptr<SidedPlane> circlePlane,
                std::shared_ptr<GeoPoint> endPoint1,
                std::shared_ptr<GeoPoint> endPoint2,
                std::shared_ptr<GeoPoint> center,
                std::shared_ptr<GeoPoint> check);

    virtual std::wstring toString();
  };

protected:
  std::shared_ptr<GeoExactCircle> shared_from_this()
  {
    return std::static_pointer_cast<GeoExactCircle>(
        GeoBaseCircle::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
