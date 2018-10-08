using namespace std;

#include "GeoPathTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using org::junit::Test;
//    import static org.apache.lucene.util.SloppyMath.toRadians;
//    import static org.junit.Assert.assertEquals;
//    import static org.junit.Assert.assertFalse;
//    import static org.junit.Assert.assertTrue;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPathDistance()
void GeoPathTest::testPathDistance()
{
  // Start with a really simple case
  shared_ptr<GeoStandardPath> p;
  shared_ptr<GeoPoint> gp;
  p = make_shared<GeoStandardPath>(PlanetModel::SPHERE, 0.1);
  p->addPoint(0.0, 0.0);
  p->addPoint(0.0, 0.1);
  p->addPoint(0.0, 0.2);
  p->done();
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, M_PI * 0.5, 0.15);
  assertEquals(numeric_limits<double>::infinity(),
               p->computeDistance(DistanceStyle::ARC, gp), 0.0);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.05, 0.15);
  assertEquals(0.15 + 0.05, p->computeDistance(DistanceStyle::ARC, gp),
               0.000001);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, 0.12);
  assertEquals(0.12 + 0.0, p->computeDistance(DistanceStyle::ARC, gp),
               0.000001);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -0.15, 0.05);
  assertEquals(numeric_limits<double>::infinity(),
               p->computeDistance(DistanceStyle::ARC, gp), 0.000001);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, 0.25);
  assertEquals(0.20 + 0.05, p->computeDistance(DistanceStyle::ARC, gp),
               0.000001);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, -0.05);
  assertEquals(0.0 + 0.05, p->computeDistance(DistanceStyle::ARC, gp),
               0.000001);

  // Compute path distances now
  p = make_shared<GeoStandardPath>(PlanetModel::SPHERE, 0.1);
  p->addPoint(0.0, 0.0);
  p->addPoint(0.0, 0.1);
  p->addPoint(0.0, 0.2);
  p->done();
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.05, 0.15);
  assertEquals(0.15 + 0.05, p->computeDistance(DistanceStyle::ARC, gp),
               0.000001);
  assertEquals(0.15, p->computeNearestDistance(DistanceStyle::ARC, gp),
               0.000001);
  assertEquals(0.10, p->computeDeltaDistance(DistanceStyle::ARC, gp), 0.000001);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, 0.12);
  assertEquals(0.12, p->computeDistance(DistanceStyle::ARC, gp), 0.000001);
  assertEquals(0.12, p->computeNearestDistance(DistanceStyle::ARC, gp),
               0.000001);
  assertEquals(0.0, p->computeDeltaDistance(DistanceStyle::ARC, gp), 0.000001);

  // Now try a vertical path, and make sure distances are as expected
  p = make_shared<GeoStandardPath>(PlanetModel::SPHERE, 0.1);
  p->addPoint(-M_PI * 0.25, -0.5);
  p->addPoint(M_PI * 0.25, -0.5);
  p->done();
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, 0.0);
  assertEquals(numeric_limits<double>::infinity(),
               p->computeDistance(DistanceStyle::ARC, gp), 0.0);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -0.1, -1.0);
  assertEquals(numeric_limits<double>::infinity(),
               p->computeDistance(DistanceStyle::ARC, gp), 0.0);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, M_PI * 0.25 + 0.05, -0.5);
  assertEquals(M_PI * 0.5 + 0.05, p->computeDistance(DistanceStyle::ARC, gp),
               0.000001);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -M_PI * 0.25 - 0.05, -0.5);
  assertEquals(0.0 + 0.05, p->computeDistance(DistanceStyle::ARC, gp),
               0.000001);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPathPointWithin()
void GeoPathTest::testPathPointWithin()
{
  // Tests whether we can properly detect whether a point is within a path or
  // not
  shared_ptr<GeoStandardPath> p;
  shared_ptr<GeoPoint> gp;
  p = make_shared<GeoStandardPath>(PlanetModel::SPHERE, 0.1);
  // Build a diagonal path crossing the equator
  p->addPoint(-0.2, -0.2);
  p->addPoint(0.2, 0.2);
  p->done();
  // Test points on the path
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -0.2, -0.2);
  assertTrue(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, 0.0);
  assertTrue(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.1, 0.1);
  assertTrue(p->isWithin(gp));
  // Test points off the path
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -0.2, 0.2);
  assertFalse(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -M_PI * 0.5, 0.0);
  assertFalse(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.2, -0.2);
  assertFalse(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, M_PI);
  assertFalse(p->isWithin(gp));
  // Repeat the test, but across the terminator
  p = make_shared<GeoStandardPath>(PlanetModel::SPHERE, 0.1);
  // Build a diagonal path crossing the equator
  p->addPoint(-0.2, M_PI - 0.2);
  p->addPoint(0.2, -M_PI + 0.2);
  p->done();
  // Test points on the path
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -0.2, M_PI - 0.2);
  assertTrue(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, M_PI);
  assertTrue(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.1, -M_PI + 0.1);
  assertTrue(p->isWithin(gp));
  // Test points off the path
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -0.2, -M_PI + 0.2);
  assertFalse(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -M_PI * 0.5, 0.0);
  assertFalse(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.2, M_PI - 0.2);
  assertFalse(p->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, 0.0);
  assertFalse(p->isWithin(gp));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGetRelationship()
void GeoPathTest::testGetRelationship()
{
  shared_ptr<GeoArea> rect;
  shared_ptr<GeoStandardPath> p;
  shared_ptr<GeoStandardPath> c;
  shared_ptr<GeoPoint> point;
  shared_ptr<GeoPoint> pointApprox;
  int relationship;
  shared_ptr<GeoArea> area;
  shared_ptr<PlanetModel> planetModel;

  planetModel = make_shared<PlanetModel>(1.151145876105594, 0.8488541238944061);
  c = make_shared<GeoStandardPath>(planetModel, 0.008726646259971648);
  c->addPoint(-0.6925658899376476, 0.6316613927914589);
  c->addPoint(0.27828548161836364, 0.6785795524104564);
  c->done();
  point = make_shared<GeoPoint>(planetModel, -0.49298555067758226,
                                0.9892440995026406);
  pointApprox = make_shared<GeoPoint>(0.5110940362119821, 0.7774603209946239,
                                      -0.49984312299556544);
  area = GeoAreaFactory::makeGeoArea(
      planetModel, 0.49937141144985997, 0.5161765426256085, 0.3337218719537796,
      0.8544419570901649, -0.6347692823688085, 0.3069696588119369);
  assertTrue(!c->isWithin(point));

  // Start by testing the basic kinds of relationship, increasing in order of
  // difficulty.

  p = make_shared<GeoStandardPath>(PlanetModel::SPHERE, 0.1);
  p->addPoint(-0.3, -0.3);
  p->addPoint(0.3, 0.3);
  p->done();
  // Easiest: The path is wholly contains the georect
  rect =
      make_shared<GeoRectangle>(PlanetModel::SPHERE, 0.05, -0.05, -0.05, 0.05);
  assertEquals(GeoArea::CONTAINS, rect->getRelationship(p));
  // Next easiest: Some endpoints of the rectangle are inside, and some are
  // outside.
  rect =
      make_shared<GeoRectangle>(PlanetModel::SPHERE, 0.05, -0.05, -0.05, 0.5);
  assertEquals(GeoArea::OVERLAPS, rect->getRelationship(p));
  // Now, all points are outside, but the figures intersect
  rect = make_shared<GeoRectangle>(PlanetModel::SPHERE, 0.05, -0.05, -0.5, 0.5);
  assertEquals(GeoArea::OVERLAPS, rect->getRelationship(p));
  // Finally, all points are outside, and the figures *do not* intersect
  rect = make_shared<GeoRectangle>(PlanetModel::SPHERE, 0.5, -0.5, -0.5, 0.5);
  assertEquals(GeoArea::WITHIN, rect->getRelationship(p));
  // Check that segment edge overlap detection works
  rect = make_shared<GeoRectangle>(PlanetModel::SPHERE, 0.1, 0.0, -0.1, 0.0);
  assertEquals(GeoArea::OVERLAPS, rect->getRelationship(p));
  rect = make_shared<GeoRectangle>(PlanetModel::SPHERE, 0.2, 0.1, -0.2, -0.1);
  assertEquals(GeoArea::DISJOINT, rect->getRelationship(p));
  // Check if overlap at endpoints behaves as expected next
  rect = make_shared<GeoRectangle>(PlanetModel::SPHERE, 0.5, -0.5, -0.5, -0.35);
  assertEquals(GeoArea::OVERLAPS, rect->getRelationship(p));
  rect = make_shared<GeoRectangle>(PlanetModel::SPHERE, 0.5, -0.5, -0.5, -0.45);
  assertEquals(GeoArea::DISJOINT, rect->getRelationship(p));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPathBounds()
void GeoPathTest::testPathBounds()
{
  shared_ptr<GeoStandardPath> c;
  shared_ptr<LatLonBounds> b;
  shared_ptr<XYZBounds> xyzb;
  shared_ptr<GeoPoint> point;
  int relationship;
  shared_ptr<GeoArea> area;
  shared_ptr<PlanetModel> planetModel;

  planetModel = make_shared<PlanetModel>(0.751521665790406, 1.248478334209594);
  c = make_shared<GeoStandardPath>(planetModel, 0.7504915783575618);
  c->addPoint(0.10869761172400265, 0.08895880215465272);
  c->addPoint(0.22467878641991612, 0.10972973084229565);
  c->addPoint(-0.7398772468744732, -0.4465812941383364);
  c->addPoint(-0.18462055300079366, -0.6713857796763727);
  c->done();
  point = make_shared<GeoPoint>(planetModel, -0.626645355125733,
                                -1.409304625439381);
  xyzb = make_shared<XYZBounds>();
  c->getBounds(xyzb);
  area = GeoAreaFactory::makeGeoArea(planetModel, xyzb->getMinimumX(),
                                     xyzb->getMaximumX(), xyzb->getMinimumY(),
                                     xyzb->getMaximumY(), xyzb->getMinimumZ(),
                                     xyzb->getMaximumZ());
  relationship = area->getRelationship(c);
  assertTrue(relationship == GeoArea::WITHIN ||
             relationship == GeoArea::OVERLAPS);
  assertTrue(area->isWithin(point));
  // No longer true due to fixed GeoStandardPath waypoints.
  // assertTrue(c.isWithin(point));

  c = make_shared<GeoStandardPath>(PlanetModel::WGS84, 0.6894050545377601);
  c->addPoint(-0.0788176065762948, 0.9431251741731624);
  c->addPoint(0.510387871458147, 0.5327078872484678);
  c->addPoint(-0.5624521609859962, 1.5398841746888388);
  c->addPoint(-0.5025171434638661, -0.5895998642788894);
  c->done();
  point = make_shared<GeoPoint>(PlanetModel::WGS84, 0.023652082107211682,
                                0.023131910152748437);
  // System.err.println("Point.x = "+point.x+"; point.y="+point.y+";
  // point.z="+point.z);
  assertTrue(c->isWithin(point));
  xyzb = make_shared<XYZBounds>();
  c->getBounds(xyzb);
  area = GeoAreaFactory::makeGeoArea(PlanetModel::WGS84, xyzb->getMinimumX(),
                                     xyzb->getMaximumX(), xyzb->getMinimumY(),
                                     xyzb->getMaximumY(), xyzb->getMinimumZ(),
                                     xyzb->getMaximumZ());
  // System.err.println("minx="+xyzb.getMinimumX()+" maxx="+xyzb.getMaximumX()+"
  // miny="+xyzb.getMinimumY()+" maxy="+xyzb.getMaximumY()+"
  // minz="+xyzb.getMinimumZ()+" maxz="+xyzb.getMaximumZ());
  // System.err.println("point.x="+point.x+" point.y="+point.y+"
  // point.z="+point.z);
  relationship = area->getRelationship(c);
  assertTrue(relationship == GeoArea::WITHIN ||
             relationship == GeoArea::OVERLAPS);
  assertTrue(area->isWithin(point));

  c = make_shared<GeoStandardPath>(PlanetModel::WGS84, 0.7766715171374766);
  c->addPoint(-0.2751718361148076, -0.7786721269011477);
  c->addPoint(0.5728375851539309, -1.2700115736820465);
  c->done();
  point = make_shared<GeoPoint>(PlanetModel::WGS84, -0.01580760332365284,
                                -0.03956004622490505);
  assertTrue(c->isWithin(point));
  xyzb = make_shared<XYZBounds>();
  c->getBounds(xyzb);
  area = GeoAreaFactory::makeGeoArea(PlanetModel::WGS84, xyzb->getMinimumX(),
                                     xyzb->getMaximumX(), xyzb->getMinimumY(),
                                     xyzb->getMaximumY(), xyzb->getMinimumZ(),
                                     xyzb->getMaximumZ());
  // System.err.println("minx="+xyzb.getMinimumX()+" maxx="+xyzb.getMaximumX()+"
  // miny="+xyzb.getMinimumY()+" maxy="+xyzb.getMaximumY()+"
  // minz="+xyzb.getMinimumZ()+" maxz="+xyzb.getMaximumZ());
  // System.err.println("point.x="+point.x+" point.y="+point.y+"
  // point.z="+point.z);
  relationship = area->getRelationship(c);
  assertTrue(relationship == GeoArea::WITHIN ||
             relationship == GeoArea::OVERLAPS);
  assertTrue(area->isWithin(point));

  c = make_shared<GeoStandardPath>(PlanetModel::SPHERE, 0.1);
  c->addPoint(-0.3, -0.3);
  c->addPoint(0.3, 0.3);
  c->done();

  b = make_shared<LatLonBounds>();
  c->getBounds(b);
  assertFalse(b->checkNoLongitudeBound());
  assertFalse(b->checkNoTopLatitudeBound());
  assertFalse(b->checkNoBottomLatitudeBound());
  assertEquals(-0.4046919, b->getLeftLongitude(), 0.000001);
  assertEquals(0.4046919, b->getRightLongitude(), 0.000001);
  assertEquals(-0.3999999, b->getMinLatitude(), 0.000001);
  assertEquals(0.3999999, b->getMaxLatitude(), 0.000001);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCoLinear()
void GeoPathTest::testCoLinear()
{
  // p1: (12,-90), p2: (11, -55), (129, -90)
  shared_ptr<GeoStandardPath> p =
      make_shared<GeoStandardPath>(PlanetModel::SPHERE, 0.1);
  p->addPoint(toRadians(-90), toRadians(12)); // south pole
  p->addPoint(toRadians(-55), toRadians(11));
  p->addPoint(toRadians(-90), toRadians(129)); // south pole again
  p->done(); // at least test this doesn't bomb like it used too -- LUCENE-6520
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFailure1()
void GeoPathTest::testFailure1()
{
  /*
 GeoStandardPath: {planetmodel=PlanetModel.WGS84, width=1.117010721276371(64.0),
 points={[ [lat=2.18531083006635E-12,
 lon=-3.141592653589793([X=-1.0011188539924791, Y=-1.226017000107956E-16,
 Z=2.187755873813378E-12])], [lat=0.0,
 lon=-3.141592653589793([X=-1.0011188539924791, Y=-1.226017000107956E-16,
 Z=0.0])]]}}
  */
  std::deque<std::shared_ptr<GeoPoint>> points = {
      make_shared<GeoPoint>(PlanetModel::WGS84, 2.18531083006635E-12,
                            -3.141592653589793),
      make_shared<GeoPoint>(PlanetModel::WGS84, 0.0, -3.141592653589793)};

  shared_ptr<GeoPath> *const path;
  try {
    path = GeoPathFactory::makeGeoPath(PlanetModel::WGS84, 1.117010721276371,
                                       points);
  } catch (const invalid_argument &e) {
    return;
  }
  assertTrue(false);

  shared_ptr<GeoPoint> *const point = make_shared<GeoPoint>(
      PlanetModel::WGS84, -2.848117399637174E-91, -1.1092122135274942);
  System::err::println(L"point = " + point);

  shared_ptr<XYZBounds> *const bounds = make_shared<XYZBounds>();
  path->getBounds(bounds);

  shared_ptr<XYZSolid> *const solid = XYZSolidFactory::makeXYZSolid(
      PlanetModel::WGS84, bounds->getMinimumX(), bounds->getMaximumX(),
      bounds->getMinimumY(), bounds->getMaximumY(), bounds->getMinimumZ(),
      bounds->getMaximumZ());

  assertTrue(path->isWithin(point));
  assertTrue(solid->isWithin(point));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testInterpolation()
void GeoPathTest::testInterpolation()
{
  constexpr double lat = 52.51607;
  constexpr double lon = 13.37698;
  const std::deque<double> pathLats =
      std::deque<double>{52.5355, 52.54,   52.5626, 52.5665, 52.6007,
                          52.6135, 52.6303, 52.6651, 52.7074};
  const std::deque<double> pathLons =
      std::deque<double>{13.3634, 13.3704, 13.3307, 13.3076, 13.2806,
                          13.2484, 13.2406, 13.241,  13.1926};

  // Set up a point in the right way
  shared_ptr<GeoPoint> *const carPoint = make_shared<GeoPoint>(
      PlanetModel::SPHERE, toRadians(lat), toRadians(lon));
  // Create the path, but use a tiny width (e.g. zero)
  std::deque<std::shared_ptr<GeoPoint>> pathPoints(pathLats.size());
  for (int i = 0; i < pathPoints.size(); i++) {
    pathPoints[i] = make_shared<GeoPoint>(
        PlanetModel::SPHERE, toRadians(pathLats[i]), toRadians(pathLons[i]));
  }
  // Construct a path with no width
  shared_ptr<GeoPath> *const thisPath =
      GeoPathFactory::makeGeoPath(PlanetModel::SPHERE, 0.0, pathPoints);
  // Construct a path with a width
  shared_ptr<GeoPath> *const legacyPath =
      GeoPathFactory::makeGeoPath(PlanetModel::SPHERE, 1e-6, pathPoints);
  // Compute the inside distance to the atPoint using zero-width path
  constexpr double distance =
      thisPath->computeNearestDistance(DistanceStyle::ARC, carPoint);
  // Compute the inside distance using legacy path
  constexpr double legacyDistance =
      legacyPath->computeNearestDistance(DistanceStyle::ARC, carPoint);
  // Compute the inside distance using the legacy formula
  constexpr double oldFormulaDistance =
      thisPath->computeDistance(DistanceStyle::ARC, carPoint);
  // Compute the inside distance using the legacy formula with the legacy shape
  constexpr double oldFormulaLegacyDistance =
      legacyPath->computeDistance(DistanceStyle::ARC, carPoint);

  // These should be about the same
  assertEquals(legacyDistance, distance, 1e-12);
  assertEquals(oldFormulaLegacyDistance, oldFormulaDistance, 1e-12);
  // This isn't true because example search center is off of the path.
  // assertEquals(oldFormulaDistance, distance, 1e-12);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testInterpolation2()
void GeoPathTest::testInterpolation2()
{
  constexpr double lat = 52.5665;
  constexpr double lon = 13.3076;
  const std::deque<double> pathLats =
      std::deque<double>{52.5355, 52.54,   52.5626, 52.5665, 52.6007,
                          52.6135, 52.6303, 52.6651, 52.7074};
  const std::deque<double> pathLons =
      std::deque<double>{13.3634, 13.3704, 13.3307, 13.3076, 13.2806,
                          13.2484, 13.2406, 13.241,  13.1926};

  shared_ptr<GeoPoint> *const carPoint = make_shared<GeoPoint>(
      PlanetModel::SPHERE, toRadians(lat), toRadians(lon));
  std::deque<std::shared_ptr<GeoPoint>> pathPoints(pathLats.size());
  for (int i = 0; i < pathPoints.size(); i++) {
    pathPoints[i] = make_shared<GeoPoint>(
        PlanetModel::SPHERE, toRadians(pathLats[i]), toRadians(pathLons[i]));
  }

  // Construct a path with no width
  shared_ptr<GeoPath> *const thisPath =
      GeoPathFactory::makeGeoPath(PlanetModel::SPHERE, 0.0, pathPoints);
  // Construct a path with a width
  shared_ptr<GeoPath> *const legacyPath =
      GeoPathFactory::makeGeoPath(PlanetModel::SPHERE, 1e-6, pathPoints);

  // Compute the inside distance to the atPoint using zero-width path
  constexpr double distance =
      thisPath->computeNearestDistance(DistanceStyle::ARC, carPoint);
  // Compute the inside distance using legacy path
  constexpr double legacyDistance =
      legacyPath->computeNearestDistance(DistanceStyle::ARC, carPoint);

  // Compute the inside distance using the legacy formula
  constexpr double oldFormulaDistance =
      thisPath->computeDistance(DistanceStyle::ARC, carPoint);
  // Compute the inside distance using the legacy formula with the legacy shape
  constexpr double oldFormulaLegacyDistance =
      legacyPath->computeDistance(DistanceStyle::ARC, carPoint);

  // These should be about the same
  assertEquals(legacyDistance, distance, 1e-12);

  assertEquals(oldFormulaLegacyDistance, oldFormulaDistance, 1e-12);

  // Since the point we picked is actually on the path, this should also be true
  assertEquals(oldFormulaDistance, distance, 1e-12);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testIdenticalPoints()
void GeoPathTest::testIdenticalPoints()
{
  shared_ptr<PlanetModel> planetModel = PlanetModel::WGS84;
  shared_ptr<GeoPoint> point1 = make_shared<GeoPoint>(
      planetModel, 1.5707963267948963, -2.4818290647609542E-148);
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(planetModel, 1.570796326794895, -3.5E-323);
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(planetModel, 4.4E-323, -3.1415926535897896);
  shared_ptr<GeoPath> path = GeoPathFactory::makeGeoPath(
      planetModel, 0,
      std::deque<std::shared_ptr<GeoPoint>>{point1, point2, point3});
  shared_ptr<GeoPoint> point = make_shared<GeoPoint>(
      planetModel, -1.5707963267948952, 2.369064805649877E-284);
  // If not filtered the point is wrongly in set
  assertFalse(path->isWithin(point));
  // If not filtered it throws error
  path = GeoPathFactory::makeGeoPath(
      planetModel, 1e-6,
      std::deque<std::shared_ptr<GeoPoint>>{point1, point2, point3});
  assertFalse(path->isWithin(point));

  shared_ptr<GeoPoint> point4 = make_shared<GeoPoint>(planetModel, 1.5, 0);
  shared_ptr<GeoPoint> point5 = make_shared<GeoPoint>(planetModel, 1.5, 0);
  shared_ptr<GeoPoint> point6 =
      make_shared<GeoPoint>(planetModel, 4.4E-323, -3.1415926535897896);
  // If not filtered creates a degenerated Vector
  path = GeoPathFactory::makeGeoPath(
      planetModel, 0,
      std::deque<std::shared_ptr<GeoPoint>>{point4, point5, point6});
  path = GeoPathFactory::makeGeoPath(
      planetModel, 0.5,
      std::deque<std::shared_ptr<GeoPoint>>{point4, point5, point6});
}
} // namespace org::apache::lucene::spatial3d::geom