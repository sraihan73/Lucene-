#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/search/Builder.h"

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

/** Test BooleanQuery2 against BooleanQuery by overriding the standard query
 * parser. This also tests the scoring order of BooleanQuery.
 */
class TestBoolean2 : public LuceneTestCase
{
  GET_CLASS_NAME(TestBoolean2)
private:
  static std::shared_ptr<IndexSearcher> searcher;
  static std::shared_ptr<IndexSearcher> singleSegmentSearcher;
  static std::shared_ptr<IndexSearcher> bigSearcher;
  static std::shared_ptr<IndexReader> reader;
  static std::shared_ptr<IndexReader> littleReader;
  static std::shared_ptr<IndexReader> singleSegmentReader;
  /** num of empty docs injected between every doc in the (main) index */
  static int NUM_FILLER_DOCS;
  /** num of empty docs injected prior to the first doc in the (main) index */
  static int PRE_FILLER_DOCS;
  /** num "extra" docs containing value in "field2" added to the "big" clone of
   * the index */
  static constexpr int NUM_EXTRA_DOCS = 6000;

public:
  static const std::wstring field;

private:
  static std::shared_ptr<Directory> directory;
  static std::shared_ptr<Directory> singleSegmentDirectory;
  static std::shared_ptr<Directory> dir2;
  static int mulFactor;

  static std::shared_ptr<Directory>
  copyOf(std::shared_ptr<Directory> dir) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

private:
  static std::deque<std::wstring> docFields;

public:
  virtual void
  queriesTest(std::shared_ptr<Query> query,
              std::deque<int> &expDocNrs) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries01() throws Exception
  virtual void testQueries01() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries02() throws Exception
  virtual void testQueries02() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries03() throws Exception
  virtual void testQueries03() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries04() throws Exception
  virtual void testQueries04() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries05() throws Exception
  virtual void testQueries05() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries06() throws Exception
  virtual void testQueries06() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries07() throws Exception
  virtual void testQueries07() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries08() throws Exception
  virtual void testQueries08() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testQueries09() throws Exception
  virtual void testQueries09() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRandomQueries() throws Exception
  virtual void testRandomQueries() ;

  // used to set properties or change every BooleanQuery
  // generated from randBoolQuery.
public:
  class Callback
  {
    GET_CLASS_NAME(Callback)
  public:
    virtual void postCreate(std::shared_ptr<BooleanQuery::Builder> q) = 0;
  };

  // Random rnd is passed in so that the exact same random query may be created
  // more than once.
public:
  static std::shared_ptr<BooleanQuery::Builder>
  randBoolQuery(std::shared_ptr<Random> rnd, bool allowMust, int level,
                const std::wstring &field, std::deque<std::wstring> &vals,
                std::shared_ptr<Callback> cb);

protected:
  std::shared_ptr<TestBoolean2> shared_from_this()
  {
    return std::static_pointer_cast<TestBoolean2>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
