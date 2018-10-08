#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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
using ScoreDoc = org::apache::lucene::search::ScoreDoc;

class TestFunctionRangeQuery : public FunctionTestSetup
{
  GET_CLASS_NAME(TestFunctionRangeQuery)

public:
  std::shared_ptr<IndexReader> indexReader;
  std::shared_ptr<IndexSearcher> indexSearcher;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

protected:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before protected void before() throws java.io.IOException
  virtual void before() ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void after() throws java.io.IOException
  virtual void after() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRangeInt() throws java.io.IOException
  virtual void testRangeInt() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRangeIntMultiValued() throws
  // java.io.IOException
  virtual void testRangeIntMultiValued() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRangeFloat() throws
  // java.io.IOException
  virtual void testRangeFloat() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRangeFloatMultiValued() throws
  // java.io.IOException
  virtual void testRangeFloatMultiValued() ;

private:
  void doTestRange(std::shared_ptr<ValueSource> valueSource) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDeleted() throws java.io.IOException
  virtual void testDeleted() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDeletedMultiValued() throws
  // java.io.IOException
  virtual void testDeletedMultiValued() ;

private:
  void
  doTestDeleted(std::shared_ptr<ValueSource> valueSource) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExplain() throws java.io.IOException
  virtual void testExplain() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExplainMultiValued() throws
  // java.io.IOException
  virtual void testExplainMultiValued() ;

private:
  void expectScores(std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs,
                    std::deque<int> &docScores);

protected:
  std::shared_ptr<TestFunctionRangeQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestFunctionRangeQuery>(
        FunctionTestSetup::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/
