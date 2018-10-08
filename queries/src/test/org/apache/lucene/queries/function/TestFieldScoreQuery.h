#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"

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

using Query = org::apache::lucene::search::Query;

/**
 * Test FieldScoreQuery search.
 * <p>
 * Tests here create an index with a few documents, each having
 * an int value indexed  field and a float value indexed field.
 * The values of these fields are later used for scoring.
 * <p>
 * The rank tests use Hits to verify that docs are ordered (by score) as
 * expected. <p> The exact score tests use TopDocs top to verify the exact
 * score.
 */
class TestFieldScoreQuery : public FunctionTestSetup
{
  GET_CLASS_NAME(TestFieldScoreQuery)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  /** Test that FieldScoreQuery of Type.INT returns docs in expected order. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRankInt() throws Exception
  virtual void testRankInt() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRankIntMultiValued() throws Exception
  virtual void testRankIntMultiValued() ;

  /** Test that FieldScoreQuery of Type.FLOAT returns docs in expected order. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRankFloat() throws Exception
  virtual void testRankFloat() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRankFloatMultiValued() throws
  // Exception
  virtual void testRankFloatMultiValued() ;

  // Test that FieldScoreQuery returns docs in expected order.
private:
  void doTestRank(std::shared_ptr<ValueSource> valueSource) throw(
      std::runtime_error);

  /** Test that FieldScoreQuery of Type.INT returns the expected scores. */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExactScoreInt() throws Exception
  virtual void testExactScoreInt() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExactScoreIntMultiValued() throws
  // Exception
  virtual void testExactScoreIntMultiValued() ;

  /** Test that FieldScoreQuery of Type.FLOAT returns the expected scores. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExactScoreFloat() throws Exception
  virtual void testExactScoreFloat() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExactScoreFloatMultiValued() throws
  // Exception
  virtual void testExactScoreFloatMultiValued() ;

  // Test that FieldScoreQuery returns docs with expected score.
private:
  void doTestExactScore(std::shared_ptr<ValueSource> valueSource) throw(
      std::runtime_error);

protected:
  virtual std::shared_ptr<Query>
  getFunctionQuery(std::shared_ptr<ValueSource> valueSource);

protected:
  std::shared_ptr<TestFieldScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestFieldScoreQuery>(
        FunctionTestSetup::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/
