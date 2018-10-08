using namespace std;

#include "Geo3dShapeSphereModelRectRelationTest.h"

namespace org::apache::lucene::spatial::spatial4j
{
using GeoArea = org::apache::lucene::spatial3d::geom::GeoArea;
using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;
using GeoBBoxFactory = org::apache::lucene::spatial3d::geom::GeoBBoxFactory;
using GeoCircle = org::apache::lucene::spatial3d::geom::GeoCircle;
using GeoCircleFactory = org::apache::lucene::spatial3d::geom::GeoCircleFactory;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using GeoPolygonFactory =
    org::apache::lucene::spatial3d::geom::GeoPolygonFactory;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::junit::Test;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::SpatialRelation;

Geo3dShapeSphereModelRectRelationTest::Geo3dShapeSphereModelRectRelationTest()
{
  shared_ptr<Geo3dSpatialContextFactory> factory =
      make_shared<Geo3dSpatialContextFactory>();
  factory->planetModel = planetModel;
  this->ctx = factory->newSpatialContext();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailure1()
void Geo3dShapeSphereModelRectRelationTest::testFailure1()
{
  shared_ptr<GeoBBox> *const rect = GeoBBoxFactory::makeGeoBBox(
      planetModel, 88 * RADIANS_PER_DEGREE, 30 * RADIANS_PER_DEGREE,
      -30 * RADIANS_PER_DEGREE, 62 * RADIANS_PER_DEGREE);
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(make_shared<GeoPoint>(planetModel,
                                         30.4579218227 * RADIANS_PER_DEGREE,
                                         14.5238410082 * RADIANS_PER_DEGREE));
  points.push_back(make_shared<GeoPoint>(planetModel,
                                         43.684447915 * RADIANS_PER_DEGREE,
                                         46.2210986329 * RADIANS_PER_DEGREE));
  points.push_back(make_shared<GeoPoint>(planetModel,
                                         66.2465299717 * RADIANS_PER_DEGREE,
                                         -29.1786158537 * RADIANS_PER_DEGREE));
  shared_ptr<GeoShape> *const path =
      GeoPolygonFactory::makeGeoPolygon(planetModel, points);

  shared_ptr<GeoPoint> *const point =
      make_shared<GeoPoint>(planetModel, 34.2730264413182 * RADIANS_PER_DEGREE,
                            82.75500168892472 * RADIANS_PER_DEGREE);

  // Apparently the rectangle thinks the polygon is completely within it...
  // "shape inside rectangle"
  assertTrue(GeoArea::WITHIN == rect->getRelationship(path));

  // Point is within path? Apparently not...
  assertFalse(path->isWithin(point));

  // If it is within the path, it must be within the rectangle, and similarly
  // visa versa
  assertFalse(rect->isWithin(point));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailure2_LUCENE6475()
void Geo3dShapeSphereModelRectRelationTest::testFailure2_LUCENE6475()
{
  shared_ptr<GeoCircle> geo3dCircle = GeoCircleFactory::makeGeoCircle(
      planetModel, 1.6282053147165243E-4 * RADIANS_PER_DEGREE,
      -70.1600629789353 * RADIANS_PER_DEGREE, 86 * RADIANS_PER_DEGREE);
  shared_ptr<Geo3dShape> geo3dShape = make_shared<Geo3dShape>(geo3dCircle, ctx);
  shared_ptr<Rectangle> rect = ctx->makeRectangle(-118, -114, -2.0, 32.0);
  assertTrue(geo3dShape->relate(rect)->intersects());
  // thus the bounding box must intersect too
  assertTrue(geo3dShape->getBoundingBox()->relate(rect).intersects());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void pointBearingTest()
void Geo3dShapeSphereModelRectRelationTest::pointBearingTest()
{
  double radius = 136;
  double distance = 135.97;
  double bearing = 188;
  shared_ptr<Point> p = ctx->getShapeFactory().pointXY(35, 85);
  shared_ptr<Circle> circle = ctx->getShapeFactory().circle(p, radius);
  shared_ptr<Point> bPoint = ctx->getDistCalc().pointOnBearing(
      p, distance, bearing, ctx, std::static_pointer_cast<Point>(nullptr));

  double d = ctx->getDistCalc().distance(p, bPoint);
  assertEquals(d, distance, 10 - 8);

  assertEquals(circle->relate(bPoint), SpatialRelation::CONTAINS);
}
} // namespace org::apache::lucene::spatial::spatial4j