#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class SpatialPrefixTree;
}

namespace org::apache::lucene::spatial::prefix
{
class RecursivePrefixTreeStrategy;
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
namespace org::apache::lucene::spatial::composite
{

using RandomSpatialOpStrategyTestCase =
    org::apache::lucene::spatial::prefix::RandomSpatialOpStrategyTestCase;
using RecursivePrefixTreeStrategy =
    org::apache::lucene::spatial::prefix::RecursivePrefixTreeStrategy;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using org::locationtech::spatial4j::shape::Shape;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomBoolean;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomDouble;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

class CompositeStrategyTest : public RandomSpatialOpStrategyTestCase
{
  GET_CLASS_NAME(CompositeStrategyTest)

private:
  std::shared_ptr<SpatialPrefixTree> grid;
  std::shared_ptr<RecursivePrefixTreeStrategy> rptStrategy;

  void setupQuadGrid(int maxLevels);

  void setupGeohashGrid(int maxLevels);

protected:
  virtual std::shared_ptr<RecursivePrefixTreeStrategy> newRPT();

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 20) public void testOperations()
  // throws java.io.IOException
  virtual void testOperations() ;

protected:
  std::shared_ptr<Shape> randomIndexedShape() override;

  std::shared_ptr<Shape> randomQueryShape() override;

private:
  std::shared_ptr<Shape> randomShape();

  // TODO move up
  std::shared_ptr<Shape> randomCircle();

protected:
  std::shared_ptr<CompositeStrategyTest> shared_from_this()
  {
    return std::static_pointer_cast<CompositeStrategyTest>(
        org.apache.lucene.spatial.prefix
            .RandomSpatialOpStrategyTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::composite
