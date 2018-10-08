#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/QueryCachingPolicy.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexSearcher : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexSearcher)
public:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;

  void setUp()  override;

  void tearDown()  override;

  // should not throw exception
  virtual void testHugeN() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSearchAfterPassedMaxDoc() throws
  // Exception
  virtual void testSearchAfterPassedMaxDoc() ;

  virtual void testCount() ;

  virtual void testGetQueryCache() ;

private:
  class QueryCacheAnonymousInnerClass
      : public std::enable_shared_from_this<QueryCacheAnonymousInnerClass>,
        public QueryCache
  {
    GET_CLASS_NAME(QueryCacheAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexSearcher> outerInstance;

  public:
    QueryCacheAnonymousInnerClass(
        std::shared_ptr<TestIndexSearcher> outerInstance);

    std::shared_ptr<Weight>
    doCache(std::shared_ptr<Weight> weight,
            std::shared_ptr<QueryCachingPolicy> policy) override;
  };

public:
  virtual void testGetQueryCachingPolicy() ;

private:
  class QueryCachingPolicyAnonymousInnerClass
      : public std::enable_shared_from_this<
            QueryCachingPolicyAnonymousInnerClass>,
        public QueryCachingPolicy
  {
    GET_CLASS_NAME(QueryCachingPolicyAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexSearcher> outerInstance;

  public:
    QueryCachingPolicyAnonymousInnerClass(
        std::shared_ptr<TestIndexSearcher> outerInstance);

    bool shouldCache(std::shared_ptr<Query> query)  override;
    void onUse(std::shared_ptr<Query> query) override;
  };

protected:
  std::shared_ptr<TestIndexSearcher> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexSearcher>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
