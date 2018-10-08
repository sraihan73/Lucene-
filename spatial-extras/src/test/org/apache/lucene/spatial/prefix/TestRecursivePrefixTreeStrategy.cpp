using namespace std;

#include "TestRecursivePrefixTreeStrategy.h"

namespace org::apache::lucene::spatial::prefix
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;
using SpatialMatchConcern = org::apache::lucene::spatial::SpatialMatchConcern;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using org::junit::Test;

void TestRecursivePrefixTreeStrategy::init(int maxLength)
{
  this->maxLength = maxLength;
  this->ctx = SpatialContext::GEO;
  shared_ptr<GeohashPrefixTree> grid =
      make_shared<GeohashPrefixTree>(ctx, maxLength);
  this->strategy = make_shared<RecursivePrefixTreeStrategy>(
      grid, getClass().getSimpleName());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFilterWithVariableScanLevel() throws
// java.io.IOException
void TestRecursivePrefixTreeStrategy::testFilterWithVariableScanLevel() throw(
    IOException)
{
  init(GeohashPrefixTree::getMaxLevelsPossible());
  getAddAndVerifyIndexedDocuments(DATA_WORLD_CITIES_POINTS);

  // execute queries for each prefix grid scan level
  for (int i = 0; i <= maxLength; i++) {
    (std::static_pointer_cast<RecursivePrefixTreeStrategy>(strategy))
        ->setPrefixGridScanLevel(i);
    executeQueries(SpatialMatchConcern::FILTER, {QTEST_Cities_Intersects_BBox});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOneMeterPrecision()
void TestRecursivePrefixTreeStrategy::testOneMeterPrecision()
{
  init(GeohashPrefixTree::getMaxLevelsPossible());
  shared_ptr<GeohashPrefixTree> grid =
      std::static_pointer_cast<GeohashPrefixTree>(
          (std::static_pointer_cast<RecursivePrefixTreeStrategy>(strategy))
              ->getGrid());
  // DWS: I know this to be true.  11 is needed for one meter
  double degrees =
      DistanceUtils::dist2Degrees(0.001, DistanceUtils::EARTH_MEAN_RADIUS_KM);
  assertEquals(11, grid->getLevelForDistance(degrees));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPrecision() throws java.io.IOException
void TestRecursivePrefixTreeStrategy::testPrecision() 
{
  init(GeohashPrefixTree::getMaxLevelsPossible());

  shared_ptr<Point> iPt =
      ctx->makePoint(2.8028712999999925, 48.3708044); // lon, lat
  addDocument(newDoc(L"iPt", iPt));
  commit();

  shared_ptr<Point> qPt = ctx->makePoint(2.4632387000000335, 48.6003516);

  constexpr double KM2DEG =
      DistanceUtils::dist2Degrees(1, DistanceUtils::EARTH_MEAN_RADIUS_KM);
  constexpr double DEG2KM = 1 / KM2DEG;

  constexpr double DIST = 35.75; // 35.7499...
  assertEquals(DIST, ctx->getDistCalc().distance(iPt, qPt) * DEG2KM, 0.001);

  // distErrPct will affect the query shape precision. The indexed precision
  // was set to nearly zilch via init(GeohashPrefixTree.getMaxLevelsPossible());
  constexpr double distErrPct = 0.025; // the suggested default, by the way
  constexpr double distMult = 1 + distErrPct;

  assertTrue(35.74 * distMult >= DIST);
  checkHits(q(qPt, 35.74 * KM2DEG, distErrPct), 1, nullptr);

  assertTrue(30 * distMult < DIST);
  checkHits(q(qPt, 30 * KM2DEG, distErrPct), 0, nullptr);

  assertTrue(33 * distMult < DIST);
  checkHits(q(qPt, 33 * KM2DEG, distErrPct), 0, nullptr);

  assertTrue(34 * distMult < DIST);
  checkHits(q(qPt, 34 * KM2DEG, distErrPct), 0, nullptr);
}

shared_ptr<SpatialArgs> TestRecursivePrefixTreeStrategy::q(shared_ptr<Point> pt,
                                                           double distDEG,
                                                           double distErrPct)
{
  shared_ptr<Shape> shape = ctx->makeCircle(pt, distDEG);
  shared_ptr<SpatialArgs> args =
      make_shared<SpatialArgs>(SpatialOperation::Intersects, shape);
  args->setDistErrPct(distErrPct);
  return args;
}

void TestRecursivePrefixTreeStrategy::checkHits(shared_ptr<SpatialArgs> args,
                                                int assertNumFound,
                                                std::deque<int> &assertIds)
{
  shared_ptr<SearchResults> got = executeQuery(strategy->makeQuery(args), 100);
  assertEquals(L"" + args, assertNumFound, got->numFound);
  if (assertIds.size() > 0) {
    shared_ptr<Set<int>> gotIds = unordered_set<int>();
    for (auto result : got->results) {
      gotIds->add(static_cast<Integer>(result->document[L"id"]));
    }
    for (auto assertId : assertIds) {
      assertTrue(L"has " + to_wstring(assertId), gotIds->contains(assertId));
    }
  }
}
} // namespace org::apache::lucene::spatial::prefix