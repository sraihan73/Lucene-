#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/Vector.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/DistanceStyle.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/GeoPoint.h"
#include  "core/src/java/org/apache/lucene/spatial3d/geom/Bounds.h"

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
 * Distance shapes have capabilities of both geohashing and distance
 * computation (which also includes point membership determination).
 *
 * @lucene.experimental
 */
class GeoBaseDistanceShape : public GeoBaseAreaShape, public GeoDistanceShape
{
  GET_CLASS_NAME(GeoBaseDistanceShape)

  /** Constructor.
   *@param planetModel is the planet model to use.
   */
public:
  GeoBaseDistanceShape(std::shared_ptr<PlanetModel> planetModel);

  bool isWithin(std::shared_ptr<Vector> point) override;

  double computeDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         std::shared_ptr<GeoPoint> point) override;

  double computeDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                         double const x, double const y,
                         double const z) override;

  /** Called by a {@code computeDistance} method if X/Y/Z is within this shape.
   */
protected:
  virtual double distance(std::shared_ptr<DistanceStyle> distanceStyle,
                          double const x, double const y, double const z) = 0;

public:
  double computeDeltaDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                              std::shared_ptr<GeoPoint> point) override;

  double computeDeltaDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                              double const x, double const y,
                              double const z) override;

  /** Called by a {@code computeDeltaDistance} method if X/Y/Z is within this
   * shape. */
protected:
  virtual double deltaDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                               double const x, double const y, double const z);

public:
  void getDistanceBounds(std::shared_ptr<Bounds> bounds,
                         std::shared_ptr<DistanceStyle> distanceStyle,
                         double const distanceValue) override;

  /** Called by a {@code getDistanceBounds} method if distanceValue is not
   * Double.POSITIVE_INFINITY. */
protected:
  virtual void distanceBounds(std::shared_ptr<Bounds> bounds,
                              std::shared_ptr<DistanceStyle> distanceStyle,
                              double const distanceValue) = 0;

protected:
  std::shared_ptr<GeoBaseDistanceShape> shared_from_this()
  {
    return std::static_pointer_cast<GeoBaseDistanceShape>(
        GeoBaseAreaShape::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
