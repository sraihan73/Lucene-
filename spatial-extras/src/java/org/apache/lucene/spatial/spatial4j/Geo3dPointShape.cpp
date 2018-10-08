using namespace std;

#include "Geo3dPointShape.h"

namespace org::apache::lucene::spatial::spatial4j
{
using GeoPointShape = org::apache::lucene::spatial3d::geom::GeoPointShape;
using GeoPointShapeFactory =
    org::apache::lucene::spatial3d::geom::GeoPointShapeFactory;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

Geo3dPointShape::Geo3dPointShape(shared_ptr<GeoPointShape> shape,
                                 shared_ptr<SpatialContext> spatialcontext)
    : Geo3dShape<org::apache::lucene::spatial3d::geom::GeoPointShape>(
          shape, spatialcontext)
{
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  center = shared_from_this();
}

void Geo3dPointShape::reset(double x, double y)
{
  shape = GeoPointShapeFactory::makeGeoPointShape(
      shape->getPlanetModel(), y * DistanceUtils::DEGREES_TO_RADIANS,
      x * DistanceUtils::DEGREES_TO_RADIANS);
  center = shared_from_this();
  boundingBox.reset();
}

double Geo3dPointShape::getX()
{
  return shape->getCenter().getLongitude() * DistanceUtils::RADIANS_TO_DEGREES;
}

double Geo3dPointShape::getY()
{
  return shape->getCenter().getLatitude() * DistanceUtils::RADIANS_TO_DEGREES;
}

shared_ptr<Rectangle> Geo3dPointShape::getBoundingBox()
{
  shared_ptr<Rectangle> bbox = this->boundingBox; // volatile read once
  if (bbox == nullptr) {
    bbox = make_shared<Geo3dRectangleShape>(shape, spatialcontext);
    this->boundingBox = bbox;
  }
  return bbox;
}

shared_ptr<Shape>
Geo3dPointShape::getBuffered(double distance,
                             shared_ptr<SpatialContext> spatialContext)
{
  return spatialContext->getShapeFactory().circle(getX(), getY(), distance);
}

bool Geo3dPointShape::hasArea() { return false; }
} // namespace org::apache::lucene::spatial::spatial4j