#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/SpatialStrategy.h"

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
namespace org::apache::lucene::spatial
{

using org::locationtech::spatial4j::shape::Point;

class DistanceStrategyTest : public StrategyTestCase
{
  GET_CLASS_NAME(DistanceStrategyTest)
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @ParametersFactory(argumentFormatting = "strategy=%s")
  // public static Iterable<Object[]> parameters()
  static std::deque<std::deque<std::any>> parameters();

  DistanceStrategyTest(const std::wstring &suiteName,
                       std::shared_ptr<SpatialStrategy> strategy);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDistanceOrder() throws
  // java.io.IOException
  virtual void testDistanceOrder() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRecipScore() throws
  // java.io.IOException
  virtual void testRecipScore() ;

  virtual void
  checkDistValueSource(std::shared_ptr<Point> pt,
                       std::deque<float> &distances) ;

protected:
  std::shared_ptr<DistanceStrategyTest> shared_from_this()
  {
    return std::static_pointer_cast<DistanceStrategyTest>(
        StrategyTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/
