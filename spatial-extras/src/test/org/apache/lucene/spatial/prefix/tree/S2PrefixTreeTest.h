#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test for S2 Spatial prefix tree.
 */
class S2PrefixTreeTest : public LuceneTestCase
{
  GET_CLASS_NAME(S2PrefixTreeTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 10) public void testCells()
  virtual void testCells();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 10) public void
  // testDistanceAndLevels()
  virtual void testDistanceAndLevels();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test @Repeat(iterations = 10) public void testPrecision()
  virtual void testPrecision();

protected:
  std::shared_ptr<S2PrefixTreeTest> shared_from_this()
  {
    return std::static_pointer_cast<S2PrefixTreeTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
