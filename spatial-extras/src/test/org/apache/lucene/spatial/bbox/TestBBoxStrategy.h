#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::bbox
{
class BBoxStrategy;
}

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
namespace org::apache::lucene::spatial::bbox
{

using RandomSpatialOpStrategyTestCase =
    org::apache::lucene::spatial::prefix::RandomSpatialOpStrategyTestCase;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

class TestBBoxStrategy : public RandomSpatialOpStrategyTestCase
{
  GET_CLASS_NAME(TestBBoxStrategy)

protected:
  std::shared_ptr<Shape> randomIndexedShape() override;

  /** next int, inclusive, rounds to multiple of 10 if given evenly divisible.
   */
private:
  int nextIntInclusive(int toInc);

protected:
  std::shared_ptr<Shape> randomQueryShape() override;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 15) public void testOperations()
  // throws java.io.IOException
  virtual void testOperations() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testIntersectsBugDatelineEdge() throws
  // java.io.IOException
  virtual void testIntersectsBugDatelineEdge() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testIntersectsWorldDatelineEdge() throws
  // java.io.IOException
  virtual void testIntersectsWorldDatelineEdge() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithinBugDatelineEdge() throws
  // java.io.IOException
  virtual void testWithinBugDatelineEdge() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testContainsBugDatelineEdge() throws
  // java.io.IOException
  virtual void testContainsBugDatelineEdge() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWorldContainsXDL() throws
  // java.io.IOException
  virtual void testWorldContainsXDL() ;

  /** See https://github.com/spatial4j/spatial4j/issues/85 */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAlongDatelineOppositeSign() throws
  // java.io.IOException
  virtual void testAlongDatelineOppositeSign() ;

private:
  void setupGeo();

  // OLD STATIC TESTS (worthless?)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Ignore("Overlaps not supported") public void
  // testBasicOperaions() throws java.io.IOException
  virtual void testBasicOperaions() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testStatesBBox() throws
  // java.io.IOException
  virtual void testStatesBBox() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCitiesIntersectsBBox() throws
  // java.io.IOException
  virtual void testCitiesIntersectsBBox() ;

  /* Convert DATA_WORLD_CITIES_POINTS to bbox */
protected:
  std::shared_ptr<Shape>
  convertShapeFromGetDocuments(std::shared_ptr<Shape> shape) override;

private:
  std::shared_ptr<BBoxStrategy> setupNeedsDocValuesOnly() ;

public:
  virtual void testOverlapRatio() ;

private:
  std::shared_ptr<Rectangle> shiftedRect(double minX, double maxX, double minY,
                                         double maxY, int xShift);

public:
  virtual void testAreaValueSource() ;

protected:
  std::shared_ptr<TestBBoxStrategy> shared_from_this()
  {
    return std::static_pointer_cast<TestBBoxStrategy>(
        org.apache.lucene.spatial.prefix
            .RandomSpatialOpStrategyTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::bbox
