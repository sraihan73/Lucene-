#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class DateRangePrefixTree;
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
namespace org::apache::lucene::spatial::prefix
{

using DateRangePrefixTree =
    org::apache::lucene::spatial::prefix::tree::DateRangePrefixTree;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

class DateNRStrategyTest : public RandomSpatialOpStrategyTestCase
{
  GET_CLASS_NAME(DateNRStrategyTest)

public:
  static constexpr int ITERATIONS = 10;

  std::shared_ptr<DateRangePrefixTree> tree;

  int64_t randomCalWindowMs = 0;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
  // testIntersects() throws java.io.IOException
  virtual void testIntersects() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
  // testWithin() throws java.io.IOException
  virtual void testWithin() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = ITERATIONS) public void
  // testContains() throws java.io.IOException
  virtual void testContains() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithinSame() throws
  // java.io.IOException
  virtual void testWithinSame() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWorld() throws java.io.IOException
  virtual void testWorld() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBugInitIterOptimization() throws
  // Exception
  virtual void testBugInitIterOptimization() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLastMillionYearPeriod() throws
  // Exception
  virtual void testLastMillionYearPeriod() ;

protected:
  std::shared_ptr<Shape> randomIndexedShape() override;

  std::shared_ptr<Shape> randomQueryShape() override;

private:
  std::shared_ptr<Shape> randomShape();

  std::shared_ptr<Calendar> randomCalendar();

protected:
  std::shared_ptr<DateNRStrategyTest> shared_from_this()
  {
    return std::static_pointer_cast<DateNRStrategyTest>(
        RandomSpatialOpStrategyTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix
