#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
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

namespace org::apache::lucene::queries::function
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

class TestFunctionScoreQuery : public FunctionTestSetup
{
  GET_CLASS_NAME(TestFunctionScoreQuery)

public:
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<IndexSearcher> searcher;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testEqualities();

  // FunctionQuery equivalent
  virtual void testSimpleSourceScore() ;

  // CustomScoreQuery and BoostedQuery equivalent
  virtual void testScoreModifyingSource() ;

  // BoostingQuery equivalent
  virtual void testCombiningMultipleQueryScores() ;

  // check boosts with non-distributive score source
  virtual void testBoostsAreAppliedLast() ;

protected:
  std::shared_ptr<TestFunctionScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestFunctionScoreQuery>(
        FunctionTestSetup::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function
