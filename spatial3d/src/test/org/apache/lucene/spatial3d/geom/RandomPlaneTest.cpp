using namespace std;

#include "RandomPlaneTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void testPlaneAccuracy()
void RandomPlaneTest::testPlaneAccuracy()
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  shared_ptr<GeoPoint> point1 = randomGeoPoint(planetModel);
  for (int i = 0; i < 1000; i++) {
    double dist = random()->nextDouble() * Vector::MINIMUM_ANGULAR_RESOLUTION +
                  Vector::MINIMUM_ANGULAR_RESOLUTION;
    double bearing = random()->nextDouble() * 2 * M_PI;
    shared_ptr<GeoPoint> point2 =
        planetModel->surfacePointOnBearing(point1, dist, bearing);
    shared_ptr<GeoPoint> check = randomGeoPoint(planetModel);
    if (!point1->isNumericallyIdentical(point2)) {
      shared_ptr<SidedPlane> plane =
          make_shared<SidedPlane>(check, point1, point2);
      wstring msg = to_wstring(dist) + L" point 1: " + point1 + L", point 2: " +
                    point2 + L" , check: " + check;
      assertTrue(msg, plane->isWithin(check));
      assertTrue(msg, plane->isWithin(point2));
      assertTrue(msg, plane->isWithin(point1));
    } else {
      assertFalse(L"numerically identical", true);
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
// testPlaneThreePointsAccuracy()
void RandomPlaneTest::testPlaneThreePointsAccuracy()
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  for (int i = 0; i < 1000; i++) {
    shared_ptr<GeoPoint> point1 = randomGeoPoint(planetModel);
    double dist =
        random()->nextDouble() * M_PI - Vector::MINIMUM_ANGULAR_RESOLUTION;
    double bearing = random()->nextDouble() * 2 * M_PI;
    shared_ptr<GeoPoint> point2 =
        planetModel->surfacePointOnBearing(point1, dist, bearing);
    dist = random()->nextDouble() * Vector::MINIMUM_ANGULAR_RESOLUTION +
           Vector::MINIMUM_ANGULAR_RESOLUTION;
    bearing = random()->nextDouble() * 2 * M_PI;
    shared_ptr<GeoPoint> point3 =
        planetModel->surfacePointOnBearing(point1, dist, bearing);
    shared_ptr<GeoPoint> check = randomGeoPoint(planetModel);
    shared_ptr<SidedPlane> plane =
        SidedPlane::constructNormalizedThreePointSidedPlane(check, point1,
                                                            point2, point3);
    wstring msg = planetModel + L" point 1: " + point1 + L", point 2: " +
                  point2 + L", point 3: " + point3 + L" , check: " + check;
    if (plane == nullptr) {
      fail(msg);
    }
    // This is not expected
    // assertTrue(plane.evaluate(check) + " " + msg, plane.isWithin(check));
    assertTrue(to_wstring(plane->evaluate(point1)) + L" " + msg,
               plane->isWithin(point1));
    assertTrue(to_wstring(plane->evaluate(point2)) + L" " + msg,
               plane->isWithin(point2));
    assertTrue(to_wstring(plane->evaluate(point3)) + L" " + msg,
               plane->isWithin(point3));
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
// testPolygonAccuracy()
void RandomPlaneTest::testPolygonAccuracy()
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  shared_ptr<GeoPoint> point1 = randomGeoPoint(planetModel);
  for (int i = 0; i < 1000; i++) {
    double dist =
        random()->nextDouble() * 1e-6 + Vector::MINIMUM_ANGULAR_RESOLUTION;
    shared_ptr<GeoPoint> point2 =
        planetModel->surfacePointOnBearing(point1, dist, 0);
    shared_ptr<GeoPoint> point3 =
        planetModel->surfacePointOnBearing(point1, dist, 0.5 * M_PI);

    deque<std::shared_ptr<GeoPoint>> points =
        deque<std::shared_ptr<GeoPoint>>();
    points.push_back(point1);
    points.push_back(point2);
    points.push_back(point3);
    GeoPolygonFactory::makeGeoPolygon(planetModel, points);
  }
}
} // namespace org::apache::lucene::spatial3d::geom