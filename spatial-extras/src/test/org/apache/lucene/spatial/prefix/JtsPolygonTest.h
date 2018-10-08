#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialArgs.h"

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
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;

class JtsPolygonTest : public StrategyTestCase
{
  GET_CLASS_NAME(JtsPolygonTest)

private:
  static constexpr double LUCENE_4464_distErrPct =
      SpatialArgs::DEFAULT_DISTERRPCT; // DEFAULT 2.5%

public:
  JtsPolygonTest();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCloseButNoMatch() throws Exception
  virtual void testCloseButNoMatch() ;

private:
  std::shared_ptr<SpatialArgs> q(const std::wstring &shapeStr,
                                 double distErrPct) ;

  /**
   * A PrefixTree pruning optimization gone bad.
   * See <a
   * href="https://issues.apache.org/jira/browse/LUCENE-4770">LUCENE-4770</a>.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBadPrefixTreePrune() throws Exception
  virtual void testBadPrefixTreePrune() ;

protected:
  std::shared_ptr<JtsPolygonTest> shared_from_this()
  {
    return std::static_pointer_cast<JtsPolygonTest>(
        org.apache.lucene.spatial.StrategyTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
