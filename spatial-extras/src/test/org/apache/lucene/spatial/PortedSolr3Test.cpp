using namespace std;

#include "PortedSolr3Test.h"

namespace org::apache::lucene::spatial
{
using com::carrotsearch::randomizedtesting::annotations::ParametersFactory;
using Query = org::apache::lucene::search::Query;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using TermQueryPrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::TermQueryPrefixTreeStrategy;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using PointVectorStrategy =
    org::apache::lucene::spatial::deque::PointVectorStrategy;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @ParametersFactory(argumentFormatting = "strategy=%s") public
// static Iterable<Object[]> parameters()
deque<std::deque<any>> PortedSolr3Test::parameters()
{
  deque<std::deque<any>> ctorArgs = deque<std::deque<any>>();

  shared_ptr<SpatialContext> ctx = SpatialContext::GEO;
  shared_ptr<SpatialPrefixTree> grid;
  shared_ptr<SpatialStrategy> strategy;

  grid = make_shared<GeohashPrefixTree>(ctx, 12);
  strategy =
      make_shared<RecursivePrefixTreeStrategy>(grid, L"recursive_geohash");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  grid = make_shared<QuadPrefixTree>(ctx, 25);
  strategy = make_shared<RecursivePrefixTreeStrategy>(grid, L"recursive_quad");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  grid = make_shared<GeohashPrefixTree>(ctx, 12);
  strategy =
      make_shared<TermQueryPrefixTreeStrategy>(grid, L"termquery_geohash");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  strategy = PointVectorStrategy::newInstance(ctx, L"pointvector");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  strategy = PointVectorStrategy::newInstance(ctx, L"pointvector_legacy");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  return ctorArgs;
}

PortedSolr3Test::PortedSolr3Test(const wstring &suiteName,
                                 shared_ptr<SpatialStrategy> strategy)
{
  this->ctx = strategy->getSpatialContext();
  this->strategy = strategy;
}

void PortedSolr3Test::setupDocs() 
{
  StrategyTestCase::deleteAll();
  adoc(L"1", ctx->makePoint(-79.9289094, 32.7693246));
  adoc(L"2", ctx->makePoint(-80.9289094, 33.7693246));
  adoc(L"3", ctx->makePoint(50.9289094, -32.7693246));
  adoc(L"4", ctx->makePoint(60.9289094, -50.7693246));
  adoc(L"5", ctx->makePoint(0, 0));
  adoc(L"6", ctx->makePoint(0.1, 0.1));
  adoc(L"7", ctx->makePoint(-0.1, -0.1));
  adoc(L"8", ctx->makePoint(179.9, 0));
  adoc(L"9", ctx->makePoint(-179.9, 0));
  adoc(L"10", ctx->makePoint(50, 89.9));
  adoc(L"11", ctx->makePoint(-130, 89.9));
  adoc(L"12", ctx->makePoint(50, -89.9));
  adoc(L"13", ctx->makePoint(-130, -89.9));
  commit();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testIntersections() throws Exception
void PortedSolr3Test::testIntersections() 
{
  setupDocs();
  // Try some edge cases
  // NOTE: 2nd arg is distance in kilometers
  checkHitsCircle(ctx->makePoint(1, 1), 175, 3, {5, 6, 7});
  checkHitsCircle(ctx->makePoint(179.8, 0), 200, 2, {8, 9});
  checkHitsCircle(ctx->makePoint(50, 89.8), 200, 2,
                  {10, 11}); // this goes over the north pole
  checkHitsCircle(ctx->makePoint(50, -89.8), 200, 2,
                  {12, 13}); // this goes over the south pole
  // try some normal cases
  checkHitsCircle(ctx->makePoint(-80.0, 33.0), 300, 2);
  // large distance
  checkHitsCircle(ctx->makePoint(1, 1), 5000, 3, {5, 6, 7});
  // Because we are generating a box based on the west/east longitudes and the
  // south/north latitudes, which then translates to a range query, which is
  // slightly more inclusive.  Thus, even though 0.0 is 15.725 kms away, it will
  // be included, b/c of the box calculation.
  checkHitsBBox(ctx->makePoint(0.1, 0.1), 15, 2, {5, 6});
  // try some more
  deleteAll();
  adoc(L"14", ctx->makePoint(5, 0));
  adoc(L"15", ctx->makePoint(15, 0));
  // 3000KM from 0,0, see http://www.movable-type.co.uk/scripts/latlong.html
  adoc(L"16", ctx->makePoint(19.79750, 18.71111));
  adoc(L"17", ctx->makePoint(-95.436643, 44.043900));
  commit();

  checkHitsCircle(ctx->makePoint(0, 0), 1000, 1, {14});
  checkHitsCircle(ctx->makePoint(0, 0), 2000, 2, {14, 15});
  checkHitsBBox(ctx->makePoint(0, 0), 3000, 3, {14, 15, 16});
  checkHitsCircle(ctx->makePoint(0, 0), 3001, 3, {14, 15, 16});
  checkHitsCircle(ctx->makePoint(0, 0), 3000.1, 3, {14, 15, 16});

  // really fine grained distance and reflects some of the vagaries of how we
  // are calculating the box
  checkHitsCircle(ctx->makePoint(-96.789603, 43.517030), 109, 0);

  // falls outside of the real distance, but inside the bounding box
  checkHitsCircle(ctx->makePoint(-96.789603, 43.517030), 110, 0);
  checkHitsBBox(ctx->makePoint(-96.789603, 43.517030), 110, 1, {17});
}

void PortedSolr3Test::checkHitsCircle(shared_ptr<Point> pt, double distKM,
                                      int assertNumFound,
                                      deque<int> &assertIds)
{
  _checkHits(false, pt, distKM, assertNumFound, {assertIds});
}

void PortedSolr3Test::checkHitsBBox(shared_ptr<Point> pt, double distKM,
                                    int assertNumFound, deque<int> &assertIds)
{
  _checkHits(true, pt, distKM, assertNumFound, {assertIds});
}

void PortedSolr3Test::_checkHits(bool bbox, shared_ptr<Point> pt, double distKM,
                                 int assertNumFound, deque<int> &assertIds)
{
  shared_ptr<SpatialOperation> op = SpatialOperation::Intersects;
  double distDEG =
      DistanceUtils::dist2Degrees(distKM, DistanceUtils::EARTH_MEAN_RADIUS_KM);
  shared_ptr<Shape> shape = ctx->makeCircle(pt, distDEG);
  if (bbox) {
    shape = shape->getBoundingBox();
  }

  shared_ptr<SpatialArgs> args = make_shared<SpatialArgs>(op, shape);
  // args.setDistPrecision(0.025);
  shared_ptr<Query> query = strategy->makeQuery(args);
  shared_ptr<SearchResults> results = executeQuery(query, 100);
  assertEquals(L"" + shape, assertNumFound, results->numFound);
  if (assertIds != nullptr) {
    shared_ptr<Set<int>> resultIds = unordered_set<int>();
    for (auto result : results->results) {
      resultIds->add(static_cast<Integer>(result->document[L"id"]));
    }
    for (int assertId : assertIds) {
      assertTrue(L"has " + to_wstring(assertId), resultIds->contains(assertId));
    }
  }
}
} // namespace org::apache::lucene::spatial