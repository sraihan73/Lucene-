using namespace std;

#include "CompositeStrategyTest.h"

namespace org::apache::lucene::spatial::composite
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using RandomSpatialOpStrategyTestCase =
    org::apache::lucene::spatial::prefix::RandomSpatialOpStrategyTestCase;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using GeohashPrefixTree =
    org::apache::lucene::spatial::prefix::tree::GeohashPrefixTree;
using QuadPrefixTree =
    org::apache::lucene::spatial::prefix::tree::QuadPrefixTree;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using SerializedDVStrategy =
    org::apache::lucene::spatial::serialized::SerializedDVStrategy;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::context::SpatialContextFactory;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::impl::RectangleImpl;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomBoolean; import
//    static com.carrotsearch.randomizedtesting.RandomizedTest.randomDouble;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

void CompositeStrategyTest::setupQuadGrid(int maxLevels)
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
  // A fairly shallow grid
  if (maxLevels == -1) {
    maxLevels = randomIntBetween(1, 8); // max 64k cells (4^8), also 256*256
  }
  this->grid = make_shared<QuadPrefixTree>(ctx, maxLevels);
  this->rptStrategy = newRPT();
}

void CompositeStrategyTest::setupGeohashGrid(int maxLevels)
{
  this->ctx = SpatialContext::GEO;
  // A fairly shallow grid
  if (maxLevels == -1) {
    maxLevels = randomIntBetween(1, 3); // max 16k cells (32^3)
  }
  this->grid = make_shared<GeohashPrefixTree>(ctx, maxLevels);
  this->rptStrategy = newRPT();
}

shared_ptr<RecursivePrefixTreeStrategy> CompositeStrategyTest::newRPT()
{
  shared_ptr<RecursivePrefixTreeStrategy> *const rpt =
      make_shared<RecursivePrefixTreeStrategy>(
          this->grid, getClass().getSimpleName() + L"_rpt");
  rpt->setDistErrPct(0.10); // not too many cells
  return rpt;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @Repeat(iterations = 20) public void testOperations()
// throws java.io.IOException
void CompositeStrategyTest::testOperations() 
{
  // setup
  if (randomBoolean()) {
    setupQuadGrid(-1);
  } else {
    setupGeohashGrid(-1);
  }
  shared_ptr<SerializedDVStrategy> serializedDVStrategy =
      make_shared<SerializedDVStrategy>(ctx,
                                        getClass().getSimpleName() + L"_sdv");
  this->strategy = make_shared<CompositeSpatialStrategy>(
      L"composite_" + getClass().getSimpleName(), rptStrategy,
      serializedDVStrategy);

  // Do it!

  for (auto pred : SpatialOperation::values()) {
    if (pred == SpatialOperation::BBoxIntersects ||
        pred == SpatialOperation::BBoxWithin) {
      continue;
    }
    if (pred == SpatialOperation::IsDisjointTo) { // TODO
      continue;
    }
    testOperationRandomShapes(pred);
    deleteAll();
    commit();
  }
}

shared_ptr<Shape> CompositeStrategyTest::randomIndexedShape()
{
  return randomShape();
}

shared_ptr<Shape> CompositeStrategyTest::randomQueryShape()
{
  return randomShape();
}

shared_ptr<Shape> CompositeStrategyTest::randomShape()
{
  return random()->nextBoolean() ? randomCircle() : randomRectangle();
}

shared_ptr<Shape> CompositeStrategyTest::randomCircle()
{
  shared_ptr<Point> *const point = randomPoint();
  // TODO pick using gaussian
  double radius;
  if (ctx->isGeo()) {
    radius = randomDouble() * 100;
  } else {
    // find distance to closest edge
    shared_ptr<Rectangle> *const worldBounds = ctx->getWorldBounds();
    double maxRad = point->getX() - worldBounds->getMinX();
    maxRad = min(maxRad, worldBounds->getMaxX() - point->getX());
    maxRad = min(maxRad, point->getY() - worldBounds->getMinY());
    maxRad = min(maxRad, worldBounds->getMaxY() - point->getY());
    radius = randomDouble() * maxRad;
  }

  return ctx->makeCircle(point, radius);
}
} // namespace org::apache::lucene::spatial::composite