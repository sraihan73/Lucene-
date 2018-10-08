using namespace std;

#include "SimpleGeoPolygonRelationshipsTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using org::junit::Test;
//    import static org.junit.Assert.assertEquals;
//    import static org.junit.Assert.assertTrue;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygon1()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygon1()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 19.845091 -60.452631)) disjoint
  shared_ptr<GeoPolygon> originalConvexPol =
      buildConvexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                            23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalConcavePol =
      buildConcaveGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalComplexPol =
      buildComplexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> polConvex =
      buildConvexGeoPolygon(20.0, -60.4, 20.1, -60.4, 20.1, -60.3, 20.0, -60.3);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.0, -60.4, 20.1, -60.4, 20.1, -60.3, 20.0, -60.3);

  // Convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::WITHIN, rel); // Check

  // Concave
  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // Complex
  rel = originalComplexPol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalComplexPol);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = originalComplexPol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(originalComplexPol);
  assertEquals(GeoArea::WITHIN, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygon2()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygon2()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 19.845091 -60.452631)) disjoint
  shared_ptr<GeoPolygon> originalConvexPol =
      buildConvexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                            23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalConcavePol =
      buildConcaveGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalComplexPol =
      buildComplexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  // POLYGON ((20.0 -60.4, 23.1 -60.4, 23.1 -60.3, 20.0  -60.3,20.0 -60.4))
  shared_ptr<GeoPolygon> polConvex =
      buildConvexGeoPolygon(20.0, -60.4, 23.1, -60.4, 23.1, -60.3, 20.0, -60.3);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.0, -60.4, 23.1, -60.4, 23.1, -60.3, 20.0, -60.3);

  // Convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // Concave
  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // Complex
  rel = originalComplexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalComplexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalComplexPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalComplexPol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygon3()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygon3()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 19.845091 -60.452631)) disjoint
  shared_ptr<GeoPolygon> originalConvexPol =
      buildConvexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                            23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalConcavePol =
      buildConcaveGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalComplexPol =
      buildComplexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  // POLYGON ((20.0 -61.1, 20.1 -61.1, 20.1 -60.5, 20.0  -60.5,20.0 -61.1))
  shared_ptr<GeoPolygon> polConvex =
      buildConvexGeoPolygon(20.0, -61.1, 20.1, -61.1, 20.1, -60.5, 20.0, -60.5);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.0, -61.1, 20.1, -61.1, 20.1, -60.5, 20.0, -60.5);

  // Convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // Concave
  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::WITHIN, rel); // check

  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::DISJOINT, rel);

  // Complex
  rel = originalComplexPol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalComplexPol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalComplexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalComplexPol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygon4()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygon4()
{
  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 19.845091 -60.452631)) disjoint
  shared_ptr<GeoPolygon> originalConvexPol =
      buildConvexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                            23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalConcavePol =
      buildConcaveGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalComplexPol =
      buildComplexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  // POLYGON ((20.0 -62.4, 20.1 -62.4, 20.1 -60.3, 20.0  -60.3,20.0 -62.4))
  // intersects no points inside
  shared_ptr<GeoPolygon> polConvex =
      buildConvexGeoPolygon(20.0, -62.4, 20.1, -62.4, 20.1, -60.3, 20.0, -60.3);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.0, -62.4, 20.1, -62.4, 20.1, -60.3, 20.0, -60.3);

  // Convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // concave
  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // Complex
  rel = originalComplexPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalComplexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalComplexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalComplexPol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygonWithHole1()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygonWithHole1()
{
  // POLYGON((-135 -31, -135 -30, -137 -30, -137 -31, -135 -31),(-135.5 -30.7,
  // -135.5 -30.4, -136.5 -30.4, -136.5 -30.7, -135.5 -30.7))
  shared_ptr<GeoPolygon> hole = buildConcaveGeoPolygon(
      -135.5, -30.7, -135.5, -30.4, -136.5, -30.4, -136.5, -30.7);
  shared_ptr<GeoPolygon> originalConvexPol = buildConvexGeoPolygonWithHole(
      -135, -31, -135, -30, -137, -30, -137, -31, hole);

  shared_ptr<GeoPolygon> holeInv =
      buildConvexGeoPolygon(-135, -31, -135, -30, -137, -30, -137, -31);

  shared_ptr<GeoPolygon> originalConvexPolInv = buildConcaveGeoPolygonWithHole(
      -135.5, -30.7, -135.5, -30.4, -136.5, -30.4, -136.5, -30.7, holeInv);

  // POLYGON((-135.7 -30.6, -135.7 -30.45, -136 -30.45, -136 -30.6, -135.7
  // -30.6)) in the hole
  shared_ptr<GeoPolygon> polConvex = buildConvexGeoPolygon(
      -135.7, -30.6, -135.7, -30.45, -136, -30.45, -136, -30.6);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -135.7, -30.6, -135.7, -30.45, -136, -30.45, -136, -30.6);

  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::WITHIN, rel);

  rel = originalConvexPolInv->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalConvexPolInv);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = originalConvexPolInv->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(originalConvexPolInv);
  assertEquals(GeoArea::WITHIN, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygonWithHole2()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygonWithHole2()
{
  // POLYGON((-135 -31, -135 -30, -137 -30, -137 -31, -135 -31),(-135.5 -30.7,
  // -135.5 -30.4, -136.5 -30.4, -136.5 -30.7, -135.5 -30.7))
  shared_ptr<GeoPolygon> hole = buildConcaveGeoPolygon(
      -135.5, -30.7, -135.5, -30.4, -136.5, -30.4, -136.5, -30.7);
  shared_ptr<GeoPolygon> originalConvexPol = buildConvexGeoPolygonWithHole(
      -135, -31, -135, -30, -137, -30, -137, -31, hole);

  shared_ptr<GeoPolygon> holeInv =
      buildConvexGeoPolygon(-135, -31, -135, -30, -137, -30, -137, -31);

  shared_ptr<GeoPolygon> originalConvexPolInv = buildConcaveGeoPolygonWithHole(
      -135.5, -30.7, -135.5, -30.4, -136.5, -30.4, -136.5, -30.7, holeInv);

  // POLYGON((-135.5 -31.2, -135.5 -30.8, -136 -30.8, -136 -31.2, -135.5 -31.2))
  // intersects the hole
  shared_ptr<GeoPolygon> polConvex = buildConvexGeoPolygon(
      -135.5, -30.2, -135.5, -30.8, -136, -30.8, -136, -30.2);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -135.5, -30.2, -135.5, -30.8, -136, -30.8, -136, -30.2);

  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPolInv->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConvexPolInv);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPolInv->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPolInv);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygonWithHole3()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygonWithHole3()
{
  // POLYGON((-135 -31, -135 -30, -137 -30, -137 -31, -135 -31),(-135.5 -30.7,
  // -135.5 -30.4, -136.5 -30.4, -136.5 -30.7, -135.5 -30.7))
  shared_ptr<GeoPolygon> hole = buildConcaveGeoPolygon(
      -135.5, -30.7, -135.5, -30.4, -136.5, -30.4, -136.5, -30.7);
  shared_ptr<GeoPolygon> originalConvexPol = buildConvexGeoPolygonWithHole(
      -135, -31, -135, -30, -137, -30, -137, -31, hole);

  shared_ptr<GeoPolygon> holeInv =
      buildConvexGeoPolygon(-135, -31, -135, -30, -137, -30, -137, -31);

  shared_ptr<GeoPolygon> originalConvexPolInv = buildConcaveGeoPolygonWithHole(
      -135.5, -30.7, -135.5, -30.4, -136.5, -30.4, -136.5, -30.7, holeInv);

  // POLYGON((-135.2 -30.8, -135.2 -30.2, -136.8 -30.2, -136.8 -30.8, -135.2
  // -30.8)) inside the polygon covering the hole
  shared_ptr<GeoPolygon> polConvex = buildConvexGeoPolygon(
      -135.2, -30.8, -135.2, -30.3, -136.8, -30.2, -136.8, -30.8);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -135.2, -30.8, -135.2, -30.3, -136.8, -30.2, -136.8, -30.8);

  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPolInv->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConvexPolInv);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPolInv->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPolInv);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygonWithHole4()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygonWithHole4()
{
  // POLYGON((-135 -31, -135 -30, -137 -30, -137 -31, -135 -31),(-135.5 -30.7,
  // -135.5 -30.4, -136.5 -30.4, -136.5 -30.7, -135.5 -30.7))
  shared_ptr<GeoPolygon> hole = buildConcaveGeoPolygon(
      -135.5, -30.7, -135.5, -30.4, -136.5, -30.4, -136.5, -30.7);
  shared_ptr<GeoPolygon> originalConvexPol = buildConvexGeoPolygonWithHole(
      -135, -31, -135, -30, -137, -30, -137, -31, hole);

  shared_ptr<GeoPolygon> holeInv =
      buildConvexGeoPolygon(-135, -31, -135, -30, -137, -30, -137, -31);

  shared_ptr<GeoPolygon> originalConvexPolInv = buildConcaveGeoPolygonWithHole(
      -135.5, -30.7, -135.5, -30.4, -136.5, -30.4, -136.5, -30.7, holeInv);

  // POLYGON((-135.7 -30.3, -135.7 -30.2, -136 -30.2, -136 -30.3, -135.7
  // -30.3))inside the polygon
  shared_ptr<GeoPolygon> polConvex = buildConvexGeoPolygon(
      -135.7, -30.3, -135.7, -30.2, -136, -30.2, -136, -30.3);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -135.7, -30.3, -135.7, -30.2, -136, -30.2, -136, -30.3);

  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPolInv->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalConvexPolInv);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConvexPolInv->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPolInv);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygonWithCircle()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygonWithCircle()
{
  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 19.845091 -60.452631)) disjoint
  shared_ptr<GeoPolygon> originalConvexPol =
      buildConvexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                            23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalConcavePol =
      buildConcaveGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalComplexPol =
      buildComplexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoCircle> outCircle = GeoCircleFactory::makeGeoCircle(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-70),
      Geo3DUtil::fromDegrees(23), Geo3DUtil::fromDegrees(1));
  int rel = originalConvexPol->getRelationship(outCircle);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = originalConcavePol->getRelationship(outCircle);
  assertEquals(GeoArea::WITHIN, rel);
  rel = originalComplexPol->getRelationship(outCircle);
  assertEquals(GeoArea::DISJOINT, rel);

  shared_ptr<GeoCircle> overlapCircle = GeoCircleFactory::makeGeoCircle(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61.5),
      Geo3DUtil::fromDegrees(20), Geo3DUtil::fromDegrees(1));
  rel = originalConvexPol->getRelationship(overlapCircle);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = originalConcavePol->getRelationship(overlapCircle);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = originalComplexPol->getRelationship(overlapCircle);
  assertEquals(GeoArea::OVERLAPS, rel);

  shared_ptr<GeoCircle> inCircle = GeoCircleFactory::makeGeoCircle(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61),
      Geo3DUtil::fromDegrees(21), Geo3DUtil::fromDegrees(0.1));
  rel = originalConvexPol->getRelationship(inCircle);
  assertEquals(GeoArea::WITHIN, rel);
  rel = originalConcavePol->getRelationship(inCircle);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = originalComplexPol->getRelationship(inCircle);
  assertEquals(GeoArea::WITHIN, rel);

  shared_ptr<GeoCircle> onCircle = GeoCircleFactory::makeGeoCircle(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61),
      Geo3DUtil::fromDegrees(21), Geo3DUtil::fromDegrees(10.0));
  rel = originalConvexPol->getRelationship(onCircle);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = originalConcavePol->getRelationship(onCircle);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = originalComplexPol->getRelationship(onCircle);
  assertEquals(GeoArea::CONTAINS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygonWithBBox()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygonWithBBox()
{
  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 19.845091 -60.452631)) disjoint
  shared_ptr<GeoPolygon> originalConvexPol =
      buildConvexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                            23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalConcavePol =
      buildConcaveGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoPolygon> originalComplexPol =
      buildComplexGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652,
                             23.207901, -61.453298, 22.820804, -60.257713);

  shared_ptr<GeoBBox> outRectangle = GeoBBoxFactory::makeGeoBBox(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-69),
      Geo3DUtil::fromDegrees(-70), Geo3DUtil::fromDegrees(22),
      Geo3DUtil::fromDegrees(23));
  int rel = originalConvexPol->getRelationship(outRectangle);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = outRectangle->getRelationship(originalConvexPol);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = originalConcavePol->getRelationship(outRectangle);
  assertEquals(GeoArea::WITHIN, rel);
  rel = originalComplexPol->getRelationship(outRectangle);
  assertEquals(GeoArea::DISJOINT, rel);

  shared_ptr<GeoBBox> overlapRectangle = GeoBBoxFactory::makeGeoBBox(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61),
      Geo3DUtil::fromDegrees(-62), Geo3DUtil::fromDegrees(22),
      Geo3DUtil::fromDegrees(23));
  rel = originalConvexPol->getRelationship(overlapRectangle);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = overlapRectangle->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = originalConcavePol->getRelationship(overlapRectangle);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = originalComplexPol->getRelationship(overlapRectangle);
  assertEquals(GeoArea::OVERLAPS, rel);

  shared_ptr<GeoBBox> inRectangle = GeoBBoxFactory::makeGeoBBox(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61),
      Geo3DUtil::fromDegrees(-61.1), Geo3DUtil::fromDegrees(22.5),
      Geo3DUtil::fromDegrees(23));
  rel = originalConvexPol->getRelationship(inRectangle);
  assertEquals(GeoArea::WITHIN, rel);
  rel = inRectangle->getRelationship(originalConvexPol);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = originalConcavePol->getRelationship(inRectangle);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = originalComplexPol->getRelationship(inRectangle);
  assertEquals(GeoArea::WITHIN, rel);

  shared_ptr<GeoBBox> onRectangle = GeoBBoxFactory::makeGeoBBox(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-59),
      Geo3DUtil::fromDegrees(-64.1), Geo3DUtil::fromDegrees(18.5),
      Geo3DUtil::fromDegrees(27));
  rel = originalConvexPol->getRelationship(onRectangle);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = onRectangle->getRelationship(originalConvexPol);
  assertEquals(GeoArea::WITHIN, rel);
  rel = originalConcavePol->getRelationship(onRectangle);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = originalComplexPol->getRelationship(onRectangle);
  assertEquals(GeoArea::CONTAINS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoSimplePolygonWithComposite()
void SimpleGeoPolygonRelationshipsTest::testGeoSimplePolygonWithComposite()
{
  shared_ptr<GeoShape> shape = getCompositeShape();

  // POLYGON((-145.8555 -5.13, -145.8540 -5.13, -145.8540 -5.12, -145.8555
  // -5.12, -145.8555 -5.13))
  shared_ptr<GeoPolygon> polConvex = buildConvexGeoPolygon(
      -145.8555, -5.13, -145.8540, -5.13, -145.8540, -5.12, -145.8555, -5.12);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -145.8555, -5.13, -145.8540, -5.13, -145.8540, -5.12, -145.8555, -5.12);

  int rel = polConvex->getRelationship(shape);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConcave->getRelationship(shape);
  assertEquals(GeoArea::WITHIN, rel);

  // POLYGON((-145.8555 -5.13, -145.85 -5.13, -145.85 -5.12, -145.8555 -5.12,
  // -145.8555 -5.13))
  polConvex = buildConvexGeoPolygon(-145.8555, -5.13, -145.85, -5.13, -145.85,
                                    -5.12, -145.8555, -5.12);

  polConcave = buildConcaveGeoPolygon(-145.8555, -5.13, -145.85, -5.13, -145.85,
                                      -5.12, -145.8555, -5.12);

  rel = polConvex->getRelationship(shape);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(shape);
  assertEquals(GeoArea::OVERLAPS, rel);

  // POLYGON((-146 -5.18, -145.854 -5.18, -145.854 -5.11, -146 -5.11, -146
  // -5.18)) Case overlaping on of the shapes
  polConvex = buildConvexGeoPolygon(-146, -5.18, -145.854, -5.18, -145.854,
                                    -5.11, -146, -5.11);

  polConcave = buildConcaveGeoPolygon(-146, -5.18, -145.854, -5.18, -145.854,
                                      -5.11, -146, -5.11);

  rel = polConvex->getRelationship(shape);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(shape);
  assertEquals(GeoArea::OVERLAPS, rel);

  // POLYGON((-145.88 -5.13, -145.87 -5.13, -145.87 -5.12, -145.88 -5.12,
  // -145.88 -5.13))
  polConvex = buildConvexGeoPolygon(-145.88, -5.13, -145.87, -5.13, -145.87,
                                    -5.12, -145.88, -5.12);

  polConcave = buildConcaveGeoPolygon(-145.88, -5.13, -145.87, -5.13, -145.87,
                                      -5.12, -145.88, -5.12);

  rel = polConvex->getRelationship(shape);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(shape);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDegeneratedPointIntersectShape()
void SimpleGeoPolygonRelationshipsTest::testDegeneratedPointIntersectShape()
{
  shared_ptr<GeoBBox> bBox1 =
      GeoBBoxFactory::makeGeoBBox(PlanetModel::SPHERE, 1, 0, 0, 1);
  shared_ptr<GeoBBox> bBox2 =
      GeoBBoxFactory::makeGeoBBox(PlanetModel::SPHERE, 1, 1, 1, 1);
  int rel = bBox1->getRelationship(bBox2);
  // OVERLAPS instead of WITHIN. In this case the degenerated point lies on the
  // edge of the shape. intersects() returns true for one plane of the BBox and
  // hence method return OVERLAPS.
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = bBox2->getRelationship(bBox1);
  // The degenerated point cannot compute if it is on the edge. Uses WITHIN that
  // is true and therefore CONTAINS
  assertEquals(GeoArea::CONTAINS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDegeneratedPointInPole()
void SimpleGeoPolygonRelationshipsTest::testDegeneratedPointInPole()
{
  shared_ptr<GeoBBox> bBox1 = GeoBBoxFactory::makeGeoBBox(
      PlanetModel::SPHERE, M_PI * 0.5, M_PI * 0.5, 0, 0);
  shared_ptr<GeoPoint> point =
      make_shared<GeoPoint>(PlanetModel::SPHERE, M_PI * 0.5, M_PI);
  wcout << L"bbox1 = " << bBox1 << L"; point = " << point << endl;
  assertTrue(bBox1->isWithin(point));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDegeneratePathShape()
void SimpleGeoPolygonRelationshipsTest::testDegeneratePathShape()
{
  shared_ptr<GeoPoint> point1 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, 0, 0);
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, 0, 1);
  std::deque<std::shared_ptr<GeoPoint>> pointPath1 = {point1, point2};
  shared_ptr<GeoPath> path1 =
      GeoPathFactory::makeGeoPath(PlanetModel::SPHERE, 0, pointPath1);
  shared_ptr<GeoPath> path2 =
      GeoPathFactory::makeGeoPath(PlanetModel::SPHERE, 1, pointPath1);
  int rel = path1->getRelationship(path2);
  // if an end point is inside the shape it will always return intersects
  assertEquals(GeoArea::CONTAINS, rel); // should be contains?
  rel = path2->getRelationship(path1);
  assertEquals(GeoArea::WITHIN, rel);
}

shared_ptr<GeoPolygon> SimpleGeoPolygonRelationshipsTest::buildConvexGeoPolygon(
    double lon1, double lat1, double lon2, double lat2, double lon3,
    double lat3, double lon4, double lat4)
{
  shared_ptr<GeoPoint> point1 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat1),
                            Geo3DUtil::fromDegrees(lon1));
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat2),
                            Geo3DUtil::fromDegrees(lon2));
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat3),
                            Geo3DUtil::fromDegrees(lon3));
  shared_ptr<GeoPoint> point4 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat4),
                            Geo3DUtil::fromDegrees(lon4));
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  return GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points);
}

shared_ptr<GeoPolygon>
SimpleGeoPolygonRelationshipsTest::buildConcaveGeoPolygon(
    double lon1, double lat1, double lon2, double lat2, double lon3,
    double lat3, double lon4, double lat4)
{
  shared_ptr<GeoPoint> point1 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat1),
                            Geo3DUtil::fromDegrees(lon1));
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat2),
                            Geo3DUtil::fromDegrees(lon2));
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat3),
                            Geo3DUtil::fromDegrees(lon3));
  shared_ptr<GeoPoint> point4 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat4),
                            Geo3DUtil::fromDegrees(lon4));
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  return GeoPolygonFactory::makeGeoConcavePolygon(PlanetModel::SPHERE, points);
}

shared_ptr<GeoPolygon>
SimpleGeoPolygonRelationshipsTest::buildComplexGeoPolygon(
    double lon1, double lat1, double lon2, double lat2, double lon3,
    double lat3, double lon4, double lat4)
{
  shared_ptr<GeoPoint> point1 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat1),
                            Geo3DUtil::fromDegrees(lon1));
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat2),
                            Geo3DUtil::fromDegrees(lon2));
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat3),
                            Geo3DUtil::fromDegrees(lon3));
  shared_ptr<GeoPoint> point4 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat4),
                            Geo3DUtil::fromDegrees(lon4));
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  shared_ptr<GeoPolygonFactory::PolygonDescription> pd =
      make_shared<GeoPolygonFactory::PolygonDescription>(points);
  return GeoPolygonFactory::makeLargeGeoPolygon(PlanetModel::SPHERE,
                                                Collections::singletonList(pd));
}

shared_ptr<GeoPolygon>
SimpleGeoPolygonRelationshipsTest::buildConvexGeoPolygonWithHole(
    double lon1, double lat1, double lon2, double lat2, double lon3,
    double lat3, double lon4, double lat4, shared_ptr<GeoPolygon> hole)
{
  shared_ptr<GeoPoint> point1 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat1),
                            Geo3DUtil::fromDegrees(lon1));
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat2),
                            Geo3DUtil::fromDegrees(lon2));
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat3),
                            Geo3DUtil::fromDegrees(lon3));
  shared_ptr<GeoPoint> point4 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat4),
                            Geo3DUtil::fromDegrees(lon4));
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  // return new GeoConvexPolygon(PlanetModel.SPHERE,points,
  // Collections.singletonList(hole));
  return GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points,
                                           Collections::singletonList(hole));
}

shared_ptr<GeoPolygon>
SimpleGeoPolygonRelationshipsTest::buildConcaveGeoPolygonWithHole(
    double lon1, double lat1, double lon2, double lat2, double lon3,
    double lat3, double lon4, double lat4, shared_ptr<GeoPolygon> hole)
{
  shared_ptr<GeoPoint> point1 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat1),
                            Geo3DUtil::fromDegrees(lon1));
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat2),
                            Geo3DUtil::fromDegrees(lon2));
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat3),
                            Geo3DUtil::fromDegrees(lon3));
  shared_ptr<GeoPoint> point4 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat4),
                            Geo3DUtil::fromDegrees(lon4));
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  return GeoPolygonFactory::makeGeoConcavePolygon(
      PlanetModel::SPHERE, points, Collections::singletonList(hole));
}

shared_ptr<GeoShape> SimpleGeoPolygonRelationshipsTest::getCompositeShape()
{
  // MULTIPOLYGON(((-145.790967486 -5.17543698881, -145.790854979
  // -5.11348060995, -145.853073512 -5.11339421216, -145.853192037
  // -5.17535061936, -145.790967486 -5.17543698881)),
  //((-145.8563923 -5.17527125408, -145.856222168 -5.11332154814, -145.918433943
  //-5.11317773171, -145.918610092 -5.17512738429, -145.8563923
  //-5.17527125408)))
  shared_ptr<GeoPoint> point1 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-5.17543698881),
      Geo3DUtil::fromDegrees(-145.790967486));
  shared_ptr<GeoPoint> point2 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-5.11348060995),
      Geo3DUtil::fromDegrees(-145.790854979));
  shared_ptr<GeoPoint> point3 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-5.11339421216),
      Geo3DUtil::fromDegrees(-145.853073512));
  shared_ptr<GeoPoint> point4 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-5.17535061936),
      Geo3DUtil::fromDegrees(-145.853192037));
  const deque<std::shared_ptr<GeoPoint>> points1 =
      deque<std::shared_ptr<GeoPoint>>();
  points1.push_back(point1);
  points1.push_back(point2);
  points1.push_back(point3);
  points1.push_back(point4);
  shared_ptr<GeoPolygon> pol1 =
      GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points1);
  shared_ptr<GeoPoint> point5 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-5.17527125408),
      Geo3DUtil::fromDegrees(-145.8563923));
  shared_ptr<GeoPoint> point6 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-5.11332154814),
      Geo3DUtil::fromDegrees(-145.856222168));
  shared_ptr<GeoPoint> point7 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-5.11317773171),
      Geo3DUtil::fromDegrees(-145.918433943));
  shared_ptr<GeoPoint> point8 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-5.17512738429),
      Geo3DUtil::fromDegrees(-145.918610092));
  const deque<std::shared_ptr<GeoPoint>> points2 =
      deque<std::shared_ptr<GeoPoint>>();
  points2.push_back(point5);
  points2.push_back(point6);
  points2.push_back(point7);
  points2.push_back(point8);
  shared_ptr<GeoPolygon> pol2 =
      GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points2);
  shared_ptr<GeoCompositeMembershipShape> composite =
      make_shared<GeoCompositeMembershipShape>(PlanetModel::SPHERE);
  composite->addShape(pol1);
  composite->addShape(pol2);
  return composite;
}
} // namespace org::apache::lucene::spatial3d::geom