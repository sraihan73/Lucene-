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
class EitherBound;
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
namespace org::apache::lucene::spatial3d::geom
{
class Vector;
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
 * Bounding box wider than PI but limited on four sides (top lat,
 * bottom lat, left lon, right lon).
 *
 * @lucene.internal
 */
class GeoWideRectangle : public GeoBaseBBox
{
  GET_CLASS_NAME(GeoWideRectangle)
  /** The top latitude */
protected:
  const double topLat;
  /** The bottom latitude */
  const double bottomLat;
  /** The left longitude */
  const double leftLon;
  /** The right longitude */
  const double rightLon;

  /** Cosine of the middle latitude */
  const double cosMiddleLat;

  /** Upper left hand corner point */
  const std::shared_ptr<GeoPoint> ULHC;
  /** Lower right hand corner point */
  const std::shared_ptr<GeoPoint> URHC;
  /** Lower right hand corner point */
  const std::shared_ptr<GeoPoint> LRHC;
  /** Lower left hand corner point */
  const std::shared_ptr<GeoPoint> LLHC;

  /** Top plane */
  const std::shared_ptr<SidedPlane> topPlane;
  /** Bottom plane */
  const std::shared_ptr<SidedPlane> bottomPlane;
  /** Left plane */
  const std::shared_ptr<SidedPlane> leftPlane;
  /** Right plane */
  const std::shared_ptr<SidedPlane> rightPlane;

  /** Top plane's notable points */
  std::deque<std::shared_ptr<GeoPoint>> const topPlanePoints;
  /** Bottom plane's notable points */
  std::deque<std::shared_ptr<GeoPoint>> const bottomPlanePoints;
  /** Left plane's notable points */
  std::deque<std::shared_ptr<GeoPoint>> const leftPlanePoints;
  /** Right plane's notable points */
  std::deque<std::shared_ptr<GeoPoint>> const rightPlanePoints;

  /** Center point */
  const std::shared_ptr<GeoPoint> centerPoint;

  /** Combined left/right bounds */
  const std::shared_ptr<EitherBound> eitherBound;

  /** A point on the edge */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /**
   * Accepts only values in the following ranges: lat: {@code -PI/2 -> PI/2},
   * lon: {@code -PI -> PI}. Horizontal angle must be greater than or equal to
   * PI.
   * @param planetModel is the planet model.
   * @param topLat is the top latitude.
   * @param bottomLat is the bottom latitude.
   * @param leftLon is the left longitude.
   * @param rightLon is the right longitude.
   */
public:
  GeoWideRectangle(std::shared_ptr<PlanetModel> planetModel,
                   double const topLat, double const bottomLat,
                   double const leftLon, double rightLon);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoWideRectangle(std::shared_ptr<PlanetModel> planetModel,
                   std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  std::shared_ptr<GeoBBox> expand(double const angle) override;

  bool isWithin(double const x, double const y, double const z) override;

  double getRadius() override;

  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

  /**
   * Returns the center of a circle into which the area will be inscribed.
   *
   * @return the center.
   */
  std::shared_ptr<GeoPoint> getCenter() override;

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

  /** A membership implementation representing a wide (more than 180) left/right
   * bound.
   */
protected:
  class EitherBound : public std::enable_shared_from_this<EitherBound>,
                      public Membership
  {
    GET_CLASS_NAME(EitherBound)
  private:
    std::shared_ptr<GeoWideRectangle> outerInstance;

    /** Constructor.
     */
  public:
    EitherBound(std::shared_ptr<GeoWideRectangle> outerInstance);

    bool isWithin(std::shared_ptr<Vector> v) override;

    bool isWithin(double const x, double const y, double const z) override;
  };

protected:
  std::shared_ptr<GeoWideRectangle> shared_from_this()
  {
    return std::static_pointer_cast<GeoWideRectangle>(
        GeoBaseBBox::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
