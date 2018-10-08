using namespace std;

#include "TestPolygon2D.h"

namespace org::apache::lucene::geo
{
//    import static org.apache.lucene.geo.GeoTestUtil.nextLatitude;
//    import static org.apache.lucene.geo.GeoTestUtil.nextLongitude;
//    import static org.apache.lucene.geo.GeoTestUtil.nextPolygon;
using Relation = org::apache::lucene::index::PointValues::Relation;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPolygon2D::testMultiPolygon()
{
  shared_ptr<Polygon> hole =
      make_shared<Polygon>(std::deque<double>{-10, -10, 10, 10, -10},
                           std::deque<double>{-10, 10, 10, -10, -10});
  shared_ptr<Polygon> outer =
      make_shared<Polygon>(std::deque<double>{-50, -50, 50, 50, -50},
                           std::deque<double>{-50, 50, 50, -50, -50}, hole);
  shared_ptr<Polygon> island =
      make_shared<Polygon>(std::deque<double>{-5, -5, 5, 5, -5},
                           std::deque<double>{-5, 5, 5, -5, -5});
  shared_ptr<Polygon2D> polygon = Polygon2D::create({outer, island});

  // contains(point)
  assertTrue(polygon->contains(-2, 2));    // on the island
  assertFalse(polygon->contains(-6, 6));   // in the hole
  assertTrue(polygon->contains(-25, 25));  // on the mainland
  assertFalse(polygon->contains(-51, 51)); // in the ocean

  // relate(box): this can conservatively return CELL_CROSSES_QUERY
  assertEquals(Relation::CELL_INSIDE_QUERY,
               polygon->relate(-2, 2, -2, 2)); // on the island
  assertEquals(Relation::CELL_OUTSIDE_QUERY,
               polygon->relate(6, 7, 6, 7)); // in the hole
  assertEquals(Relation::CELL_INSIDE_QUERY,
               polygon->relate(24, 25, 24, 25)); // on the mainland
  assertEquals(Relation::CELL_OUTSIDE_QUERY,
               polygon->relate(51, 52, 51, 52)); // in the ocean
  assertEquals(Relation::CELL_CROSSES_QUERY,
               polygon->relate(-60, 60, -60, 60)); // enclosing us completely
  assertEquals(Relation::CELL_CROSSES_QUERY,
               polygon->relate(49, 51, 49, 51)); // overlapping the mainland
  assertEquals(Relation::CELL_CROSSES_QUERY,
               polygon->relate(9, 11, 9, 11)); // overlapping the hole
  assertEquals(Relation::CELL_CROSSES_QUERY,
               polygon->relate(5, 6, 5, 6)); // overlapping the island
}

void TestPolygon2D::testPacMan() 
{
  // pacman
  std::deque<double> px = {0, 10, 10, 0, -8, -10, -8, 0, 10, 10, 0};
  std::deque<double> py = {0, 5, 9, 10, 9, 0, -9, -10, -9, -5, 0};

  // candidate crosses cell
  double xMin = 2;  //-5;
  double xMax = 11; // 0.000001;
  double yMin = -1; // 0;
  double yMax = 1;  // 5;

  // test cell crossing poly
  shared_ptr<Polygon2D> polygon =
      Polygon2D::create({make_shared<Polygon>(py, px)});
  assertEquals(Relation::CELL_CROSSES_QUERY,
               polygon->relate(yMin, yMax, xMin, xMax));
}

void TestPolygon2D::testBoundingBox() 
{
  for (int i = 0; i < 100; i++) {
    shared_ptr<Polygon2D> polygon = Polygon2D::create({nextPolygon()});

    for (int j = 0; j < 100; j++) {
      double latitude = nextLatitude();
      double longitude = nextLongitude();
      // if the point is within poly, then it should be in our bounding box
      if (polygon->contains(latitude, longitude)) {
        assertTrue(latitude >= polygon->minLat && latitude <= polygon->maxLat);
        assertTrue(longitude >= polygon->minLon &&
                   longitude <= polygon->maxLon);
      }
    }
  }
}

void TestPolygon2D::testBoundingBoxEdgeCases() 
{
  for (int i = 0; i < 100; i++) {
    shared_ptr<Polygon> polygon = nextPolygon();
    shared_ptr<Polygon2D> impl = Polygon2D::create({polygon});

    for (int j = 0; j < 100; j++) {
      std::deque<double> point = GeoTestUtil::nextPointNear(polygon);
      double latitude = point[0];
      double longitude = point[1];
      // if the point is within poly, then it should be in our bounding box
      if (impl->contains(latitude, longitude)) {
        assertTrue(latitude >= polygon->minLat && latitude <= polygon->maxLat);
        assertTrue(longitude >= polygon->minLon &&
                   longitude <= polygon->maxLon);
      }
    }
  }
}

void TestPolygon2D::testContainsRandom() 
{
  int iters = atLeast(50);
  for (int i = 0; i < iters; i++) {
    shared_ptr<Polygon> polygon = nextPolygon();
    shared_ptr<Polygon2D> impl = Polygon2D::create({polygon});

    for (int j = 0; j < 100; j++) {
      shared_ptr<Rectangle> rectangle = GeoTestUtil::nextBoxNear(polygon);
      // allowed to conservatively return false
      if (impl->relate(rectangle->minLat, rectangle->maxLat, rectangle->minLon,
                       rectangle->maxLon) == Relation::CELL_INSIDE_QUERY) {
        for (int k = 0; k < 500; k++) {
          // this tests in our range but sometimes outside! so we have to
          // double-check its really in other box
          std::deque<double> point = GeoTestUtil::nextPointNear(rectangle);
          double latitude = point[0];
          double longitude = point[1];
          // check for sure its in our box
          if (latitude >= rectangle->minLat && latitude <= rectangle->maxLat &&
              longitude >= rectangle->minLon &&
              longitude <= rectangle->maxLon) {
            assertTrue(impl->contains(latitude, longitude));
          }
        }
        for (int k = 0; k < 100; k++) {
          // this tests in our range but sometimes outside! so we have to
          // double-check its really in other box
          std::deque<double> point = GeoTestUtil::nextPointNear(polygon);
          double latitude = point[0];
          double longitude = point[1];
          // check for sure its in our box
          if (latitude >= rectangle->minLat && latitude <= rectangle->maxLat &&
              longitude >= rectangle->minLon &&
              longitude <= rectangle->maxLon) {
            assertTrue(impl->contains(latitude, longitude));
          }
        }
      }
    }
  }
}

void TestPolygon2D::testContainsEdgeCases() 
{
  for (int i = 0; i < 1000; i++) {
    shared_ptr<Polygon> polygon = nextPolygon();
    shared_ptr<Polygon2D> impl = Polygon2D::create({polygon});

    for (int j = 0; j < 10; j++) {
      shared_ptr<Rectangle> rectangle = GeoTestUtil::nextBoxNear(polygon);
      // allowed to conservatively return false
      if (impl->relate(rectangle->minLat, rectangle->maxLat, rectangle->minLon,
                       rectangle->maxLon) == Relation::CELL_INSIDE_QUERY) {
        for (int k = 0; k < 100; k++) {
          // this tests in our range but sometimes outside! so we have to
          // double-check its really in other box
          std::deque<double> point = GeoTestUtil::nextPointNear(rectangle);
          double latitude = point[0];
          double longitude = point[1];
          // check for sure its in our box
          if (latitude >= rectangle->minLat && latitude <= rectangle->maxLat &&
              longitude >= rectangle->minLon &&
              longitude <= rectangle->maxLon) {
            assertTrue(impl->contains(latitude, longitude));
          }
        }
        for (int k = 0; k < 20; k++) {
          // this tests in our range but sometimes outside! so we have to
          // double-check its really in other box
          std::deque<double> point = GeoTestUtil::nextPointNear(polygon);
          double latitude = point[0];
          double longitude = point[1];
          // check for sure its in our box
          if (latitude >= rectangle->minLat && latitude <= rectangle->maxLat &&
              longitude >= rectangle->minLon &&
              longitude <= rectangle->maxLon) {
            assertTrue(impl->contains(latitude, longitude));
          }
        }
      }
    }
  }
}

void TestPolygon2D::testIntersectRandom()
{
  int iters = atLeast(10);
  for (int i = 0; i < iters; i++) {
    shared_ptr<Polygon> polygon = nextPolygon();
    shared_ptr<Polygon2D> impl = Polygon2D::create({polygon});

    for (int j = 0; j < 100; j++) {
      shared_ptr<Rectangle> rectangle = GeoTestUtil::nextBoxNear(polygon);
      // allowed to conservatively return true.
      if (impl->relate(rectangle->minLat, rectangle->maxLat, rectangle->minLon,
                       rectangle->maxLon) == Relation::CELL_OUTSIDE_QUERY) {
        for (int k = 0; k < 1000; k++) {
          std::deque<double> point = GeoTestUtil::nextPointNear(rectangle);
          // this tests in our range but sometimes outside! so we have to
          // double-check its really in other box
          double latitude = point[0];
          double longitude = point[1];
          // check for sure its in our box
          if (latitude >= rectangle->minLat && latitude <= rectangle->maxLat &&
              longitude >= rectangle->minLon &&
              longitude <= rectangle->maxLon) {
            assertFalse(impl->contains(latitude, longitude));
          }
        }
        for (int k = 0; k < 100; k++) {
          std::deque<double> point = GeoTestUtil::nextPointNear(polygon);
          // this tests in our range but sometimes outside! so we have to
          // double-check its really in other box
          double latitude = point[0];
          double longitude = point[1];
          // check for sure its in our box
          if (latitude >= rectangle->minLat && latitude <= rectangle->maxLat &&
              longitude >= rectangle->minLon &&
              longitude <= rectangle->maxLon) {
            assertFalse(impl->contains(latitude, longitude));
          }
        }
      }
    }
  }
}

void TestPolygon2D::testIntersectEdgeCases()
{
  for (int i = 0; i < 100; i++) {
    shared_ptr<Polygon> polygon = nextPolygon();
    shared_ptr<Polygon2D> impl = Polygon2D::create({polygon});

    for (int j = 0; j < 10; j++) {
      shared_ptr<Rectangle> rectangle = GeoTestUtil::nextBoxNear(polygon);
      // allowed to conservatively return false.
      if (impl->relate(rectangle->minLat, rectangle->maxLat, rectangle->minLon,
                       rectangle->maxLon) == Relation::CELL_OUTSIDE_QUERY) {
        for (int k = 0; k < 100; k++) {
          // this tests in our range but sometimes outside! so we have to
          // double-check its really in other box
          std::deque<double> point = GeoTestUtil::nextPointNear(rectangle);
          double latitude = point[0];
          double longitude = point[1];
          // check for sure its in our box
          if (latitude >= rectangle->minLat && latitude <= rectangle->maxLat &&
              longitude >= rectangle->minLon &&
              longitude <= rectangle->maxLon) {
            assertFalse(impl->contains(latitude, longitude));
          }
        }
        for (int k = 0; k < 50; k++) {
          // this tests in our range but sometimes outside! so we have to
          // double-check its really in other box
          std::deque<double> point = GeoTestUtil::nextPointNear(polygon);
          double latitude = point[0];
          double longitude = point[1];
          // check for sure its in our box
          if (latitude >= rectangle->minLat && latitude <= rectangle->maxLat &&
              longitude >= rectangle->minLon &&
              longitude <= rectangle->maxLon) {
            assertFalse(impl->contains(latitude, longitude));
          }
        }
      }
    }
  }
}

void TestPolygon2D::testEdgeInsideness()
{
  shared_ptr<Polygon2D> poly = Polygon2D::create(
      {make_shared<Polygon>(std::deque<double>{-2, -2, 2, 2, -2},
                            std::deque<double>{-2, 2, 2, -2, -2})});
  assertTrue(poly->contains(-2, -2)); // bottom left corner: true
  assertFalse(poly->contains(-2, 2)); // bottom right corner: false
  assertFalse(poly->contains(2, -2)); // top left corner: false
  assertFalse(poly->contains(2, 2));  // top right corner: false
  assertTrue(poly->contains(-2, -1)); // bottom side: true
  assertTrue(poly->contains(-2, 0));  // bottom side: true
  assertTrue(poly->contains(-2, 1));  // bottom side: true
  assertFalse(poly->contains(2, -1)); // top side: false
  assertFalse(poly->contains(2, 0));  // top side: false
  assertFalse(poly->contains(2, 1));  // top side: false
  assertFalse(poly->contains(-1, 2)); // right side: false
  assertFalse(poly->contains(0, 2));  // right side: false
  assertFalse(poly->contains(1, 2));  // right side: false
  assertTrue(poly->contains(-1, -2)); // left side: true
  assertTrue(poly->contains(0, -2));  // left side: true
  assertTrue(poly->contains(1, -2));  // left side: true
}

void TestPolygon2D::testContainsAgainstOriginal()
{
  int iters = atLeast(100);
  for (int i = 0; i < iters; i++) {
    shared_ptr<Polygon> polygon = nextPolygon();
    // currently we don't generate these, but this test does not want holes.
    while (polygon->getHoles().size() > 0) {
      polygon = nextPolygon();
    }
    shared_ptr<Polygon2D> impl = Polygon2D::create({polygon});

    // random lat/lons against polygon
    for (int j = 0; j < 1000; j++) {
      std::deque<double> point = GeoTestUtil::nextPointNear(polygon);
      double latitude = point[0];
      double longitude = point[1];
      bool expected = GeoTestUtil::containsSlowly(polygon, latitude, longitude);
      assertEquals(expected, impl->contains(latitude, longitude));
    }
  }
}
} // namespace org::apache::lucene::geo