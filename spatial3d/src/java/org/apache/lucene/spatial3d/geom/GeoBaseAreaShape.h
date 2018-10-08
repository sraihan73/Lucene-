#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial3d::geom
{
class PlanetModel;
}

namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
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
 * Base extended areaShape object.
 *
 * @lucene.internal
 */
class GeoBaseAreaShape : public GeoBaseMembershipShape, public GeoAreaShape
{
  GET_CLASS_NAME(GeoBaseAreaShape)

  /** Constructor.
   *@param planetModel is the planet model to use.
   */
public:
  GeoBaseAreaShape(std::shared_ptr<PlanetModel> planetModel);

  /** All edgepoints inside shape */
protected:
  static constexpr int ALL_INSIDE = 0;
  /** Some edgepoints inside shape */
  static constexpr int SOME_INSIDE = 1;
  /** No edgepoints inside shape */
  static constexpr int NONE_INSIDE = 2;

  /** Determine the relationship between the GeoAreShape and the
   * shape's edgepoints.
   *@param geoShape is the shape.
   *@return the relationship.
   */
  virtual int isShapeInsideGeoAreaShape(std::shared_ptr<GeoShape> geoShape);

  /** Determine the relationship between the GeoAreaShape's edgepoints and the
   * provided shape.
   *@param geoshape is the shape.
   *@return the relationship.
   */
  virtual int isGeoAreaShapeInsideShape(std::shared_ptr<GeoShape> geoshape);

public:
  int getRelationship(std::shared_ptr<GeoShape> geoShape) override;

protected:
  std::shared_ptr<GeoBaseAreaShape> shared_from_this()
  {
    return std::static_pointer_cast<GeoBaseAreaShape>(
        GeoBaseMembershipShape::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
