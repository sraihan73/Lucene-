#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <deque>

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
 * Base class to create a composite of GeoShapes.
 *
 * @param <T> is the type of GeoShapes of the composite.
 * @lucene.experimental
 */
template <typename T>
class GeoBaseCompositeShape : public BasePlanetObject, public GeoShape
{
  GET_CLASS_NAME(GeoBaseCompositeShape)
  static_assert(std::is_base_of<GeoShape, T>::value,
                L"T must inherit from GeoShape");

  /**
   * Shape's container
   */
protected:
  const std::deque<T> shapes = std::deque<T>();

  /**
   * Constructor.
   */
public:
  GeoBaseCompositeShape(std::shared_ptr<PlanetModel> planetModel)
      : BasePlanetObject(planetModel)
  {
  }

  /**
   * Add a shape to the composite.
   *
   * @param shape is the shape to add.
   */
  virtual void addShape(T const shape)
  {
    if (!shape->getPlanetModel()->equals(planetModel)) {
      throw std::invalid_argument(
          "Cannot add a shape into a composite with different planet models.");
    }
    shapes.push_back(shape);
  }

  /**
   * Get the number of shapes in the composite
   *
   * @return the number of shapes
   */
  virtual int size() { return shapes.size(); }

  /**
   * Get shape at index
   *
   * @return the shape at given index
   */
  virtual T getShape(int index) { return shapes[index]; }

  /**
   * Constructor for deserialization.
   * @param planetModel is the planet model.
   * @param inputStream is the input stream.
   * @param clazz is the class of the generic.
   */
  GeoBaseCompositeShape(std::shared_ptr<PlanetModel> planetModel,
                        std::shared_ptr<InputStream> inputStream,
                        std::type_info<T> &clazz) 
      : GeoBaseCompositeShape(planetModel)
  {
    const std::deque<T> array_ = SerializableObject::readHeterogeneousArray(
        planetModel, inputStream, clazz);
    for (auto member : array_) {
      addShape(clazz.cast(member));
    }
  }

  void
  write(std::shared_ptr<OutputStream> outputStream)  override
  {
    SerializableObject::writeHeterogeneousArray(outputStream, shapes);
  }

  bool isWithin(std::shared_ptr<Vector> point) override
  {
    return isWithin(point->x, point->y, point->z);
  }

  bool isWithin(double const x, double const y, double const z) override
  {
    for (auto shape : shapes) {
      if (shape->isWithin(x, y, z)) {
        return true;
      }
    }
    return false;
  }

  std::deque<std::shared_ptr<GeoPoint>> getEdgePoints() override
  {
    std::deque<std::shared_ptr<GeoPoint>> edgePoints;
    for (auto shape : shapes) {
      edgePoints.addAll(Arrays::asList(shape->getEdgePoints()));
    }
    return edgePoints.toArray(
        std::deque<std::shared_ptr<GeoPoint>>(edgePoints.size()));
  }

  bool intersects(std::shared_ptr<Plane> p,
                  std::deque<std::shared_ptr<GeoPoint>> &notablePoints,
                  std::deque<Membership> &bounds) override
  {
    for (auto shape : shapes) {
      if (shape->intersects(p, notablePoints, {bounds})) {
        return true;
      }
    }
    return false;
  }

  void getBounds(std::shared_ptr<Bounds> bounds) override
  {
    for (auto shape : shapes) {
      shape->getBounds(bounds);
    }
  }

  int hashCode() override
  {
    return BasePlanetObject::hashCode() + shapes.hashCode();
  }

  bool equals(std::any o) override
  {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: if (!(o instanceof GeoBaseCompositeShape<?>))
        if (!(std::dynamic_pointer_cast<GeoBaseCompositeShape<?>>(o) != nullptr))
        {
          return false;
        }
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: GeoBaseCompositeShape<?> other =
        // (GeoBaseCompositeShape<?>) o;
        std::shared_ptr < GeoBaseCompositeShape <
            ? >> other = std::any_cast < GeoBaseCompositeShape < ? >> (o);
        return BasePlanetObject::equals(other) && shapes.equals(other->shapes);
  }

protected:
  std::shared_ptr<GeoBaseCompositeShape> shared_from_this()
  {
    return std::static_pointer_cast<GeoBaseCompositeShape>(
        BasePlanetObject::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
