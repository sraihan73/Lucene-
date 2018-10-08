using namespace std;

#include "DistanceStrategyTest.h"

namespace org::apache::lucene::spatial
{
using com::carrotsearch::randomizedtesting::annotations::ParametersFactory;
using BBoxStrategy = org::apache::lucene::spatial::bbox::BBoxStrategy;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using TermQueryPrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::TermQueryPrefixTreeStrategy;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using PackedQuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::PackedQuadPrefixTree;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SerializedDVStrategy =
    org::apache::lucene::spatial::serialized::SerializedDVStrategy;
using PointVectorStrategy =
    org::apache::lucene::spatial::deque::PointVectorStrategy;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @ParametersFactory(argumentFormatting = "strategy=%s") public
// static Iterable<Object[]> parameters()
deque<std::deque<any>> DistanceStrategyTest::parameters()
{
  deque<std::deque<any>> ctorArgs = deque<std::deque<any>>();

  shared_ptr<SpatialContext> ctx = SpatialContext::GEO;
  shared_ptr<SpatialPrefixTree> grid;
  shared_ptr<SpatialStrategy> strategy;

  grid = make_shared<QuadPrefixTree>(ctx, 25);
  strategy = make_shared<RecursivePrefixTreeStrategy>(grid, L"recursive_quad");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  grid = make_shared<GeohashPrefixTree>(ctx, 12);
  strategy =
      make_shared<TermQueryPrefixTreeStrategy>(grid, L"termquery_geohash");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  grid = make_shared<PackedQuadPrefixTree>(ctx, 25);
  strategy =
      make_shared<RecursivePrefixTreeStrategy>(grid, L"recursive_packedquad");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  strategy = PointVectorStrategy::newInstance(ctx, L"pointvector");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  //  Can't test this without un-inverting since PVS legacy config didn't have
  //  docValues.
  //    However, note that Solr's tests use UninvertingReader and thus test
  //    this. strategy = PointVectorStrategy.newLegacyInstance(ctx,
  //    "pointvector_legacy"); ctorArgs.add(new
  //    Object[]{strategy.getFieldName(), strategy});

  strategy = BBoxStrategy::newInstance(ctx, L"bbox");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  strategy = make_shared<SerializedDVStrategy>(ctx, L"serialized");
  ctorArgs.push_back(std::deque<any>{strategy->getFieldName(), strategy});

  return ctorArgs;
}

DistanceStrategyTest::DistanceStrategyTest(const wstring &suiteName,
                                           shared_ptr<SpatialStrategy> strategy)
{
  this->ctx = strategy->getSpatialContext();
  this->strategy = strategy;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDistanceOrder() throws
// java.io.IOException
void DistanceStrategyTest::testDistanceOrder() 
{
  adoc(L"100", ctx->makePoint(2, 1));
  adoc(L"101", ctx->makePoint(-1, 4));
  adoc(L"103",
       std::static_pointer_cast<Shape>(nullptr)); // test score for nothing
  commit();
  // FYI distances are in docid order
  checkDistValueSource(ctx->makePoint(4, 3), {2.8274937f, 5.0898066f, 180.0f});
  checkDistValueSource(ctx->makePoint(0, 4), {3.6043684f, 0.9975641f, 180.0f});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRecipScore() throws java.io.IOException
void DistanceStrategyTest::testRecipScore() 
{
  shared_ptr<Point> p100 = ctx->makePoint(2.02, 0.98);
  adoc(L"100", p100);
  shared_ptr<Point> p101 = ctx->makePoint(-1.001, 4.001);
  adoc(L"101", p101);
  adoc(L"103",
       std::static_pointer_cast<Shape>(nullptr)); // test score for nothing
  commit();

  double dist = ctx->getDistCalc().distance(p100, p101);
  shared_ptr<Shape> queryShape = ctx->makeCircle(2.01, 0.99, dist);
  checkValueSource(strategy->makeRecipDistanceValueSource(queryShape),
                   std::deque<float>{1.00f, 0.10f, 0.0f}, 0.09f);
}

void DistanceStrategyTest::checkDistValueSource(
    shared_ptr<Point> pt, deque<float> &distances) 
{
  float multiplier = random()->nextFloat() * 100.0f;
  std::deque<float> dists2 = Arrays::copyOf(distances, distances->length);
  for (int i = 0; i < dists2.size(); i++) {
    dists2[i] *= multiplier;
  }
  checkValueSource(strategy->makeDistanceValueSource(pt, multiplier), dists2,
                   1.0e-3f);
}
} // namespace org::apache::lucene::spatial