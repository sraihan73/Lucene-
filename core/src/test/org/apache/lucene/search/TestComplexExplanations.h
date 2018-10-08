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

using namespace org::apache::lucene::search::spans;

/**
 * TestExplanations subclass that builds up super crazy complex queries
 * on the assumption that if the explanations work out right for them,
 * they should work for anything.
 */
class TestComplexExplanations : public BaseExplanationTestCase
{
  GET_CLASS_NAME(TestComplexExplanations)

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void test1() ;

  virtual void test2() ;

  // :TODO: we really need more crazy complex cases.

  // //////////////////////////////////////////////////////////////////

  // The rest of these aren't that complex, but they are <i>somewhat</i>
  // complex, and they expose weakness in dealing with queries that match
  // with scores of 0 wrapped in other queries

  virtual void testT3() ;

  virtual void testMA3() ;

  virtual void testFQ5() ;

  virtual void testCSQ4() ;

  virtual void testDMQ10() ;

  virtual void testMPQ7() ;

  virtual void testBQ12() ;
  virtual void testBQ13() ;
  virtual void testBQ18() ;
  virtual void testBQ21() ;
  virtual void testBQ22() ;

  virtual void testST3() ;
  virtual void testST6() ;

  virtual void testSF3() ;
  virtual void testSF7() ;

  virtual void testSNot3() ;
  virtual void testSNot6() ;

  virtual void testSNot8() ;
  virtual void testSNot9() ;

protected:
  std::shared_ptr<TestComplexExplanations> shared_from_this()
  {
    return std::static_pointer_cast<TestComplexExplanations>(
        BaseExplanationTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
