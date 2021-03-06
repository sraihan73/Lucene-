#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetResult.h"

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

namespace org::apache::lucene::facet
{

using FacetResult = org::apache::lucene::facet::FacetResult;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Tests long value facets. */
class TestLongValueFacetCounts : public LuceneTestCase
{
  GET_CLASS_NAME(TestLongValueFacetCounts)

public:
  virtual void testBasic() ;

  virtual void testOnlyBigLongs() ;

  virtual void testGetAllDims() ;

  virtual void testRandom() ;

  virtual void testRandomMultiValued() ;

private:
  static void assertSame(
      const std::wstring &desc,
      std::deque<std::unordered_map::Entry<int64_t, int>> &expectedCounts,
      int expectedChildCount, int expectedTotalCount,
      std::shared_ptr<FacetResult> actual, int topN);

protected:
  std::shared_ptr<TestLongValueFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<TestLongValueFacetCounts>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
