#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialOperation.h"

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

using org::locationtech::spatial4j::shape::Shape;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

/** Base test harness, ideally for SpatialStrategy impls that have exact results
 * (not grid approximated), hence "not fuzzy".
 */
class RandomSpatialOpStrategyTestCase : public StrategyTestCase
{
  GET_CLASS_NAME(RandomSpatialOpStrategyTestCase)

  // Note: this is partially redundant with StrategyTestCase.runTestQuery &
  // testOperation

protected:
  virtual void testOperationRandomShapes(
      std::shared_ptr<SpatialOperation> operation) ;

  virtual void testOperation(std::shared_ptr<SpatialOperation> operation,
                             std::deque<std::shared_ptr<Shape>> &indexedShapes,
                             std::deque<std::shared_ptr<Shape>> &queryShapes,
                             bool havoc) ;

private:
  void fail(const std::wstring &label, const std::wstring &id,
            std::deque<std::shared_ptr<Shape>> &indexedShapes,
            std::shared_ptr<Shape> queryShape,
            std::shared_ptr<SpatialOperation> operation);

protected:
  virtual void preQueryHavoc();

  virtual std::shared_ptr<Shape> randomIndexedShape() = 0;

  virtual std::shared_ptr<Shape> randomQueryShape() = 0;

protected:
  std::shared_ptr<RandomSpatialOpStrategyTestCase> shared_from_this()
  {
    return std::static_pointer_cast<RandomSpatialOpStrategyTestCase>(
        org.apache.lucene.spatial.StrategyTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
