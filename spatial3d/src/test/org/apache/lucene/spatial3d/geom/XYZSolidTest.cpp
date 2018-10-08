using namespace std;

#include "XYZSolidTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNonDegenerateRelationships()
void XYZSolidTest::testNonDegenerateRelationships()
{
  shared_ptr<XYZSolid> s;
  shared_ptr<GeoShape> shape;
  // Something bigger than the world
  s = make_shared<StandardXYZSolid>(PlanetModel::SPHERE, -2.0, 2.0, -2.0, 2.0,
                                    -2.0, 2.0);
  // Any shape, except whole world, should be within.
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::WITHIN, s->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  // An XYZSolid represents a surface shape, which when larger than the world is
  // in fact the entire world, so it should overlap the world.
  assertEquals(GeoArea::OVERLAPS, s->getRelationship(shape));

  // Something overlapping the world on only one side
  s = make_shared<StandardXYZSolid>(PlanetModel::SPHERE, -2.0, 0.0, -2.0, 2.0,
                                    -2.0, 2.0);
  // Some things should be disjoint...
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, s->getRelationship(shape));
  // And, some things should be within...
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI, 0.1);
  assertEquals(GeoArea::WITHIN, s->getRelationship(shape));
  // And, some things should overlap.
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI * 0.5, 0.1);
  assertEquals(GeoArea::OVERLAPS, s->getRelationship(shape));

  // Partial world should be contained by GeoWorld object...
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::CONTAINS, s->getRelationship(shape));

  // Something inside the world
  s = make_shared<StandardXYZSolid>(PlanetModel::SPHERE, -0.1, 0.1, -0.1, 0.1,
                                    -0.1, 0.1);
  // All shapes should be disjoint
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, s->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::DISJOINT, s->getRelationship(shape));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDegenerateRelationships()
void XYZSolidTest::testDegenerateRelationships()
{
  shared_ptr<GeoArea> solid;
  shared_ptr<GeoShape> shape;

  // Basic test of the factory method - non-degenerate
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, -2.0, 2.0, -2.0, 2.0,
                                      -2.0, 2.0);
  // Any shape, except whole world, should be within.
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::WITHIN, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  // An XYZSolid represents a surface shape, which when larger than the world is
  // in fact the entire world, so it should overlap the world.
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));

  // Build a degenerate point, not on sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0);
  // disjoint with everything?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));

  // Build a degenerate point that IS on the sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 1.0, 1.0, 0.0, 0.0,
                                      0.0, 0.0);
  // inside everything that it touches?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));

  // Build a shape degenerate in (x,y), which has no points on sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, 0.0, 0.0,
                                      -0.1, 0.1);
  // disjoint with everything?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));

  // Build a shape degenerate in (x,y) which has one point on sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, 0.0, 0.0,
                                      -0.1, 1.1);
  // inside everything that it touches?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, M_PI * 0.5, 0.0, 0.1);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, -M_PI * 0.5, 0.0,
                                         0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));

  // Build a shape degenerate in (x,y) which has two points on sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, 0.0, 0.0,
                                      -1.1, 1.1);
  // inside everything that it touches?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, M_PI * 0.5, 0.0, 0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, -M_PI * 0.5, 0.0,
                                         0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));

  // Build a shape degenerate in (x,z), which has no points on sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, -0.1, 0.1,
                                      0.0, 0.0);
  // disjoint with everything?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));

  // Build a shape degenerate in (x,z) which has one point on sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, -0.1, 1.1,
                                      0.0, 0.0);
  // inside everything that it touches?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI * 0.5, 0.1);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, -M_PI * 0.5,
                                         0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));

  // Build a shape degenerate in (x,y) which has two points on sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, -1.1, 1.1,
                                      0.0, 0.0);
  // inside everything that it touches?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI * 0.5, 0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, -M_PI * 0.5,
                                         0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));

  // MHL for y-z check

  // Build a shape that is degenerate in x, which has zero points intersecting
  // sphere
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, -0.1, 0.1,
                                      -0.1, 0.1);
  // disjoint with everything?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));

  // Build a shape that is degenerate in x, which has zero points intersecting
  // sphere, second variation
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, -0.1, 0.1,
                                      1.1, 1.2);
  // disjoint with everything?
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));

  // Build a shape that is disjoint in X but intersects sphere in a complete
  // circle
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, -1.1, 1.1,
                                      -1.1, 1.1);
  // inside everything that it touches?
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI * 0.5, 0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, -M_PI * 0.5,
                                         0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, M_PI * 0.5, 0.0, 0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, -M_PI * 0.5, 0.0,
                                         0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));

  // Build a shape that is disjoint in X but intersects sphere in a half circle
  // in Y
  solid = GeoAreaFactory::makeGeoArea(PlanetModel::SPHERE, 0.0, 0.0, 0.0, 1.1,
                                      -1.1, 1.1);
  // inside everything that it touches?
  shape = make_shared<GeoWorld>(PlanetModel::SPHERE);
  assertEquals(GeoArea::CONTAINS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, 0.0, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI, 0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, M_PI * 0.5, 0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, 0.0, -M_PI * 0.5,
                                         0.1);
  assertEquals(GeoArea::DISJOINT, solid->getRelationship(shape));
  shape =
      make_shared<GeoStandardCircle>(PlanetModel::SPHERE, M_PI * 0.5, 0.0, 0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));
  shape = make_shared<GeoStandardCircle>(PlanetModel::SPHERE, -M_PI * 0.5, 0.0,
                                         0.1);
  assertEquals(GeoArea::OVERLAPS, solid->getRelationship(shape));

  // MHL for degenerate Y
  // MHL for degenerate Z
}
} // namespace org::apache::lucene::spatial3d::geom