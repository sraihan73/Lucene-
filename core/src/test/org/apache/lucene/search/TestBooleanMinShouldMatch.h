#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/search/Builder.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Test that BooleanQuery.setMinimumNumberShouldMatch works.
 */
class TestBooleanMinShouldMatch : public LuceneTestCase
{
  GET_CLASS_NAME(TestBooleanMinShouldMatch)

private:
  static std::shared_ptr<Directory> index;
  static std::shared_ptr<IndexReader> r;
  static std::shared_ptr<IndexSearcher> s;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void verifyNrHits(std::shared_ptr<Query> q,
                            int expected) ;

  virtual void testAllOptional() ;

  virtual void testOneReqAndSomeOptional() ;

  virtual void testSomeReqAndSomeOptional() ;

  virtual void testOneProhibAndSomeOptional() ;

  virtual void testSomeProhibAndSomeOptional() ;

  virtual void testOneReqOneProhibAndSomeOptional() ;

  virtual void testSomeReqOneProhibAndSomeOptional() ;

  virtual void testOneReqSomeProhibAndSomeOptional() ;

  virtual void testSomeReqSomeProhibAndSomeOptional() ;

  virtual void testMinHigherThenNumOptional() ;

  virtual void testMinEqualToNumOptional() ;

  virtual void testOneOptionalEqualToMin() ;

  virtual void testNoOptionalButMin() ;

  virtual void testNoOptionalButMin2() ;

  virtual void testRandomQueries() ;

private:
  class CallbackAnonymousInnerClass
      : public std::enable_shared_from_this<CallbackAnonymousInnerClass>,
        public TestBoolean2::Callback
  {
    GET_CLASS_NAME(CallbackAnonymousInnerClass)
  private:
    std::shared_ptr<TestBooleanMinShouldMatch> outerInstance;

    std::wstring field;
    std::deque<std::wstring> vals;

  public:
    CallbackAnonymousInnerClass(
        std::shared_ptr<TestBooleanMinShouldMatch> outerInstance,
        const std::wstring &field, std::deque<std::wstring> &vals);

    void postCreate(std::shared_ptr<BooleanQuery::Builder> q) override;
  };

private:
  void assertSubsetOfSameScores(std::shared_ptr<Query> q,
                                std::shared_ptr<TopDocs> top1,
                                std::shared_ptr<TopDocs> top2);

public:
  virtual void testRewriteMSM1() ;

  virtual void testRewriteNegate() ;

protected:
  virtual void
  printHits(const std::wstring &test, std::deque<std::shared_ptr<ScoreDoc>> &h,
            std::shared_ptr<IndexSearcher> searcher) ;

protected:
  std::shared_ptr<TestBooleanMinShouldMatch> shared_from_this()
  {
    return std::static_pointer_cast<TestBooleanMinShouldMatch>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
