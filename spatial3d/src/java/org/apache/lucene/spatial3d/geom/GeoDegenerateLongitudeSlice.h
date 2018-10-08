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
class Plane;
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
 * Degenerate longitude slice.
 *
 * @lucene.internal
 */
class GeoDegenerateLongitudeSlice : public GeoBaseBBox
{
  GET_CLASS_NAME(GeoDegenerateLongitudeSlice)
  /** The longitude of the slice */
protected:
  const double longitude;

  /** The bounding plane for the slice (through both poles, perpendicular to the
   * slice) */
  const std::shared_ptr<SidedPlane> boundingPlane;
  /** The plane of the slice */
  const std::shared_ptr<Plane> plane;
  /** A point on the slice */
  const std::shared_ptr<GeoPoint> interiorPoint;
  /** An array consisting of the one point chosen on the slice */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;
  /** Notable points for the slice (north and south poles) */
  std::deque<std::shared_ptr<GeoPoint>> const planePoints;

  /**
   * Accepts only values in the following ranges: lon: {@code -PI -> PI}
   */
public:
  GeoDegenerateLongitudeSlice(std::shared_ptr<PlanetModel> planetModel,
                              double const longitude);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoDegenerateLongitudeSlice(
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

  int getRelationship(std::shared_ptr<GeoShape> path) override;

protected:
  double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y,
                         double const z) override;

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

protected:
  std::shared_ptr<GeoDegenerateLongitudeSlice> shared_from_this()
  {
    return std::static_pointer_cast<GeoDegenerateLongitudeSlice>(
        GeoBaseBBox::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
