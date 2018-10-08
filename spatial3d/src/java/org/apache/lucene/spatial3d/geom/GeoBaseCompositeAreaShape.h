#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
 * Base class to create a composite of GeoAreaShapes
 *
 * @param <T> is the type of GeoAreaShapes of the composite.
 * @lucene.internal
 */
template <typename T>
class GeoBaseCompositeAreaShape : public GeoBaseCompositeMembershipShape<T>,
                                  public GeoAreaShape
{
  GET_CLASS_NAME(GeoBaseCompositeAreaShape)
  static_assert(std::is_base_of<GeoAreaShape, T>::value,
                L"T must inherit from GeoAreaShape");

  /** All edgepoints inside shape */
protected:
  static constexpr int ALL_INSIDE = 0;
  /** Some edgepoints inside shape */
  static constexpr int SOME_INSIDE = 1;
  /** No edgepoints inside shape */
  static constexpr int NONE_INSIDE = 2;

  /**
   * Constructor.
   */
public:
  GeoBaseCompositeAreaShape(std::shared_ptr<PlanetModel> planetModel)
      : GeoBaseCompositeMembershipShape<T>(planetModel)
  {
  }

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   * @param clazz is the class of the generic.
   */
  GeoBaseCompositeAreaShape(std::shared_ptr<PlanetModel> planetModel,
                            std::shared_ptr<InputStream> inputStream,
                            std::type_info<T> &clazz) 
      : GeoBaseCompositeMembershipShape<T>(planetModel, inputStream, clazz)
  {
  }

  bool intersects(std::shared_ptr<GeoShape> geoShape) override
  {
    for (std::shared_ptr<GeoAreaShape> geoAreaShape : shapes) {
      if (geoAreaShape->intersects(geoShape)) {
        return true;
      }
    }
    return false;
  }

  int getRelationship(std::shared_ptr<GeoShape> geoShape) override
  {
    if (!geoShape->getPlanetModel()->equals(planetModel)) {
      throw std::invalid_argument(
          "Cannot relate shapes with different planet models.");
    }
    constexpr int insideGeoAreaShape = isShapeInsideGeoAreaShape(geoShape);
    if (insideGeoAreaShape == SOME_INSIDE) {
      return GeoArea::OVERLAPS;
    }

    constexpr int insideShape = isGeoAreaShapeInsideShape(geoShape);
    if (insideShape == SOME_INSIDE) {
      return GeoArea::OVERLAPS;
    }

    if (insideGeoAreaShape == ALL_INSIDE && insideShape == ALL_INSIDE) {
      return GeoArea::OVERLAPS;
    }

    if (intersects(geoShape)) {
      return GeoArea::OVERLAPS;
    }

    if (insideGeoAreaShape == ALL_INSIDE) {
      return GeoArea::WITHIN;
    }

    if (insideShape == ALL_INSIDE) {
      return GeoArea::CONTAINS;
    }

    return GeoArea::DISJOINT;
  }

  /** Determine the relationship between the GeoAreShape and the
   * shape's edgepoints.
   *@param geoShape is the shape.
   *@return the relationship.
   */
protected:
  virtual int isShapeInsideGeoAreaShape(std::shared_ptr<GeoShape> geoShape)
  {
    bool foundOutside = false;
    bool foundInside = false;
    for (auto p : geoShape->getEdgePoints()) {
      if (isWithin(p)) {
        foundInside = true;
      } else {
        foundOutside = true;
      }
      if (foundInside && foundOutside) {
        return SOME_INSIDE;
      }
    }
    if (!foundInside && !foundOutside) {
      return NONE_INSIDE;
    }
    if (foundInside && !foundOutside) {
      return ALL_INSIDE;
    }
    if (foundOutside && !foundInside) {
      return NONE_INSIDE;
    }
    return SOME_INSIDE;
  }

  /** Determine the relationship between the GeoAreShape's edgepoints and the
   * provided shape.
   *@param geoshape is the shape.
   *@return the relationship.
   */
  virtual int isGeoAreaShapeInsideShape(std::shared_ptr<GeoShape> geoshape)
  {
    bool foundOutside = false;
    bool foundInside = false;
    for (std::shared_ptr<GeoPoint> p : getEdgePoints()) {
      if (geoshape->isWithin(p)) {
        foundInside = true;
      } else {
        foundOutside = true;
      }
      if (foundInside && foundOutside) {
        return SOME_INSIDE;
      }
    }
    if (!foundInside && !foundOutside) {
      return NONE_INSIDE;
    }
    if (foundInside && !foundOutside) {
      return ALL_INSIDE;
    }
    if (foundOutside && !foundInside) {
      return NONE_INSIDE;
    }
    return SOME_INSIDE;
  }

protected:
  std::shared_ptr<GeoBaseCompositeAreaShape> shared_from_this()
  {
    return std::static_pointer_cast<GeoBaseCompositeAreaShape>(
        GeoBaseCompositeMembershipShape<T>::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial3d::geom
