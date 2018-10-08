using namespace std;

#include "RandomizedShapeTestCase.h"

namespace org::apache::lucene::spatial::spatial4j
{
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using org::locationtech::spatial4j::shape::impl::Range;
//    import static org.locationtech.spatial4j.shape.SpatialRelation.CONTAINS;
//    import static org.locationtech.spatial4j.shape.SpatialRelation.WITHIN;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.*;

RandomizedShapeTestCase::RandomizedShapeTestCase() {}

RandomizedShapeTestCase::RandomizedShapeTestCase(shared_ptr<SpatialContext> ctx)
{
  this->ctx = ctx;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public static void
// checkShapesImplementEquals(Class[] classes)
void RandomizedShapeTestCase::checkShapesImplementEquals(
    std::deque<type_info> &classes)
{
  for (auto clazz : classes) {
    try {
      clazz.getDeclaredMethod(L"equals", any::typeid);
    } catch (const runtime_error &e) {
      fail(L"Shape needs to define 'equals' : " + clazz.getName());
    }
    try {
      clazz.getDeclaredMethod(L"hashCode");
    } catch (const runtime_error &e) {
      fail(L"Shape needs to define 'hashCode' : " + clazz.getName());
    }
  }
}

double RandomizedShapeTestCase::normX(double x)
{
  return ctx->isGeo() ? DistanceUtils::normLonDEG(x) : x;
}

double RandomizedShapeTestCase::normY(double y)
{
  return ctx->isGeo() ? DistanceUtils::normLatDEG(y) : y;
}

shared_ptr<Rectangle> RandomizedShapeTestCase::makeNormRect(double minX,
                                                            double maxX,
                                                            double minY,
                                                            double maxY)
{
  if (ctx->isGeo()) {
    if (abs(maxX - minX) >= 360) {
      minX = -180;
      maxX = 180;
    } else {
      minX = DistanceUtils::normLonDEG(minX);
      maxX = DistanceUtils::normLonDEG(maxX);
    }

  } else {
    if (maxX < minX) {
      double t = minX;
      minX = maxX;
      maxX = t;
    }
    minX = boundX(minX, ctx->getWorldBounds());
    maxX = boundX(maxX, ctx->getWorldBounds());
  }
  if (maxY < minY) {
    double t = minY;
    minY = maxY;
    maxY = t;
  }
  minY = boundY(minY, ctx->getWorldBounds());
  maxY = boundY(maxY, ctx->getWorldBounds());
  return ctx->makeRectangle(minX, maxX, minY, maxY);
}

double RandomizedShapeTestCase::divisible(double v, double divisible)
{
  return static_cast<int>(round(v / divisible) * divisible);
}

double RandomizedShapeTestCase::divisible(double v)
{
  return divisible(v, DIVISIBLE);
}

shared_ptr<Point> RandomizedShapeTestCase::divisible(shared_ptr<Point> p)
{
  shared_ptr<Rectangle> bounds = ctx->getWorldBounds();
  double newX = boundX(divisible(p->getX()), bounds);
  double newY = boundY(divisible(p->getY()), bounds);
  p->reset(newX, newY);
  return p;
}

double RandomizedShapeTestCase::boundX(double i, shared_ptr<Rectangle> bounds)
{
  return bound(i, bounds->getMinX(), bounds->getMaxX());
}

double RandomizedShapeTestCase::boundY(double i, shared_ptr<Rectangle> bounds)
{
  return bound(i, bounds->getMinY(), bounds->getMaxY());
}

double RandomizedShapeTestCase::bound(double i, double min, double max)
{
  if (i < min) {
    return min;
  }
  if (i > max) {
    return max;
  }
  return i;
}

void RandomizedShapeTestCase::assertRelation(
    shared_ptr<SpatialRelation> expected, shared_ptr<Shape> a,
    shared_ptr<Shape> b)
{
  assertRelation(L"", expected, a, b);
}

void RandomizedShapeTestCase::assertRelation(
    const wstring &msg, shared_ptr<SpatialRelation> expected,
    shared_ptr<Shape> a, shared_ptr<Shape> b)
{
  _assertIntersect(msg, expected, a, b);
  // check flipped a & b w/ transpose(), while we're at it
  _assertIntersect(msg, expected->transpose(), b, a);
}

void RandomizedShapeTestCase::_assertIntersect(
    const wstring &msg, shared_ptr<SpatialRelation> expected,
    shared_ptr<Shape> a, shared_ptr<Shape> b)
{
  shared_ptr<SpatialRelation> sect = a->relate(b);
  if (sect == expected) {
    return;
  }
  msg = ((msg == L"") ? L"" : msg + L"\r") + a + L" intersect " + b;
  if (expected == WITHIN || expected == CONTAINS) {
    if (a->getClass().equals(b->getClass())) // they are the same shape type
    {
      assertEquals(msg, a, b);
    } else {
      // they are effectively points or lines that are the same location
      assertTrue(msg, !a->hasArea());
      assertTrue(msg, !b->hasArea());

      shared_ptr<Rectangle> aBBox = a->getBoundingBox();
      shared_ptr<Rectangle> bBBox = b->getBoundingBox();
      if (aBBox->getHeight() == 0 && bBBox->getHeight() == 0 &&
          (aBBox->getMaxY() == 90 && bBBox->getMaxY() == 90 ||
           aBBox->getMinY() == -90 && bBBox->getMinY() == -90)) {
        ; //== a point at the pole
      } else {
        assertEquals(msg, aBBox, bBBox);
      }
    }
  } else {
    assertEquals(msg, expected, sect); // always fails
  }
}

void RandomizedShapeTestCase::assertEqualsRatio(const wstring &msg,
                                                double expected, double actual)
{
  double delta = abs(actual - expected);
  double base = min(actual, expected);
  double deltaRatio = base == 0 ? delta : min(delta, delta / base);
  assertEquals(msg, 0, deltaRatio, EPS);
}

int RandomizedShapeTestCase::randomIntBetweenDivisible(int start, int end)
{
  return randomIntBetweenDivisible(start, end, static_cast<int>(DIVISIBLE));
}

int RandomizedShapeTestCase::randomIntBetweenDivisible(int start, int end,
                                                       int divisible)
{
  // DWS: I tested this
  int divisStart =
      static_cast<int>(ceil((start + 1) / static_cast<double>(divisible)));
  int divisEnd =
      static_cast<int>(floor((end - 1) / static_cast<double>(divisible)));
  int divisRange = max(0, divisEnd - divisStart + 1);
  int r = randomInt(1 + divisRange); // remember that '0' is counted
  if (r == 0) {
    return start;
  }
  if (r == 1) {
    return end;
  }
  return (r - 2 + divisStart) * divisible;
}

shared_ptr<Rectangle>
RandomizedShapeTestCase::randomRectangle(shared_ptr<Point> nearP)
{
  shared_ptr<Rectangle> bounds = ctx->getWorldBounds();
  if (nearP == nullptr) {
    nearP = randomPointIn(bounds);
  }

  shared_ptr<Range> xRange =
      randomRange(rarely() ? 0 : nearP->getX(), Range::xRange(bounds, ctx));
  shared_ptr<Range> yRange =
      randomRange(rarely() ? 0 : nearP->getY(), Range::yRange(bounds, ctx));

  return makeNormRect(divisible(xRange->getMin()), divisible(xRange->getMax()),
                      divisible(yRange->getMin()), divisible(yRange->getMax()));
}

shared_ptr<Range> RandomizedShapeTestCase::randomRange(double near,
                                                       shared_ptr<Range> bounds)
{
  double mid = near + randomGaussian() * bounds->getWidth() / 6;
  double width = abs(randomGaussian()) * bounds->getWidth() / 6; // 1/3rd
  return make_shared<Range>(mid - width / 2, mid + width / 2);
}

double RandomizedShapeTestCase::randomGaussianZeroTo(double max)
{
  if (max == 0) {
    return max;
  }
  assert(max > 0);
  double r;
  do {
    r = abs(randomGaussian()) * (max * 0.50);
  } while (r > max);
  return r;
}

shared_ptr<Rectangle> RandomizedShapeTestCase::randomRectangle(int divisible)
{
  double rX = randomIntBetweenDivisible(-180, 180, divisible);
  double rW = randomIntBetweenDivisible(0, 360, divisible);
  double rY1 = randomIntBetweenDivisible(-90, 90, divisible);
  double rY2 = randomIntBetweenDivisible(-90, 90, divisible);
  double rYmin = min(rY1, rY2);
  double rYmax = max(rY1, rY2);
  if (rW > 0 && rX == 180) {
    rX = -180;
  }
  return makeNormRect(rX, rX + rW, rYmin, rYmax);
}

shared_ptr<Point> RandomizedShapeTestCase::randomPoint()
{
  return randomPointIn(ctx->getWorldBounds());
}

shared_ptr<Point> RandomizedShapeTestCase::randomPointIn(shared_ptr<Circle> c)
{
  double d = c->getRadius() * randomDouble();
  double angleDEG = 360 * randomDouble();
  shared_ptr<Point> p = ctx->getDistCalc().pointOnBearing(
      c->getCenter(), d, angleDEG, ctx, nullptr);
  assertEquals(CONTAINS, c->relate(p));
  return p;
}

shared_ptr<Point>
RandomizedShapeTestCase::randomPointIn(shared_ptr<Rectangle> r)
{
  double x = r->getMinX() + randomDouble() * r->getWidth();
  double y = r->getMinY() + randomDouble() * r->getHeight();
  x = normX(x);
  y = normY(y);
  shared_ptr<Point> p = ctx->makePoint(x, y);
  assertEquals(CONTAINS, r->relate(p));
  return p;
}

shared_ptr<Point>
RandomizedShapeTestCase::randomPointInOrNull(shared_ptr<Shape> shape)
{
  if (!shape->hasArea()) // or try the center?
  {
    throw make_shared<UnsupportedOperationException>(
        L"Need area to define shape!");
  }
  shared_ptr<Rectangle> bbox = shape->getBoundingBox();
  for (int i = 0; i < 1000; i++) {
    shared_ptr<Point> p = randomPointIn(bbox);
    if (shape->relate(p).intersects()) {
      return p;
    }
  }
  return nullptr; // tried too many times and failed
}
} // namespace org::apache::lucene::spatial::spatial4j