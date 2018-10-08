using namespace std;

#include "ShapeRectRelationTestCase.h"

namespace org::apache::lucene::spatial::spatial4j
{
using org::junit::Rule;
using org::junit::Test;
using org::locationtech::spatial4j::TestLog;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::RectIntersectionTestHelper;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::ShapeFactory;
//    import static
//    org.locationtech.spatial4j.distance.DistanceUtils.DEGREES_TO_RADIANS;
const double ShapeRectRelationTestCase::RADIANS_PER_DEGREE = M_PI / 180.0;

ShapeRectRelationTestCase::ShapeRectRelationTestCase()
    : RandomizedShapeTestCase(SpatialContext::GEO)
{
}

ShapeRectRelationTestCase::AbstractRectIntersectionTestHelper::
    AbstractRectIntersectionTestHelper(
        shared_ptr<ShapeRectRelationTestCase> outerInstance,
        shared_ptr<SpatialContext> ctx)
    : org::locationtech::spatial4j::shape::RectIntersectionTestHelper<
          org::locationtech::spatial4j::shape::Shape>(ctx),
      outerInstance(outerInstance)
{
}

int ShapeRectRelationTestCase::AbstractRectIntersectionTestHelper::
    getContainsMinimum(int laps)
{
  return 20;
}

int ShapeRectRelationTestCase::AbstractRectIntersectionTestHelper::
    getIntersectsMinimum(int laps)
{
  return 20;
}

int ShapeRectRelationTestCase::AbstractRectIntersectionTestHelper::
    getWithinMinimum(int laps)
{
  return 2;
}

int ShapeRectRelationTestCase::AbstractRectIntersectionTestHelper::
    getDisjointMinimum(int laps)
{
  return 20;
}

int ShapeRectRelationTestCase::AbstractRectIntersectionTestHelper::
    getBoundingMinimum(int laps)
{
  return 20;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoCircleRect()
void ShapeRectRelationTestCase::testGeoCircleRect()
{
  make_shared<AbstractRectIntersectionTestHelperAnonymousInnerClass>(
      shared_from_this(), ctx)
      .testRelateWithRectangle();
}

ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass::
        AbstractRectIntersectionTestHelperAnonymousInnerClass(
            shared_ptr<ShapeRectRelationTestCase> outerInstance,
            shared_ptr<SpatialContext> ctx)
    : AbstractRectIntersectionTestHelper(outerInstance, ctx)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Shape> ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass::generateRandomShape(
        shared_ptr<Point> nearP)
{
  constexpr int circleRadius =
      outerInstance->maxRadius -
      random()->nextInt(outerInstance->maxRadius); // no 0-radius
  return outerInstance->ctx->getShapeFactory().circle(nearP, circleRadius);
}

shared_ptr<Point> ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass::
        randomPointInEmptyShape(shared_ptr<Shape> shape)
{
  return shape->getCenter();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoBBoxRect()
void ShapeRectRelationTestCase::testGeoBBoxRect()
{
  make_shared<AbstractRectIntersectionTestHelperAnonymousInnerClass2>(
      shared_from_this(), ctx)
      .testRelateWithRectangle();
}

ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass2::
        AbstractRectIntersectionTestHelperAnonymousInnerClass2(
            shared_ptr<ShapeRectRelationTestCase> outerInstance,
            shared_ptr<SpatialContext> ctx)
    : AbstractRectIntersectionTestHelper(outerInstance, ctx)
{
  this->outerInstance = outerInstance;
}

bool ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass2::
        isRandomShapeRectangular()
{
  return true;
}

shared_ptr<Shape> ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass2::generateRandomShape(
        shared_ptr<Point> nearP)
{
  shared_ptr<Point> upperRight = outerInstance->randomPoint();
  shared_ptr<Point> lowerLeft = outerInstance->randomPoint();
  if (upperRight->getY() < lowerLeft->getY()) {
    // swap
    shared_ptr<Point> temp = upperRight;
    upperRight = lowerLeft;
    lowerLeft = temp;
  }
  return outerInstance->ctx->getShapeFactory().rect(lowerLeft, upperRight);
}

shared_ptr<Point> ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass2::
        randomPointInEmptyShape(shared_ptr<Shape> shape)
{
  return shape->getCenter();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoPolygonRect()
void ShapeRectRelationTestCase::testGeoPolygonRect()
{
  make_shared<AbstractRectIntersectionTestHelperAnonymousInnerClass3>(
      shared_from_this(), ctx)
      .testRelateWithRectangle();
}

ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass3::
        AbstractRectIntersectionTestHelperAnonymousInnerClass3(
            shared_ptr<ShapeRectRelationTestCase> outerInstance,
            shared_ptr<SpatialContext> ctx)
    : AbstractRectIntersectionTestHelper(outerInstance, ctx)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Shape> ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass3::generateRandomShape(
        shared_ptr<Point> nearP)
{
  shared_ptr<Point> *const centerPoint = outerInstance->randomPoint();
  constexpr int maxDistance =
      random()->nextInt(outerInstance->maxRadius - 20) + 20;
  shared_ptr<Circle> *const pointZone =
      outerInstance->ctx->getShapeFactory().circle(centerPoint, maxDistance);
  constexpr int vertexCount = random()->nextInt(3) + 3;
  while (true) {
    shared_ptr<ShapeFactory::PolygonBuilder> builder =
        outerInstance->ctx->getShapeFactory().polygon();
    for (int i = 0; i < vertexCount; i++) {
      shared_ptr<Point> *const point = outerInstance->randomPointIn(pointZone);
      builder->pointXY(point->getX(), point->getY());
    }
    try {
      return builder->build();
    } catch (const invalid_argument &e) {
      // This is what happens when we create a shape that is invalid.  Although
      // it is conceivable that there are cases where the exception is thrown
      // incorrectly, we aren't going to be able to do that in this random test.
      continue;
    }
  }
}

shared_ptr<Point> ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass3::
        randomPointInEmptyShape(shared_ptr<Shape> shape)
{
  throw make_shared<IllegalStateException>(
      L"unexpected; need to finish test code");
}

int ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass3::getWithinMinimum(
        int laps)
{
  // Long/thin so lets just find 1.
  return 1;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testGeoPathRect()
void ShapeRectRelationTestCase::testGeoPathRect()
{
  make_shared<AbstractRectIntersectionTestHelperAnonymousInnerClass4>(
      shared_from_this(), ctx)
      .testRelateWithRectangle();
}

ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass4::
        AbstractRectIntersectionTestHelperAnonymousInnerClass4(
            shared_ptr<ShapeRectRelationTestCase> outerInstance,
            shared_ptr<SpatialContext> ctx)
    : AbstractRectIntersectionTestHelper(outerInstance, ctx)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Shape> ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass4::generateRandomShape(
        shared_ptr<Point> nearP)
{
  shared_ptr<Point> *const centerPoint = outerInstance->randomPoint();
  constexpr int maxDistance =
      random()->nextInt(outerInstance->maxRadius - 20) + 20;
  shared_ptr<Circle> *const pointZone =
      outerInstance->ctx->getShapeFactory().circle(centerPoint, maxDistance);
  constexpr int pointCount = random()->nextInt(5) + 1;
  constexpr double width = (random()->nextInt(89) + 1) * DEGREES_TO_RADIANS;
  shared_ptr<ShapeFactory::LineStringBuilder> *const builder =
      outerInstance->ctx->getShapeFactory().lineString();
  while (true) {
    for (int i = 0; i < pointCount; i++) {
      shared_ptr<Point> *const nextPoint =
          outerInstance->randomPointIn(pointZone);
      builder->pointXY(nextPoint->getX(), nextPoint->getY());
    }
    builder->buffer(width);
    try {
      return builder->build();
    } catch (const invalid_argument &e) {
      // This is what happens when we create a shape that is invalid.  Although
      // it is conceivable that there are cases where the exception is thrown
      // incorrectly, we aren't going to be able to do that in this random test.
      continue;
    }
  }
}

shared_ptr<Point> ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass4::
        randomPointInEmptyShape(shared_ptr<Shape> shape)
{
  throw make_shared<IllegalStateException>(
      L"unexpected; need to finish test code");
}

int ShapeRectRelationTestCase::
    AbstractRectIntersectionTestHelperAnonymousInnerClass4::getWithinMinimum(
        int laps)
{
  // Long/thin so lets just find 1.
  return 1;
}
} // namespace org::apache::lucene::spatial::spatial4j