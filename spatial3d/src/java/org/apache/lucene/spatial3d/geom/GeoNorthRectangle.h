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
 * Bounding box limited on three sides (bottom lat, left lon, right lon),
 * including the north pole. The left-right maximum extent for this shape is PI;
 * for anything larger, use
 * {@link GeoWideNorthRectangle}.
 *
 * @lucene.internal
 */
class GeoNorthRectangle : public GeoBaseBBox
{
  GET_CLASS_NAME(GeoNorthRectangle)
  /** The bottom latitude of the rectangle */
protected:
  const double bottomLat;
  /** The left longitude */
  const double leftLon;
  /** The right longitude */
  const double rightLon;
  /** Cosine of the middle latitude */
  const double cosMiddleLat;
  /** Lower right hand corner point */
  const std::shared_ptr<GeoPoint> LRHC;
  /** Lower left hand corner point */
  const std::shared_ptr<GeoPoint> LLHC;
  /** Bottom edge plane */
  const std::shared_ptr<SidedPlane> bottomPlane;
  /** Left-side plane */
  const std::shared_ptr<SidedPlane> leftPlane;
  /** Right-side plane */
  const std::shared_ptr<SidedPlane> rightPlane;
  /** Bottom plane notable points */
  std::deque<std::shared_ptr<GeoPoint>> const bottomPlanePoints;
  /** Left plane notable points */
  std::deque<std::shared_ptr<GeoPoint>> const leftPlanePoints;
  /** Right plane notable points */
  std::deque<std::shared_ptr<GeoPoint>> const rightPlanePoints;
  /** Center point */
  const std::shared_ptr<GeoPoint> centerPoint;
  /** A point on the edge */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /**
   * Accepts only values in the following ranges: lat: {@code -PI/2 -> PI/2},
   *lon: {@code -PI -> PI}
   *@param planetModel is the planet model.
   *@param bottomLat is the bottom latitude.
   *@param leftLon is the left longitude.
   *@param rightLon is the right longitude.
   */
public:
  GeoNorthRectangle(std::shared_ptr<PlanetModel> planetModel,
                    double const bottomLat, double const leftLon,
                    double rightLon);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoNorthRectangle(
      std::shared_ptr<PlanetModel> planetModel,
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

protected:
  std::shared_ptr<GeoNorthRectangle> shared_from_this()
  {
    return std::static_pointer_cast<GeoNorthRectangle>(
        GeoBaseBBox::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
