using namespace std;

#include "TestSloppyMath.h"

namespace org::apache::lucene::util
{
//    import static org.apache.lucene.util.SloppyMath.cos;
//    import static org.apache.lucene.util.SloppyMath.asin;
//    import static org.apache.lucene.util.SloppyMath.haversinMeters;
//    import static org.apache.lucene.util.SloppyMath.haversinSortKey;
using GeoTestUtil = org::apache::lucene::geo::GeoTestUtil;
double TestSloppyMath::COS_DELTA = 1E-15;
double TestSloppyMath::ASIN_DELTA = 1E-7;
double TestSloppyMath::HAVERSIN_DELTA = 38E-2;
double TestSloppyMath::REASONABLE_HAVERSIN_DELTA = 1E-5;

void TestSloppyMath::testCos()
{
  assertTrue(isnan(cos(NAN)));
  assertTrue(isnan(cos(-numeric_limits<double>::infinity())));
  assertTrue(isnan(cos(numeric_limits<double>::infinity())));
  assertEquals(cos(1), cos(1), COS_DELTA);
  assertEquals(cos(0), cos(0), COS_DELTA);
  assertEquals(cos(M_PI / 2), cos(M_PI / 2), COS_DELTA);
  assertEquals(cos(-M_PI / 2), cos(-M_PI / 2), COS_DELTA);
  assertEquals(cos(M_PI / 4), cos(M_PI / 4), COS_DELTA);
  assertEquals(cos(-M_PI / 4), cos(-M_PI / 4), COS_DELTA);
  assertEquals(cos(M_PI * 2 / 3), cos(M_PI * 2 / 3), COS_DELTA);
  assertEquals(cos(-M_PI * 2 / 3), cos(-M_PI * 2 / 3), COS_DELTA);
  assertEquals(cos(M_PI / 6), cos(M_PI / 6), COS_DELTA);
  assertEquals(cos(-M_PI / 6), cos(-M_PI / 6), COS_DELTA);

  // testing purely random longs is inefficent, as for stupid parameters we just
  // pass thru to Math.cos() instead of doing some huperduper arg reduction
  for (int i = 0; i < 10000; i++) {
    double d =
        random()->nextDouble() * SloppyMath::SIN_COS_MAX_VALUE_FOR_INT_MODULO;
    if (random()->nextBoolean()) {
      d = -d;
    }
    assertEquals(cos(d), cos(d), COS_DELTA);
  }
}

void TestSloppyMath::testAsin()
{
  assertTrue(isnan(asin(NAN)));
  assertTrue(isnan(asin(2)));
  assertTrue(isnan(asin(-2)));
  assertEquals(-M_PI / 2, asin(-1), ASIN_DELTA);
  assertEquals(-M_PI / 3, asin(-0.8660254), ASIN_DELTA);
  assertEquals(-M_PI / 4, asin(-0.7071068), ASIN_DELTA);
  assertEquals(-M_PI / 6, asin(-0.5), ASIN_DELTA);
  assertEquals(0, asin(0), ASIN_DELTA);
  assertEquals(M_PI / 6, asin(0.5), ASIN_DELTA);
  assertEquals(M_PI / 4, asin(0.7071068), ASIN_DELTA);
  assertEquals(M_PI / 3, asin(0.8660254), ASIN_DELTA);
  assertEquals(M_PI / 2, asin(1), ASIN_DELTA);
  // only values -1..1 are useful
  for (int i = 0; i < 10000; i++) {
    double d = random()->nextDouble();
    if (random()->nextBoolean()) {
      d = -d;
    }
    assertEquals(asin(d), asin(d), ASIN_DELTA);
    assertTrue(asin(d) >= -M_PI / 2);
    assertTrue(asin(d) <= M_PI / 2);
  }
}

void TestSloppyMath::testHaversin()
{
  assertTrue(isnan(haversinMeters(1, 1, 1, NAN)));
  assertTrue(isnan(haversinMeters(1, 1, NAN, 1)));
  assertTrue(isnan(haversinMeters(1, NAN, 1, 1)));
  assertTrue(isnan(haversinMeters(NAN, 1, 1, 1)));

  assertEquals(0, haversinMeters(0, 0, 0, 0), 0);
  assertEquals(0, haversinMeters(0, -180, 0, -180), 0);
  assertEquals(0, haversinMeters(0, -180, 0, 180), 0);
  assertEquals(0, haversinMeters(0, 180, 0, 180), 0);
  assertEquals(0, haversinMeters(90, 0, 90, 0), 0);
  assertEquals(0, haversinMeters(90, -180, 90, -180), 0);
  assertEquals(0, haversinMeters(90, -180, 90, 180), 0);
  assertEquals(0, haversinMeters(90, 180, 90, 180), 0);

  // Test half a circle on the equator, using WGS84 mean earth radius in meters
  double earthRadiusMs = 6'371'008.7714;
  double halfCircle = earthRadiusMs * M_PI;
  assertEquals(halfCircle, haversinMeters(0, 0, 0, 180), 0);

  shared_ptr<Random> r = random();
  double randomLat1 = 40.7143528 + (r->nextInt(10) - 5) * 360;
  double randomLon1 = -74.0059731 + (r->nextInt(10) - 5) * 360;

  double randomLat2 = 40.65 + (r->nextInt(10) - 5) * 360;
  double randomLon2 = -73.95 + (r->nextInt(10) - 5) * 360;

  assertEquals(8'572.1137,
               haversinMeters(randomLat1, randomLon1, randomLat2, randomLon2),
               0.01);

  // from solr and ES tests (with their respective epsilons)
  assertEquals(
      0, haversinMeters(40.7143528, -74.0059731, 40.7143528, -74.0059731), 0);
  assertEquals(5'285.89,
               haversinMeters(40.7143528, -74.0059731, 40.759011, -73.9844722),
               0.01);
  assertEquals(462.10,
               haversinMeters(40.7143528, -74.0059731, 40.718266, -74.007819),
               0.01);
  assertEquals(1'054.98,
               haversinMeters(40.7143528, -74.0059731, 40.7051157, -74.0088305),
               0.01);
  assertEquals(1'258.12,
               haversinMeters(40.7143528, -74.0059731, 40.7247222, -74), 0.01);
  assertEquals(2'028.52,
               haversinMeters(40.7143528, -74.0059731, 40.731033, -73.9962255),
               0.01);
  assertEquals(8'572.11, haversinMeters(40.7143528, -74.0059731, 40.65, -73.95),
               0.01);
}

void TestSloppyMath::testHaversinSortKey()
{
  for (int i = 0; i < 100000; i++) {
    double centerLat = GeoTestUtil::nextLatitude();
    double centerLon = GeoTestUtil::nextLongitude();

    double lat1 = GeoTestUtil::nextLatitude();
    double lon1 = GeoTestUtil::nextLongitude();

    double lat2 = GeoTestUtil::nextLatitude();
    double lon2 = GeoTestUtil::nextLongitude();

    int expected = Integer::signum(
        Double::compare(haversinMeters(centerLat, centerLon, lat1, lon1),
                        haversinMeters(centerLat, centerLon, lat2, lon2)));
    int actual = Integer::signum(
        Double::compare(haversinSortKey(centerLat, centerLon, lat1, lon1),
                        haversinSortKey(centerLat, centerLon, lat2, lon2)));
    assertEquals(expected, actual);
    assertEquals(
        haversinMeters(centerLat, centerLon, lat1, lon1),
        haversinMeters(haversinSortKey(centerLat, centerLon, lat1, lon1)), 0.0);
    assertEquals(
        haversinMeters(centerLat, centerLon, lat2, lon2),
        haversinMeters(haversinSortKey(centerLat, centerLon, lat2, lon2)), 0.0);
  }
}

void TestSloppyMath::testHaversinFromSortKey()
{
  assertEquals(0.0, haversinMeters(0), 0.0);
}

void TestSloppyMath::testAgainstSlowVersion()
{
  for (int i = 0; i < 100'000; i++) {
    double lat1 = GeoTestUtil::nextLatitude();
    double lon1 = GeoTestUtil::nextLongitude();
    double lat2 = GeoTestUtil::nextLatitude();
    double lon2 = GeoTestUtil::nextLongitude();

    double expected = slowHaversin(lat1, lon1, lat2, lon2);
    double actual = haversinMeters(lat1, lon1, lat2, lon2);
    assertEquals(expected, actual, HAVERSIN_DELTA);
  }
}

void TestSloppyMath::testAcrossWholeWorldSteps()
{
  for (int lat1 = -90; lat1 <= 90; lat1 += 10) {
    for (int lon1 = -180; lon1 <= 180; lon1 += 10) {
      for (int lat2 = -90; lat2 <= 90; lat2 += 10) {
        for (int lon2 = -180; lon2 <= 180; lon2 += 10) {
          double expected = slowHaversin(lat1, lon1, lat2, lon2);
          double actual = haversinMeters(lat1, lon1, lat2, lon2);
          assertEquals(expected, actual, HAVERSIN_DELTA);
        }
      }
    }
  }
}

void TestSloppyMath::testAgainstSlowVersionReasonable()
{
  for (int i = 0; i < 100'000; i++) {
    double lat1 = GeoTestUtil::nextLatitude();
    double lon1 = GeoTestUtil::nextLongitude();
    double lat2 = GeoTestUtil::nextLatitude();
    double lon2 = GeoTestUtil::nextLongitude();

    double expected = haversinMeters(lat1, lon1, lat2, lon2);
    if (expected < 1'000'000) {
      double actual = slowHaversin(lat1, lon1, lat2, lon2);
      assertEquals(expected, actual, REASONABLE_HAVERSIN_DELTA);
    }
  }
}

double TestSloppyMath::slowHaversin(double lat1, double lon1, double lat2,
                                    double lon2)
{
  double h1 = (1 - cos(Math::toRadians(lat2) - Math::toRadians(lat1))) / 2;
  double h2 = (1 - cos(Math::toRadians(lon2) - Math::toRadians(lon1))) / 2;
  double h = h1 + cos(Math::toRadians(lat1)) * cos(Math::toRadians(lat2)) * h2;
  return 2 * 6371008.7714 * asin(min(1, sqrt(h)));
}
} // namespace org::apache::lucene::util