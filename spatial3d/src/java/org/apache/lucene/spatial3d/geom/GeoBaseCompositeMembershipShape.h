#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <type_traits>
#include <typeinfo>

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
 * Base class to create a composite of GeoMembershipShapes
 *
 * @param <T> is the type of GeoMembershipShapes of the composite.
 * @lucene.internal
 */
template <typename T>
class GeoBaseCompositeMembershipShape : public GeoBaseCompositeShape<T>,
                                        public GeoMembershipShape
{
  GET_CLASS_NAME(GeoBaseCompositeMembershipShape)
  static_assert(std::is_base_of<GeoMembershipShape, T>::value,
                L"T must inherit from GeoMembershipShape");

  /**
   * Constructor.
   */
public:
  GeoBaseCompositeMembershipShape(std::shared_ptr<PlanetModel> planetModel)
      : GeoBaseCompositeShape<T>(planetModel)
  {
  }

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   * @param clazz is the class of the generic.
   */
  GeoBaseCompositeMembershipShape(std::shared_ptr<PlanetModel> planetModel,
                                  std::shared_ptr<InputStream> inputStream,
                                  std::type_info<T> &clazz) 
      : GeoBaseCompositeShape<T>(planetModel, inputStream, clazz)
  {
  }

  double computeOutsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                std::shared_ptr<GeoPoint> point) override
  {
    return computeOutsideDistance(distanceStyle, point->x, point->y, point->z);
  }

  double computeOutsideDistance(std::shared_ptr<DistanceStyle> distanceStyle,
                                double const x, double const y,
                                double const z) override
  {
    if (isWithin(x, y, z)) {
      return 0.0;
    }
    double distance = std::numeric_limits<double>::infinity();
    for (std::shared_ptr<GeoMembershipShape> shape : shapes) {
      constexpr double normalDistance =
          shape->computeOutsideDistance(distanceStyle, x, y, z);
      if (normalDistance < distance) {
        distance = normalDistance;
      }
    }
    return distance;
  }

protected:
  std::shared_ptr<GeoBaseCompositeMembershipShape> shared_from_this()
  {
    return std::static_pointer_cast<GeoBaseCompositeMembershipShape>(
        GeoBaseCompositeShape<T>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
