using namespace std;

#include "Geo3dShapeFactory.h"

namespace org::apache::lucene::spatial::spatial4j
{
using com::google::common::geometry::S2Cell;
using com::google::common::geometry::S2CellId;
using com::google::common::geometry::S2Point;
using S2ShapeFactory =
    org::apache::lucene::spatial::prefix::tree::S2ShapeFactory;
using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;
using GeoBBoxFactory = org::apache::lucene::spatial3d::geom::GeoBBoxFactory;
using GeoCircle = org::apache::lucene::spatial3d::geom::GeoCircle;
using GeoCircleFactory = org::apache::lucene::spatial3d::geom::GeoCircleFactory;
using GeoCompositeAreaShape =
    org::apache::lucene::spatial3d::geom::GeoCompositeAreaShape;
using GeoPath = org::apache::lucene::spatial3d::geom::GeoPath;
using GeoPathFactory = org::apache::lucene::spatial3d::geom::GeoPathFactory;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoPointShape = org::apache::lucene::spatial3d::geom::GeoPointShape;
using GeoPointShapeFactory =
    org::apache::lucene::spatial3d::geom::GeoPointShapeFactory;
using GeoPolygon = org::apache::lucene::spatial3d::geom::GeoPolygon;
using GeoPolygonFactory =
    org::apache::lucene::spatial3d::geom::GeoPolygonFactory;
using GeoS2ShapeFactory =
    org::apache::lucene::spatial3d::geom::GeoS2ShapeFactory;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::exception::InvalidShapeException;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::ShapeCollection;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public
// Geo3dShapeFactory(org.locationtech.spatial4j.context.SpatialContext context,
// org.locationtech.spatial4j.context.SpatialContextFactory factory)
Geo3dShapeFactory::Geo3dShapeFactory(shared_ptr<SpatialContext> context,
                                     shared_ptr<SpatialContextFactory> factory)
    : normWrapLongitude(context->isGeo() && factory->normWrapLongitude)
{
  this->context = context;
  this->planetModel =
      (std::static_pointer_cast<Geo3dSpatialContextFactory>(factory))
          ->planetModel;
}

shared_ptr<SpatialContext> Geo3dShapeFactory::getSpatialContext()
{
  return context;
}

void Geo3dShapeFactory::setCircleAccuracy(double circleAccuracy)
{
  this->circleAccuracy = circleAccuracy;
}

bool Geo3dShapeFactory::isNormWrapLongitude() { return normWrapLongitude; }

double Geo3dShapeFactory::normX(double x)
{
  if (this->normWrapLongitude) {
    x = DistanceUtils::normLonDEG(x);
  }
  return x;
}

double Geo3dShapeFactory::normY(double y) { return y; }

double Geo3dShapeFactory::normZ(double z) { return z; }

double Geo3dShapeFactory::normDist(double distance) { return distance; }

void Geo3dShapeFactory::verifyX(double x)
{
  shared_ptr<Rectangle> bounds = this->context->getWorldBounds();
  if (x < bounds->getMinX() || x > bounds->getMaxX()) {
    // C++ TODO: The following line could not be converted:
    throw org.locationtech.spatial4j.exception.InvalidShapeException(
        L"Bad X value " + x + L" is not in boundary " + bounds);
  }
}

void Geo3dShapeFactory::verifyY(double y)
{
  shared_ptr<Rectangle> bounds = this->context->getWorldBounds();
  if (y < bounds->getMinY() || y > bounds->getMaxY()) {
    // C++ TODO: The following line could not be converted:
    throw org.locationtech.spatial4j.exception.InvalidShapeException(
        L"Bad Y value " + y + L" is not in boundary " + bounds);
  }
}

void Geo3dShapeFactory::verifyZ(double v) {}

shared_ptr<Point> Geo3dShapeFactory::pointXY(double x, double y)
{
  shared_ptr<GeoPointShape> point = GeoPointShapeFactory::makeGeoPointShape(
      planetModel, y * DistanceUtils::DEGREES_TO_RADIANS,
      x * DistanceUtils::DEGREES_TO_RADIANS);
  return make_shared<Geo3dPointShape>(point, context);
}

shared_ptr<Point> Geo3dShapeFactory::pointXYZ(double x, double y, double z)
{
  shared_ptr<GeoPoint> point = make_shared<GeoPoint>(x, y, z);
  shared_ptr<GeoPointShape> pointShape =
      GeoPointShapeFactory::makeGeoPointShape(planetModel, point->getLatitude(),
                                              point->getLongitude());
  return make_shared<Geo3dPointShape>(pointShape, context);
  // throw new UnsupportedOperationException();
}

shared_ptr<Rectangle> Geo3dShapeFactory::rect(shared_ptr<Point> point,
                                              shared_ptr<Point> point1)
{
  return rect(point->getX(), point1->getX(), point->getY(), point1->getY());
}

shared_ptr<Rectangle> Geo3dShapeFactory::rect(double minX, double maxX,
                                              double minY, double maxY)
{
  shared_ptr<GeoBBox> bBox = GeoBBoxFactory::makeGeoBBox(
      planetModel, maxY * DistanceUtils::DEGREES_TO_RADIANS,
      minY * DistanceUtils::DEGREES_TO_RADIANS,
      minX * DistanceUtils::DEGREES_TO_RADIANS,
      maxX * DistanceUtils::DEGREES_TO_RADIANS);
  return make_shared<Geo3dRectangleShape>(bBox, context, minX, maxX, minY,
                                          maxY);
}

shared_ptr<Circle> Geo3dShapeFactory::circle(double x, double y,
                                             double distance)
{
  shared_ptr<GeoCircle> circle;
  if (planetModel->isSphere()) {
    circle = GeoCircleFactory::makeGeoCircle(
        planetModel, y * DistanceUtils::DEGREES_TO_RADIANS,
        x * DistanceUtils::DEGREES_TO_RADIANS,
        distance * DistanceUtils::DEGREES_TO_RADIANS);
  } else {
    // accuracy is defined as a linear distance in this class. At tiny
    // distances, linear distance can be approximated to surface distance in
    // radians.
    circle = GeoCircleFactory::makeExactGeoCircle(
        planetModel, y * DistanceUtils::DEGREES_TO_RADIANS,
        x * DistanceUtils::DEGREES_TO_RADIANS,
        distance * DistanceUtils::DEGREES_TO_RADIANS,
        circleAccuracy * DistanceUtils::DEGREES_TO_RADIANS);
  }
  return make_shared<Geo3dCircleShape>(circle, context);
}

shared_ptr<Circle> Geo3dShapeFactory::circle(shared_ptr<Point> point,
                                             double distance)
{
  return circle(point->getX(), point->getY(), distance);
}

shared_ptr<Shape>
Geo3dShapeFactory::lineString(deque<std::shared_ptr<Point>> &deque,
                              double distance)
{
  shared_ptr<LineStringBuilder> builder = lineString();
  for (auto point : deque) {
    builder->pointXY(point->getX(), point->getY());
  }
  builder->buffer(distance);
  return builder->build();
}

template <typename S>
shared_ptr<ShapeCollection<S>> Geo3dShapeFactory::multiShape(deque<S> &deque)
{
  static_assert(is_base_of<org.locationtech.spatial4j.shape.Shape, S>::value,
                L"S must inherit from org.locationtech.spatial4j.shape.Shape");

  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<LineStringBuilder> Geo3dShapeFactory::lineString()
{
  return make_shared<Geo3dLineStringBuilder>(shared_from_this());
}

shared_ptr<PolygonBuilder> Geo3dShapeFactory::polygon()
{
  return make_shared<Geo3dPolygonBuilder>(shared_from_this());
}

template <typename T>
shared_ptr<MultiShapeBuilder<T>>
Geo3dShapeFactory::multiShape(type_info<T> &aClass)
{
  static_assert(is_base_of<org.locationtech.spatial4j.shape.Shape, T>::value,
                L"T must inherit from org.locationtech.spatial4j.shape.Shape");

  return make_shared<Geo3dMultiShapeBuilder<>>();
}

shared_ptr<MultiPointBuilder> Geo3dShapeFactory::multiPoint()
{
  return make_shared<Geo3dMultiPointBuilder>(shared_from_this());
}

shared_ptr<MultiLineStringBuilder> Geo3dShapeFactory::multiLineString()
{
  return make_shared<Geo3dMultiLineBuilder>(shared_from_this());
}

shared_ptr<MultiPolygonBuilder> Geo3dShapeFactory::multiPolygon()
{
  return make_shared<Geo3dMultiPolygonBuilder>(shared_from_this());
}

shared_ptr<Shape> Geo3dShapeFactory::getS2CellShape(shared_ptr<S2CellId> cellId)
{
  shared_ptr<S2Cell> cell = make_shared<S2Cell>(cellId);
  shared_ptr<GeoPoint> point1 = getGeoPoint(cell->getVertexRaw(0));
  shared_ptr<GeoPoint> point2 = getGeoPoint(cell->getVertexRaw(1));
  shared_ptr<GeoPoint> point3 = getGeoPoint(cell->getVertexRaw(2));
  shared_ptr<GeoPoint> point4 = getGeoPoint(cell->getVertexRaw(3));
  return make_shared<Geo3dShape<>>(
      GeoS2ShapeFactory::makeGeoS2Shape(planetModel, point1, point2, point3,
                                        point4),
      context);
}

shared_ptr<GeoPoint> Geo3dShapeFactory::getGeoPoint(shared_ptr<S2Point> point)
{
  return planetModel->createSurfacePoint(point->get(0), point->get(1),
                                         point->get(2));
}

Geo3dShapeFactory::Geo3dLineStringBuilder::Geo3dLineStringBuilder(
    shared_ptr<Geo3dShapeFactory> outerInstance)
    : Geo3dPointBuilder(outerInstance), outerInstance(outerInstance)
{
}

shared_ptr<LineStringBuilder>
Geo3dShapeFactory::Geo3dLineStringBuilder::buffer(double distance)
{
  this->distance = distance;
  return shared_from_this();
}

shared_ptr<Shape> Geo3dShapeFactory::Geo3dLineStringBuilder::build()
{
  shared_ptr<GeoPath> path = GeoPathFactory::makeGeoPath(
      outerInstance->planetModel, distance,
      points.toArray(std::deque<std::shared_ptr<GeoPoint>>(points.size())));
  return make_shared<Geo3dShape<>>(path, outerInstance->context);
}

Geo3dShapeFactory::Geo3dPolygonBuilder::Geo3dPolygonBuilder(
    shared_ptr<Geo3dShapeFactory> outerInstance)
    : Geo3dPointBuilder(outerInstance), outerInstance(outerInstance)
{
}

shared_ptr<HoleBuilder> Geo3dShapeFactory::Geo3dPolygonBuilder::hole()
{
  return make_shared<Geo3dHoleBuilder>(shared_from_this());
}

Geo3dShapeFactory::Geo3dPolygonBuilder::Geo3dHoleBuilder::Geo3dHoleBuilder(
    shared_ptr<Geo3dShapeFactory::Geo3dPolygonBuilder> outerInstance)
    : Geo3dPointBuilder(outerInstance->outerInstance),
      outerInstance(outerInstance)
{
}

shared_ptr<PolygonBuilder>
Geo3dShapeFactory::Geo3dPolygonBuilder::Geo3dHoleBuilder::endHole()
{
  outerInstance->polyHoles.push_back(
      make_shared<GeoPolygonFactory::PolygonDescription>(points));
  return outerInstance;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
// org.locationtech.spatial4j.shape.Shape build()
shared_ptr<Shape> Geo3dShapeFactory::Geo3dPolygonBuilder::build()
{
  shared_ptr<GeoPolygonFactory::PolygonDescription> description =
      make_shared<GeoPolygonFactory::PolygonDescription>(points, polyHoles);
  shared_ptr<GeoPolygon> polygon = GeoPolygonFactory::makeGeoPolygon(
      outerInstance->planetModel, description);
  return make_shared<Geo3dShape<>>(polygon, outerInstance->context);
}

shared_ptr<Shape> Geo3dShapeFactory::Geo3dPolygonBuilder::buildOrRect()
{
  return build();
}

Geo3dShapeFactory::Geo3dMultiPointBuilder::Geo3dMultiPointBuilder(
    shared_ptr<Geo3dShapeFactory> outerInstance)
    : Geo3dPointBuilder(outerInstance), outerInstance(outerInstance)
{
}

shared_ptr<Shape> Geo3dShapeFactory::Geo3dMultiPointBuilder::build()
{
  shared_ptr<GeoCompositeAreaShape> areaShape =
      make_shared<GeoCompositeAreaShape>(outerInstance->planetModel);
  for (auto point : points) {
    shared_ptr<GeoPointShape> pointShape =
        GeoPointShapeFactory::makeGeoPointShape(outerInstance->planetModel,
                                                point->getLatitude(),
                                                point->getLongitude());
    areaShape->addShape(pointShape);
  }
  return make_shared<Geo3dShape<>>(areaShape, outerInstance->context);
}

Geo3dShapeFactory::Geo3dMultiLineBuilder::Geo3dMultiLineBuilder(
    shared_ptr<Geo3dShapeFactory> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<LineStringBuilder>
Geo3dShapeFactory::Geo3dMultiLineBuilder::lineString()
{
  return make_shared<Geo3dLineStringBuilder>(outerInstance);
}

shared_ptr<MultiLineStringBuilder>
Geo3dShapeFactory::Geo3dMultiLineBuilder::add(
    shared_ptr<LineStringBuilder> lineStringBuilder)
{
  builders.push_back(lineStringBuilder);
  return shared_from_this();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
// org.locationtech.spatial4j.shape.Shape build()
shared_ptr<Shape> Geo3dShapeFactory::Geo3dMultiLineBuilder::build()
{
  shared_ptr<GeoCompositeAreaShape> areaShape =
      make_shared<GeoCompositeAreaShape>(outerInstance->planetModel);
  for (auto builder : builders) {
    shared_ptr<Geo3dShape<std::shared_ptr<GeoPolygon>>> shape =
        std::static_pointer_cast<Geo3dShape<std::shared_ptr<GeoPolygon>>>(
            builder->build());
    areaShape->addShape(shape->shape);
  }
  return make_shared<Geo3dShape<>>(areaShape, outerInstance->context);
}

Geo3dShapeFactory::Geo3dMultiPolygonBuilder::Geo3dMultiPolygonBuilder(
    shared_ptr<Geo3dShapeFactory> outerInstance)
    : outerInstance(outerInstance)
{
}

shared_ptr<PolygonBuilder>
Geo3dShapeFactory::Geo3dMultiPolygonBuilder::polygon()
{
  return make_shared<Geo3dPolygonBuilder>(outerInstance);
}

shared_ptr<MultiPolygonBuilder>
Geo3dShapeFactory::Geo3dMultiPolygonBuilder::add(
    shared_ptr<PolygonBuilder> polygonBuilder)
{
  builders.push_back(polygonBuilder);
  return shared_from_this();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") @Override public
// org.locationtech.spatial4j.shape.Shape build()
shared_ptr<Shape> Geo3dShapeFactory::Geo3dMultiPolygonBuilder::build()
{
  shared_ptr<GeoCompositeAreaShape> areaShape =
      make_shared<GeoCompositeAreaShape>(outerInstance->planetModel);
  for (auto builder : builders) {
    shared_ptr<Geo3dShape<std::shared_ptr<GeoPolygon>>> shape =
        std::static_pointer_cast<Geo3dShape<std::shared_ptr<GeoPolygon>>>(
            builder->build());
    areaShape->addShape(shape->shape);
  }
  return make_shared<Geo3dShape<>>(areaShape, outerInstance->context);
}
} // namespace org::apache::lucene::spatial::spatial4j