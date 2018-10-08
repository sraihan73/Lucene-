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
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/SidedPlane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/EitherBound.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoBBox.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Membership.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Plane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Bounds.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/DistanceStyle.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Vector.h"

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
 * Bounding box wider than PI but limited on three sides (
 * bottom lat, left lon, right lon).
 *
 * @lucene.internal
 */
class GeoWideNorthRectangle : public GeoBaseBBox
{
  GET_CLASS_NAME(GeoWideNorthRectangle)
  /** Bottom latitude */
protected:
  const double bottomLat;
  /** Left longitude */
  const double leftLon;
  /** Right longitude */
  const double rightLon;

  /** The cosine of the middle latitude */
  const double cosMiddleLat;

  /** The lower right hand corner point */
  const std::shared_ptr<GeoPoint> LRHC;
  /** The lower left hand corner point */
  const std::shared_ptr<GeoPoint> LLHC;

  /** The bottom plane */
  const std::shared_ptr<SidedPlane> bottomPlane;
  /** The left plane */
  const std::shared_ptr<SidedPlane> leftPlane;
  /** The right plane */
  const std::shared_ptr<SidedPlane> rightPlane;

  /** Notable points for the bottom plane */
  std::deque<std::shared_ptr<GeoPoint>> const bottomPlanePoints;
  /** Notable points for the left plane */
  std::deque<std::shared_ptr<GeoPoint>> const leftPlanePoints;
  /** Notable points for the right plane */
  std::deque<std::shared_ptr<GeoPoint>> const rightPlanePoints;

  /** Center point */
  const std::shared_ptr<GeoPoint> centerPoint;

  /** Composite left/right bounds */
  const std::shared_ptr<EitherBound> eitherBound;

  /** A point on the edge */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /**
   * Accepts only values in the following ranges: lat: {@code -PI/2 -> PI/2},
   * lon: {@code -PI -> PI}. Horizontal angle must be greater than or equal to
   * PI.
   * @param planetModel is the planet model.
   * @param bottomLat is the bottom latitude.
   * @param leftLon is the left longitude.
   * @param rightLon is the right longitude.
   */
public:
  GeoWideNorthRectangle(std::shared_ptr<PlanetModel> planetModel,
                        double const bottomLat, double const leftLon,
                        double rightLon);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoWideNorthRectangle(
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

  /** Membership implementation representing a wide (more than 180 degree)
   * bound.
   */
protected:
  class EitherBound : public std::enable_shared_from_this<EitherBound>,
                      public Membership
  {
    GET_CLASS_NAME(EitherBound)
  private:
    std::shared_ptr<GeoWideNorthRectangle> outerInstance;

    /** Constructor.
     */
  public:
    EitherBound(std::shared_ptr<GeoWideNorthRectangle> outerInstance);

    bool isWithin(std::shared_ptr<Vector> v) override;

    bool isWithin(double const x, double const y, double const z) override;
  };

protected:
  std::shared_ptr<GeoWideNorthRectangle> shared_from_this()
  {
    return std::static_pointer_cast<GeoWideNorthRectangle>(
        GeoBaseBBox::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
