using namespace std;

#include "Geo3dRectangleShape.h"

namespace org::apache::lucene::spatial::spatial4j
{
using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;
using GeoBBoxFactory = org::apache::lucene::spatial3d::geom::GeoBBoxFactory;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoPointShapeFactory =
    org::apache::lucene::spatial3d::geom::GeoPointShapeFactory;
using LatLonBounds = org::apache::lucene::spatial3d::geom::LatLonBounds;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

Geo3dRectangleShape::Geo3dRectangleShape(
    shared_ptr<GeoBBox> shape, shared_ptr<SpatialContext> spatialcontext,
    double minX, double maxX, double minY, double maxY)
    : Geo3dShape<org::apache::lucene::spatial3d::geom::GeoBBox>(shape,
                                                                spatialcontext)
{
  this->minX = minX;
  this->maxX = maxX;
  this->minY = minY;
  this->maxY = maxY;
}

Geo3dRectangleShape::Geo3dRectangleShape(
    shared_ptr<GeoBBox> shape, shared_ptr<SpatialContext> spatialcontext)
    : Geo3dShape<org::apache::lucene::spatial3d::geom::GeoBBox>(shape,
                                                                spatialcontext)
{
  setBoundsFromshape();
}

void Geo3dRectangleShape::setBoundsFromshape()
{
  shared_ptr<LatLonBounds> bounds = make_shared<LatLonBounds>();
  shape->getBounds(bounds);
  minX = bounds->checkNoLongitudeBound()
             ? -180.0
             : bounds->getLeftLongitude() * DistanceUtils::RADIANS_TO_DEGREES;
  minY = bounds->checkNoBottomLatitudeBound()
             ? -90.0
             : bounds->getMinLatitude() * DistanceUtils::RADIANS_TO_DEGREES;
  maxX = bounds->checkNoLongitudeBound()
             ? 180.0
             : bounds->getRightLongitude() * DistanceUtils::RADIANS_TO_DEGREES;
  maxY = bounds->checkNoTopLatitudeBound()
             ? 90.0
             : bounds->getMaxLatitude() * DistanceUtils::RADIANS_TO_DEGREES;
}

shared_ptr<Point> Geo3dRectangleShape::getCenter()
{
  shared_ptr<Point> center = this->center; // volatile read once
  if (center == nullptr) {
    shared_ptr<GeoPoint> point = shape->getCenter();
    center = make_shared<Geo3dPointShape>(
        GeoPointShapeFactory::makeGeoPointShape(shape->getPlanetModel(),
                                                point->getLatitude(),
                                                point->getLongitude()),
        spatialcontext);
    this->center = center;
  }
  return center;
}

void Geo3dRectangleShape::reset(double minX, double maxX, double minY,
                                double maxY)
{
  shape = GeoBBoxFactory::makeGeoBBox(shape->getPlanetModel(),
                                      maxY * DistanceUtils::DEGREES_TO_RADIANS,
                                      minY * DistanceUtils::DEGREES_TO_RADIANS,
                                      minX * DistanceUtils::DEGREES_TO_RADIANS,
                                      maxX * DistanceUtils::DEGREES_TO_RADIANS);
  center.reset();
  boundingBox.reset();
}

shared_ptr<Rectangle> Geo3dRectangleShape::getBoundingBox()
{
  return shared_from_this();
}

double Geo3dRectangleShape::getWidth()
{
  double result = getMaxX() - getMinX();
  if (result < 0) {
    result += 360;
  }
  return result;
}

double Geo3dRectangleShape::getHeight() { return getMaxY() - getMinY(); }

double Geo3dRectangleShape::getMinX() { return minX; }

double Geo3dRectangleShape::getMinY() { return minY; }

double Geo3dRectangleShape::getMaxX() { return maxX; }

double Geo3dRectangleShape::getMaxY() { return maxY; }

bool Geo3dRectangleShape::getCrossesDateLine()
{
  return (getMaxX() > 0 && getMinX() < 0);
}

shared_ptr<SpatialRelation> Geo3dRectangleShape::relateYRange(double minY,
                                                              double maxY)
{
  shared_ptr<Rectangle> r =
      spatialcontext->getShapeFactory().rect(-180, 180, minY, maxY);
  return relate(r);
}

shared_ptr<SpatialRelation> Geo3dRectangleShape::relateXRange(double minX,
                                                              double maxX)
{
  shared_ptr<Rectangle> r =
      spatialcontext->getShapeFactory().rect(minX, maxX, -90, 90);
  return relate(r);
}

shared_ptr<Shape>
Geo3dRectangleShape::getBuffered(double distance,
                                 shared_ptr<SpatialContext> spatialContext)
{
  shared_ptr<GeoBBox> bBox =
      shape->expand(distance * DistanceUtils::DEGREES_TO_RADIANS);
  return make_shared<Geo3dRectangleShape>(bBox, spatialContext);
}
} // namespace org::apache::lucene::spatial::spatial4j