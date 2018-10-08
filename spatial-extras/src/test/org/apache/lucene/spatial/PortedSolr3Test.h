#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
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

/**
 * Based off of Solr 3's SpatialFilterTest.
 */
class PortedSolr3Test : public StrategyTestCase
{
  GET_CLASS_NAME(PortedSolr3Test)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @ParametersFactory(argumentFormatting = "strategy=%s")
  // public static Iterable<Object[]> parameters()
  static std::deque<std::deque<std::any>> parameters();

  PortedSolr3Test(const std::wstring &suiteName,
                  std::shared_ptr<SpatialStrategy> strategy);

private:
  void setupDocs() ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testIntersections() throws Exception
  virtual void testIntersections() ;

  //---- these are similar to Solr test methods

private:
  void checkHitsCircle(std::shared_ptr<Point> pt, double distKM,
                       int assertNumFound, std::deque<int> &assertIds);
  void checkHitsBBox(std::shared_ptr<Point> pt, double distKM,
                     int assertNumFound, std::deque<int> &assertIds);

  void _checkHits(bool bbox, std::shared_ptr<Point> pt, double distKM,
                  int assertNumFound, std::deque<int> &assertIds);

protected:
  std::shared_ptr<PortedSolr3Test> shared_from_this()
  {
    return std::static_pointer_cast<PortedSolr3Test>(
        StrategyTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/
