using namespace std;

#include "TestBBoxStrategy.h"

namespace org::apache::lucene::spatial::bbox
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using FieldType = org::apache::lucene::document::FieldType;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using Query = org::apache::lucene::search::Query;
using SpatialMatchConcern = org::apache::lucene::spatial::SpatialMatchConcern;
using RandomSpatialOpStrategyTestCase =
    org::apache::lucene::spatial::prefix::RandomSpatialOpStrategyTestCase;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using ShapeAreaValueSource =
    org::apache::lucene::spatial::util::ShapeAreaValueSource;
using org::junit::Ignore;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::impl::RectangleImpl;

shared_ptr<Shape> TestBBoxStrategy::randomIndexedShape()
{
  shared_ptr<Rectangle> world = ctx->getWorldBounds();
  if (random()->nextInt(10) == 0) // increased chance of getting one of these
  {
    return world;
  }

  int worldWidth = static_cast<int>(round(world->getWidth()));
  int deltaLeft = nextIntInclusive(worldWidth);
  int deltaRight = nextIntInclusive(worldWidth - deltaLeft);
  int worldHeight = static_cast<int>(round(world->getHeight()));
  int deltaTop = nextIntInclusive(worldHeight);
  int deltaBottom = nextIntInclusive(worldHeight - deltaTop);
  if (ctx->isGeo() && (deltaLeft != 0 || deltaRight != 0)) {
    // if geo & doesn't world-wrap, we shift randomly to potentially cross
    // dateline
    int shift = nextIntInclusive(360);
    return ctx->makeRectangle(
        DistanceUtils::normLonDEG(world->getMinX() + deltaLeft + shift),
        DistanceUtils::normLonDEG(world->getMaxX() - deltaRight + shift),
        world->getMinY() + deltaBottom, world->getMaxY() - deltaTop);
  } else {
    return ctx->makeRectangle(
        world->getMinX() + deltaLeft, world->getMaxX() - deltaRight,
        world->getMinY() + deltaBottom, world->getMaxY() - deltaTop);
  }
}

int TestBBoxStrategy::nextIntInclusive(int toInc)
{
  constexpr int DIVIS = 10;
  if (toInc % DIVIS == 0) {
    return random()->nextInt(toInc / DIVIS + 1) * DIVIS;
  } else {
    return random()->nextInt(toInc + 1);
  }
}

shared_ptr<Shape> TestBBoxStrategy::randomQueryShape()
{
  return randomIndexedShape();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 15) public void testOperations()
// throws java.io.IOException
void TestBBoxStrategy::testOperations() 
{
  // setup
  if (random()->nextInt(4) >
      0) { // 75% of the time choose geo (more interesting to test)
    this->ctx = SpatialContext::GEO;
  } else {
    shared_ptr<SpatialContextFactory> factory =
        make_shared<SpatialContextFactory>();
    factory->geo = false;
    factory->worldBounds =
        make_shared<RectangleImpl>(-300, 300, -100, 100, nullptr);
    this->ctx = factory->newSpatialContext();
  }
  this->strategy = BBoxStrategy::newInstance(ctx, L"bbox");
  // test we can disable docValues for predicate tests
  if (random()->nextBoolean()) {
    shared_ptr<FieldType> fieldType = make_shared<FieldType>(
        (std::static_pointer_cast<BBoxStrategy>(strategy))->getFieldType());
    fieldType->setDocValuesType(DocValuesType::NONE);
    strategy =
        make_shared<BBoxStrategy>(ctx, strategy->getFieldName(), fieldType);
  }
  for (auto operation : SpatialOperation::values()) {
    if (operation == SpatialOperation::Overlaps) {
      continue; // unsupported
    }
    testOperationRandomShapes(operation);

    deleteAll();
    commit();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testIntersectsBugDatelineEdge() throws
// java.io.IOException
void TestBBoxStrategy::testIntersectsBugDatelineEdge() 
{
  setupGeo();
  testOperation(ctx->makeRectangle(160, 180, -10, 10),
                SpatialOperation::Intersects,
                ctx->makeRectangle(-180, -160, -10, 10), true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testIntersectsWorldDatelineEdge() throws
// java.io.IOException
void TestBBoxStrategy::testIntersectsWorldDatelineEdge() 
{
  setupGeo();
  testOperation(ctx->makeRectangle(-180, 180, -10, 10),
                SpatialOperation::Intersects,
                ctx->makeRectangle(180, 180, -10, 10), true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithinBugDatelineEdge() throws
// java.io.IOException
void TestBBoxStrategy::testWithinBugDatelineEdge() 
{
  setupGeo();
  testOperation(ctx->makeRectangle(180, 180, -10, 10),
                SpatialOperation::IsWithin,
                ctx->makeRectangle(-180, -100, -10, 10), true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testContainsBugDatelineEdge() throws
// java.io.IOException
void TestBBoxStrategy::testContainsBugDatelineEdge() 
{
  setupGeo();
  testOperation(ctx->makeRectangle(-180, -150, -10, 10),
                SpatialOperation::Contains,
                ctx->makeRectangle(180, 180, -10, 10), true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWorldContainsXDL() throws
// java.io.IOException
void TestBBoxStrategy::testWorldContainsXDL() 
{
  setupGeo();
  testOperation(ctx->makeRectangle(-180, 180, -10, 10),
                SpatialOperation::Contains,
                ctx->makeRectangle(170, -170, -10, 10), true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAlongDatelineOppositeSign() throws
// java.io.IOException
void TestBBoxStrategy::testAlongDatelineOppositeSign() 
{
  // Due to Spatial4j bug #85, we can't simply do:
  //    testOperation(indexedShape,
  //        SpatialOperation.IsWithin,
  //        queryShape, true);

  // both on dateline but expressed using opposite signs
  setupGeo();
  shared_ptr<Rectangle> *const indexedShape =
      ctx->makeRectangle(180, 180, -10, 10);
  shared_ptr<Rectangle> *const queryShape =
      ctx->makeRectangle(-180, -180, -20, 20);
  shared_ptr<SpatialOperation> *const operation = SpatialOperation::IsWithin;
  constexpr bool match = true; // yes it is within

  // the rest is super.testOperation without leading assert:

  adoc(L"0", indexedShape);
  commit();
  shared_ptr<Query> query =
      strategy->makeQuery(make_shared<SpatialArgs>(operation, queryShape));
  shared_ptr<SearchResults> got = executeQuery(query, 1);
  assert((got->numFound <= 1, L"unclean test env"));
  if ((got->numFound == 1) != match) {
    fail(operation + L" I:" + indexedShape + L" Q:" + queryShape);
  }
  deleteAll(); // clean up after ourselves
}

void TestBBoxStrategy::setupGeo()
{
  this->ctx = SpatialContext::GEO;
  this->strategy = BBoxStrategy::newInstance(ctx, L"bbox");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Ignore("Overlaps not supported") public void
// testBasicOperaions() throws java.io.IOException
void TestBBoxStrategy::testBasicOperaions() 
{
  setupGeo();
  getAddAndVerifyIndexedDocuments(DATA_SIMPLE_BBOX);

  executeQueries(SpatialMatchConcern::EXACT, {QTEST_Simple_Queries_BBox});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testStatesBBox() throws java.io.IOException
void TestBBoxStrategy::testStatesBBox() 
{
  setupGeo();
  getAddAndVerifyIndexedDocuments(DATA_STATES_BBOX);

  executeQueries(SpatialMatchConcern::FILTER, {QTEST_States_IsWithin_BBox});
  executeQueries(SpatialMatchConcern::FILTER, {QTEST_States_Intersects_BBox});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCitiesIntersectsBBox() throws
// java.io.IOException
void TestBBoxStrategy::testCitiesIntersectsBBox() 
{
  setupGeo();
  getAddAndVerifyIndexedDocuments(DATA_WORLD_CITIES_POINTS);

  executeQueries(SpatialMatchConcern::FILTER, {QTEST_Cities_Intersects_BBox});
}

shared_ptr<Shape>
TestBBoxStrategy::convertShapeFromGetDocuments(shared_ptr<Shape> shape)
{
  return shape->getBoundingBox();
}

shared_ptr<BBoxStrategy>
TestBBoxStrategy::setupNeedsDocValuesOnly() 
{
  this->ctx = SpatialContext::GEO;
  shared_ptr<FieldType> fieldType;
  // random  legacy or not legacy
  wstring FIELD_PREFIX = L"bbox";
  fieldType = make_shared<FieldType>(BBoxStrategy::DEFAULT_FIELDTYPE);
  if (random()->nextBoolean()) {
    fieldType->setDimensions(0, 0);
  }

  strategy = make_shared<BBoxStrategy>(ctx, FIELD_PREFIX, fieldType);
  return std::static_pointer_cast<BBoxStrategy>(strategy);
}

void TestBBoxStrategy::testOverlapRatio() 
{
  setupNeedsDocValuesOnly();

  // Simply assert null shape results in 0
  adoc(L"999", std::static_pointer_cast<Shape>(nullptr));
  commit();
  shared_ptr<BBoxStrategy> bboxStrategy =
      std::static_pointer_cast<BBoxStrategy>(strategy);
  checkValueSource(
      bboxStrategy->makeOverlapRatioValueSource(randomRectangle(), 0.0),
      std::deque<float>{0.0f}, 0.0f);

  // we test raw BBoxOverlapRatioValueSource without actual indexing
  for (int SHIFT = 0; SHIFT < 360; SHIFT += 10) {
    shared_ptr<Rectangle> queryBox =
        shiftedRect(0, 40, -20, 20, SHIFT); // 40x40, 1600 area

    constexpr bool MSL = random()->nextBoolean();
    constexpr double minSideLength = MSL ? 0.1 : 0.0;
    shared_ptr<BBoxOverlapRatioValueSource> sim =
        make_shared<BBoxOverlapRatioValueSource>(nullptr, true, queryBox, 0.5,
                                                 minSideLength);
    int nudge = SHIFT == 0 ? 0
                           : random()->nextInt(3) * 10 -
                                 10; //-10, 0, or 10.  Keep 0 on first round.

    constexpr double EPS = 0.0000001;

    assertEquals(L"within", (200 / 1600 * 0.5) + (0.5),
                 sim->score(shiftedRect(10, 30, 0, 10, SHIFT + nudge), nullptr),
                 EPS);

    assertEquals(L"in25%", 0.25,
                 sim->score(shiftedRect(30, 70, -20, 20, SHIFT), nullptr), EPS);

    assertEquals(
        L"wrap", 0.2794117,
        sim->score(shiftedRect(30, 10, -20, 20, SHIFT + nudge), nullptr), EPS);

    assertEquals(L"no intersection H", 0.0,
                 sim->score(shiftedRect(-10, -10, -20, 20, SHIFT), nullptr),
                 EPS);
    assertEquals(L"no intersection V", 0.0,
                 sim->score(shiftedRect(0, 20, -30, -30, SHIFT), nullptr), EPS);

    assertEquals(L"point", 0.5 + (MSL ? (0.1 * 0.1 / 1600.0 / 2.0) : 0),
                 sim->score(shiftedRect(0, 0, 0, 0, SHIFT), nullptr), EPS);

    assertEquals(L"line 25% intersection",
                 0.25 / 2 + (MSL ? (10.0 * 0.1 / 1600.0 / 2.0) : 0.0),
                 sim->score(shiftedRect(-30, 10, 0, 0, SHIFT), nullptr), EPS);

    // test with point query
    sim = make_shared<BBoxOverlapRatioValueSource>(
        nullptr, true, shiftedRect(0, 0, 0, 0, SHIFT), 0.5, minSideLength);
    assertEquals(L"same", 1.0,
                 sim->score(shiftedRect(0, 0, 0, 0, SHIFT), nullptr), EPS);
    assertEquals(L"contains", 0.5 + (MSL ? (0.1 * 0.1 / (30 * 10) / 2.0) : 0.0),
                 sim->score(shiftedRect(0, 30, 0, 10, SHIFT), nullptr), EPS);

    // test with line query (vertical this time)
    sim = make_shared<BBoxOverlapRatioValueSource>(
        nullptr, true, shiftedRect(0, 0, 20, 40, SHIFT), 0.5, minSideLength);
    assertEquals(L"line 50%", 0.5,
                 sim->score(shiftedRect(0, 0, 10, 30, SHIFT), nullptr), EPS);
    assertEquals(L"point", 0.5 + (MSL ? (0.1 * 0.1 / (20 * 0.1) / 2.0) : 0.0),
                 sim->score(shiftedRect(0, 0, 30, 30, SHIFT), nullptr), EPS);
  }
}

shared_ptr<Rectangle> TestBBoxStrategy::shiftedRect(double minX, double maxX,
                                                    double minY, double maxY,
                                                    int xShift)
{
  return ctx->makeRectangle(DistanceUtils::normLonDEG(minX + xShift),
                            DistanceUtils::normLonDEG(maxX + xShift), minY,
                            maxY);
}

void TestBBoxStrategy::testAreaValueSource() 
{
  shared_ptr<BBoxStrategy> bboxStrategy = setupNeedsDocValuesOnly();

  adoc(L"100", ctx->makeRectangle(0, 20, 40, 80));
  adoc(L"999", std::static_pointer_cast<Shape>(nullptr));
  commit();
  checkValueSource(make_shared<ShapeAreaValueSource>(
                       bboxStrategy->makeShapeValueSource(), ctx, false, 1.0),
                   std::deque<float>{800.0f, 0.0f}, 0.0f);
  checkValueSource(make_shared<ShapeAreaValueSource>(
                       bboxStrategy->makeShapeValueSource(), ctx, true, 1.0),
                   std::deque<float>{391.93f, 0.0f}, 0.01f);
  checkValueSource(make_shared<ShapeAreaValueSource>(
                       bboxStrategy->makeShapeValueSource(), ctx, true, 2.0),
                   std::deque<float>{783.86f, 0.0f},
                   0.01f); // testing with a different multiplier
}
} // namespace org::apache::lucene::spatial::bbox