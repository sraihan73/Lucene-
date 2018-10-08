#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

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

namespace org::apache::lucene::spatial::spatial4j
{

using GeoAreaShape = org::apache::lucene::spatial3d::geom::GeoAreaShape;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

/**
 * A Spatial4j Shape wrapping a {@link GeoAreaShape} ("Geo3D") -- a 3D planar
 * geometry based Spatial4j Shape implementation. Geo3D implements shapes on the
 * surface of a sphere or ellipsoid.
 *
 * @param <T> is the type of {@link GeoAreaShape}
 * @lucene.experimental
 */

template <typename T>
class Geo3dShape : public std::enable_shared_from_this<Geo3dShape>, public Shape
{
  GET_CLASS_NAME(Geo3dShape)
  static_assert(
      std::is_base_of<org.apache.lucene.spatial3d.geom.GeoAreaShape, T>::value,
      L"T must inherit from org.apache.lucene.spatial3d.geom.GeoAreaShape");

protected:
  const std::shared_ptr<SpatialContext> spatialcontext;

  T shape;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile
  // org.locationtech.spatial4j.shape.Rectangle boundingBox = null;
  std::shared_ptr<Rectangle> boundingBox = nullptr; // lazy initialized
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile org.locationtech.spatial4j.shape.Point
  // center = null;
  std::shared_ptr<Point> center = nullptr; // lazy initialized

public:
  Geo3dShape(T const shape, std::shared_ptr<SpatialContext> spatialcontext)
      : spatialcontext(spatialcontext)
  {
    this->shape = shape;
  }

  std::shared_ptr<SpatialRelation> relate(std::shared_ptr<Shape> other) override
  {
    int relationship;
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: if (other instanceof Geo3dShape<?>)
        if (std::dynamic_pointer_cast<Geo3dShape<?>>(other) != nullptr)
        {
          // C++ TODO: Java wildcard generics are not converted to C++:
          // ORIGINAL LINE: relationship = relate((Geo3dShape<?>) other);
          relationship = relate(std::static_pointer_cast<Geo3dShape<?>>(other));
        } else if (std::dynamic_pointer_cast<Rectangle>(other) != nullptr) {
          relationship = relate(std::static_pointer_cast<Rectangle>(other));
        } else if (std::dynamic_pointer_cast<Point>(other) != nullptr) {
          relationship = relate(std::static_pointer_cast<Point>(other));
        } else {
          throw std::runtime_error(
              "Unimplemented shape relationship determination: " +
              other->getClass());
        }

        switch (relationship) {
        case GeoArea::DISJOINT:
          return SpatialRelation::DISJOINT;
        case GeoArea::OVERLAPS:
          return (std::dynamic_pointer_cast<Point>(other) != nullptr
                      ? SpatialRelation::CONTAINS
                      : SpatialRelation::INTERSECTS);
        case GeoArea::CONTAINS:
          return (std::dynamic_pointer_cast<Point>(other) != nullptr
                      ? SpatialRelation::CONTAINS
                      : SpatialRelation::WITHIN);
        case GeoArea::WITHIN:
          return SpatialRelation::CONTAINS;
        }

        throw std::runtime_error("Undetermined shape relationship: " +
                                 std::to_wstring(relationship));
  }

private:
  template <typename T1>
  int relate(std::shared_ptr<Geo3dShape<T1>> s)
  {
    return shape->getRelationship(s->shape);
  }

  int relate(std::shared_ptr<Rectangle> r)
  {
    // Construct the right kind of GeoArea first
    std::shared_ptr<GeoArea> geoArea = GeoAreaFactory::makeGeoArea(
        shape->getPlanetModel(),
        r->getMaxY() * DistanceUtils::DEGREES_TO_RADIANS,
        r->getMinY() * DistanceUtils::DEGREES_TO_RADIANS,
        r->getMinX() * DistanceUtils::DEGREES_TO_RADIANS,
        r->getMaxX() * DistanceUtils::DEGREES_TO_RADIANS);

    return geoArea->getRelationship(shape);
  }

  int relate(std::shared_ptr<Point> p)
  {
    std::shared_ptr<GeoPoint> point = std::make_shared<GeoPoint>(
        shape->getPlanetModel(), p->getY() * DistanceUtils::DEGREES_TO_RADIANS,
        p->getX() * DistanceUtils::DEGREES_TO_RADIANS);

    if (shape->isWithin(point)) {
      return GeoArea::WITHIN;
    }
    return GeoArea::DISJOINT;
  }

public:
  std::shared_ptr<Rectangle> getBoundingBox() override
  {
    std::shared_ptr<Rectangle> bbox = this->boundingBox; // volatile read once
    if (bbox == nullptr) {
      std::shared_ptr<LatLonBounds> bounds = std::make_shared<LatLonBounds>();
      shape->getBounds(bounds);
      std::shared_ptr<GeoBBox> geoBBox =
          GeoBBoxFactory::makeGeoBBox(shape->getPlanetModel(), bounds);
      bbox = std::make_shared<Geo3dRectangleShape>(geoBBox, spatialcontext);
      this->boundingBox = bbox;
    }
    return bbox;
  }

  bool hasArea() override { return true; }

  double getArea(std::shared_ptr<SpatialContext> spatialContext) override
  {
    throw std::make_shared<UnsupportedOperationException>();
  }

  std::shared_ptr<Point> getCenter() override
  {
    std::shared_ptr<Point> center = this->center; // volatile read once
    if (center == nullptr) {
      center = getBoundingBox()->getCenter();
      this->center = center;
    }
    return center;
  }

  std::shared_ptr<Shape>
  getBuffered(double distance,
              std::shared_ptr<SpatialContext> spatialContext) override
  {
    throw std::make_shared<UnsupportedOperationException>();
  }

  bool isEmpty() override { return false; }

  std::shared_ptr<SpatialContext> getContext() override
  {
    return spatialcontext;
  }

  std::wstring toString() override
  {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return L"Geo3D:" + shape->toString();
  } // note: the shape usually prints its planet model

  bool equals(std::any o) override
  {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: if (!(o instanceof Geo3dShape<?>))
        if (!(std::dynamic_pointer_cast<Geo3dShape<?>>(o) != nullptr))
        {
          return false;
        }
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: final Geo3dShape<?> other = (Geo3dShape<?>) o;
        std::shared_ptr < Geo3dShape <
            ? >> *const other = std::any_cast < Geo3dShape < ? >> (o);
        return (other->spatialcontext->equals(spatialcontext) &&
                other->shape->equals(shape));
  }

  int hashCode() override
  {
    return spatialcontext->hashCode() + shape->hashCode();
  }
};

} // namespace org::apache::lucene::spatial::spatial4j
