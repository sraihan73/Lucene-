#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class GeoCircle;
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
 * Class which constructs a GeoCircle representing an arbitrary circle.
 *
 * @lucene.experimental
 */
class GeoCircleFactory : public std::enable_shared_from_this<GeoCircleFactory>
{
  GET_CLASS_NAME(GeoCircleFactory)
private:
  GeoCircleFactory();

  /**
   * Create a GeoCircle from a center and a cutoff angle. The resulting shape is
   * a circle in spherical planets, otherwise is an ellipse. It is the most
   * efficient shape to represent a circle on a sphere.
   *
   * @param planetModel is the planet model.
   * @param latitude is the center latitude.
   * @param longitude is the center longitude.
   * @param cutoffAngle is the cutoff angle.
   * @return a GeoCircle corresponding to what was specified.
   */
public:
  static std::shared_ptr<GeoCircle>
  makeGeoCircle(std::shared_ptr<PlanetModel> planetModel, double const latitude,
                double const longitude, double const cutoffAngle);

  /**
   * Create an GeoCircle from a center, a radius and a desired accuracy. It is
   * the most accurate shape to represent a circle in non-spherical planets. <p>
   * The accuracy of the circle is defined as the maximum linear distance
   * between any point on the surface circle and planes that describe the
   * circle. Therefore, with planet model WSG84, since the radius of earth is
   * 6,371,000 meters, an accuracy of 1e-6 corresponds to 6.3 meters. For an
   * accuracy of 1.0 meters, the accuracy value would be 1.6e-7. The maximum
   * accuracy possible is 1e-12. <p> Note that this method may thrown an
   * IllegalArgumentException if the circle being specified cannot be
   * represented by plane approximation given the planet model provided.
   *
   * @param planetModel is the planet model.
   * @param latitude is the center latitude.
   * @param longitude is the center longitude.
   * @param radius is the radius surface distance.
   * @param accuracy is the maximum linear distance between the circle
   * approximation and the real circle, as computed using the Vincenty formula.
   * @return a GeoCircle corresponding to what was specified.
   */
  static std::shared_ptr<GeoCircle>
  makeExactGeoCircle(std::shared_ptr<PlanetModel> planetModel,
                     double const latitude, double const longitude,
                     double const radius, double const accuracy);
};

} // namespace org::apache::lucene::spatial3d::geom
