using namespace std;

#include "TestGeoUtils.h"

namespace org::apache::lucene::geo
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using SloppyMath = org::apache::lucene::util::SloppyMath;

void TestGeoUtils::testRandomCircleToBBox() 
{
  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {

    double centerLat = GeoTestUtil::nextLatitude();
    double centerLon = GeoTestUtil::nextLongitude();

    constexpr double radiusMeters;
    if (random()->nextBoolean()) {
      // Approx 4 degrees lon at the equator:
      radiusMeters = random()->nextDouble() * 444000;
    } else {
      radiusMeters = random()->nextDouble() * 50000000;
    }

    // TODO: randomly quantize radius too, to provoke exact math errors?

    shared_ptr<Rectangle> bbox =
        Rectangle::fromPointDistance(centerLat, centerLon, radiusMeters);

    int numPointsToTry = 1000;
    for (int i = 0; i < numPointsToTry; i++) {

      std::deque<double> point = GeoTestUtil::nextPointNear(bbox);
      double lat = point[0];
      double lon = point[1];

      double distanceMeters =
          SloppyMath::haversinMeters(centerLat, centerLon, lat, lon);

      // Haversin says it's within the circle:
      bool haversinSays = distanceMeters <= radiusMeters;

      // BBox says its within the box:
      bool bboxSays;
      if (bbox->crossesDateline()) {
        if (lat >= bbox->minLat && lat <= bbox->maxLat) {
          bboxSays = lon <= bbox->maxLon || lon >= bbox->minLon;
        } else {
          bboxSays = false;
        }
      } else {
        bboxSays = lat >= bbox->minLat && lat <= bbox->maxLat &&
                   lon >= bbox->minLon && lon <= bbox->maxLon;
      }

      if (haversinSays) {
        if (bboxSays == false) {
          wcout << L"centerLat=" << centerLat << L" centerLon=" << centerLon
                << L" radiusMeters=" << radiusMeters << endl;
          wcout << L"  bbox: lat=" << bbox->minLat << L" to " << bbox->maxLat
                << L" lon=" << bbox->minLon << L" to " << bbox->maxLon << endl;
          wcout << L"  point: lat=" << lat << L" lon=" << lon << endl;
          wcout << L"  haversin: " << distanceMeters << endl;
          fail(L"point was within the distance according to haversin, but the "
               L"bbox doesn't contain it");
        }
      } else {
        // it's fine if haversin said it was outside the radius and bbox said it
        // was inside the box
      }
    }
  }
}

void TestGeoUtils::testBoundingBoxOpto()
{
  int iters = atLeast(100);
  for (int i = 0; i < iters; i++) {
    double lat = GeoTestUtil::nextLatitude();
    double lon = GeoTestUtil::nextLongitude();
    double radius = 50000000 * random()->nextDouble();
    shared_ptr<Rectangle> box = Rectangle::fromPointDistance(lat, lon, radius);
    shared_ptr<Rectangle> *const box1;
    shared_ptr<Rectangle> *const box2;
    if (box->crossesDateline()) {
      box1 =
          make_shared<Rectangle>(box->minLat, box->maxLat, -180, box->maxLon);
      box2 = make_shared<Rectangle>(box->minLat, box->maxLat, box->minLon, 180);
    } else {
      box1 = box;
      box2.reset();
    }

    for (int j = 0; j < 1000; j++) {
      std::deque<double> point = GeoTestUtil::nextPointNear(box);
      double lat2 = point[0];
      double lon2 = point[1];
      // if the point is within radius, then it should be in our bounding box
      if (SloppyMath::haversinMeters(lat, lon, lat2, lon2) <= radius) {
        assertTrue(lat >= box->minLat && lat <= box->maxLat);
        assertTrue(
            lon >= box1->minLon && lon <= box1->maxLon ||
            (box2 != nullptr && lon >= box2->minLon && lon <= box2->maxLon));
      }
    }
  }
}

void TestGeoUtils::testHaversinOpto()
{
  int iters = atLeast(100);
  for (int i = 0; i < iters; i++) {
    double lat = GeoTestUtil::nextLatitude();
    double lon = GeoTestUtil::nextLongitude();
    double radius = 50000000 * random()->nextDouble();
    shared_ptr<Rectangle> box = Rectangle::fromPointDistance(lat, lon, radius);

    if (box->maxLon - lon < 90 && lon - box->minLon < 90) {
      double minPartialDistance =
          max(SloppyMath::haversinSortKey(lat, lon, lat, box->maxLon),
              SloppyMath::haversinSortKey(lat, lon, box->maxLat, lon));

      for (int j = 0; j < 10000; j++) {
        std::deque<double> point = GeoTestUtil::nextPointNear(box);
        double lat2 = point[0];
        double lon2 = point[1];
        // if the point is within radius, then it should be <= our sort key
        if (SloppyMath::haversinMeters(lat, lon, lat2, lon2) <= radius) {
          assertTrue(SloppyMath::haversinSortKey(lat, lon, lat2, lon2) <=
                     minPartialDistance);
        }
      }
    }
  }
}

void TestGeoUtils::testInfiniteRect()
{
  for (int i = 0; i < 1000; i++) {
    double centerLat = GeoTestUtil::nextLatitude();
    double centerLon = GeoTestUtil::nextLongitude();
    shared_ptr<Rectangle> rect = Rectangle::fromPointDistance(
        centerLat, centerLon, numeric_limits<double>::infinity());
    assertEquals(-180.0, rect->minLon, 0.0);
    assertEquals(180.0, rect->maxLon, 0.0);
    assertEquals(-90.0, rect->minLat, 0.0);
    assertEquals(90.0, rect->maxLat, 0.0);
    assertFalse(rect->crossesDateline());
  }
}

void TestGeoUtils::testAxisLat()
{
  double earthCircumference = 2 * M_PI * GeoUtils::EARTH_MEAN_RADIUS_METERS;
  assertEquals(90, Rectangle::axisLat(0, earthCircumference / 4), 0.0);

  for (int i = 0; i < 100; ++i) {
    bool reallyBig = random()->nextInt(10) == 0;
    constexpr double maxRadius =
        reallyBig ? 1.1 * earthCircumference : earthCircumference / 8;
    constexpr double radius = maxRadius * random()->nextDouble();
    double prevAxisLat = Rectangle::axisLat(0.0, radius);
    for (double lat = 0.1; lat < 90; lat += 0.1) {
      double nextAxisLat = Rectangle::axisLat(lat, radius);
      shared_ptr<Rectangle> bbox =
          Rectangle::fromPointDistance(lat, 180, radius);
      double dist =
          SloppyMath::haversinMeters(lat, 180, nextAxisLat, bbox->maxLon);
      if (nextAxisLat < GeoUtils::MAX_LAT_INCL) {
        assertEquals(L"lat = " + to_wstring(lat), dist, radius, 0.1);
      }
      assertTrue(L"lat = " + to_wstring(lat), prevAxisLat <= nextAxisLat);
      prevAxisLat = nextAxisLat;
    }

    prevAxisLat = Rectangle::axisLat(-0.0, radius);
    for (double lat = -0.1; lat > -90; lat -= 0.1) {
      double nextAxisLat = Rectangle::axisLat(lat, radius);
      shared_ptr<Rectangle> bbox =
          Rectangle::fromPointDistance(lat, 180, radius);
      double dist =
          SloppyMath::haversinMeters(lat, 180, nextAxisLat, bbox->maxLon);
      if (nextAxisLat > GeoUtils::MIN_LAT_INCL) {
        assertEquals(L"lat = " + to_wstring(lat), dist, radius, 0.1);
      }
      assertTrue(L"lat = " + to_wstring(lat), prevAxisLat >= nextAxisLat);
      prevAxisLat = nextAxisLat;
    }
  }
}

void TestGeoUtils::testCircleOpto() 
{
  int iters = atLeast(20);
  for (int i = 0; i < iters; i++) {
    // circle
    constexpr double centerLat = -90 + 180.0 * random()->nextDouble();
    constexpr double centerLon = -180 + 360.0 * random()->nextDouble();
    constexpr double radius = 50'000'000 * random()->nextDouble();
    shared_ptr<Rectangle> *const box =
        Rectangle::fromPointDistance(centerLat, centerLon, radius);
    // TODO: remove this leniency!
    if (box->crossesDateline()) {
      --i; // try again...
      continue;
    }
    constexpr double axisLat = Rectangle::axisLat(centerLat, radius);

    for (int k = 0; k < 1000; ++k) {

      std::deque<double> latBounds = {-90, box->minLat, axisLat, box->maxLat,
                                       90};
      std::deque<double> lonBounds = {-180, box->minLon, centerLon,
                                       box->maxLon, 180};
      // first choose an upper left corner
      int maxLatRow = random()->nextInt(4);
      double latMax =
          randomInRange(latBounds[maxLatRow], latBounds[maxLatRow + 1]);
      int minLonCol = random()->nextInt(4);
      double lonMin =
          randomInRange(lonBounds[minLonCol], lonBounds[minLonCol + 1]);
      // now choose a lower right corner
      int minLatMaxRow =
          maxLatRow == 3
              ? 3
              : maxLatRow + 1; // make sure it will at least cross into the bbox
      int minLatRow = random()->nextInt(minLatMaxRow);
      double latMin = randomInRange(latBounds[minLatRow],
                                    min(latBounds[minLatRow + 1], latMax));
      int maxLonMinCol =
          max(minLonCol, 1); // make sure it will at least cross into the bbox
      int maxLonCol = maxLonMinCol + random()->nextInt(4 - maxLonMinCol);
      double lonMax = randomInRange(max(lonBounds[maxLonCol], lonMin),
                                    lonBounds[maxLonCol + 1]);

      assert(latMax >= latMin);
      assert(lonMax >= lonMin);

      if (isDisjoint(centerLat, centerLon, radius, axisLat, latMin, latMax,
                     lonMin, lonMax)) {
        // intersects says false: test a ton of points
        for (int j = 0; j < 200; j++) {
          double lat = latMin + (latMax - latMin) * random()->nextDouble();
          double lon = lonMin + (lonMax - lonMin) * random()->nextDouble();

          if (random()->nextBoolean()) {
            // explicitly test an edge
            int edge = random()->nextInt(4);
            if (edge == 0) {
              lat = latMin;
            } else if (edge == 1) {
              lat = latMax;
            } else if (edge == 2) {
              lon = lonMin;
            } else if (edge == 3) {
              lon = lonMax;
            }
          }
          double distance =
              SloppyMath::haversinMeters(centerLat, centerLon, lat, lon);
          try {
            assertTrue(
                wstring::format(
                    Locale::ROOT,
                    wstring(L"\nisDisjoint(\n") + L"centerLat=%s\n" +
                        L"centerLon=%s\n" + L"radius=%s\n" + L"latMin=%s\n" +
                        L"latMax=%s\n" + L"lonMin=%s\n" +
                        L"lonMax=%s) == false BUT\n" +
                        L"haversin(%s, %s, %s, %s) = %s\nbbox=%s",
                    centerLat, centerLon, radius, latMin, latMax, lonMin,
                    lonMax, centerLat, centerLon, lat, lon, distance,
                    Rectangle::fromPointDistance(centerLat, centerLon, radius)),
                distance > radius);
          } catch (const AssertionError &e) {
            shared_ptr<EarthDebugger> ed = make_shared<EarthDebugger>();
            ed->addRect(latMin, latMax, lonMin, lonMax);
            ed->addCircle(centerLat, centerLon, radius, true);
            wcout << ed->finish() << endl;
            throw e;
          }
        }
      }
    }
  }
}

double TestGeoUtils::randomInRange(double min, double max)
{
  return min + (max - min) * random()->nextDouble();
}

bool TestGeoUtils::isDisjoint(double centerLat, double centerLon, double radius,
                              double axisLat, double latMin, double latMax,
                              double lonMin, double lonMax)
{
  if ((centerLon < lonMin || centerLon > lonMax) &&
      (axisLat + Rectangle::AXISLAT_ERROR < latMin ||
       axisLat - Rectangle::AXISLAT_ERROR > latMax)) {
    // circle not fully inside / crossing axis
    if (SloppyMath::haversinMeters(centerLat, centerLon, latMin, lonMin) >
            radius &&
        SloppyMath::haversinMeters(centerLat, centerLon, latMin, lonMax) >
            radius &&
        SloppyMath::haversinMeters(centerLat, centerLon, latMax, lonMin) >
            radius &&
        SloppyMath::haversinMeters(centerLat, centerLon, latMax, lonMax) >
            radius) {
      // no points inside
      return true;
    }
  }

  return false;
}

void TestGeoUtils::testWithin90LonDegrees()
{
  assertTrue(GeoUtils::within90LonDegrees(0, -80, 80));
  assertFalse(GeoUtils::within90LonDegrees(0, -100, 80));
  assertFalse(GeoUtils::within90LonDegrees(0, -80, 100));

  assertTrue(GeoUtils::within90LonDegrees(-150, 140, 170));
  assertFalse(GeoUtils::within90LonDegrees(-150, 120, 150));

  assertTrue(GeoUtils::within90LonDegrees(150, -170, -140));
  assertFalse(GeoUtils::within90LonDegrees(150, -150, -120));
}
} // namespace org::apache::lucene::geo