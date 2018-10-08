#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldPhraseList.h"

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
namespace org::apache::lucene::search::vectorhighlight
{

using namespace org::apache::lucene::search;

class SimpleFragListBuilderTest : public AbstractTestCase
{
  GET_CLASS_NAME(SimpleFragListBuilderTest)

public:
  virtual void testNullFieldFragList() ;

  virtual void testTooSmallFragSize() ;

  virtual void testSmallerFragSizeThanTermQuery() ;

  virtual void testSmallerFragSizeThanPhraseQuery() ;

  virtual void test1TermIndex() ;

  virtual void test2TermsIndex1Frag() ;

  virtual void test2TermsIndex2Frags() ;

  virtual void test2TermsQuery() ;

  virtual void testPhraseQuery() ;

  virtual void testPhraseQuerySlop() ;

private:
  std::shared_ptr<FieldPhraseList>
  fpl(std::shared_ptr<Query> query,
      const std::wstring &indexValue) ;

public:
  virtual void test1PhraseShortMV() ;

  virtual void test1PhraseLongMV() ;

  virtual void test1PhraseLongMVB() ;

protected:
  std::shared_ptr<SimpleFragListBuilderTest> shared_from_this()
  {
    return std::static_pointer_cast<SimpleFragListBuilderTest>(
        AbstractTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
