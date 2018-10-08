#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::query
{
class SpatialArgs;
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

using org::locationtech::spatial4j::shape::Point;
using StrategyTestCase = org::apache::lucene::spatial::StrategyTestCase;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;

class TestRecursivePrefixTreeStrategy : public StrategyTestCase
{
  GET_CLASS_NAME(TestRecursivePrefixTreeStrategy)

private:
  int maxLength = 0;

  // Tests should call this first.
  void init(int maxLength);

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFilterWithVariableScanLevel() throws
  // java.io.IOException
  virtual void testFilterWithVariableScanLevel() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOneMeterPrecision()
  virtual void testOneMeterPrecision();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPrecision() throws java.io.IOException
  virtual void testPrecision() ;

private:
  std::shared_ptr<SpatialArgs> q(std::shared_ptr<Point> pt, double distDEG,
                                 double distErrPct);

  void checkHits(std::shared_ptr<SpatialArgs> args, int assertNumFound,
                 std::deque<int> &assertIds);

protected:
  std::shared_ptr<TestRecursivePrefixTreeStrategy> shared_from_this()
  {
    return std::static_pointer_cast<TestRecursivePrefixTreeStrategy>(
        org.apache.lucene.spatial.StrategyTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix
