using namespace std;

#include "Geo3dCircleShape.h"

namespace org::apache::lucene::spatial::spatial4j
{
using GeoCircle = org::apache::lucene::spatial3d::geom::GeoCircle;
using GeoCircleFactory = org::apache::lucene::spatial3d::geom::GeoCircleFactory;
using GeoPointShapeFactory =
    org::apache::lucene::spatial3d::geom::GeoPointShapeFactory;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;

Geo3dCircleShape::Geo3dCircleShape(shared_ptr<GeoCircle> shape,
                                   shared_ptr<SpatialContext> spatialcontext)
    : Geo3dShape<org::apache::lucene::spatial3d::geom::GeoCircle>(
          shape, spatialcontext)
{
}

void Geo3dCircleShape::reset(double x, double y, double radiusDEG)
{
  shape = GeoCircleFactory::makeGeoCircle(
      shape->getPlanetModel(), y * DistanceUtils::DEGREES_TO_RADIANS,
      x * DistanceUtils::DEGREES_TO_RADIANS,
      radiusDEG * DistanceUtils::DEGREES_TO_RADIANS);
  center.reset();
  boundingBox.reset();
}

double Geo3dCircleShape::getRadius()
{
  return shape->getRadius() * DistanceUtils::RADIANS_TO_DEGREES;
}

shared_ptr<Point> Geo3dCircleShape::getCenter()
{
  shared_ptr<Point> center = this->center; // volatile read once
  if (center == nullptr) {
    center = make_shared<Geo3dPointShape>(
        GeoPointShapeFactory::makeGeoPointShape(
            shape->getPlanetModel(), shape->getCenter().getLatitude(),
            shape->getCenter().getLongitude()),
        spatialcontext);
    this->center = center;
  }
  return center;
}
} // namespace org::apache::lucene::spatial::spatial4j