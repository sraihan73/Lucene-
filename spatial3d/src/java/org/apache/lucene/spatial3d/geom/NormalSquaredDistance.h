#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
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
class PlanetModel;
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
 * Normal squared distance computation style.
 *
 * @lucene.experimental
 */
class NormalSquaredDistance
    : public std::enable_shared_from_this<NormalSquaredDistance>,
      public DistanceStyle
{
  GET_CLASS_NAME(NormalSquaredDistance)

  /** A convenient instance */
public:
  static const std::shared_ptr<NormalSquaredDistance> INSTANCE;

  /** Constructor.
   */
  NormalSquaredDistance();

  double computeDistance(std::shared_ptr<GeoPoint> point1,
                         std::shared_ptr<GeoPoint> point2) override;

  double computeDistance(std::shared_ptr<GeoPoint> point1, double const x2,
                         double const y2, double const z2) override;

  double computeDistance(std::shared_ptr<PlanetModel> planetModel,
                         std::shared_ptr<Plane> plane,
                         std::shared_ptr<GeoPoint> point,
                         std::deque<Membership> &bounds) override;

  double computeDistance(std::shared_ptr<PlanetModel> planetModel,
                         std::shared_ptr<Plane> plane, double const x,
                         double const y, double const z,
                         std::deque<Membership> &bounds) override;

  double toAggregationForm(double const distance) override;

  double fromAggregationForm(double const aggregateDistance) override;

  std::deque<std::shared_ptr<GeoPoint>> findDistancePoints(
      std::shared_ptr<PlanetModel> planetModel, double const distanceValue,
      std::shared_ptr<GeoPoint> startPoint, std::shared_ptr<Plane> plane,
      std::deque<Membership> &bounds) override;

  double findMinimumArcDistance(std::shared_ptr<PlanetModel> planetModel,
                                double const distanceValue) override;

  double findMaximumArcDistance(std::shared_ptr<PlanetModel> planetModel,
                                double const distanceValue) override;
};

} // namespace org::apache::lucene::spatial3d::geom
