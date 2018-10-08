using namespace std;

#include "GeoConvexPolygonTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using org::junit::Test;
//    import static org.junit.Assert.assertEquals;
//    import static org.junit.Assert.assertFalse;
//    import static org.junit.Assert.assertTrue;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPolygonPointWithin()
void GeoConvexPolygonTest::testPolygonPointWithin()
{
  shared_ptr<GeoConvexPolygon> c;
  shared_ptr<GeoPoint> gp;
  c = make_shared<GeoConvexPolygon>(PlanetModel::SPHERE, -0.1, -0.5);
  c->addPoint(0.0, -0.6, false);
  c->addPoint(0.1, -0.5, false);
  c->addPoint(0.0, -0.4, false);
  c->done(false);
  // Sample some points within
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, -0.5);
  assertTrue(c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, -0.55);
  assertTrue(c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, -0.45);
  assertTrue(c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -0.05, -0.5);
  assertTrue(c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.05, -0.5);
  assertTrue(c->isWithin(gp));
  // Sample some nearby points outside, and compute distance-to-shape for them
  // as well
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, -0.65);
  assertFalse(c->isWithin(gp));
  assertEquals(0.05, c->computeOutsideDistance(DistanceStyle::ARC, gp), 1e-12);
  assertEquals(0.05, c->computeOutsideDistance(DistanceStyle::NORMAL, gp),
               1e-3);
  assertEquals(0.05, c->computeOutsideDistance(DistanceStyle::LINEAR, gp),
               1e-3);
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, -0.35);
  assertFalse(c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, -0.15, -0.5);
  assertFalse(c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.15, -0.5);
  assertFalse(c->isWithin(gp));
  // Random points outside
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, 0.0);
  assertFalse(c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, M_PI * 0.5, 0.0);
  assertFalse(c->isWithin(gp));
  gp = make_shared<GeoPoint>(PlanetModel::SPHERE, 0.0, M_PI);
  assertFalse(c->isWithin(gp));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPolygonBounds()
void GeoConvexPolygonTest::testPolygonBounds()
{
  shared_ptr<GeoConvexPolygon> c;
  shared_ptr<LatLonBounds> b;

  c = make_shared<GeoConvexPolygon>(PlanetModel::SPHERE, -0.1, -0.5);
  c->addPoint(0.0, -0.6, false);
  c->addPoint(0.1, -0.5, false);
  c->addPoint(0.0, -0.4, false);
  c->done(false);

  b = make_shared<LatLonBounds>();
  c->getBounds(b);
  assertFalse(b->checkNoLongitudeBound());
  assertFalse(b->checkNoTopLatitudeBound());
  assertFalse(b->checkNoBottomLatitudeBound());
  assertEquals(-0.6, b->getLeftLongitude(), 0.000001);
  assertEquals(-0.4, b->getRightLongitude(), 0.000001);
  assertEquals(-0.1, b->getMinLatitude(), 0.000001);
  assertEquals(0.1, b->getMaxLatitude(), 0.000001);
}
} // namespace org::apache::lucene::spatial3d::geom