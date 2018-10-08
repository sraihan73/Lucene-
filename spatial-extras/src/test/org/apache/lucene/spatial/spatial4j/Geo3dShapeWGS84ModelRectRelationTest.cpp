using namespace std;

#include "Geo3dShapeWGS84ModelRectRelationTest.h"

namespace org::apache::lucene::spatial::spatial4j
{
using GeoArea = org::apache::lucene::spatial3d::geom::GeoArea;
using GeoBBox = org::apache::lucene::spatial3d::geom::GeoBBox;
using GeoBBoxFactory = org::apache::lucene::spatial3d::geom::GeoBBoxFactory;
using GeoCircleFactory = org::apache::lucene::spatial3d::geom::GeoCircleFactory;
using GeoCircle = org::apache::lucene::spatial3d::geom::GeoCircle;
using GeoPathFactory = org::apache::lucene::spatial3d::geom::GeoPathFactory;
using GeoPath = org::apache::lucene::spatial3d::geom::GeoPath;
using GeoPoint = org::apache::lucene::spatial3d::geom::GeoPoint;
using PlanetModel = org::apache::lucene::spatial3d::geom::PlanetModel;
using org::junit::Test;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::SpatialRelation;

Geo3dShapeWGS84ModelRectRelationTest::Geo3dShapeWGS84ModelRectRelationTest()
{
  shared_ptr<Geo3dSpatialContextFactory> factory =
      make_shared<Geo3dSpatialContextFactory>();
  factory->planetModel = planetModel;
  this->ctx = factory->newSpatialContext();
  this->maxRadius = 178;
  (std::static_pointer_cast<Geo3dShapeFactory>(ctx->getShapeFactory()))
      ->setCircleAccuracy(1e-12);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailure1()
void Geo3dShapeWGS84ModelRectRelationTest::testFailure1()
{
  shared_ptr<GeoBBox> *const rect = GeoBBoxFactory::makeGeoBBox(
      planetModel, 90 * RADIANS_PER_DEGREE, 74 * RADIANS_PER_DEGREE,
      40 * RADIANS_PER_DEGREE, 60 * RADIANS_PER_DEGREE);
  std::deque<std::shared_ptr<GeoPoint>> pathPoints = {
      make_shared<GeoPoint>(planetModel, 84.4987594274 * RADIANS_PER_DEGREE,
                            -22.8345484402 * RADIANS_PER_DEGREE)};
  shared_ptr<GeoPath> *const path = GeoPathFactory::makeGeoPath(
      planetModel, 4 * RADIANS_PER_DEGREE, pathPoints);
  assertTrue(GeoArea::DISJOINT == rect->getRelationship(path));
  // This is what the test failure claimed...
  // assertTrue(GeoArea.CONTAINS == rect.getRelationship(path));
  // final GeoBBox bbox = getBoundingBox(path);
  // assertFalse(GeoArea.DISJOINT == rect.getRelationship(bbox));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailure2()
void Geo3dShapeWGS84ModelRectRelationTest::testFailure2()
{
  shared_ptr<GeoBBox> *const rect = GeoBBoxFactory::makeGeoBBox(
      planetModel, -74 * RADIANS_PER_DEGREE, -90 * RADIANS_PER_DEGREE,
      0 * RADIANS_PER_DEGREE, 26 * RADIANS_PER_DEGREE);
  shared_ptr<GeoCircle> *const circle = GeoCircleFactory::makeGeoCircle(
      planetModel, -87.3647352103 * RADIANS_PER_DEGREE,
      52.3769709972 * RADIANS_PER_DEGREE, 1 * RADIANS_PER_DEGREE);
  assertTrue(GeoArea::DISJOINT == rect->getRelationship(circle));
  // This is what the test failure claimed...
  // assertTrue(GeoArea.CONTAINS == rect.getRelationship(circle));
  // final GeoBBox bbox = getBoundingBox(circle);
  // assertFalse(GeoArea.DISJOINT == rect.getRelationship(bbox));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailure3()
void Geo3dShapeWGS84ModelRectRelationTest::testFailure3()
{
  /*
 [junit4]   1> S-R Rel: {}, Shape {}, Rectangle {}    lap# {} [CONTAINS,
 Geo3dShape{planetmodel=PlanetModel: {ab=1.0011188180710464,
 c=0.9977622539852008}, shape=GeoPath: {planetmodel=PlanetModel:
 {ab=1.0011188180710464, c=0.9977622539852008},
 width=1.53588974175501(87.99999999999999), points={[[X=0.12097657665150223,
 Y=-0.6754177666095532, Z=0.7265376136709238], [X=-0.3837892785614207,
 Y=0.4258049113530899, Z=0.8180007850434892]]}}},
  Rect(minX=4.0,maxX=36.0,minY=16.0,maxY=16.0), 6981](no slf4j subst; sorry)
 [junit4] FAILURE 0.59s | Geo3dWGS84ShapeRectRelationTest.testGeoPathRect <<<
 [junit4]    > Throwable #1: java.lang.AssertionError:
 Geo3dShape{planetmodel=PlanetModel: {ab=1.0011188180710464,
 c=0.9977622539852008}, shape=GeoPath: {planetmodel=PlanetModel:
 {ab=1.0011188180710464, c=0.9977622539852008},
 width=1.53588974175501(87.99999999999999), points={[[X=0.12097657665150223,
 Y=-0.6754177666095532, Z=0.7265376136709238], [X=-0.3837892785614207,
 Y=0.4258049113530899, Z=0.8180007850434892]]}}} intersect
 Pt(x=23.81626064835212,y=16.0) [junit4]    >  at
 __randomizedtesting.SeedInfo.seed([2595268DA3F13FEA:6CC30D8C83453E5D]:0)
 [junit4]    >  at
 org.apache.lucene.spatial.spatial4j.RandomizedShapeTestCase._assertIntersect(RandomizedShapeTestCase.java:168)
 [junit4]    >  at
 org.apache.lucene.spatial.spatial4j.RandomizedShapeTestCase.assertRelation(RandomizedShapeTestCase.java:153)
 [junit4]    >  at
 org.apache.lucene.spatial.spatial4j.RectIntersectionTestHelper.testRelateWithRectangle(RectIntersectionTestHelper.java:128)
 [junit4]    >  at
 org.apache.lucene.spatial.spatial4j.Geo3dWGS84ShapeRectRelationTest.testGeoPathRect(Geo3dWGS84ShapeRectRelationTest.java:265)
*/
  shared_ptr<GeoBBox> *const rect = GeoBBoxFactory::makeGeoBBox(
      planetModel, 16 * RADIANS_PER_DEGREE, 16 * RADIANS_PER_DEGREE,
      4 * RADIANS_PER_DEGREE, 36 * RADIANS_PER_DEGREE);
  shared_ptr<GeoPoint> *const pt =
      make_shared<GeoPoint>(planetModel, 16 * RADIANS_PER_DEGREE,
                            23.81626064835212 * RADIANS_PER_DEGREE);
  std::deque<std::shared_ptr<GeoPoint>> pathPoints = {
      make_shared<GeoPoint>(planetModel, 46.6369060853 * RADIANS_PER_DEGREE,
                            -79.8452213228 * RADIANS_PER_DEGREE),
      make_shared<GeoPoint>(planetModel, 54.9779334519 * RADIANS_PER_DEGREE,
                            132.029177424 * RADIANS_PER_DEGREE)};
  shared_ptr<GeoPath> *const path = GeoPathFactory::makeGeoPath(
      planetModel, 88 * RADIANS_PER_DEGREE, pathPoints);
  wcout << L"rect=" << rect << endl;
  // Rectangle is within path (this is wrong; it's on the other side.  Should be
  // OVERLAPS)
  assertTrue(GeoArea::OVERLAPS == rect->getRelationship(path));
  // Rectangle contains point
  // assertTrue(rect.isWithin(pt));
  // Path contains point (THIS FAILS)
  // assertTrue(path.isWithin(pt));
  // What happens: (1) The center point of the horizontal line is within the
  // path, in fact within a radius of one of the endpoints. (2) The point
  // mentioned is NOT inside either SegmentEndpoint. (3) The point mentioned is
  // NOT inside the path segment, either.  (I think it should be...)
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void pointBearingTest()
void Geo3dShapeWGS84ModelRectRelationTest::pointBearingTest()
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