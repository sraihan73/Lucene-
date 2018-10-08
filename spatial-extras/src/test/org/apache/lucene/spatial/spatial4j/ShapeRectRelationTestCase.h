#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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

using org::locationtech::spatial4j::TestLog;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::RectIntersectionTestHelper;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.locationtech.spatial4j.distance.DistanceUtils.DEGREES_TO_RADIANS;

class ShapeRectRelationTestCase : public RandomizedShapeTestCase
{
  GET_CLASS_NAME(ShapeRectRelationTestCase)
protected:
  static const double RADIANS_PER_DEGREE;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Rule public final org.locationtech.spatial4j.TestLog
  // testLog = org.locationtech.spatial4j.TestLog.instance;
  const std::shared_ptr<TestLog> testLog = TestLog::instance;

protected:
  int maxRadius = 180;

public:
  ShapeRectRelationTestCase();

public:
  class AbstractRectIntersectionTestHelper
      : public RectIntersectionTestHelper<std::shared_ptr<Shape>>
  {
    GET_CLASS_NAME(AbstractRectIntersectionTestHelper)
  private:
    std::shared_ptr<ShapeRectRelationTestCase> outerInstance;

  public:
    AbstractRectIntersectionTestHelper(
        std::shared_ptr<ShapeRectRelationTestCase> outerInstance,
        std::shared_ptr<SpatialContext> ctx);

    // 20 times each -- should be plenty

  protected:
    virtual int getContainsMinimum(int laps);

    virtual int getIntersectsMinimum(int laps);

    // producing "within" cases in Geo3D based on our random shapes doesn't
    // happen often. It'd be nice to increase this.
    virtual int getWithinMinimum(int laps);

    virtual int getDisjointMinimum(int laps);

    virtual int getBoundingMinimum(int laps);

  protected:
    std::shared_ptr<AbstractRectIntersectionTestHelper> shared_from_this()
    {
      return std::static_pointer_cast<AbstractRectIntersectionTestHelper>(
          org.locationtech.spatial4j.shape.RectIntersectionTestHelper<
              org.locationtech.spatial4j.shape.Shape>::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoCircleRect()
  virtual void testGeoCircleRect();

private:
  class AbstractRectIntersectionTestHelperAnonymousInnerClass
      : public AbstractRectIntersectionTestHelper
  {
    GET_CLASS_NAME(AbstractRectIntersectionTestHelperAnonymousInnerClass)
  private:
    std::shared_ptr<ShapeRectRelationTestCase> outerInstance;

  public:
    AbstractRectIntersectionTestHelperAnonymousInnerClass(
        std::shared_ptr<ShapeRectRelationTestCase> outerInstance,
        std::shared_ptr<SpatialContext> ctx);

  protected:
    std::shared_ptr<Shape>
    generateRandomShape(std::shared_ptr<Point> nearP) override;

    std::shared_ptr<Point>
    randomPointInEmptyShape(std::shared_ptr<Shape> shape) override;

  protected:
    std::shared_ptr<AbstractRectIntersectionTestHelperAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          AbstractRectIntersectionTestHelperAnonymousInnerClass>(
          AbstractRectIntersectionTestHelper::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoBBoxRect()
  virtual void testGeoBBoxRect();

private:
  class AbstractRectIntersectionTestHelperAnonymousInnerClass2
      : public AbstractRectIntersectionTestHelper
  {
    GET_CLASS_NAME(AbstractRectIntersectionTestHelperAnonymousInnerClass2)
  private:
    std::shared_ptr<ShapeRectRelationTestCase> outerInstance;

  public:
    AbstractRectIntersectionTestHelperAnonymousInnerClass2(
        std::shared_ptr<ShapeRectRelationTestCase> outerInstance,
        std::shared_ptr<SpatialContext> ctx);

  protected:
    bool isRandomShapeRectangular() override;

    std::shared_ptr<Shape>
    generateRandomShape(std::shared_ptr<Point> nearP) override;

    std::shared_ptr<Point>
    randomPointInEmptyShape(std::shared_ptr<Shape> shape) override;

  protected:
    std::shared_ptr<AbstractRectIntersectionTestHelperAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          AbstractRectIntersectionTestHelperAnonymousInnerClass2>(
          AbstractRectIntersectionTestHelper::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoPolygonRect()
  virtual void testGeoPolygonRect();

private:
  class AbstractRectIntersectionTestHelperAnonymousInnerClass3
      : public AbstractRectIntersectionTestHelper
  {
    GET_CLASS_NAME(AbstractRectIntersectionTestHelperAnonymousInnerClass3)
  private:
    std::shared_ptr<ShapeRectRelationTestCase> outerInstance;

  public:
    AbstractRectIntersectionTestHelperAnonymousInnerClass3(
        std::shared_ptr<ShapeRectRelationTestCase> outerInstance,
        std::shared_ptr<SpatialContext> ctx);

  protected:
    std::shared_ptr<Shape>
    generateRandomShape(std::shared_ptr<Point> nearP) override;

    std::shared_ptr<Point>
    randomPointInEmptyShape(std::shared_ptr<Shape> shape) override;

    int getWithinMinimum(int laps) override;

  protected:
    std::shared_ptr<AbstractRectIntersectionTestHelperAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          AbstractRectIntersectionTestHelperAnonymousInnerClass3>(
          AbstractRectIntersectionTestHelper::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGeoPathRect()
  virtual void testGeoPathRect();

private:
  class AbstractRectIntersectionTestHelperAnonymousInnerClass4
      : public AbstractRectIntersectionTestHelper
  {
    GET_CLASS_NAME(AbstractRectIntersectionTestHelperAnonymousInnerClass4)
  private:
    std::shared_ptr<ShapeRectRelationTestCase> outerInstance;

  public:
    AbstractRectIntersectionTestHelperAnonymousInnerClass4(
        std::shared_ptr<ShapeRectRelationTestCase> outerInstance,
        std::shared_ptr<SpatialContext> ctx);

  protected:
    std::shared_ptr<Shape>
    generateRandomShape(std::shared_ptr<Point> nearP) override;

    std::shared_ptr<Point>
    randomPointInEmptyShape(std::shared_ptr<Shape> shape) override;

    int getWithinMinimum(int laps) override;

  protected:
    std::shared_ptr<AbstractRectIntersectionTestHelperAnonymousInnerClass4>
    shared_from_this()
    {
      return std::static_pointer_cast<
          AbstractRectIntersectionTestHelperAnonymousInnerClass4>(
          AbstractRectIntersectionTestHelper::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ShapeRectRelationTestCase> shared_from_this()
  {
    return std::static_pointer_cast<ShapeRectRelationTestCase>(
        RandomizedShapeTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/spatial4j/
