#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::spatial::spatial4j
{

using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Circle;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using org::locationtech::spatial4j::shape::impl::Range;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.locationtech.spatial4j.shape.SpatialRelation.CONTAINS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.locationtech.spatial4j.shape.SpatialRelation.WITHIN;

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.*;

/**
 * A base test class with utility methods to help test shapes.
 * Extends from RandomizedTest.
 */
class RandomizedShapeTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(RandomizedShapeTestCase)

protected:
  static constexpr double EPS = 10e-9;

  std::shared_ptr<SpatialContext> ctx; // needs to be set ASAP

  /** Used to reduce the space of numbers to increase the likelihood that
   * random numbers become equivalent, and thus trigger different code paths.
   * Also makes some random shapes easier to manually examine.
   */
  const double DIVISIBLE = 2; // even coordinates; (not always used)

  RandomizedShapeTestCase();

public:
  RandomizedShapeTestCase(std::shared_ptr<SpatialContext> ctx);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public static void
  // checkShapesImplementEquals(Class[] classes)
  static void checkShapesImplementEquals(std::deque<std::type_info> &classes);

  // These few norm methods normalize the arguments for creating a shape to
  // account for the dateline. Some tests loop past the dateline or have offsets
  // that go past it and it's easier to have them coded that way and correct for
  // it here.  These norm methods should be used when needed, not frivolously.

protected:
  virtual double normX(double x);

  virtual double normY(double y);

  virtual std::shared_ptr<Rectangle> makeNormRect(double minX, double maxX,
                                                  double minY, double maxY);

public:
  static double divisible(double v, double divisible);

protected:
  virtual double divisible(double v);

  /** reset()'s p, and confines to world bounds. Might not be divisible if
   * the world bound isn't divisible too.
   */
  virtual std::shared_ptr<Point> divisible(std::shared_ptr<Point> p);

public:
  static double boundX(double i, std::shared_ptr<Rectangle> bounds);

  static double boundY(double i, std::shared_ptr<Rectangle> bounds);

  static double bound(double i, double min, double max);

protected:
  virtual void assertRelation(std::shared_ptr<SpatialRelation> expected,
                              std::shared_ptr<Shape> a,
                              std::shared_ptr<Shape> b);

  virtual void assertRelation(const std::wstring &msg,
                              std::shared_ptr<SpatialRelation> expected,
                              std::shared_ptr<Shape> a,
                              std::shared_ptr<Shape> b);

private:
  void _assertIntersect(const std::wstring &msg,
                        std::shared_ptr<SpatialRelation> expected,
                        std::shared_ptr<Shape> a, std::shared_ptr<Shape> b);

protected:
  virtual void assertEqualsRatio(const std::wstring &msg, double expected,
                                 double actual);

  virtual int randomIntBetweenDivisible(int start, int end);
  /** Returns a random integer between [start, end]. Integers between must be
   * divisible by the 3rd argument. */
  virtual int randomIntBetweenDivisible(int start, int end, int divisible);

  virtual std::shared_ptr<Rectangle>
  randomRectangle(std::shared_ptr<Point> nearP);

private:
  std::shared_ptr<Range> randomRange(double near,
                                     std::shared_ptr<Range> bounds);

  double randomGaussianZeroTo(double max);

protected:
  virtual std::shared_ptr<Rectangle> randomRectangle(int divisible);

  virtual std::shared_ptr<Point> randomPoint();

  virtual std::shared_ptr<Point> randomPointIn(std::shared_ptr<Circle> c);

  virtual std::shared_ptr<Point> randomPointIn(std::shared_ptr<Rectangle> r);

  virtual std::shared_ptr<Point>
  randomPointInOrNull(std::shared_ptr<Shape> shape);

protected:
  std::shared_ptr<RandomizedShapeTestCase> shared_from_this()
  {
    return std::static_pointer_cast<RandomizedShapeTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::spatial4j
