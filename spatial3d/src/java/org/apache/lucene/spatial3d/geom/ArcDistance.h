#pragma once
#include "stringhelper.h"
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
 * Arc distance computation style.
 *
 * @lucene.experimental
 */
class ArcDistance : public std::enable_shared_from_this<ArcDistance>,
                    public DistanceStyle
{
  GET_CLASS_NAME(ArcDistance)

  /** An instance of the ArcDistance DistanceStyle. */
public:
  static const std::shared_ptr<ArcDistance> INSTANCE;

  /** Constructor.
   */
  ArcDistance();

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
