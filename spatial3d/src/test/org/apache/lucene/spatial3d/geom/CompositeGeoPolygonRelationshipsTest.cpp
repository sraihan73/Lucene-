using namespace std;

#include "CompositeGeoPolygonRelationshipsTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using org::junit::Test;
//    import static org.junit.Assert.assertEquals;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCompositePolygon1()
void CompositeGeoPolygonRelationshipsTest::testGeoCompositePolygon1()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 21 -61,19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConvexPol =
      buildGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652, 23.207901,
                      -61.453298, 22.820804, -60.257713, 21, -61);

  // POLYGON ((19.845091 -60.452631, 21 -61,22.820804 -60.257713,23.207901
  // -61.453298, 20.119948 -61.655652, 19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConcavePol =
      buildGeoPolygon(19.84509, -60.452631, 21, -61, 22.820804, -60.257713,
                      23.207901, -61.453298, 20.119948, -61.655652);

  shared_ptr<GeoPolygon> polConvex = buildGeoPolygon(
      20.0, -60.4, 20.1, -60.4, 20.1, -60.3, 20.0, -60.3, 20.0, -60.3);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.0, -60.4, 20.1, -60.4, 20.1, -60.3, 20.0, -60.3);

  // convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::WITHIN, rel);

  // concave
  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCompositePolygon2()
void CompositeGeoPolygonRelationshipsTest::testGeoCompositePolygon2()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 21 -61,19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConvexPol =
      buildGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652, 23.207901,
                      -61.453298, 22.820804, -60.257713, 21, -61);

  // POLYGON ((19.845091 -60.452631, 21 -61,22.820804 -60.257713,23.207901
  // -61.453298, 20.119948 -61.655652, 19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConcavePol =
      buildGeoPolygon(19.84509, -60.452631, 21, -61, 22.820804, -60.257713,
                      23.207901, -61.453298, 20.119948, -61.655652);

  // POLYGON ((20.9 -60.8, 21.1 -60.8, 21.1 -60.6, 20.9  -60.6,20.9 -60.8))
  shared_ptr<GeoPolygon> polConvex = buildGeoPolygon(
      20.9, -60.8, 21.1, -60.8, 21.1, -60.6, 20.9, -60.6, 20.9, -60.6);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.9, -60.8, 21.1, -60.8, 21.1, -60.6, 20.9, -60.6);

  // convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::WITHIN, rel);

  // concave
  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCompositePolygon3()
void CompositeGeoPolygonRelationshipsTest::testGeoCompositePolygon3()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 21 -61,19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConvexPol =
      buildGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652, 23.207901,
                      -61.453298, 22.820804, -60.257713, 21, -61);

  // POLYGON ((19.845091 -60.452631, 21 -61,22.820804 -60.257713,23.207901
  // -61.453298, 20.119948 -61.655652, 19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConcavePol =
      buildGeoPolygon(19.84509, -60.452631, 21, -61, 22.820804, -60.257713,
                      23.207901, -61.453298, 20.119948, -61.655652);

  // POLYGON ((20.9 -61.1, 21.1 -61.1, 21.1 -60.9, 20.9  -60.9,20.9 -61.1))
  shared_ptr<GeoPolygon> polConvex = buildGeoPolygon(
      20.9, -61.1, 21.1, -61.1, 21.1, -60.9, 20.9, -60.9, 20.9, -60.9);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.9, -61.1, 21.1, -61.1, 21.1, -60.9, 20.9, -60.9);

  // convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // concave
  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCompositePolygon4()
void CompositeGeoPolygonRelationshipsTest::testGeoCompositePolygon4()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 21 -61,19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConvexPol =
      buildGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652, 23.207901,
                      -61.453298, 22.820804, -60.257713, 21, -61);

  // POLYGON ((19.845091 -60.452631, 21 -61,22.820804 -60.257713,23.207901
  // -61.453298, 20.119948 -61.655652, 19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConcavePol =
      buildGeoPolygon(19.84509, -60.452631, 21, -61, 22.820804, -60.257713,
                      23.207901, -61.453298, 20.119948, -61.655652);

  // POLYGON ((20.9 -61.4, 21.1 -61.4, 21.1 -61.2, 20.9  -61.2,20.9 -61.4))
  shared_ptr<GeoPolygon> polConvex = buildGeoPolygon(
      20.9, -61.4, 21.1, -61.4, 21.1, -61.2, 20.9, -61.2, 20.9, -61.2);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.9, -61.4, 21.1, -61.4, 21.1, -61.2, 20.9, -61.2);

  // convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // concave
  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::WITHIN, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCompositePolygon5()
void CompositeGeoPolygonRelationshipsTest::testGeoCompositePolygon5()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 21 -61,19.845091 -60.452631))
  shared_ptr<GeoPolygon> originaConvexlPol =
      buildGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652, 23.207901,
                      -61.453298, 22.820804, -60.257713, 21, -61);

  // POLYGON ((19.845091 -60.452631, 21 -61,22.820804 -60.257713,23.207901
  // -61.453298, 20.119948 -61.655652, 19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConcavePol =
      buildGeoPolygon(19.84509, -60.452631, 21, -61, 22.820804, -60.257713,
                      23.207901, -61.453298, 20.119948, -61.655652);

  // POLYGON ((19 -62, 23 -62, 23 -60, 19 -60,19 -62))
  shared_ptr<GeoPolygon> polConvex =
      buildGeoPolygon(19, -62, 23, -62, 23, -60, 19, -60, 19, -60);

  shared_ptr<GeoPolygon> polConcave =
      buildConcaveGeoPolygon(19, -62, 23, -62, 23, -60, 19, -60);

  // convex
  int rel = originaConvexlPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originaConvexlPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originaConvexlPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originaConvexlPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // concave
  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCompositePolygon6()
void CompositeGeoPolygonRelationshipsTest::testGeoCompositePolygon6()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 21 -61,19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConvexPol =
      buildGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652, 23.207901,
                      -61.453298, 22.820804, -60.257713, 21, -61);

  // POLYGON ((19.845091 -60.452631, 21 -61,22.820804 -60.257713,23.207901
  // -61.453298, 20.119948 -61.655652, 19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConcavePol =
      buildGeoPolygon(19.84509, -60.452631, 21, -61, 22.820804, -60.257713,
                      23.207901, -61.453298, 20.119948, -61.655652);

  // POLYGON ((19 -62, 24 -62, 24 -60, 19 -60,19 -62))
  shared_ptr<GeoPolygon> polConvex =
      buildGeoPolygon(19, -62, 24, -62, 24, -60, 19, -60, 19, -60);

  shared_ptr<GeoPolygon> polConcave =
      buildConcaveGeoPolygon(19, -62, 24, -62, 24, -60, 19, -60);

  // convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::WITHIN, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::DISJOINT, rel);

  // concave
  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(originalConcavePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = originalConcavePol->getRelationship(polConcave);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConcave->getRelationship(originalConcavePol);
  assertEquals(GeoArea::CONTAINS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCompositePolygon7()
void CompositeGeoPolygonRelationshipsTest::testGeoCompositePolygon7()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713, 21 -61,19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConvexPol =
      buildGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652, 23.207901,
                      -61.453298, 22.820804, -60.257713, 21, -61);

  // POLYGON ((19.845091 -60.452631, 21 -61,22.820804 -60.257713,23.207901
  // -61.453298, 20.119948 -61.655652, 19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalConcavePol =
      buildGeoPolygon(19.84509, -60.452631, 21, -61, 22.820804, -60.257713,
                      23.207901, -61.453298, 20.119948, -61.655652);

  // POLYGON ((20.2 -61.4, 20.5 -61.4, 20.5 -60.8, 20.2 -60.8,20.2  -61.4))
  shared_ptr<GeoPolygon> polConvex = buildGeoPolygon(
      20.2, -61.4, 20.5, -61.4, 20.5, -60.8, 20.2, -60.8, 20.2, -60.8);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.2, -61.4, 20.5, -61.4, 20.5, -60.8, 20.2, -60.8);

  // convex
  int rel = originalConvexPol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  // concave
  rel = originalConcavePol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(originalConvexPol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = originalConvexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(originalConvexPol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCompositePolygon8()
void CompositeGeoPolygonRelationshipsTest::testGeoCompositePolygon8()
{

  // POLYGON ((19.845091 -60.452631, 20.119948 -61.655652, 23.207901
  // -61.453298, 22.820804 -60.257713,21 -61, 19.845091 -60.452631))
  shared_ptr<GeoPolygon> originalPol =
      buildGeoPolygon(19.84509, -60.452631, 20.119948, -61.655652, 23.207901,
                      -61.453298, 22.820804, -60.257713, 21, -61);

  shared_ptr<GeoShape> shape = getInsideCompositeShape();

  int rel = originalPol->getRelationship(shape);
  assertEquals(GeoArea::WITHIN, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoPolygonPole1()
void CompositeGeoPolygonRelationshipsTest::testGeoPolygonPole1()
{
  // POLYGON((0 80, 45 85 ,90 80,135 85,180 80, -135 85, -90 80, -45 85,0 80))
  shared_ptr<GeoPolygon> compositePol = getCompositePolygon();
  shared_ptr<GeoPolygon> complexPol = getComplexPolygon();

  // POLYGON ((20.9 -61.4, 21.1 -61.4, 21.1 -61.2, 20.9  -61.2,20.9 -61.4))
  shared_ptr<GeoPolygon> polConvex = buildGeoPolygon(
      20.9, -61.4, 21.1, -61.4, 21.1, -61.2, 20.9, -61.2, 20.9, -61.2);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      20.9, -61.4, 21.1, -61.4, 21.1, -61.2, 20.9, -61.2);

  int rel = compositePol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(compositePol);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = compositePol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(compositePol);
  assertEquals(GeoArea::WITHIN, rel);

  rel = complexPol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(complexPol);
  assertEquals(GeoArea::DISJOINT, rel);

  rel = complexPol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(complexPol);
  assertEquals(GeoArea::WITHIN, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoPolygonPole2()
void CompositeGeoPolygonRelationshipsTest::testGeoPolygonPole2()
{
  // POLYGON((0 80, 45 85 ,90 80,135 85,180 80, -135 85, -90 80, -45 85,0 80))
  shared_ptr<GeoPolygon> compositePol = getCompositePolygon();
  shared_ptr<GeoPolygon> complexPol = getComplexPolygon();

  // POLYGON((-1 81, -1 79,1 79,1 81, -1 81))
  shared_ptr<GeoPolygon> polConvex =
      buildGeoPolygon(-1, 81, -1, 79, 1, 79, 1, 81, 1, 81);

  shared_ptr<GeoPolygon> polConcave =
      buildConcaveGeoPolygon(-1, 81, -1, 79, 1, 79, 1, 81);

  int rel = compositePol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(compositePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = compositePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(compositePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = complexPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(complexPol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = complexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(complexPol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoPolygonPole3()
void CompositeGeoPolygonRelationshipsTest::testGeoPolygonPole3()
{
  // POLYGON((0 80, 45 85 ,90 80,135 85,180 80, -135 85, -90 80, -45 85,0 80))
  shared_ptr<GeoPolygon> compositePol = getCompositePolygon();
  shared_ptr<GeoPolygon> complexPol = getComplexPolygon();

  // POLYGON((-1 86, -1 84,1 84,1 86, -1 86))
  shared_ptr<GeoPolygon> polConvex =
      buildGeoPolygon(-1, 86, -1, 84, 1, 84, 1, 86, 1, 86);

  shared_ptr<GeoPolygon> polConcave =
      buildConcaveGeoPolygon(-1, 86, -1, 84, 1, 84, 1, 86);

  int rel = compositePol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(compositePol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = compositePol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(compositePol);
  assertEquals(GeoArea::OVERLAPS, rel);

  rel = complexPol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(complexPol);
  assertEquals(GeoArea::CONTAINS, rel);

  rel = complexPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(complexPol);
  assertEquals(GeoArea::OVERLAPS, rel);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiPolygon1()
void CompositeGeoPolygonRelationshipsTest::testMultiPolygon1()
{
  // MULTIPOLYGON(((-145.790967486 -5.17543698881, -145.790854979
  // -5.11348060995, -145.853073512 -5.11339421216, -145.853192037
  // -5.17535061936, -145.790967486 -5.17543698881)),
  //((-145.8563923 -5.17527125408, -145.856222168 -5.11332154814, -145.918433943
  //-5.11317773171, -145.918610092 -5.17512738429, -145.8563923
  //-5.17527125408)))
  shared_ptr<GeoPolygon> multiPol = getMultiPolygon();

  // POLYGON((-145.8555 -5.13, -145.8540 -5.13, -145.8540 -5.12, -145.8555
  // -5.12, -145.8555 -5.13))
  shared_ptr<GeoPolygon> polConvex =
      buildGeoPolygon(-145.8555, -5.13, -145.8540, -5.13, -145.8540, -5.12,
                      -145.8555, -5.12, -145.8555, -5.12);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -145.8555, -5.13, -145.8540, -5.13, -145.8540, -5.12, -145.8555, -5.12);

  int rel = multiPol->getRelationship(polConvex);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConvex->getRelationship(multiPol);
  assertEquals(GeoArea::DISJOINT, rel);
  assertEquals(false, multiPol->intersects(polConvex));
  assertEquals(false, polConvex->intersects(multiPol));

  rel = multiPol->getRelationship(polConcave);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConcave->getRelationship(multiPol);
  assertEquals(GeoArea::WITHIN, rel);
  assertEquals(false, multiPol->intersects(polConcave));
  assertEquals(false, polConcave->intersects(multiPol));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiPolygon2()
void CompositeGeoPolygonRelationshipsTest::testMultiPolygon2()
{
  // MULTIPOLYGON(((-145.790967486 -5.17543698881, -145.790854979
  // -5.11348060995, -145.853073512 -5.11339421216, -145.853192037
  // -5.17535061936, -145.790967486 -5.17543698881)),
  //((-145.8563923 -5.17527125408, -145.856222168 -5.11332154814, -145.918433943
  //-5.11317773171, -145.918610092 -5.17512738429, -145.8563923
  //-5.17527125408)))
  shared_ptr<GeoPolygon> multiPol = getMultiPolygon();

  // POLYGON((-145.8555 -5.13, -145.85 -5.13, -145.85 -5.12, -145.8555 -5.12,
  // -145.8555 -5.13))
  shared_ptr<GeoPolygon> polConvex =
      buildGeoPolygon(-145.8555, -5.13, -145.85, -5.13, -145.85, -5.12,
                      -145.8555, -5.12, -145.8555, -5.12);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -145.8555, -5.13, -145.85, -5.13, -145.85, -5.12, -145.8555, -5.12);

  int rel = multiPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(multiPol);
  assertEquals(GeoArea::OVERLAPS, rel);
  assertEquals(true, multiPol->intersects(polConvex));
  assertEquals(true, polConvex->intersects(multiPol));

  rel = multiPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(multiPol);
  assertEquals(GeoArea::OVERLAPS, rel);
  assertEquals(true, multiPol->intersects(polConcave));
  assertEquals(true, polConcave->intersects(multiPol));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiPolygon3()
void CompositeGeoPolygonRelationshipsTest::testMultiPolygon3()
{
  // MULTIPOLYGON(((-145.790967486 -5.17543698881, -145.790854979
  // -5.11348060995, -145.853073512 -5.11339421216, -145.853192037
  // -5.17535061936, -145.790967486 -5.17543698881)),
  //((-145.8563923 -5.17527125408, -145.856222168 -5.11332154814, -145.918433943
  //-5.11317773171, -145.918610092 -5.17512738429, -145.8563923
  //-5.17527125408)))
  shared_ptr<GeoPolygon> multiPol = getMultiPolygon();

  // POLYGON((-146 -5.18, -145.854 -5.18, -145.854 -5.11, -146 -5.11, -146
  // -5.18)) Case overlapping one of the polygons so intersection is false!
  shared_ptr<GeoPolygon> polConvex = buildGeoPolygon(
      -146, -5.18, -145.854, -5.18, -145.854, -5.11, -146, -5.11, -146, -5.11);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -146, -5.18, -145.854, -5.18, -145.854, -5.11, -146, -5.11);

  int rel = multiPol->getRelationship(polConvex);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConvex->getRelationship(multiPol);
  assertEquals(GeoArea::OVERLAPS, rel);
  assertEquals(false, multiPol->intersects(polConvex));
  assertEquals(false, polConvex->intersects(multiPol));

  rel = multiPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(multiPol);
  assertEquals(GeoArea::OVERLAPS, rel);
  assertEquals(false, multiPol->intersects(polConcave));
  assertEquals(false, polConcave->intersects(multiPol));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiPolygon4()
void CompositeGeoPolygonRelationshipsTest::testMultiPolygon4()
{
  // MULTIPOLYGON(((-145.790967486 -5.17543698881, -145.790854979
  // -5.11348060995, -145.853073512 -5.11339421216, -145.853192037
  // -5.17535061936, -145.790967486 -5.17543698881)),
  //((-145.8563923 -5.17527125408, -145.856222168 -5.11332154814, -145.918433943
  //-5.11317773171, -145.918610092 -5.17512738429, -145.8563923
  //-5.17527125408)))
  shared_ptr<GeoPolygon> multiPol = getMultiPolygon();

  // POLYGON((-145.88 -5.13, -145.87 -5.13, -145.87 -5.12, -145.88 -5.12,
  // -145.88 -5.13))
  shared_ptr<GeoPolygon> polConvex =
      buildGeoPolygon(-145.88, -5.13, -145.87, -5.13, -145.87, -5.12, -145.88,
                      -5.12, -145.88, -5.12);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -145.88, -5.13, -145.87, -5.13, -145.87, -5.12, -145.88, -5.12);

  int rel = multiPol->getRelationship(polConvex);
  assertEquals(GeoArea::WITHIN, rel);
  rel = polConvex->getRelationship(multiPol);
  assertEquals(GeoArea::CONTAINS, rel);
  assertEquals(false, multiPol->intersects(polConvex));
  assertEquals(false, polConvex->intersects(multiPol));

  rel = multiPol->getRelationship(polConcave);
  assertEquals(GeoArea::OVERLAPS, rel);
  rel = polConcave->getRelationship(multiPol);
  assertEquals(GeoArea::OVERLAPS, rel);
  assertEquals(false, multiPol->intersects(polConcave));
  assertEquals(false, polConcave->intersects(multiPol));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiPolygon5()
void CompositeGeoPolygonRelationshipsTest::testMultiPolygon5()
{
  // MULTIPOLYGON(((-145.790967486 -5.17543698881, -145.790854979
  // -5.11348060995, -145.853073512 -5.11339421216, -145.853192037
  // -5.17535061936, -145.790967486 -5.17543698881)),
  //((-145.8563923 -5.17527125408, -145.856222168 -5.11332154814, -145.918433943
  //-5.11317773171, -145.918610092 -5.17512738429, -145.8563923
  //-5.17527125408)))
  shared_ptr<GeoPolygon> multiPol = getMultiPolygon();

  // POLYGON((-146 -5.18, -145 -5.18, -145 -5.11, -146 -5.11, -146 -5.18))
  shared_ptr<GeoPolygon> polConvex = buildGeoPolygon(
      -146, -5.18, -145, -5.18, -145, -5.11, -146, -5.11, -146, -5.11);

  shared_ptr<GeoPolygon> polConcave = buildConcaveGeoPolygon(
      -146, -5.18, -145, -5.18, -145, -5.11, -146, -5.11);

  int rel = multiPol->getRelationship(polConvex);
  assertEquals(GeoArea::CONTAINS, rel);
  rel = polConvex->getRelationship(multiPol);
  assertEquals(GeoArea::WITHIN, rel);
  assertEquals(false, multiPol->intersects(polConvex));

  rel = multiPol->getRelationship(polConcave);
  assertEquals(GeoArea::DISJOINT, rel);
  rel = polConcave->getRelationship(multiPol);
  assertEquals(GeoArea::DISJOINT, rel);
  assertEquals(false, multiPol->intersects(polConcave));
}

shared_ptr<GeoPolygon> CompositeGeoPolygonRelationshipsTest::buildGeoPolygon(
    double lon1, double lat1, double lon2, double lat2, double lon3,
    double lat3, double lon4, double lat4, double lon5, double lat5)
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
  shared_ptr<GeoPoint> point5 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(lat5),
                            Geo3DUtil::fromDegrees(lon5));
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  points.push_back(point5);
  return GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points);
}

shared_ptr<GeoPolygon>
CompositeGeoPolygonRelationshipsTest::buildConcaveGeoPolygon(
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
CompositeGeoPolygonRelationshipsTest::getCompositePolygon()
{
  // POLYGON((0 80, 45 85 ,90 80,135 85,180 80, -135 85, -90 80, -45 85,0 80))
  shared_ptr<GeoPoint> point1 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(80),
                            Geo3DUtil::fromDegrees(0));
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(85),
                            Geo3DUtil::fromDegrees(45));
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(80),
                            Geo3DUtil::fromDegrees(90));
  shared_ptr<GeoPoint> point4 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(85),
                            Geo3DUtil::fromDegrees(135));
  shared_ptr<GeoPoint> point5 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(80),
                            Geo3DUtil::fromDegrees(180));
  shared_ptr<GeoPoint> point6 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(85),
                            Geo3DUtil::fromDegrees(-135));
  shared_ptr<GeoPoint> point7 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(80),
                            Geo3DUtil::fromDegrees(-90));
  shared_ptr<GeoPoint> point8 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(85),
                            Geo3DUtil::fromDegrees(-45));
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  points.push_back(point5);
  points.push_back(point6);
  points.push_back(point7);
  points.push_back(point8);
  return GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points);
}

shared_ptr<GeoPolygon> CompositeGeoPolygonRelationshipsTest::getComplexPolygon()
{
  // POLYGON((0 80, 45 85 ,90 80,135 85,180 80, -135 85, -90 80, -45 85,0 80))
  shared_ptr<GeoPoint> point1 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(80),
                            Geo3DUtil::fromDegrees(0));
  shared_ptr<GeoPoint> point2 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(85),
                            Geo3DUtil::fromDegrees(45));
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(80),
                            Geo3DUtil::fromDegrees(90));
  shared_ptr<GeoPoint> point4 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(85),
                            Geo3DUtil::fromDegrees(135));
  shared_ptr<GeoPoint> point5 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(80),
                            Geo3DUtil::fromDegrees(180));
  shared_ptr<GeoPoint> point6 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(85),
                            Geo3DUtil::fromDegrees(-135));
  shared_ptr<GeoPoint> point7 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(80),
                            Geo3DUtil::fromDegrees(-90));
  shared_ptr<GeoPoint> point8 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(85),
                            Geo3DUtil::fromDegrees(-45));
  const deque<std::shared_ptr<GeoPoint>> points =
      deque<std::shared_ptr<GeoPoint>>();
  points.push_back(point1);
  points.push_back(point2);
  points.push_back(point3);
  points.push_back(point4);
  points.push_back(point5);
  points.push_back(point6);
  points.push_back(point7);
  points.push_back(point8);
  shared_ptr<GeoPolygonFactory::PolygonDescription> pd =
      make_shared<GeoPolygonFactory::PolygonDescription>(points);
  return GeoPolygonFactory::makeLargeGeoPolygon(PlanetModel::SPHERE,
                                                Collections::singletonList(pd));
}

shared_ptr<GeoPolygon> CompositeGeoPolygonRelationshipsTest::getMultiPolygon()
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
  shared_ptr<GeoPolygonFactory::PolygonDescription> pd1 =
      make_shared<GeoPolygonFactory::PolygonDescription>(points1);
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
  shared_ptr<GeoPolygonFactory::PolygonDescription> pd2 =
      make_shared<GeoPolygonFactory::PolygonDescription>(points2);
  const deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>> pds =
      deque<std::shared_ptr<GeoPolygonFactory::PolygonDescription>>();
  pds.push_back(pd1);
  pds.push_back(pd2);
  return GeoPolygonFactory::makeLargeGeoPolygon(PlanetModel::SPHERE, pds);
}

shared_ptr<GeoShape>
CompositeGeoPolygonRelationshipsTest::getInsideCompositeShape()
{
  // MULTIPOLYGON(((19.945091 -60.552631, 20.319948 -61.555652, 20.9 -61.5, 20.9
  // -61, 19.945091 -60.552631)),
  // ((21.1 -61.5,  23.107901 -61.253298, 22.720804 -60.457713,21.1 -61, 21.1
  // -61.5)))
  shared_ptr<GeoPoint> point1 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-60.552631),
      Geo3DUtil::fromDegrees(19.945091));
  shared_ptr<GeoPoint> point2 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61.555652),
      Geo3DUtil::fromDegrees(20.319948));
  shared_ptr<GeoPoint> point3 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61.5),
                            Geo3DUtil::fromDegrees(20.9));
  shared_ptr<GeoPoint> point4 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61),
                            Geo3DUtil::fromDegrees(20.9));
  const deque<std::shared_ptr<GeoPoint>> points1 =
      deque<std::shared_ptr<GeoPoint>>();
  points1.push_back(point1);
  points1.push_back(point2);
  points1.push_back(point3);
  points1.push_back(point4);
  shared_ptr<GeoPoint> point5 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61.5),
                            Geo3DUtil::fromDegrees(21.1));
  shared_ptr<GeoPoint> point6 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61.253298),
      Geo3DUtil::fromDegrees(23.107901));
  shared_ptr<GeoPoint> point7 = make_shared<GeoPoint>(
      PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-60.457713),
      Geo3DUtil::fromDegrees(22.720804));
  shared_ptr<GeoPoint> point8 =
      make_shared<GeoPoint>(PlanetModel::SPHERE, Geo3DUtil::fromDegrees(-61),
                            Geo3DUtil::fromDegrees(21.1));
  const deque<std::shared_ptr<GeoPoint>> points2 =
      deque<std::shared_ptr<GeoPoint>>();
  points2.push_back(point5);
  points2.push_back(point6);
  points2.push_back(point7);
  points2.push_back(point8);
  shared_ptr<GeoPolygon> p1 =
      GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points1);
  shared_ptr<GeoPolygon> p2 =
      GeoPolygonFactory::makeGeoPolygon(PlanetModel::SPHERE, points2);
  shared_ptr<GeoCompositeMembershipShape> compositeMembershipShape =
      make_shared<GeoCompositeMembershipShape>(PlanetModel::SPHERE);
  compositeMembershipShape->addShape(p1);
  compositeMembershipShape->addShape(p2);
  return compositeMembershipShape;
}
} // namespace org::apache::lucene::spatial3d::geom