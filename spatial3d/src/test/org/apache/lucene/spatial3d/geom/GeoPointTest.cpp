using namespace std;

#include "GeoPointTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomFloat;
const double GeoPointTest::DEGREES_TO_RADIANS = M_PI / 180;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testConversion()
void GeoPointTest::testConversion()
{
  testPointRoundTrip(PlanetModel::SPHERE, 90 * DEGREES_TO_RADIANS, 0, 1e-6);
  testPointRoundTrip(PlanetModel::SPHERE, -90 * DEGREES_TO_RADIANS, 0, 1e-6);
  testPointRoundTrip(PlanetModel::WGS84, 90 * DEGREES_TO_RADIANS, 0, 1e-6);
  testPointRoundTrip(PlanetModel::WGS84, -90 * DEGREES_TO_RADIANS, 0, 1e-6);

  constexpr int times = atLeast(100);
  for (int i = 0; i < times; i++) {
    constexpr double pLat = (randomFloat() * 180.0 - 90.0) * DEGREES_TO_RADIANS;
    constexpr double pLon =
        (randomFloat() * 360.0 - 180.0) * DEGREES_TO_RADIANS;
    testPointRoundTrip(
        PlanetModel::SPHERE, pLat, pLon,
        1e-6); // 1e-6 since there's a square root in there (Karl says)
    testPointRoundTrip(PlanetModel::WGS84, pLat, pLon, 1e-6);
  }
}

void GeoPointTest::testPointRoundTrip(shared_ptr<PlanetModel> planetModel,
                                      double pLat, double pLon, double epsilon)
{
  shared_ptr<GeoPoint> *const p1 =
      make_shared<GeoPoint>(planetModel, pLat, pLon);
  // In order to force the reverse conversion, we have to construct a geopoint
  // from just x,y,z
  shared_ptr<GeoPoint> *const p2 = make_shared<GeoPoint>(p1->x, p1->y, p1->z);
  // Now, construct the final point based on getLatitude() and getLongitude()
  shared_ptr<GeoPoint> *const p3 =
      make_shared<GeoPoint>(planetModel, p2->getLatitude(), p2->getLongitude());
  double dist = p1->arcDistance(p3);
  assertEquals(0, dist, epsilon);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSurfaceDistance()
void GeoPointTest::testSurfaceDistance()
{
  constexpr int times = atLeast(100);
  for (int i = 0; i < times; i++) {
    constexpr double p1Lat =
        (randomFloat() * 180.0 - 90.0) * DEGREES_TO_RADIANS;
    constexpr double p1Lon =
        (randomFloat() * 360.0 - 180.0) * DEGREES_TO_RADIANS;
    constexpr double p2Lat =
        (randomFloat() * 180.0 - 90.0) * DEGREES_TO_RADIANS;
    constexpr double p2Lon =
        (randomFloat() * 360.0 - 180.0) * DEGREES_TO_RADIANS;
    shared_ptr<GeoPoint> *const p1 =
        make_shared<GeoPoint>(PlanetModel::SPHERE, p1Lat, p1Lon);
    shared_ptr<GeoPoint> *const p2 =
        make_shared<GeoPoint>(PlanetModel::SPHERE, p2Lat, p2Lon);
    constexpr double arcDistance = p1->arcDistance(p2);
    // Compute ellipsoid distance; it should agree for a sphere
    constexpr double surfaceDistance =
        PlanetModel::SPHERE->surfaceDistance(p1, p2);
    assertEquals(arcDistance, surfaceDistance, 1e-6);
  }

  // Now try some WGS84 points (taken randomly and compared against a known-good
  // implementation)
  assertEquals(
      1.1444648695765323,
      PlanetModel::WGS84->surfaceDistance(
          make_shared<GeoPoint>(PlanetModel::WGS84, 0.038203808753702884,
                                -0.6701260455506466),
          make_shared<GeoPoint>(PlanetModel::WGS84, -0.8453720422675458,
                                0.1737353153814496)),
      1e-6);
  assertEquals(
      1.4345148695890722,
      PlanetModel::WGS84->surfaceDistance(
          make_shared<GeoPoint>(PlanetModel::WGS84, 0.5220926323378574,
                                0.6758041581907408),
          make_shared<GeoPoint>(PlanetModel::WGS84, -0.8453720422675458,
                                0.1737353153814496)),
      1e-6);
  assertEquals(
      2.32418144616446,
      PlanetModel::WGS84->surfaceDistance(
          make_shared<GeoPoint>(PlanetModel::WGS84, 0.09541335760967473,
                                1.2091829760623236),
          make_shared<GeoPoint>(PlanetModel::WGS84, -0.8501591797459979,
                                -2.3044806381627594)),
      1e-6);
  assertEquals(
      2.018421047005435,
      PlanetModel::WGS84->surfaceDistance(
          make_shared<GeoPoint>(PlanetModel::WGS84, 0.3402853531962009,
                                -0.43544195327249957),
          make_shared<GeoPoint>(PlanetModel::WGS84, -0.8501591797459979,
                                -2.3044806381627594)),
      1e-6);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBisection()
void GeoPointTest::testBisection()
{
  constexpr int times = atLeast(100);
  for (int i = 0; i < times; i++) {
    constexpr double p1Lat =
        (randomFloat() * 180.0 - 90.0) * DEGREES_TO_RADIANS;
    constexpr double p1Lon =
        (randomFloat() * 360.0 - 180.0) * DEGREES_TO_RADIANS;
    constexpr double p2Lat =
        (randomFloat() * 180.0 - 90.0) * DEGREES_TO_RADIANS;
    constexpr double p2Lon =
        (randomFloat() * 360.0 - 180.0) * DEGREES_TO_RADIANS;
    shared_ptr<GeoPoint> *const p1 =
        make_shared<GeoPoint>(PlanetModel::WGS84, p1Lat, p1Lon);
    shared_ptr<GeoPoint> *const p2 =
        make_shared<GeoPoint>(PlanetModel::WGS84, p2Lat, p2Lon);
    shared_ptr<GeoPoint> *const pMid = PlanetModel::WGS84->bisection(p1, p2);
    if (pMid != nullptr) {
      constexpr double arcDistance = p1->arcDistance(p2);
      constexpr double sum = pMid->arcDistance(p1) + pMid->arcDistance(p2);
      assertEquals(arcDistance, sum, 1e-6);
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testBadLatLon()
void GeoPointTest::testBadLatLon()
{
  make_shared<GeoPoint>(PlanetModel::SPHERE, 50.0, 32.2);
}
} // namespace org::apache::lucene::spatial3d::geom