using namespace std;

#include "GeoModelTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using org::junit::Test;
//    import static org.junit.Assert.assertEquals;
//    import static org.junit.Assert.assertFalse;
//    import static org.junit.Assert.assertTrue;
const shared_ptr<PlanetModel> GeoModelTest::scaledModel =
    make_shared<PlanetModel>(1.2, 1.5);

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicCircle()
void GeoModelTest::testBasicCircle()
{
  // The point of this test is just to make sure nothing blows up doing normal
  // things with a quite non-spherical model Make sure that the north pole is in
  // the circle, and south pole isn't
  shared_ptr<GeoPoint> *const northPole =
      make_shared<GeoPoint>(scaledModel, M_PI * 0.5, 0.0);
  shared_ptr<GeoPoint> *const southPole =
      make_shared<GeoPoint>(scaledModel, -M_PI * 0.5, 0.0);
  shared_ptr<GeoPoint> *const point1 =
      make_shared<GeoPoint>(scaledModel, M_PI * 0.25, 0.0);
  shared_ptr<GeoPoint> *const point2 =
      make_shared<GeoPoint>(scaledModel, M_PI * 0.125, 0.0);

  shared_ptr<GeoCircle> circle =
      make_shared<GeoStandardCircle>(scaledModel, M_PI * 0.5, 0.0, 0.01);
  assertTrue(circle->isWithin(northPole));
  assertFalse(circle->isWithin(southPole));
  assertFalse(circle->isWithin(point1));
  shared_ptr<LatLonBounds> bounds;
  bounds = make_shared<LatLonBounds>();
  circle->getBounds(bounds);
  assertTrue(bounds->checkNoLongitudeBound());
  assertTrue(bounds->checkNoTopLatitudeBound());
  assertFalse(bounds->checkNoBottomLatitudeBound());
  assertEquals(M_PI * 0.5 - 0.01, bounds->getMinLatitude(), 0.01);

  circle = make_shared<GeoStandardCircle>(scaledModel, M_PI * 0.25, 0.0, 0.01);
  assertTrue(circle->isWithin(point1));
  assertFalse(circle->isWithin(northPole));
  assertFalse(circle->isWithin(southPole));
  bounds = make_shared<LatLonBounds>();
  circle->getBounds(bounds);
  assertFalse(bounds->checkNoTopLatitudeBound());
  assertFalse(bounds->checkNoLongitudeBound());
  assertFalse(bounds->checkNoBottomLatitudeBound());
  assertEquals(M_PI * 0.25 + 0.01, bounds->getMaxLatitude(), 0.00001);
  assertEquals(M_PI * 0.25 - 0.01, bounds->getMinLatitude(), 0.00001);
  assertEquals(-0.0125, bounds->getLeftLongitude(), 0.0001);
  assertEquals(0.0125, bounds->getRightLongitude(), 0.0001);

  circle = make_shared<GeoStandardCircle>(scaledModel, M_PI * 0.125, 0.0, 0.01);
  assertTrue(circle->isWithin(point2));
  assertFalse(circle->isWithin(northPole));
  assertFalse(circle->isWithin(southPole));
  bounds = make_shared<LatLonBounds>();
  circle->getBounds(bounds);
  assertFalse(bounds->checkNoLongitudeBound());
  assertFalse(bounds->checkNoTopLatitudeBound());
  assertFalse(bounds->checkNoBottomLatitudeBound());
  // Symmetric, as expected
  assertEquals(M_PI * 0.125 - 0.01, bounds->getMinLatitude(), 0.00001);
  assertEquals(M_PI * 0.125 + 0.01, bounds->getMaxLatitude(), 0.00001);
  assertEquals(-0.0089, bounds->getLeftLongitude(), 0.0001);
  assertEquals(0.0089, bounds->getRightLongitude(), 0.0001);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicRectangle()
void GeoModelTest::testBasicRectangle()
{
  shared_ptr<GeoBBox> *const bbox =
      GeoBBoxFactory::makeGeoBBox(scaledModel, 1.0, 0.0, 0.0, 1.0);
  shared_ptr<GeoPoint> *const insidePoint =
      make_shared<GeoPoint>(scaledModel, 0.5, 0.5);
  assertTrue(bbox->isWithin(insidePoint));
  shared_ptr<GeoPoint> *const topOutsidePoint =
      make_shared<GeoPoint>(scaledModel, 1.01, 0.5);
  assertFalse(bbox->isWithin(topOutsidePoint));
  shared_ptr<GeoPoint> *const bottomOutsidePoint =
      make_shared<GeoPoint>(scaledModel, -0.01, 0.5);
  assertFalse(bbox->isWithin(bottomOutsidePoint));
  shared_ptr<GeoPoint> *const leftOutsidePoint =
      make_shared<GeoPoint>(scaledModel, 0.5, -0.01);
  assertFalse(bbox->isWithin(leftOutsidePoint));
  shared_ptr<GeoPoint> *const rightOutsidePoint =
      make_shared<GeoPoint>(scaledModel, 0.5, 1.01);
  assertFalse(bbox->isWithin(rightOutsidePoint));
  shared_ptr<LatLonBounds> *const bounds = make_shared<LatLonBounds>();
  bbox->getBounds(bounds);
  assertFalse(bounds->checkNoLongitudeBound());
  assertFalse(bounds->checkNoTopLatitudeBound());
  assertFalse(bounds->checkNoBottomLatitudeBound());
  assertEquals(1.0, bounds->getMaxLatitude(), 0.00001);
  assertEquals(0.0, bounds->getMinLatitude(), 0.00001);
  assertEquals(1.0, bounds->getRightLongitude(), 0.00001);
  assertEquals(0.0, bounds->getLeftLongitude(), 0.00001);
}
} // namespace org::apache::lucene::spatial3d::geom