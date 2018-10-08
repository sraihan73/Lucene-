using namespace std;

#include "RandomGeoShapeRelationshipTest.h"

namespace org::apache::lucene::spatial3d::geom
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 5) public void
// testRandomPointWithin()
void RandomGeoShapeRelationshipTest::testRandomPointWithin()
{
  int referenceShapeType = CONVEX_POLYGON;
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  int shapeType = randomShapeType();
  while (shapeType == POINT || shapeType == LINE) {
    shapeType = randomShapeType();
  }
  shared_ptr<GeoAreaShape> shape = nullptr;
  shared_ptr<GeoPoint> point = nullptr;
  while (point == nullptr) {
    shape = randomGeoAreaShape(shapeType, planetModel);
    shared_ptr<Constraints> constraints = getEmptyConstraint();
    constraints->emplace(shape, GeoArea::WITHIN);
    shared_ptr<GeoAreaShape> reference =
        randomGeoAreaShape(referenceShapeType, planetModel, constraints);
    if (reference != nullptr) {
      constraints = make_shared<Constraints>(shared_from_this());
      constraints->emplace(reference, GeoArea::WITHIN);
      point = randomGeoPoint(planetModel, constraints);
    }
  }
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"shape: " + shape + L"\n");
  b->append(L"point: " + point);
  assertTrue(b->toString(), shape->isWithin(point));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = 5) public void testRandomPointNotWithin()
void RandomGeoShapeRelationshipTest::testRandomPointNotWithin()
{
  int referenceShapeType = CONVEX_POLYGON;
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  int shapeType = randomShapeType();
  shared_ptr<GeoAreaShape> shape = nullptr;
  shared_ptr<GeoPoint> point = nullptr;
  while (point == nullptr) {
    shape = randomGeoAreaShape(shapeType, planetModel);
    shared_ptr<Constraints> constraints = getEmptyConstraint();
    constraints->emplace(shape, GeoArea::DISJOINT);
    shared_ptr<GeoAreaShape> reference =
        randomGeoAreaShape(referenceShapeType, planetModel, constraints);
    if (reference != nullptr) {
      constraints = make_shared<Constraints>(shared_from_this());
      constraints->emplace(reference, GeoArea::WITHIN);
      point = randomGeoPoint(planetModel, constraints);
    }
  }
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"shape: " + shape + L"\n");
  b->append(L"point: " + point);
  assertFalse(b->toString(), shape->isWithin(point));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 5) public void testRandomDisjoint()
void RandomGeoShapeRelationshipTest::testRandomDisjoint()
{
  int referenceShapeType = CONVEX_SIMPLE_POLYGON;
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  int geoAreaShapeType = randomGeoAreaShapeType();
  int shapeType = randomConvexShapeType();

  shared_ptr<GeoShape> shape = nullptr;
  shared_ptr<GeoAreaShape> geoAreaShape = nullptr;
  while (shape == nullptr) {
    geoAreaShape = randomGeoAreaShape(geoAreaShapeType, planetModel);
    shared_ptr<Constraints> constraints =
        make_shared<Constraints>(shared_from_this());
    constraints->emplace(geoAreaShape, GeoArea::DISJOINT);
    shared_ptr<GeoAreaShape> reference =
        randomGeoAreaShape(referenceShapeType, planetModel, constraints);
    if (reference != nullptr) {
      constraints = getEmptyConstraint();
      constraints->emplace(reference, GeoArea::WITHIN);
      shape = randomGeoShape(shapeType, planetModel, constraints);
    }
  }
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"geoAreaShape: " + geoAreaShape + L"\n");
  b->append(L"shape: " + shape);
  int rel = geoAreaShape->getRelationship(shape);
  assertEquals(b->toString(), GeoArea::DISJOINT, rel);
  if (std::dynamic_pointer_cast<GeoArea>(shape) != nullptr) {
    rel = (std::static_pointer_cast<GeoArea>(shape))
              ->getRelationship(geoAreaShape);
    assertEquals(b->toString(), GeoArea::DISJOINT, rel);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 5) public void testRandomWithIn()
void RandomGeoShapeRelationshipTest::testRandomWithIn()
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  int geoAreaShapeType = randomGeoAreaShapeType();
  // shapes cannot be point or line -- no area!
  while (geoAreaShapeType == POINT || geoAreaShapeType == LINE) {
    geoAreaShapeType = randomGeoAreaShapeType();
  }
  int shapeType = LINE; // randomShapeType();
  int referenceShapeType = CONVEX_SIMPLE_POLYGON;
  if (!isConcave(geoAreaShapeType)) {
    shapeType = randomConvexShapeType();
  }
  if (isConcave(shapeType)) { // both concave
    referenceShapeType = CONCAVE_SIMPLE_POLYGON;
  }
  shared_ptr<GeoShape> shape = nullptr;
  shared_ptr<GeoAreaShape> geoAreaShape = nullptr;
  while (shape == nullptr) {
    geoAreaShape = randomGeoAreaShape(geoAreaShapeType, planetModel);
    shared_ptr<Constraints> constraints =
        make_shared<Constraints>(shared_from_this());
    constraints->emplace(geoAreaShape, GeoArea::WITHIN);
    shared_ptr<GeoAreaShape> reference =
        randomGeoAreaShape(referenceShapeType, planetModel, constraints);
    if (reference != nullptr) {
      constraints = make_shared<Constraints>(shared_from_this());
      constraints->emplace(reference, GeoArea::WITHIN);
      shape = randomGeoShape(shapeType, planetModel, constraints);
    }
  }
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"geoAreaShape: " + geoAreaShape + L"\n");
  b->append(L"shape: " + shape);
  int rel = geoAreaShape->getRelationship(shape);
  assertEquals(b->toString(), GeoArea::WITHIN, rel);
  if (std::dynamic_pointer_cast<GeoArea>(shape) != nullptr) {
    rel = (std::static_pointer_cast<GeoArea>(shape))
              ->getRelationship(geoAreaShape);
    assertEquals(b->toString(), GeoArea::CONTAINS, rel);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 1) public void testRandomContains()
void RandomGeoShapeRelationshipTest::testRandomContains()
{
  int referenceShapeType = CONVEX_SIMPLE_POLYGON;
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  int geoAreaShapeType = randomGeoAreaShapeType();
  while (geoAreaShapeType == COLLECTION) {
    geoAreaShapeType = randomGeoAreaShapeType();
  }
  int shapeType = randomShapeType();
  while (shapeType == POINT || shapeType == LINE) {
    shapeType = randomShapeType();
  }
  if (isConcave(geoAreaShapeType)) {
    shapeType = randomConcaveShapeType();
  }
  if (isConcave(shapeType)) {
    referenceShapeType = CONCAVE_SIMPLE_POLYGON;
  }
  shared_ptr<GeoShape> shape = nullptr;
  shared_ptr<GeoAreaShape> geoAreaShape = nullptr;
  while (shape == nullptr) {
    geoAreaShape = randomGeoAreaShape(geoAreaShapeType, planetModel);
    shared_ptr<Constraints> constraints = getEmptyConstraint();
    constraints->emplace(geoAreaShape, GeoArea::CONTAINS);
    shared_ptr<GeoPolygon> reference = std::static_pointer_cast<GeoPolygon>(
        randomGeoAreaShape(referenceShapeType, planetModel, constraints));
    if (reference != nullptr) {
      constraints = getEmptyConstraint();
      constraints->emplace(reference, GeoArea::CONTAINS);
      shape = randomGeoShape(shapeType, planetModel, constraints);
    }
  }
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"geoAreaShape: " + geoAreaShape + L"\n");
  b->append(L"shape: " + shape);
  int rel = geoAreaShape->getRelationship(shape);
  assertEquals(b->toString(), GeoArea::CONTAINS, rel);
  if (std::dynamic_pointer_cast<GeoArea>(shape) != nullptr) {
    rel = (std::static_pointer_cast<GeoArea>(shape))
              ->getRelationship(geoAreaShape);
    assertEquals(b->toString(), GeoArea::WITHIN, rel);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 5) public void testRandomOverlaps()
void RandomGeoShapeRelationshipTest::testRandomOverlaps()
{
  shared_ptr<PlanetModel> planetModel = randomPlanetModel();
  int geoAreaShapeType = randomGeoAreaShapeType();
  while (geoAreaShapeType == POINT || geoAreaShapeType == LINE) {
    geoAreaShapeType = randomGeoAreaShapeType();
  }
  int shapeType = randomShapeType();
  while (shapeType == POINT || shapeType == LINE) {
    shapeType = randomShapeType();
  }
  shared_ptr<GeoShape> shape = nullptr;
  shared_ptr<GeoAreaShape> geoAreaShape = nullptr;
  while (shape == nullptr) {
    geoAreaShape = randomGeoAreaShape(geoAreaShapeType, planetModel);
    shared_ptr<Constraints> constraints = getEmptyConstraint();
    constraints->emplace(geoAreaShape, GeoArea::WITHIN);
    shared_ptr<GeoAreaShape> reference1 =
        randomGeoAreaShape(CONVEX_SIMPLE_POLYGON, planetModel, constraints);
    if (reference1 == nullptr) {
      continue;
    }
    constraints = getEmptyConstraint();
    constraints->emplace(geoAreaShape, GeoArea::WITHIN);
    constraints->emplace(reference1, GeoArea::DISJOINT);
    shared_ptr<GeoAreaShape> reference2 =
        randomGeoAreaShape(CONVEX_SIMPLE_POLYGON, planetModel, constraints);
    if (reference2 == nullptr) {
      continue;
    }
    constraints = getEmptyConstraint();
    constraints->emplace(geoAreaShape, GeoArea::DISJOINT);
    shared_ptr<GeoAreaShape> reference3 =
        randomGeoAreaShape(CONVEX_SIMPLE_POLYGON, planetModel, constraints);
    if (reference3 != nullptr) {
      constraints = make_shared<Constraints>(shared_from_this());
      constraints->emplace(reference1, GeoArea::DISJOINT);
      constraints->emplace(reference2, GeoArea::CONTAINS);
      constraints->emplace(reference3, GeoArea::CONTAINS);
      shape = randomGeoShape(shapeType, planetModel, constraints);
    }
  }
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"geoAreaShape: " + geoAreaShape + L"\n");
  b->append(L"shape: " + shape);
  int rel = geoAreaShape->getRelationship(shape);
  assertEquals(b->toString(), GeoArea::OVERLAPS, rel);
  if (std::dynamic_pointer_cast<GeoArea>(shape) != nullptr) {
    rel = (std::static_pointer_cast<GeoArea>(shape))
              ->getRelationship(geoAreaShape);
    assertEquals(b->toString(), GeoArea::OVERLAPS, rel);
  }
}
} // namespace org::apache::lucene::spatial3d::geom