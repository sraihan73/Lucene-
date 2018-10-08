#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/MultiTermQuery.h"
#include  "core/src/java/org/apache/lucene/search/RewriteMethod.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;

class TestMultiTermConstantScore : public BaseTestRangeFilter
{
  GET_CLASS_NAME(TestMultiTermConstantScore)

  /** threshold for comparing floats */
public:
  static constexpr float SCORE_COMP_THRESH = 1e-6.0f;

  static std::shared_ptr<Directory> small;
  static std::shared_ptr<IndexReader> reader;

  static void assertEquals(const std::wstring &m, int e, int a);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  /** macro for readability */
  static std::shared_ptr<Query> csrq(const std::wstring &f,
                                     const std::wstring &l,
                                     const std::wstring &h, bool il, bool ih);

  static std::shared_ptr<Query>
  csrq(const std::wstring &f, const std::wstring &l, const std::wstring &h,
       bool il, bool ih, std::shared_ptr<MultiTermQuery::RewriteMethod> method);

  /** macro for readability */
  static std::shared_ptr<Query> cspq(std::shared_ptr<Term> prefix);

  /** macro for readability */
  static std::shared_ptr<Query> cswcq(std::shared_ptr<Term> wild);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasics() throws java.io.IOException
  virtual void testBasics() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEqualScores() throws
  // java.io.IOException
  virtual void testEqualScores() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEqualScoresWhenNoHits() throws
  // java.io.IOException
  virtual void testEqualScoresWhenNoHits() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBooleanOrderUnAffected() throws
  // java.io.IOException
  virtual void testBooleanOrderUnAffected() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRangeQueryId() throws
  // java.io.IOException
  virtual void testRangeQueryId() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRangeQueryRand() throws
  // java.io.IOException
  virtual void testRangeQueryRand() ;

protected:
  std::shared_ptr<TestMultiTermConstantScore> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiTermConstantScore>(
        BaseTestRangeFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
