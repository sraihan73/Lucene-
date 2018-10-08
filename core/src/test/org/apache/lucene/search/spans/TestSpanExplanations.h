#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::search::spans
{

using namespace org::apache::lucene::search;

/**
 * TestExplanations subclass focusing on span queries
 */
class TestSpanExplanations : public BaseExplanationTestCase
{
  GET_CLASS_NAME(TestSpanExplanations)

  /* simple SpanTermQueries */

public:
  virtual void testST1() ;
  virtual void testST2() ;
  virtual void testST4() ;
  virtual void testST5() ;

  /* some SpanFirstQueries */

  virtual void testSF1() ;
  virtual void testSF2() ;
  virtual void testSF4() ;
  virtual void testSF5() ;
  virtual void testSF6() ;

  /* some SpanOrQueries */

  virtual void testSO1() ;
  virtual void testSO2() ;
  virtual void testSO3() ;
  virtual void testSO4() ;

  /* some SpanNearQueries */

  virtual void testSNear1() ;
  virtual void testSNear2() ;
  virtual void testSNear3() ;
  virtual void testSNear4() ;
  virtual void testSNear5() ;

  virtual void testSNear6() ;
  virtual void testSNear7() ;
  virtual void testSNear8() ;
  virtual void testSNear9() ;
  virtual void testSNear10() ;
  virtual void testSNear11() ;

  /* some SpanNotQueries */

  virtual void testSNot1() ;
  virtual void testSNot2() ;
  virtual void testSNot4() ;
  virtual void testSNot5() ;
  virtual void testSNot7() ;
  virtual void testSNot10() ;

protected:
  std::shared_ptr<TestSpanExplanations> shared_from_this()
  {
    return std::static_pointer_cast<TestSpanExplanations>(
        BaseExplanationTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
