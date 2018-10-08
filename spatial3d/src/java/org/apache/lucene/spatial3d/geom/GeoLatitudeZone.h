#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <any>
#include <cmath>
#include <memory>
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
 * This GeoBBox represents an area rectangle limited only in latitude.
 *
 * @lucene.internal
 */
class GeoLatitudeZone : public GeoBaseBBox
{
  GET_CLASS_NAME(GeoLatitudeZone)
  /** The top latitude of the zone */
protected:
  const double topLat;
  /** The bottom latitude of the zone */
  const double bottomLat;
  /** Cosine of the top lat */
  const double cosTopLat;
  /** Cosine of the bottom lat */
  const double cosBottomLat;
  /** The top plane */
  const std::shared_ptr<SidedPlane> topPlane;
  /** The bottom plane */
  const std::shared_ptr<SidedPlane> bottomPlane;
  /** An interior point */
  const std::shared_ptr<GeoPoint> interiorPoint;
  /** Notable points (none) */
  static std::deque<std::shared_ptr<GeoPoint>> const planePoints;

  // We need two additional points because a latitude zone's boundaries don't
  // intersect.  This is a very special case that most GeoBBox's do not have.

  /** Top boundary point */
  const std::shared_ptr<GeoPoint> topBoundaryPoint;
  /** Bottom boundary point */
  const std::shared_ptr<GeoPoint> bottomBoundaryPoint;
  /** A point on each distinct edge */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /** Constructor.
   *@param planetModel is the planet model to use.
   *@param topLat is the top latitude.
   *@param bottomLat is the bottom latitude.
   */
public:
  GeoLatitudeZone(std::shared_ptr<PlanetModel> planetModel, double const topLat,
                  double const bottomLat);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoLatitudeZone(std::shared_ptr<PlanetModel> planetModel,
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
  std::shared_ptr<GeoLatitudeZone> shared_from_this()
  {
    return std::static_pointer_cast<GeoLatitudeZone>(
        GeoBaseBBox::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
