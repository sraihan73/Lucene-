#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <any>
#include <cmath>
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
class SidedPlane;
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
 * Circular area with a center and cutoff angle that represents the latitude and
 * longitude distance from the center where the planet will be cut. The
 * resulting area is a circle for spherical planets and an ellipse otherwise.
 *
 * @lucene.experimental
 */
class GeoStandardCircle : public GeoBaseCircle
{
  GET_CLASS_NAME(GeoStandardCircle)
  /** Center of circle */
protected:
  const std::shared_ptr<GeoPoint> center;
  /** Cutoff angle of circle (not quite the same thing as radius) */
  const double cutoffAngle;
  /** The plane describing the circle (really an ellipse on a non-spherical
   * world) */
  const std::shared_ptr<SidedPlane> circlePlane;
  /** A point that is on the world and on the circle plane */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;
  /** Notable points for a circle -- there aren't any */
  static std::deque<std::shared_ptr<GeoPoint>> const circlePoints;

  /** Constructor.
   *@param planetModel is the planet model.
   *@param lat is the center latitude.
   *@param lon is the center longitude.
   *@param cutoffAngle is the cutoff angle for the circle.
   */
public:
  GeoStandardCircle(std::shared_ptr<PlanetModel> planetModel, double const lat,
                    double const lon, double const cutoffAngle);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoStandardCircle(
      std::shared_ptr<PlanetModel> planetModel,
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

  int getRelationship(std::shared_ptr<GeoShape> geoShape) override;

  void getBounds(std::shared_ptr<Bounds> bounds) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<GeoStandardCircle> shared_from_this()
  {
    return std::static_pointer_cast<GeoStandardCircle>(
        GeoBaseCircle::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
