using namespace std;

#include "RandomSpatialOpFuzzyPrefixTreeTest.h"

namespace org::apache::lucene::spatial::prefix
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using Query = org::apache::lucene::search::Query;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using PackedQuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::PackedQuadPrefixTree;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::ShapeCollection;
using org::locationtech::spatial4j::shape::SpatialRelation;
using org::locationtech::spatial4j::shape::impl::RectangleImpl;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomBoolean; import
//    static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt; import
//    static com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;
//    import static org.locationtech.spatial4j.shape.SpatialRelation.CONTAINS;
//    import static org.locationtech.spatial4j.shape.SpatialRelation.DISJOINT;
//    import static org.locationtech.spatial4j.shape.SpatialRelation.INTERSECTS;
//    import static org.locationtech.spatial4j.shape.SpatialRelation.WITHIN;

void RandomSpatialOpFuzzyPrefixTreeTest::setupGrid(int maxLevels) throw(
    IOException)
{
  if (randomBoolean()) {
    setupQuadGrid(maxLevels, randomBoolean());
  } else {
    setupGeohashGrid(maxLevels);
  }
  setupCtx2D(ctx);

  // set prune independently on strategy & grid randomly; should work
  (std::static_pointer_cast<RecursivePrefixTreeStrategy>(strategy))
      ->setPruneLeafyBranches(randomBoolean());
  if (std::dynamic_pointer_cast<PackedQuadPrefixTree>(this->grid) != nullptr) {
    (std::static_pointer_cast<PackedQuadPrefixTree>(this->grid))
        ->setPruneLeafyBranches(randomBoolean());
  }

  if (maxLevels == -1 && rarely()) {
    (std::static_pointer_cast<PrefixTreeStrategy>(strategy))
        ->setPointsOnly(true);
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  log->info(L"Strategy: " + strategy->toString());
}

void RandomSpatialOpFuzzyPrefixTreeTest::setupCtx2D(
    shared_ptr<SpatialContext> ctx)
{
  if (!ctx->isGeo()) {
    ctx2D = ctx;
  }
  // A non-geo version of ctx.
  shared_ptr<SpatialContextFactory> ctxFactory =
      make_shared<SpatialContextFactory>();
  ctxFactory->geo = false;
  ctxFactory->worldBounds = ctx->getWorldBounds();
  ctx2D = ctxFactory->newSpatialContext();
}

void RandomSpatialOpFuzzyPrefixTreeTest::setupQuadGrid(
    int maxLevels, bool packedQuadPrefixTree)
{
  // non-geospatial makes this test a little easier (in gridSnap), and using
  // boundary values 2^X raises
  // the prospect of edge conditions we want to test, plus makes for simpler
  // numbers (no decimals).
  shared_ptr<SpatialContextFactory> factory =
      make_shared<SpatialContextFactory>();
  factory->geo = false;
  factory->worldBounds = make_shared<RectangleImpl>(0, 256, -128, 128, nullptr);
  this->ctx = factory->newSpatialContext();
  // A fairly shallow grid, and default 2.5% distErrPct
  if (maxLevels == -1) {
    maxLevels = randomIntBetween(1, 8); // max 64k cells (4^8), also 256*256
  }
  if (packedQuadPrefixTree) {
    this->grid = make_shared<PackedQuadPrefixTree>(ctx, maxLevels);
  } else {
    this->grid = make_shared<QuadPrefixTree>(ctx, maxLevels);
  }
  this->strategy = newRPT();
}

void RandomSpatialOpFuzzyPrefixTreeTest::setupGeohashGrid(int maxLevels)
{
  this->ctx = SpatialContext::GEO;
  // A fairly shallow grid, and default 2.5% distErrPct
  if (maxLevels == -1) {
    maxLevels = randomIntBetween(1, 3); // max 16k cells (32^3)
  }
  this->grid = make_shared<GeohashPrefixTree>(ctx, maxLevels);
  this->strategy = newRPT();
}

shared_ptr<RecursivePrefixTreeStrategy>
RandomSpatialOpFuzzyPrefixTreeTest::newRPT()
{
  return make_shared<RecursivePrefixTreeStrategy>(this->grid,
                                                  getClass().getSimpleName());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
// testIntersects() throws java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::testIntersects() 
{
  setupGrid(-1);
  doTest(SpatialOperation::Intersects);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
// testWithin() throws java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::testWithin() 
{
  setupGrid(-1);
  doTest(SpatialOperation::IsWithin);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
// testContains() throws java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::testContains() 
{
  setupGrid(-1);
  doTest(SpatialOperation::Contains);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPackedQuadPointsOnlyBug() throws
// java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::testPackedQuadPointsOnlyBug() throw(
    IOException)
{
  setupQuadGrid(1, true); // packed quad.  maxLevels doesn't matter.
  setupCtx2D(ctx);
  (std::static_pointer_cast<PrefixTreeStrategy>(strategy))->setPointsOnly(true);
  shared_ptr<Point> point = ctx->makePoint(169.0, 107.0);
  adoc(L"0", point);
  commit();
  shared_ptr<Query> query = strategy->makeQuery(
      make_shared<SpatialArgs>(SpatialOperation::Intersects, point));
  assertEquals(1, executeQuery(query, 1)->numFound);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPointsOnlyOptBug() throws
// java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::testPointsOnlyOptBug() throw(
    IOException)
{
  setupQuadGrid(8, false);
  setupCtx2D(ctx);
  (std::static_pointer_cast<PrefixTreeStrategy>(strategy))->setPointsOnly(true);
  shared_ptr<Point> point = ctx->makePoint(86, -127.44362190053255);
  adoc(L"0", point);
  commit();
  shared_ptr<Query> query = strategy->makeQuery(make_shared<SpatialArgs>(
      SpatialOperation::Intersects, ctx->makeRectangle(point, point)));
  assertEquals(1, executeQuery(query, 1)->numFound);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testContainsPairOverlap() throws
// java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::testContainsPairOverlap() throw(
    IOException)
{
  setupQuadGrid(3, randomBoolean());
  adoc(L"0", make_shared<ShapePair>(
                 shared_from_this(), ctx->makeRectangle(0, 33, -128, 128),
                 ctx->makeRectangle(33, 128, -128, 128), true));
  commit();
  shared_ptr<Query> query = strategy->makeQuery(make_shared<SpatialArgs>(
      SpatialOperation::Contains, ctx->makeRectangle(0, 128, -16, 128)));
  shared_ptr<SearchResults> searchResults = executeQuery(query, 1);
  assertEquals(1, searchResults->numFound);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithinDisjointParts() throws
// java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::testWithinDisjointParts() throw(
    IOException)
{
  setupQuadGrid(7, randomBoolean());
  // one shape comprised of two parts, quite separated apart
  adoc(L"0", make_shared<ShapePair>(
                 shared_from_this(), ctx->makeRectangle(0, 10, -120, -100),
                 ctx->makeRectangle(220, 240, 110, 125), false));
  commit();
  // query surrounds only the second part of the indexed shape
  shared_ptr<Query> query = strategy->makeQuery(make_shared<SpatialArgs>(
      SpatialOperation::IsWithin, ctx->makeRectangle(210, 245, 105, 128)));
  shared_ptr<SearchResults> searchResults = executeQuery(query, 1);
  // we shouldn't find it because it's not completely within
  assertTrue(searchResults->numFound == 0);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithinLeafApproxRule() throws
// java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::testWithinLeafApproxRule() throw(
    IOException)
{
  setupQuadGrid(2, randomBoolean()); // 4x4 grid
  // indexed shape will simplify to entire right half (2 top cells)
  adoc(L"0", ctx->makeRectangle(192, 204, -128, 128));
  commit();

  (std::static_pointer_cast<RecursivePrefixTreeStrategy>(strategy))
      ->setPrefixGridScanLevel(randomInt(2));

  // query does NOT contain it; both indexed cells are leaves to the query, and
  // when expanded to the full grid cells, the top one's top row is disjoint
  // from the query and thus not a match.
  assertTrue(executeQuery(strategy->makeQuery(make_shared<SpatialArgs>(
                              SpatialOperation::IsWithin,
                              ctx->makeRectangle(38, 192, -72, 56))),
                          1)
                 .numFound == 0); // no-match

  // this time the rect is a little bigger and is considered a match. It's
  // an acceptable false-positive because of the grid approximation.
  assertTrue(executeQuery(strategy->makeQuery(make_shared<SpatialArgs>(
                              SpatialOperation::IsWithin,
                              ctx->makeRectangle(38, 192, -72, 80))),
                          1)
                 .numFound == 1); // match
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testShapePair()
void RandomSpatialOpFuzzyPrefixTreeTest::testShapePair()
{
  ctx = SpatialContext::GEO;
  setupCtx2D(ctx);

  shared_ptr<Shape> leftShape = make_shared<ShapePair>(
      shared_from_this(), ctx->makeRectangle(-74, -56, -8, 1),
      ctx->makeRectangle(-180, 134, -90, 90), true);
  shared_ptr<Shape> queryShape = ctx->makeRectangle(-180, 180, -90, 90);
  assertEquals(SpatialRelation::WITHIN, leftShape->relate(queryShape));
}

shared_ptr<Document>
RandomSpatialOpFuzzyPrefixTreeTest::newDoc(const wstring &id,
                                           shared_ptr<Shape> shape)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", id, Field::Store::YES));
  if (shape != nullptr) {
    shared_ptr<deque<std::shared_ptr<Shape>>> shapes;
    if (std::dynamic_pointer_cast<ShapePair>(shape) != nullptr) {
      shapes = deque<>(2);
      shapes->add((std::static_pointer_cast<ShapePair>(shape))->shape1);
      shapes->add((std::static_pointer_cast<ShapePair>(shape))->shape2);
    } else {
      shapes = Collections::singleton(shape);
    }
    for (auto shapei : shapes) {
      for (auto f : strategy->createIndexableFields(shapei)) {
        doc->push_back(f);
      }
    }
    if (storeShape) // just for diagnostics
    {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<StoredField>(strategy->getFieldName(),
                                              shape->toString()));
    }
  }
  return doc;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") private void doTest(final
// org.apache.lucene.spatial.query.SpatialOperation operation) throws
// java.io.IOException
void RandomSpatialOpFuzzyPrefixTreeTest::doTest(
    shared_ptr<SpatialOperation> operation) 
{
  {
    // first show that when there's no data, a query will result in no results
    shared_ptr<Query> query = strategy->makeQuery(
        make_shared<SpatialArgs>(operation, randomRectangle()));
    shared_ptr<SearchResults> searchResults = executeQuery(query, 1);
    assertEquals(0, searchResults->numFound);
  }

  constexpr bool biasContains = (operation == SpatialOperation::Contains);

  // Main index loop:
  unordered_map<wstring, std::shared_ptr<Shape>> indexedShapes =
      make_shared<LinkedHashMap<wstring, std::shared_ptr<Shape>>>();
  unordered_map<wstring, std::shared_ptr<Shape>> indexedShapesGS =
      make_shared<LinkedHashMap<wstring, std::shared_ptr<Shape>>>(); // grid
                                                                     // snapped
  constexpr int numIndexedShapes = randomIntBetween(1, 6);
  bool indexedAtLeastOneShapePair = false;
  constexpr bool pointsOnly =
      (std::static_pointer_cast<PrefixTreeStrategy>(strategy))->isPointsOnly();
  for (int i = 0; i < numIndexedShapes; i++) {
    wstring id = L"" + to_wstring(i);
    shared_ptr<Shape> indexedShape;
    int R = random()->nextInt(12);
    if (R == 0) { // 1 in 12
      indexedShape.reset();
    } else if (R == 1 || pointsOnly) { // 1 in 12
      indexedShape = randomPoint();    // just one point
    } else if (R <= 4) {               // 3 in 12
      // comprised of more than one shape
      indexedShape = randomShapePairRect(biasContains);
      indexedAtLeastOneShapePair = true;
    } else {
      indexedShape = randomRectangle(); // just one rect
    }

    indexedShapes.emplace(id, indexedShape);
    indexedShapesGS.emplace(id, gridSnap(indexedShape));

    adoc(id, indexedShape);

    if (random()->nextInt(10) == 0) {
      commit(); // intermediate commit, produces extra segments
    }
  }
  // delete some documents randomly
  Iterator<wstring> idIter = indexedShapes.keySet().begin();
  while (idIter->hasNext()) {
    wstring id = idIter->next();
    if (random()->nextInt(10) == 0) {
      deleteDoc(id);
      idIter->remove();
      indexedShapesGS.erase(id);
    }
    idIter++;
  }

  commit();

  // Main query loop:
  constexpr int numQueryShapes = atLeast(20);
  for (int i = 0; i < numQueryShapes; i++) {
    int scanLevel = randomInt(grid->getMaxLevels());
    (std::static_pointer_cast<RecursivePrefixTreeStrategy>(strategy))
        ->setPrefixGridScanLevel(scanLevel);

    shared_ptr<Shape> *const queryShape;
    switch (randomInt(10)) {
    case 0:
      queryShape = randomPoint();
      break;
      // LUCENE-5549
      // TODO debug: -Dtests.method=testWithin -Dtests.multiplier=3
      // -Dtests.seed=5F5294CE2E075A3E:AAD2F0F79288CA64
      //        case 1:case 2:case 3:
      //          if (!indexedAtLeastOneShapePair) { // avoids
      //          ShapePair.relate(ShapePair), which isn't reliable
      //            queryShape = randomShapePairRect(!biasContains);//invert
      //            biasContains for query side break;
      //          }

    case 4:
      // choose an existing indexed shape
      if (!indexedShapes.empty()) {
        shared_ptr<Shape> tmp = indexedShapes.values().begin()->next();
        if (std::dynamic_pointer_cast<Point>(tmp) != nullptr ||
            std::dynamic_pointer_cast<Rectangle>(tmp) !=
                nullptr) { // avoids null and shapePair
          queryShape = tmp;
          break;
        }
      } // else fall-through

    default:
      queryShape = randomRectangle();
    }
    shared_ptr<Shape> *const queryShapeGS = gridSnap(queryShape);

    constexpr bool opIsDisjoint = operation == SpatialOperation::IsDisjointTo;

    // Generate truth via brute force:
    // We ensure true-positive matches (if the predicate on the raw shapes match
    //  then the search should find those same matches).
    // approximations, false-positive matches
    shared_ptr<Set<wstring>> expectedIds =
        make_shared<LinkedHashSet<wstring>>(); // true-positives
    shared_ptr<Set<wstring>> secondaryIds =
        make_shared<LinkedHashSet<wstring>>(); // false-positives (unless
                                               // disjoint)
    for (auto entry : indexedShapes) {
      wstring id = entry.first;
      shared_ptr<Shape> indexedShapeCompare = entry.second;
      if (indexedShapeCompare == nullptr) {
        continue;
      }
      shared_ptr<Shape> queryShapeCompare = queryShape;

      if (operation->evaluate(indexedShapeCompare, queryShapeCompare)) {
        expectedIds->add(id);
        if (opIsDisjoint) {
          // if no longer intersect after buffering them, for disjoint, remember
          // this
          indexedShapeCompare = indexedShapesGS[id];
          queryShapeCompare = queryShapeGS;
          if (!operation->evaluate(indexedShapeCompare, queryShapeCompare)) {
            secondaryIds->add(id);
          }
        }
      } else if (!opIsDisjoint) {
        // buffer either the indexed or query shape (via gridSnap) and try again
        if (operation == SpatialOperation::Intersects) {
          indexedShapeCompare = indexedShapesGS[id];
          queryShapeCompare = queryShapeGS;
          // TODO Unfortunately, grid-snapping both can result in intersections
          // that otherwise
          // wouldn't happen when the grids are adjacent. Not a big deal but our
          // test is just a bit more lenient.
        } else if (operation == SpatialOperation::Contains) {
          indexedShapeCompare = indexedShapesGS[id];
        } else if (operation == SpatialOperation::IsWithin) {
          queryShapeCompare = queryShapeGS;
        }
        if (operation->evaluate(indexedShapeCompare, queryShapeCompare)) {
          secondaryIds->add(id);
        }
      }
    }

    // Search and verify results
    shared_ptr<SpatialArgs> args =
        make_shared<SpatialArgs>(operation, queryShape);
    if (std::dynamic_pointer_cast<ShapePair>(queryShape) != nullptr) {
      args->setDistErrPct(
          0.0); // a hack; we want to be more detailed than gridSnap(queryShape)
    }
    shared_ptr<Query> query = strategy->makeQuery(args);
    shared_ptr<SearchResults> got = executeQuery(query, 100);
    shared_ptr<Set<wstring>> remainingExpectedIds =
        make_shared<LinkedHashSet<wstring>>(expectedIds);
    for (auto result : got->results) {
      wstring id = result->getId();
      bool removed = remainingExpectedIds->remove(id);
      if (!removed && (!opIsDisjoint && !secondaryIds->contains(id))) {
        fail(L"Shouldn't match", id, indexedShapes, indexedShapesGS,
             queryShape);
      }
    }
    if (opIsDisjoint) {
      remainingExpectedIds->removeAll(secondaryIds);
    }
    if (!remainingExpectedIds->isEmpty()) {
      wstring id = remainingExpectedIds->begin()->next();
      fail(L"Should have matched", id, indexedShapes, indexedShapesGS,
           queryShape);
    }
  }
}

shared_ptr<Shape>
RandomSpatialOpFuzzyPrefixTreeTest::randomShapePairRect(bool biasContains)
{
  shared_ptr<Rectangle> shape1 = randomRectangle();
  shared_ptr<Rectangle> shape2 = randomRectangle();
  return make_shared<ShapePair>(shared_from_this(), shape1, shape2,
                                biasContains);
}

void RandomSpatialOpFuzzyPrefixTreeTest::fail(
    const wstring &label, const wstring &id,
    unordered_map<wstring, std::shared_ptr<Shape>> &indexedShapes,
    unordered_map<wstring, std::shared_ptr<Shape>> &indexedShapesGS,
    shared_ptr<Shape> queryShape)
{
  System::err::println(L"Ig:" + indexedShapesGS[id] + L" Qg:" +
                       gridSnap(queryShape));
  fail(label + L" I#" + id + L":" + indexedShapes[id] + L" Q:" + queryShape);
}

shared_ptr<Shape>
RandomSpatialOpFuzzyPrefixTreeTest::gridSnap(shared_ptr<Shape> snapMe)
{
  if (snapMe == nullptr) {
    return nullptr;
  }
  if (std::dynamic_pointer_cast<ShapePair>(snapMe) != nullptr) {
    shared_ptr<ShapePair> me = std::static_pointer_cast<ShapePair>(snapMe);
    return make_shared<ShapePair>(shared_from_this(), gridSnap(me->shape1),
                                  gridSnap(me->shape2),
                                  me->biasContainsThenWithin);
  }
  if (std::dynamic_pointer_cast<Point>(snapMe) != nullptr) {
    snapMe = snapMe->getBoundingBox();
  }
  // The next 4 lines mimic PrefixTreeStrategy.createIndexableFields()
  double distErrPct =
      (std::static_pointer_cast<PrefixTreeStrategy>(strategy))->getDistErrPct();
  double distErr = SpatialArgs::calcDistanceFromErrPct(snapMe, distErrPct, ctx);
  int detailLevel = grid->getLevelForDistance(distErr);
  shared_ptr<CellIterator> cells =
      grid->getTreeCellIterator(snapMe, detailLevel);

  // calc bounding box of cells.
  deque<std::shared_ptr<Shape>> cellShapes =
      deque<std::shared_ptr<Shape>>(1024);
  while (cells->hasNext()) {
    shared_ptr<Cell> cell = cells->next();
    if (!cell->isLeaf()) {
      continue;
    }
    cellShapes.push_back(cell->getShape());
    cells++;
  }
  return (make_shared<ShapeCollection<>>(cellShapes, ctx))->getBoundingBox();
}

RandomSpatialOpFuzzyPrefixTreeTest::ShapePair::ShapePair(
    shared_ptr<RandomSpatialOpFuzzyPrefixTreeTest> outerInstance,
    shared_ptr<Shape> shape1, shared_ptr<Shape> shape2, bool containsThenWithin)
    : org::locationtech::spatial4j::shape::ShapeCollection<
          org::locationtech::spatial4j::shape::Shape>(
          Arrays::asList(shape1, shape2),
          RandomSpatialOpFuzzyPrefixTreeTest::this->ctx),
      shape1(shape1), shape2(shape2), shape1_2D(toNonGeo(shape1)),
      shape2_2D(toNonGeo(shape2)), biasContainsThenWithin(containsThenWithin),
      outerInstance(outerInstance)
{
}

shared_ptr<Shape>
RandomSpatialOpFuzzyPrefixTreeTest::ShapePair::toNonGeo(shared_ptr<Shape> shape)
{
  if (!outerInstance->ctx->isGeo()) {
    return shape; // already non-geo
  }
  if (std::dynamic_pointer_cast<Rectangle>(shape) != nullptr) {
    shared_ptr<Rectangle> rect = std::static_pointer_cast<Rectangle>(shape);
    if (rect->getCrossesDateLine()) {
      return make_shared<ShapePair>(
          outerInstance->ctx2D->makeRectangle(rect->getMinX(), 180,
                                              rect->getMinY(), rect->getMaxY()),
          outerInstance->ctx2D->makeRectangle(-180, rect->getMaxX(),
                                              rect->getMinY(), rect->getMaxY()),
          biasContainsThenWithin);
    } else {
      return outerInstance->ctx2D->makeRectangle(
          rect->getMinX(), rect->getMaxX(), rect->getMinY(), rect->getMaxY());
    }
  }
  // no need to do others; this addresses the -180/+180 ambiguity corner test
  // problem
  return shape;
}

shared_ptr<SpatialRelation>
RandomSpatialOpFuzzyPrefixTreeTest::ShapePair::relate(shared_ptr<Shape> other)
{
  shared_ptr<SpatialRelation> r = relateApprox(other);
  if (r == DISJOINT) {
    return r;
  }
  if (r == CONTAINS) {
    return r;
  }
  if (r == WITHIN && !biasContainsThenWithin) {
    return r;
  }

  // See if the correct answer is actually Contains, when the indexed shapes are
  // adjacent,
  // creating a larger shape that contains the input shape.
  bool pairTouches = shape1->relate(shape2).intersects();
  if (!pairTouches) {
    return r;
  }
  // test all 4 corners
  // Note: awkwardly, we use a non-geo context for this because in geo, -180 &
  // +180 are the same place, which means
  //  that "other" might wrap the world horizontally and yet all its corners
  //  could be in shape1 (or shape2) even though shape1 is only adjacent to the
  //  dateline. I couldn't think of a better way to handle this.
  shared_ptr<Rectangle> oRect = std::static_pointer_cast<Rectangle>(other);
  if (cornerContainsNonGeo(oRect->getMinX(), oRect->getMinY()) &&
      cornerContainsNonGeo(oRect->getMinX(), oRect->getMaxY()) &&
      cornerContainsNonGeo(oRect->getMaxX(), oRect->getMinY()) &&
      cornerContainsNonGeo(oRect->getMaxX(), oRect->getMaxY())) {
    return CONTAINS;
  }
  return r;
}

bool RandomSpatialOpFuzzyPrefixTreeTest::ShapePair::cornerContainsNonGeo(
    double x, double y)
{
  shared_ptr<Shape> pt = outerInstance->ctx2D->makePoint(x, y);
  return shape1_2D->relate(pt).intersects() ||
         shape2_2D->relate(pt).intersects();
}

shared_ptr<SpatialRelation>
RandomSpatialOpFuzzyPrefixTreeTest::ShapePair::relateApprox(
    shared_ptr<Shape> other)
{
  if (biasContainsThenWithin) {
    if (shape1->relate(other) == CONTAINS || shape1->equals(other) ||
        shape2->relate(other) == CONTAINS || shape2->equals(other)) {
      return CONTAINS;
    }

    if (shape1->relate(other) == WITHIN && shape2->relate(other) == WITHIN) {
      return WITHIN;
    }

  } else {
    if ((shape1->relate(other) == WITHIN || shape1->equals(other)) &&
        (shape2->relate(other) == WITHIN || shape2->equals(other))) {
      return WITHIN;
    }

    if (shape1->relate(other) == CONTAINS ||
        shape2->relate(other) == CONTAINS) {
      return CONTAINS;
    }
  }

  if (shape1->relate(other).intersects() ||
      shape2->relate(other).intersects()) {
    return INTERSECTS; // might actually be 'CONTAINS' if the pair are adjacent
                       // but we handle that later
  }
  return DISJOINT;
}

wstring RandomSpatialOpFuzzyPrefixTreeTest::ShapePair::toString()
{
  return L"ShapePair(" + shape1 + L" , " + shape2 + L")";
}
} // namespace org::apache::lucene::spatial::prefix