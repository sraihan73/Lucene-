#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

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
namespace org::apache::lucene::spatial::prefix::tree
{

using org::locationtech::spatial4j::context::SpatialContext;
using SpatialTestCase = org::apache::lucene::spatial::SpatialTestCase;

class SpatialPrefixTreeTest : public SpatialTestCase
{
  GET_CLASS_NAME(SpatialPrefixTreeTest)

  // TODO plug in others and test them
private:
  std::shared_ptr<SpatialContext> ctx;
  std::shared_ptr<SpatialPrefixTree> trie;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCellTraverse()
  virtual void testCellTraverse();
  /**
   * A PrefixTree pruning optimization gone bad, applicable when optimize=true.
   * See <a
   * href="https://issues.apache.org/jira/browse/LUCENE-4770">LUCENE-4770</a>.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBadPrefixTreePrune() throws Exception
  virtual void testBadPrefixTreePrune() ;

protected:
  std::shared_ptr<SpatialPrefixTreeTest> shared_from_this()
  {
    return std::static_pointer_cast<SpatialPrefixTreeTest>(
        org.apache.lucene.spatial.SpatialTestCase::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
