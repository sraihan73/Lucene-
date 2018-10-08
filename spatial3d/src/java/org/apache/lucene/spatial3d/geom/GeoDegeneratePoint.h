#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"
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
 * This class represents a degenerate point bounding box.
 * It is not a simple GeoPoint because we must have the latitude and longitude.
 *
 * @lucene.internal
 */
class GeoDegeneratePoint : public GeoPoint, public GeoPointShape
{
  GET_CLASS_NAME(GeoDegeneratePoint)
  /** Current planet model, since we don't extend BasePlanetObject */
protected:
  const std::shared_ptr<PlanetModel> planetModel;
  /** Edge point is an area containing just this */
  std::deque<std::shared_ptr<GeoPoint>> const edgePoints;

  /** Constructor.
   *@param planetModel is the planet model to use.
   *@param lat is the latitude.
   *@param lon is the longitude.
   */
public:
  GeoDegeneratePoint(std::shared_ptr<PlanetModel> planetModel, double const lat,
                     double const lon);

  /** Constructor for deserialization.
   *@param planetModel is the planet model to use.
   *@param inputStream is the input stream.
   */
  GeoDegeneratePoint(
      std::shared_ptr<PlanetModel> planetModel,
      std::shared_ptr<InputStream> inputStream) ;

  std::shared_ptr<PlanetModel> getPlanetModel() override;

  std::shared_ptr<GeoBBox> expand(double const angle) override;

  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override;

  bool intersects(std::shared_ptr<Plane> plane,
                  std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                  std::deque<Membership> &bounds) override;

  bool intersects(std::shared_ptr<GeoShape> geoShape) override;

  void getBounds(std::shared_ptr<Bounds> bounds) override;

  double computeOutsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                std::shared_ptr<GeoPoint> point) override;

  double computeOutsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                double const x, double const y,
                                double const z) override;

  virtual bool equals(std::any o);

  virtual std::wstring toString();

  bool isWithin(std::shared_ptr<Vector> point) override;

  bool isWithin(double const x, double const y, double const z) override;

  double getRadius() override;

  std::shared_ptr<GeoPoint> getCenter() override;

  int getRelationship(std::shared_ptr<GeoShape> shape) override;

  double computeDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y,
                         double const z) override;

  void getDistanceBounds(std::shared_ptr<Bounds> bounds,
                         std::shared_ptr<DistanceStyle> distanceStyle,
                         double const distanceValue) override;

protected:
  std::shared_ptr<GeoDegeneratePoint> shared_from_this()
  {
    return std::static_pointer_cast<GeoDegeneratePoint>(
        GeoPoint::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
