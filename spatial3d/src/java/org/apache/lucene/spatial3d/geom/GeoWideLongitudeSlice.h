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
class GeoBBox;
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
 * Bounding box wider than PI but limited on left and right sides (
 * left lon, right lon).
 *
 * @lucene.internal
 */
class GeoWideLongitudeSlice : public GeoBaseBBox
{
  GET_CLASS_NAME(GeoWideLongitudeSlice)
  /** The left longitude */
protected:
  const double leftLon;
  /** The right longitude */
  const double rightLon;

  /** The left plane */
  const std::shared_ptr<SidedPlane> leftPlane;
  /** The right plane */
  const std::shared_ptr<SidedPlane> rightPlane;

  /** Notable points for the shape */
  std::deque<std::shared_ptr<GeoPoint>> const planePoints;

  /** Center point for the shape */
  const std::shared_ptr<GeoPoint> centerPoint;

  /** A point on the edge of the shape */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /**
   * Accepts only values in the following ranges: lon: {@code -PI -> PI}.
   * Horizantal angle must be greater than or equal to PI.
   *@param planetModel is the planet model.
   *@param leftLon is the left longitude.
   *@param rightLon is the right longitude.
   */
public:
  GeoWideLongitudeSlice(std::shared_ptr<PlanetModel> planetModel,
                        double const leftLon, double rightLon);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoWideLongitudeSlice(
      std::shared_ptr<PlanetModel> planetModel,
      std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  std::shared_ptr<GeoBBox> expand(double const angle) override;

  bool isWithin(double const x, double const y, double const z) override;

  double getRadius() override;

  std::shared_ptr<GeoPoint> getCenter() override;

  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

  bool intersects(std::shared_ptr<Plane> p,
                  std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                  std::deque<Membership> &bounds) override;

  bool intersects(std::shared_ptr<GeoShape> geoShape) override;

  void getBounds(std::shared_ptr<Bounds> bounds) override;

protected:
  double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y,
                         double const z) override;

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<GeoWideLongitudeSlice> shared_from_this()
  {
    return std::static_pointer_cast<GeoWideLongitudeSlice>(
        GeoBaseBBox::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
