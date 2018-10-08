#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}

namespace org::apache::lucene::spatial3d::geom
{
class Vector;
}
namespace org::apache::lucene::spatial3d::geom
{
class DistanceStyle;
}
namespace org::apache::lucene::spatial3d::geom
{
class GeoPoint;
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
 * Membership shapes have capabilities of both geohashing and membership
 * determination.  This is a useful baseclass for them.
 *
 * @lucene.experimental
 */
class GeoBaseMembershipShape : public GeoBaseShape, public GeoMembershipShape
{
  GET_CLASS_NAME(GeoBaseMembershipShape)

  /** Constructor.
   *@param planetModel is the planet model to use.
   */
public:
  GeoBaseMembershipShape(std::shared_ptr<PlanetModel> planetModel);

  bool isWithin(std::shared_ptr<Vector> point) override;

  double computeOutsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                std::shared_ptr<GeoPoint> point) override;

  double computeOutsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                double const x, double const y,
                                double const z) override;

  /** Called by a {@code computeOutsideDistance} method if X/Y/Z is not within
   * this shape. */
protected:
  virtual double outsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                 double const x, double const y,
                                 double const z) = 0;

protected:
  std::shared_ptr<GeoBaseMembershipShape> shared_from_this()
  {
    return std::static_pointer_cast<GeoBaseMembershipShape>(
        GeoBaseShape::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
