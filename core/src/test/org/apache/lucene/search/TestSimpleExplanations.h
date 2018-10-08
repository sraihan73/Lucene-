#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::search
{

/**
 * TestExplanations subclass focusing on basic query types
 */
class TestSimpleExplanations : public BaseExplanationTestCase
{
  GET_CLASS_NAME(TestSimpleExplanations)

  // we focus on queries that don't rewrite to other queries.
  // if we get those covered well, then the ones that rewrite should
  // also be covered.

  /* simple term tests */

public:
  virtual void testT1() ;
  virtual void testT2() ;

  /* MatchAllDocs */

  virtual void testMA1() ;
  virtual void testMA2() ;

  /* some simple phrase tests */

  virtual void testP1() ;
  virtual void testP2() ;
  virtual void testP3() ;
  virtual void testP4() ;
  virtual void testP5() ;
  virtual void testP6() ;
  virtual void testP7() ;

  /* ConstantScoreQueries */

  virtual void testCSQ1() ;
  virtual void testCSQ2() ;
  virtual void testCSQ3() ;

  /* DisjunctionMaxQuery */

  virtual void testDMQ1() ;
  virtual void testDMQ2() ;
  virtual void testDMQ3() ;
  virtual void testDMQ4() ;
  virtual void testDMQ5() ;
  virtual void testDMQ6() ;
  virtual void testDMQ7() ;
  virtual void testDMQ8() ;
  virtual void testDMQ9() ;

  /* MultiPhraseQuery */

  virtual void testMPQ1() ;
  virtual void testMPQ2() ;
  virtual void testMPQ3() ;
  virtual void testMPQ4() ;
  virtual void testMPQ5() ;
  virtual void testMPQ6() ;

  /* some simple tests of bool queries containing term queries */

  virtual void testBQ1() ;
  virtual void testBQ2() ;
  virtual void testBQ3() ;
  virtual void testBQ4() ;
  virtual void testBQ5() ;
  virtual void testBQ6() ;
  virtual void testBQ7() ;
  virtual void testBQ8() ;
  virtual void testBQ9() ;
  virtual void testBQ10() ;
  virtual void testBQ11() ;
  virtual void testBQ14() ;
  virtual void testBQ15() ;
  virtual void testBQ16() ;
  virtual void testBQ17() ;
  virtual void testBQ19() ;

  virtual void testBQ20() ;

  virtual void testBQ21() ;

  virtual void testBQ23() ;

  virtual void testBQ24() ;

  virtual void testBQ25() ;

  virtual void testBQ26() ;

  /* BQ of TQ: using alt so some fields have zero boost and some don't */

  virtual void testMultiFieldBQ1() ;
  virtual void testMultiFieldBQ2() ;
  virtual void testMultiFieldBQ3() ;
  virtual void testMultiFieldBQ4() ;
  virtual void testMultiFieldBQ5() ;
  virtual void testMultiFieldBQ6() ;
  virtual void testMultiFieldBQ7() ;
  virtual void testMultiFieldBQ8() ;
  virtual void testMultiFieldBQ9() ;
  virtual void testMultiFieldBQ10() ;

  /* BQ of PQ: using alt so some fields have zero boost and some don't */

  virtual void testMultiFieldBQofPQ1() ;
  virtual void testMultiFieldBQofPQ2() ;
  virtual void testMultiFieldBQofPQ3() ;
  virtual void testMultiFieldBQofPQ4() ;
  virtual void testMultiFieldBQofPQ5() ;
  virtual void testMultiFieldBQofPQ6() ;
  virtual void testMultiFieldBQofPQ7() ;

  virtual void testSynonymQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEquality()
  virtual void testEquality();

protected:
  std::shared_ptr<TestSimpleExplanations> shared_from_this()
  {
    return std::static_pointer_cast<TestSimpleExplanations>(
        BaseExplanationTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
