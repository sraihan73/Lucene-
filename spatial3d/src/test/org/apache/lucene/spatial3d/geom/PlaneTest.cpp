using namespace std;

#include "PlaneTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using org::junit::Test;
//    import static org.junit.Assert.assertFalse;
//    import static org.junit.Assert.assertTrue;
//    import static org.junit.Assert.assertEquals;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testIdenticalPlanes()
void PlaneTest::testIdenticalPlanes()
{
  shared_ptr<GeoPoint> *const p =
      make_shared<GeoPoint>(PlanetModel::SPHERE, 0.123, -0.456);
  shared_ptr<Plane> *const plane1 = make_shared<Plane>(p, 0.0);
  shared_ptr<Plane> *const plane2 = make_shared<Plane>(p, 0.0);
  assertTrue(plane1->isNumericallyIdentical(plane2));
  shared_ptr<Plane> *const plane3 = make_shared<Plane>(p, 0.1);
  assertFalse(plane1->isNumericallyIdentical(plane3));
  shared_ptr<Vector> *const v1 = make_shared<Vector>(0.1, -0.732, 0.9);
  constexpr double constant = 0.432;
  shared_ptr<Vector> *const v2 =
      make_shared<Vector>(v1->x * constant, v1->y * constant, v1->z * constant);
  shared_ptr<Plane> *const p1 = make_shared<Plane>(v1, 0.2);
  shared_ptr<Plane> *const p2 = make_shared<Plane>(v2, 0.2 * constant);
  assertTrue(p1->isNumericallyIdentical(p2));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testIdenticalVector()
void PlaneTest::testIdenticalVector()
{
  shared_ptr<Vector> *const v1 = make_shared<Vector>(1, 0, 0);
  shared_ptr<Vector> *const v2 = make_shared<Vector>(1, 0, 0);
  shared_ptr<Vector> *const v3 = make_shared<Vector>(-1, 0, 0);
  assertTrue(v1->isNumericallyIdentical(v2));
  assertFalse(v1->isNumericallyIdentical(v3));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testInterpolation()
void PlaneTest::testInterpolation()
{
  // [X=0.35168818443386646, Y=-0.19637966197066342, Z=0.9152870857244183],
  // [X=0.5003343189532654, Y=0.522128543226148, Z=0.6906861469771293],

  shared_ptr<GeoPoint> *const start = make_shared<GeoPoint>(
      0.35168818443386646, -0.19637966197066342, 0.9152870857244183);
  shared_ptr<GeoPoint> *const end = make_shared<GeoPoint>(
      0.5003343189532654, 0.522128543226148, 0.6906861469771293);

  // [A=-0.6135342247741855, B=0.21504338363863665, C=0.28188192383666794,
  // D=0.0, side=-1.0] internal? false;
  shared_ptr<Plane> *const p = make_shared<Plane>(
      -0.6135342247741855, 0.21504338363863665, 0.28188192383666794, 0.0);

  std::deque<std::shared_ptr<GeoPoint>> points =
      p->interpolate(start, end, std::deque<double>{0.25, 0.50, 0.75});

  for (auto point : points) {
    assertTrue(p->evaluateIsZero(point));
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFindArcPoints()
void PlaneTest::testFindArcPoints()
{
  // Create two points
  shared_ptr<GeoPoint> *const p1 =
      make_shared<GeoPoint>(PlanetModel::WGS84, 0.123, -0.456);
  shared_ptr<GeoPoint> *const p2 =
      make_shared<GeoPoint>(PlanetModel::WGS84, -0.368, 0.888);
  // Create a plane that links them.
  shared_ptr<Plane> *const plane = make_shared<Plane>(p1, p2);
  // Now, use that plane to find points that are a certain distance from the
  // original
  std::deque<std::shared_ptr<GeoPoint>> newPoints =
      plane->findArcDistancePoints(PlanetModel::WGS84, 0.20, p1);
  assertTrue(newPoints.size() == 2);
  assertTrue(plane->evaluateIsZero(newPoints[0]));
  assertTrue(plane->evaluateIsZero(newPoints[1]));
  assertTrue(PlanetModel::WGS84->pointOnSurface(newPoints[0]));
  assertTrue(PlanetModel::WGS84->pointOnSurface(newPoints[1]));
  assertEquals(0.20, p1->arcDistance(newPoints[0]), 1e-6);
  assertEquals(0.20, p1->arcDistance(newPoints[1]), 1e-6);
}
} // namespace org::apache::lucene::spatial3d::geom