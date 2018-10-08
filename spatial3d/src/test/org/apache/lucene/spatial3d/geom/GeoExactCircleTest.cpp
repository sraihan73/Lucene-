using namespace std;

#include "GeoExactCircleTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExactCircle()
void GeoExactCircleTest::testExactCircle()
{
  shared_ptr<GeoCircle> c;
  shared_ptr<GeoPoint> gp;

  // Construct a variety of circles to see how many actual planes are involved
  c = make_shared<GeoExactCircle>(PlanetModel::WGS84, 0.0, 0.0, 0.1, 1e-6);
  gp = make_shared<GeoPoint>(PlanetModel::WGS84, 0.0, 0.2);
  assertTrue(!c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::WGS84, 0.0, 0.0);
  assertTrue(c->isWithin(gp));

  c = make_shared<GeoExactCircle>(PlanetModel::WGS84, 0.1, 0.0, 0.1, 1e-6);

  c = make_shared<GeoExactCircle>(PlanetModel::WGS84, 0.2, 0.0, 0.1, 1e-6);

  c = make_shared<GeoExactCircle>(PlanetModel::WGS84, 0.3, 0.0, 0.1, 1e-6);

  c = make_shared<GeoExactCircle>(PlanetModel::WGS84, 0.4, 0.0, 0.1, 1e-6);

  c = make_shared<GeoExactCircle>(PlanetModel::WGS84, M_PI * 0.5, 0.0, 0.1,
                                  1e-6);
  gp = make_shared<GeoPoint>(PlanetModel::WGS84, M_PI * 0.5 - 0.2, 0.0);
  assertTrue(!c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::WGS84, M_PI * 0.5, 0.0);
  assertTrue(c->isWithin(gp));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSurfacePointOnBearingScale()
void GeoExactCircleTest::testSurfacePointOnBearingScale()
{
  shared_ptr<PlanetModel> p1 = PlanetModel::WGS84;
  shared_ptr<PlanetModel> p2 = make_shared<PlanetModel>(
      0.5 * PlanetModel::WGS84->ab, 0.5 * PlanetModel::WGS84->c);
  shared_ptr<GeoPoint> point1P1 = make_shared<GeoPoint>(p1, 0, 0);
  shared_ptr<GeoPoint> point2P1 = make_shared<GeoPoint>(p1, 1, 1);
  shared_ptr<GeoPoint> point1P2 = make_shared<GeoPoint>(
      p2, point1P1->getLatitude(), point1P1->getLongitude());
  shared_ptr<GeoPoint> point2P2 = make_shared<GeoPoint>(
      p2, point2P1->getLatitude(), point2P1->getLongitude());

  double dist = 0.2 * M_PI;
  double bearing = 0.2 * M_PI;

  shared_ptr<GeoPoint> new1 =
      p1->surfacePointOnBearing(point2P1, dist, bearing);
  shared_ptr<GeoPoint> new2 =
      p2->surfacePointOnBearing(point2P2, dist, bearing);

  assertEquals(new1->getLatitude(), new2->getLatitude(), 1e-12);
  assertEquals(new1->getLongitude(), new2->getLongitude(), 1e-12);
  // This is true if surfaceDistance return results always in radians
  double d1 = p1->surfaceDistance(point1P1, point2P1);
  double d2 = p2->surfaceDistance(point1P2, point2P2);
  assertEquals(d1, d2, 1e-12);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 100) public void
// RandomPointBearingWGS84Test()
void GeoExactCircleTest::RandomPointBearingWGS84Test()
{
  shared_ptr<PlanetModel> planetModel = PlanetModel::WGS84;
  shared_ptr<RandomGeo3dShapeGenerator> generator =
      make_shared<RandomGeo3dShapeGenerator>();
  shared_ptr<GeoPoint> center = generator->randomGeoPoint(planetModel);
  double radius = random()->nextDouble() * M_PI;
  checkBearingPoint(planetModel, center, radius, 0);
  checkBearingPoint(planetModel, center, radius, 0.5 * M_PI);
  checkBearingPoint(planetModel, center, radius, M_PI);
  checkBearingPoint(planetModel, center, radius, 1.5 * M_PI);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 100) public void
// RandomPointBearingCardinalTest()
void GeoExactCircleTest::RandomPointBearingCardinalTest()
{
  // surface distance calculations methods start not converging when
  // planet flattening > 0.4
  shared_ptr<PlanetModel> planetModel;
  do {
    double ab = random()->nextDouble() * 2;
    double c = random()->nextDouble() * 2;
    if (random()->nextBoolean()) {
      planetModel = make_shared<PlanetModel>(ab, c);
    } else {
      planetModel = make_shared<PlanetModel>(c, ab);
    }
  } while (abs(planetModel->flattening) > 0.4);
  shared_ptr<GeoPoint> center = randomGeoPoint(planetModel);
  double radius =
      random()->nextDouble() * 0.9 * planetModel->minimumPoleDistance;
  checkBearingPoint(planetModel, center, radius, 0);
  checkBearingPoint(planetModel, center, radius, 0.5 * M_PI);
  checkBearingPoint(planetModel, center, radius, M_PI);
  checkBearingPoint(planetModel, center, radius, 1.5 * M_PI);
}

void GeoExactCircleTest::checkBearingPoint(shared_ptr<PlanetModel> planetModel,
                                           shared_ptr<GeoPoint> center,
                                           double radius, double bearingAngle)
{
  shared_ptr<GeoPoint> point =
      planetModel->surfacePointOnBearing(center, radius, bearingAngle);
  double surfaceDistance = planetModel->surfaceDistance(center, point);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTrue(planetModel->toString() + L" " +
                 Double::toString(surfaceDistance - radius) + L" " +
                 Double::toString(radius),
             surfaceDistance - radius < Vector::MINIMUM_ANGULAR_RESOLUTION);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExactCircleBounds()
void GeoExactCircleTest::testExactCircleBounds()
{

  shared_ptr<GeoPoint> center = make_shared<GeoPoint>(PlanetModel::WGS84, 0, 0);
  // Construct four cardinal points, and then we'll build the first two planes
  shared_ptr<GeoPoint> *const northPoint =
      PlanetModel::WGS84->surfacePointOnBearing(center, 1, 0.0);
  shared_ptr<GeoPoint> *const southPoint =
      PlanetModel::WGS84->surfacePointOnBearing(center, 1, M_PI);
  shared_ptr<GeoPoint> *const eastPoint =
      PlanetModel::WGS84->surfacePointOnBearing(center, 1, M_PI * 0.5);
  shared_ptr<GeoPoint> *const westPoint =
      PlanetModel::WGS84->surfacePointOnBearing(center, 1, M_PI * 1.5);

  shared_ptr<GeoCircle> circle =
      GeoCircleFactory::makeExactGeoCircle(PlanetModel::WGS84, 0, 0, 1, 1e-6);
  shared_ptr<LatLonBounds> bounds = make_shared<LatLonBounds>();
  circle->getBounds(bounds);
  assertEquals(northPoint->getLatitude(), bounds->getMaxLatitude(), 1e-2);
  assertEquals(southPoint->getLatitude(), bounds->getMinLatitude(), 1e-2);
  assertEquals(westPoint->getLongitude(), bounds->getLeftLongitude(), 1e-2);
  assertEquals(eastPoint->getLongitude(), bounds->getRightLongitude(), 1e-2);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void exactCircleLargeTest()
void GeoExactCircleTest::exactCircleLargeTest()
{
  bool success = true;
  try {
    shared_ptr<GeoCircle> circle = GeoCircleFactory::makeExactGeoCircle(
        make_shared<PlanetModel>(0.99, 1.05), 0.25 * M_PI, 0, 0.35 * M_PI,
        1e-12);
  } catch (const invalid_argument &e) {
    success = false;
  }
  assertTrue(success);
  success = false;
  try {
    shared_ptr<GeoCircle> circle = GeoCircleFactory::makeExactGeoCircle(
        PlanetModel::WGS84, 0.25 * M_PI, 0, 0.9996 * M_PI, 1e-12);
  } catch (const invalid_argument &e) {
    success = true;
  }
  assertTrue(success);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExactCircleDoesNotFit()
void GeoExactCircleTest::testExactCircleDoesNotFit()
{
  bool exception = false;
  try {
    shared_ptr<GeoCircle> circle = GeoCircleFactory::makeExactGeoCircle(
        PlanetModel::WGS84, 1.5633796542562415, -1.0387149580695152,
        3.1409865861032844, 1e-12);
  } catch (const invalid_argument &e) {
    exception = true;
  }
  assertTrue(exception);
}

void GeoExactCircleTest::testBigCircleInSphere()
{
  // In Planet model Sphere if circle is close to Math.PI we can get the
  // situation where circle slice planes are bigger than half of a hemisphere. We
  // need to make sure we divide the circle in at least 4 slices.
  shared_ptr<GeoCircle> circle1 = GeoCircleFactory::makeExactGeoCircle(
      PlanetModel::SPHERE, 1.1306735252307394, -0.7374283438171261,
      3.1415760537549234, 4.816939220262406E-12);
  shared_ptr<GeoPoint> point =
      make_shared<GeoPoint>(PlanetModel::SPHERE, -1.5707963267948966, 0.0);
  assertTrue(circle1->isWithin(point));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 100) public void
// testRandomLUCENE8054()
void GeoExactCircleTest::testRandomLUCENE8054()
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  shared_ptr<GeoCircle> circle1 = std::static_pointer_cast<GeoCircle>(
      randomGeoAreaShape(EXACT_CIRCLE, planetModel));
  // new radius, a bit smaller than the generated one!
  double radius = circle1->getRadius() * (1 - 0.01 * random()->nextDouble());
  // circle with same center and new radius
  shared_ptr<GeoCircle> circle2 = GeoCircleFactory::makeExactGeoCircle(
      planetModel, circle1->getCenter()->getLatitude(),
      circle1->getCenter()->getLongitude(), radius, 1e-5);
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"circle1: " + circle1 + L"\n");
  b->append(L"circle2: " + circle2);
  // It cannot be disjoint, same center!
  assertTrue(b->toString(),
             circle1->getRelationship(circle2) != GeoArea::DISJOINT);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLUCENE8054()
void GeoExactCircleTest::testLUCENE8054()
{
  shared_ptr<GeoCircle> circle1 = GeoCircleFactory::makeExactGeoCircle(
      PlanetModel::WGS84, -1.0394053553992673, -1.9037325881389144,
      1.1546166170607672, 4.231100485201301E-4);
  shared_ptr<GeoCircle> circle2 = GeoCircleFactory::makeExactGeoCircle(
      PlanetModel::WGS84, -1.3165961602008989, -1.887137823746273,
      1.432516663588956, 3.172052880854355E-4);
  // Relationship between circles must be different than DISJOINT as centers are
  // closer than the radius.
  int rel = circle1->getRelationship(circle2);
  assertTrue(rel != GeoArea::DISJOINT);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLUCENE8056()
void GeoExactCircleTest::testLUCENE8056()
{
  shared_ptr<GeoCircle> circle = GeoCircleFactory::makeExactGeoCircle(
      PlanetModel::WGS84, 0.647941905154693, 0.8542472362428436,
      0.8917883700569315, 1.2173787103955335E-8);
  shared_ptr<GeoBBox> bBox = GeoBBoxFactory::makeGeoBBox(
      PlanetModel::WGS84, 0.5890486225480862, 0.4908738521234052,
      1.9634954084936207, 2.159844949342983);
  // Center iis out of the shape
  assertFalse(circle->isWithin(bBox->getCenter()));
  // Edge point is in the shape
  assertTrue(circle->isWithin(bBox->getEdgePoints()[0]));
  // Shape should intersect!!!
  assertTrue(bBox->getRelationship(circle) == GeoArea::OVERLAPS);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExactCircleLUCENE8054()
void GeoExactCircleTest::testExactCircleLUCENE8054()
{
  // [junit4]    > Throwable #1: java.lang.AssertionError: circle1:
  // GeoExactCircle: {planetmodel=PlanetModel.WGS84,
  // center=[lat=-1.2097332228999564,
  // lon=0.749061883738567([X=0.25823775418663625, Y=0.2401212674846636,
  // Z=-0.9338185278804293])],
  //  radius=0.20785254459485322(11.909073566339822),
  //  accuracy=6.710701666727661E-9}
  // [junit4]    > circle2: GeoExactCircle: {planetmodel=PlanetModel.WGS84,
  // center=[lat=-1.2097332228999564,
  // lon=0.749061883738567([X=0.25823775418663625, Y=0.2401212674846636,
  // Z=-0.9338185278804293])], radius=0.20701584142315682(11.861134005896407),
  // accuracy=1.0E-5}
  shared_ptr<GeoCircle> *const c1 = make_shared<GeoExactCircle>(
      PlanetModel::WGS84, -1.2097332228999564, 0.749061883738567,
      0.20785254459485322, 6.710701666727661E-9);
  shared_ptr<GeoCircle> *const c2 = make_shared<GeoExactCircle>(
      PlanetModel::WGS84, -1.2097332228999564, 0.749061883738567,
      0.20701584142315682, 1.0E-5);
  assertTrue(L"cannot be disjoint",
             c1->getRelationship(c2) != GeoArea::DISJOINT);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLUCENE8065()
void GeoExactCircleTest::testLUCENE8065()
{
  // Circle planes are convex
  shared_ptr<GeoCircle> circle1 = GeoCircleFactory::makeExactGeoCircle(
      PlanetModel::WGS84, 0.03186456479560385, -2.2254294002683617,
      1.5702573535090856, 8.184299676008562E-6);
  shared_ptr<GeoCircle> circle2 = GeoCircleFactory::makeExactGeoCircle(
      PlanetModel::WGS84, 0.03186456479560385, -2.2254294002683617,
      1.5698163157923914, 1.0E-5);
  assertTrue(circle1->getRelationship(circle2) != GeoArea::DISJOINT);
}

void GeoExactCircleTest::testLUCENE8080()
{
  shared_ptr<PlanetModel> planetModel =
      make_shared<PlanetModel>(1.6304230055804751, 1.0199671157571204);
  bool fail = false;
  try {
    shared_ptr<GeoCircle> circle = GeoCircleFactory::makeExactGeoCircle(
        planetModel, 0.8853814403571284, 0.9784990176851283, 0.9071033527030907,
        1e-11);
  } catch (const invalid_argument &e) {
    fail = true;
  }
  assertTrue(fail);
}
} // namespace org::apache::lucene::spatial3d::geom