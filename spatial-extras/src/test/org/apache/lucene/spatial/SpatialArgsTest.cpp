using namespace std;

#include "SpatialArgsTest.h"

namespace org::apache::lucene::spatial
{
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using Geo3dSpatialContextFactory =
    org::apache::lucene::spatial::spatial4j::Geo3dSpatialContextFactory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void calcDistanceFromErrPct()
void SpatialArgsTest::calcDistanceFromErrPct()
{
  shared_ptr<SpatialContext> *const ctx =
      usually()
          ? SpatialContext::GEO
          : (make_shared<Geo3dSpatialContextFactory>())->newSpatialContext();
  constexpr double DEP = 0.5; // distErrPct

  // the result is the diagonal distance from the center to the closest corner,
  // times distErrPct

  shared_ptr<Shape> superwide = ctx->makeRectangle(-180, 180, 0, 0);
  // 0 distErrPct means 0 distance always
  assertEquals(0, SpatialArgs::calcDistanceFromErrPct(superwide, 0, ctx), 0);
  assertEquals(180 * DEP,
               SpatialArgs::calcDistanceFromErrPct(superwide, DEP, ctx), 0);

  shared_ptr<Shape> supertall = ctx->makeRectangle(0, 0, -90, 90);
  assertEquals(90 * DEP,
               SpatialArgs::calcDistanceFromErrPct(supertall, DEP, ctx), 0);

  shared_ptr<Shape> upperhalf = ctx->makeRectangle(-180, 180, 0, 90);
  assertEquals(45 * DEP,
               SpatialArgs::calcDistanceFromErrPct(upperhalf, DEP, ctx),
               0.0001);

  shared_ptr<Shape> midCircle = ctx->makeCircle(0, 0, 45);
  assertEquals(60 * DEP,
               SpatialArgs::calcDistanceFromErrPct(midCircle, DEP, ctx),
               0.0001);
}
} // namespace org::apache::lucene::spatial