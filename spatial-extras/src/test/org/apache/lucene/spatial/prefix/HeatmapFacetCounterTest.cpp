using namespace std;

#include "HeatmapFacetCounterTest.h"

namespace org::apache::lucene::spatial::prefix
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using Query = org::apache::lucene::search::Query;
using TotalHitCountCollector =
    org::apache::lucene::search::TotalHitCountCollector;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using Bits = org::apache::lucene::util::Bits;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using org::locationtech::spatial4j::shape::impl::RectangleImpl;
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.atMost;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void setUp() throws Exception
void HeatmapFacetCounterTest::setUp() 
{
  StrategyTestCase::setUp();
  cellsValidated = cellValidatedNonZero = 0;
  ctx = SpatialContext::GEO;
  grid = make_shared<QuadPrefixTree>(ctx, randomIntBetween(1, 8));
  strategy =
      make_shared<RecursivePrefixTreeStrategy>(grid, getTestClass().name());
  if (rarely()) {
    (std::static_pointer_cast<PrefixTreeStrategy>(strategy))
        ->setPointsOnly(true);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void after()
void HeatmapFacetCounterTest::after()
{
  log->info(L"Validated " + to_wstring(cellsValidated) + L" cells, " +
            to_wstring(cellValidatedNonZero) + L" non-zero");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testStatic() throws java.io.IOException
void HeatmapFacetCounterTest::testStatic() 
{
  // Some specific tests (static, not random).
  adoc(L"0",
       ctx->makeRectangle(179.8, -170, -90, -80)); // barely crosses equator
  adoc(L"1", ctx->makePoint(-180, -85));           // a pt within the above rect
  adoc(L"2", ctx->makePoint(172, -85));            // a pt to left of rect
  commit();

  validateHeatmapResultLoop(ctx->makeRectangle(+170, +180, -90, -85), 1, 100);
  validateHeatmapResultLoop(ctx->makeRectangle(-180, -160, -89, -50), 1, 100);
  validateHeatmapResultLoop(ctx->makeRectangle(179, 179, -89, -50), 1,
                            100); // line
  // We could test anything and everything at this point... I prefer we leave
  // that to random testing and then add specific tests if we find a bug.
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLucene7291Dateline() throws
// java.io.IOException
void HeatmapFacetCounterTest::testLucene7291Dateline() 
{
  grid = make_shared<QuadPrefixTree>(
      ctx, 2); // only 2, and we wind up with some big leaf cells
  strategy =
      make_shared<RecursivePrefixTreeStrategy>(grid, getTestClass().name());
  adoc(L"0", ctx->makeRectangle(-102, -83, 43, 52));
  commit();
  validateHeatmapResultLoop(ctx->makeRectangle(179, -179, 62, 63), 2,
                            100); // HM crosses dateline
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testQueryCircle() throws java.io.IOException
void HeatmapFacetCounterTest::testQueryCircle() 
{
  // overwrite setUp; non-geo bounds is more straight-forward; otherwise 88,88
  // would actually be practically north,
  shared_ptr<SpatialContextFactory> *const spatialContextFactory =
      make_shared<SpatialContextFactory>();
  spatialContextFactory->geo = false;
  spatialContextFactory->worldBounds =
      make_shared<RectangleImpl>(-90, 90, -90, 90, nullptr);
  ctx = spatialContextFactory->newSpatialContext();
  constexpr int LEVEL = 4;
  grid = make_shared<QuadPrefixTree>(ctx, LEVEL);
  strategy =
      make_shared<RecursivePrefixTreeStrategy>(grid, getTestClass().name());
  shared_ptr<Circle> circle = ctx->makeCircle(0, 0, 89);
  adoc(L"0",
       ctx->makePoint(
           88, 88)); // top-right, inside bbox of circle but not the circle
  adoc(L"1", ctx->makePoint(0, 0)); // clearly inside; dead center in fact
  commit();
  shared_ptr<HeatmapFacetCounter::Heatmap> *const heatmap =
      HeatmapFacetCounter::calcFacets(
          std::static_pointer_cast<PrefixTreeStrategy>(strategy),
          indexSearcher->getTopReaderContext(), nullptr, circle, LEVEL, 1000);
  // assert that only one point is found, not 2
  bool foundOne = false;
  for (auto count : heatmap->counts) {
    switch (count) {
    case 0:
      break;
    case 1:
      assertFalse(foundOne); // this is the first
      foundOne = true;
      break;
    default:
      fail(L"counts should be 0 or 1: " + to_wstring(count));
    }
  }
  assertTrue(foundOne);
}

void HeatmapFacetCounterTest::validateHeatmapResultLoop(
    shared_ptr<Rectangle> inputRange, int facetLevel,
    int cellCountRecursThreshold) 
{
  if (facetLevel > grid->getMaxLevels()) {
    return;
  }
  constexpr int maxCells = 10'000;
  shared_ptr<HeatmapFacetCounter::Heatmap> *const heatmap =
      HeatmapFacetCounter::calcFacets(
          std::static_pointer_cast<PrefixTreeStrategy>(strategy),
          indexSearcher->getTopReaderContext(), nullptr, inputRange, facetLevel,
          maxCells);
  int preNonZero = cellValidatedNonZero;
  validateHeatmapResult(inputRange, facetLevel, heatmap);
  assert(cellValidatedNonZero - preNonZero >
         0); // we validated more non-zero cells
  if (heatmap->counts.size() < cellCountRecursThreshold) {
    validateHeatmapResultLoop(inputRange, facetLevel + 1,
                              cellCountRecursThreshold);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 20) public void testRandom() throws
// java.io.IOException
void HeatmapFacetCounterTest::testRandom() 
{
  // Tests using random index shapes & query shapes. This has found all sorts of
  // edge case bugs (e.g. dateline, cell border, overflow(?)).

  constexpr int numIndexedShapes = 1 + atMost(9);
  deque<std::shared_ptr<Shape>> indexedShapes =
      deque<std::shared_ptr<Shape>>(numIndexedShapes);
  for (int i = 0; i < numIndexedShapes; i++) {
    indexedShapes.push_back(randomIndexedShape());
  }

  // Main index loop:
  for (int i = 0; i < indexedShapes.size(); i++) {
    shared_ptr<Shape> shape = indexedShapes[i];
    adoc(L"" + to_wstring(i), shape);

    if (random()->nextInt(10) == 0) {
      commit(); // intermediate commit, produces extra segments
    }
  }
  // delete some documents randomly
  for (int id = 0; id < indexedShapes.size(); id++) {
    if (random()->nextInt(10) == 0) {
      deleteDoc(L"" + to_wstring(id));
      indexedShapes[id].reset();
    }
  }

  commit();

  // once without dateline wrap
  shared_ptr<Rectangle> *const rect = randomRectangle();
  queryHeatmapRecursive(usually() ? ctx->getWorldBounds() : rect, 1);
  // and once with dateline wrap
  if (rect->getWidth() > 0) {
    double shift = random()->nextDouble() % rect->getWidth();
    queryHeatmapRecursive(
        ctx->makeRectangle(DistanceUtils::normLonDEG(rect->getMinX() - shift),
                           DistanceUtils::normLonDEG(rect->getMaxX() - shift),
                           rect->getMinY(), rect->getMaxY()),
        1);
  }
}

bool HeatmapFacetCounterTest::queryHeatmapRecursive(
    shared_ptr<Rectangle> inputRange, int facetLevel) 
{
  if (!inputRange->hasArea()) {
    // Don't test line inputs. It's not that we don't support it but it is more
    // challenging to test if per-chance it coincides with a grid line due due
    // to edge overlap issue for some grid implementations (geo & quad).
    return false;
  }
  shared_ptr<Bits> filter =
      nullptr; // FYI testing filtering of underlying PrefixTreeFacetCounter is
               // done in another test
  // Calculate facets
  constexpr int maxCells = 10'000;
  shared_ptr<HeatmapFacetCounter::Heatmap> *const heatmap =
      HeatmapFacetCounter::calcFacets(
          std::static_pointer_cast<PrefixTreeStrategy>(strategy),
          indexSearcher->getTopReaderContext(), filter, inputRange, facetLevel,
          maxCells);

  validateHeatmapResult(inputRange, facetLevel, heatmap);

  bool foundNonZeroCount = false;
  for (auto count : heatmap->counts) {
    if (count > 0) {
      foundNonZeroCount = true;
      break;
    }
  }

  // Test again recursively to higher facetLevel (more detailed cells)
  if (foundNonZeroCount && cellsValidated <= 500 &&
      facetLevel != grid->getMaxLevels() && inputRange->hasArea()) {
    for (int i = 0; i < 5;
         i++) { // try multiple times until we find non-zero counts
      if (queryHeatmapRecursive(randomRectangle(inputRange), facetLevel + 1)) {
        break; // we found data here so we needn't try again
      }
    }
  }
  return foundNonZeroCount;
}

void HeatmapFacetCounterTest::validateHeatmapResult(
    shared_ptr<Rectangle> inputRange, int facetLevel,
    shared_ptr<HeatmapFacetCounter::Heatmap> heatmap) 
{
  shared_ptr<Rectangle> *const heatRect = heatmap->region;
  assertTrue(heatRect->relate(inputRange) == SpatialRelation::CONTAINS ||
             heatRect->equals(inputRange));
  constexpr double cellWidth = heatRect->getWidth() / heatmap->columns;
  constexpr double cellHeight = heatRect->getHeight() / heatmap->rows;
  for (int c = 0; c < heatmap->columns; c++) {
    for (int r = 0; r < heatmap->rows; r++) {
      constexpr int facetCount = heatmap->getCount(c, r);
      double x = DistanceUtils::normLonDEG(heatRect->getMinX() + c * cellWidth +
                                           cellWidth / 2);
      double y = DistanceUtils::normLatDEG(heatRect->getMinY() +
                                           r * cellHeight + cellHeight / 2);
      shared_ptr<Point> pt = ctx->makePoint(x, y);
      assertEquals(countMatchingDocsAtLevel(pt, facetLevel), facetCount);
    }
  }
}

int HeatmapFacetCounterTest::countMatchingDocsAtLevel(
    shared_ptr<Point> pt, int facetLevel) 
{
  // we use IntersectsPrefixTreeFilter directly so that we can specify the level
  // to go to exactly.
  shared_ptr<RecursivePrefixTreeStrategy> strategy =
      std::static_pointer_cast<RecursivePrefixTreeStrategy>(this->strategy);
  shared_ptr<Query> filter = make_shared<IntersectsPrefixTreeQuery>(
      pt, strategy->getFieldName(), grid, facetLevel, grid->getMaxLevels());
  shared_ptr<TotalHitCountCollector> *const collector =
      make_shared<TotalHitCountCollector>();
  indexSearcher->search(filter, collector);
  cellsValidated++;
  if (collector->getTotalHits() > 0) {
    cellValidatedNonZero++;
  }
  return collector->getTotalHits();
}

shared_ptr<Shape> HeatmapFacetCounterTest::randomIndexedShape()
{
  if ((std::static_pointer_cast<PrefixTreeStrategy>(strategy))
          ->isPointsOnly() ||
      random()->nextBoolean()) {
    return randomPoint();
  } else {
    return randomRectangle();
  }
}
} // namespace org::apache::lucene::spatial::prefix