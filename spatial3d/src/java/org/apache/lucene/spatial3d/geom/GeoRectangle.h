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
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoBBox.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Membership.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Plane.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoShape.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Bounds.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/DistanceStyle.h"

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
 * Bounding box limited on four sides (top lat, bottom lat, left lon, right
 * lon). The left-right maximum extent for this shape is PI; for anything
 * larger, use GeoWideRectangle.
 *
 * @lucene.internal
 */
class GeoRectangle : public GeoBaseBBox
{
  GET_CLASS_NAME(GeoRectangle)
  /** The top latitude of the rect */
protected:
  const double topLat;
  /** The bottom latitude of the rect */
  const double bottomLat;
  /** The left longitude of the rect */
  const double leftLon;
  /** The right longitude of the rect */
  const double rightLon;
  /** The cosine of a middle latitude */
  const double cosMiddleLat;

  /** The upper left hand corner point */
  const std::shared_ptr<GeoPoint> ULHC;
  /** The upper right hand corner point */
  const std::shared_ptr<GeoPoint> URHC;
  /** The lower right hand corner point */
  const std::shared_ptr<GeoPoint> LRHC;
  /** The lower left hand corner point */
  const std::shared_ptr<GeoPoint> LLHC;

  /** The top plane */
  const std::shared_ptr<SidedPlane> topPlane;
  /** The bottom plane */
  const std::shared_ptr<SidedPlane> bottomPlane;
  /** The left plane */
  const std::shared_ptr<SidedPlane> leftPlane;
  /** The right plane */
  const std::shared_ptr<SidedPlane> rightPlane;

  /** Notable points for the top plane */
  std::deque<std::shared_ptr<GeoPoint>> const topPlanePoints;
  /** Notable points for the bottom plane */
  std::deque<std::shared_ptr<GeoPoint>> const bottomPlanePoints;
  /** Notable points for the left plane */
  std::deque<std::shared_ptr<GeoPoint>> const leftPlanePoints;
  /** Notable points for the right plane */
  std::deque<std::shared_ptr<GeoPoint>> const rightPlanePoints;

  /** Center point */
  const std::shared_ptr<GeoPoint> centerPoint;

  /** Edge point for this rectangle */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /**
   * Accepts only values in the following ranges: lat: {@code -PI/2 -> PI/2},
   *lon: {@code -PI -> PI}
   *@param planetModel is the planet model.
   *@param topLat is the top latitude.
   *@param bottomLat is the bottom latitude.
   *@param leftLon is the left longitude.
   *@param rightLon is the right longitude.
   */
public:
  GeoRectangle(std::shared_ptr<PlanetModel> planetModel, double const topLat,
               double const bottomLat, double const leftLon, double rightLon);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoRectangle(std::shared_ptr<PlanetModel> planetModel,
               std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  std::shared_ptr<GeoBBox> expand(double const angle) override;

  bool isWithin(double const x, double const y, double const z) override;

  double getRadius() override;

  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

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

protected:
  std::shared_ptr<GeoRectangle> shared_from_this()
  {
    return std::static_pointer_cast<GeoRectangle>(
        GeoBaseBBox::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
