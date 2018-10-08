#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class SpatialPrefixTree;
}

namespace org::apache::lucene::spatial::prefix
{
class HeatmapFacetCounter;
}
namespace org::apache::lucene::spatial::prefix
{
class Heatmap;
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

using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.atMost;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

class HeatmapFacetCounterTest : public StrategyTestCase
{
  GET_CLASS_NAME(HeatmapFacetCounterTest)

public:
  std::shared_ptr<SpatialPrefixTree> grid;

  int cellsValidated = 0;
  int cellValidatedNonZero = 0;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void after()
  virtual void after();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testStatic() throws java.io.IOException
  virtual void testStatic() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLucene7291Dateline() throws
  // java.io.IOException
  virtual void testLucene7291Dateline() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueryCircle() throws
  // java.io.IOException
  virtual void testQueryCircle() ;

  /** Recursively facet & validate at higher resolutions until we've seen
   * enough. We assume there are some non-zero cells. */
private:
  void
  validateHeatmapResultLoop(std::shared_ptr<Rectangle> inputRange,
                            int facetLevel,
                            int cellCountRecursThreshold) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 20) public void testRandom()
  // throws java.io.IOException
  virtual void testRandom() ;

  /** Build heatmap, validate results, then descend recursively to another facet
   * level. */
private:
  bool queryHeatmapRecursive(std::shared_ptr<Rectangle> inputRange,
                             int facetLevel) ;

  void validateHeatmapResult(
      std::shared_ptr<Rectangle> inputRange, int facetLevel,
      std::shared_ptr<HeatmapFacetCounter::Heatmap> heatmap) ;

  int countMatchingDocsAtLevel(std::shared_ptr<Point> pt,
                               int facetLevel) ;

  std::shared_ptr<Shape> randomIndexedShape();

protected:
  std::shared_ptr<HeatmapFacetCounterTest> shared_from_this()
  {
    return std::static_pointer_cast<HeatmapFacetCounterTest>(
        org.apache.lucene.spatial.StrategyTestCase::shared_from_this());
  }
};
} // namespace org::apache::lucene::spatial::prefix
