#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <any>
#include <cmath>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/SidedPlane.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"
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
 * This GeoBBox represents an area rectangle limited only in north latitude.
 *
 * @lucene.internal
 */
class GeoSouthLatitudeZone : public GeoBaseBBox
{
  GET_CLASS_NAME(GeoSouthLatitudeZone)
  /** The top latitude of the zone */
protected:
  const double topLat;
  /** The cosine of the top latitude of the zone */
  const double cosTopLat;
  /** The top plane of the zone */
  const std::shared_ptr<SidedPlane> topPlane;
  /** An interior point of the zone */
  const std::shared_ptr<GeoPoint> interiorPoint;
  /** Notable points for the plane (none) */
  static std::deque<std::shared_ptr<GeoPoint>> const planePoints;
  /** A point on the top boundary */
  const std::shared_ptr<GeoPoint> topBoundaryPoint;
  /** Edge points; a reference to the topBoundaryPoint */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /** Constructor.
   *@param planetModel is the planet model.
   *@param topLat is the top latitude of the zone.
   */
public:
  GeoSouthLatitudeZone(std::shared_ptr<PlanetModel> planetModel,
                       double const topLat);

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   */
  GeoSouthLatitudeZone(
      std::shared_ptr<PlanetModel> planetModel,
      std::shared_ptr<InputStream> inputStream) ;

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  std::shared_ptr<GeoBBox> expand(double const angle) override;

  bool isWithin(double const x, double const y, double const z) override;

  double getRadius() override;

  /**
   * Returns the center of a circle into which the area will be inscribed.
   *
   * @return the center.
   */
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
  std::shared_ptr<GeoSouthLatitudeZone> shared_from_this()
  {
    return std::static_pointer_cast<GeoSouthLatitudeZone>(
        GeoBaseBBox::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
