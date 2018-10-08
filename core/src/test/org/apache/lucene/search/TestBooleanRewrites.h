#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::search
{
class TopDocs;
}

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBooleanRewrites : public LuceneTestCase
{
  GET_CLASS_NAME(TestBooleanRewrites)

public:
  virtual void testOneClauseRewriteOptimization() ;

  virtual void testSingleFilterClause() ;

  virtual void testSingleMustMatchAll() ;

  virtual void testSingleMustMatchAllWithShouldClauses() ;

  virtual void testDeduplicateMustAndFilter() ;

  // Duplicate Should and Filter query is converted to Must (with minShouldMatch
  // -1)
  virtual void testConvertShouldAndFilterToMust() ;

  // Duplicate Must or Filter with MustNot returns no match
  virtual void testDuplicateMustOrFilterWithMustNot() ;

  // MatchAllQuery as MUST_NOT clause cannot return anything
  virtual void testMatchAllMustNot() ;

  virtual void testRemoveMatchAllFilter() ;

  virtual void testRandom() ;

private:
  class IndexSearcherAnonymousInnerClass : public IndexSearcher
  {
    GET_CLASS_NAME(IndexSearcherAnonymousInnerClass)
  private:
    std::shared_ptr<TestBooleanRewrites> outerInstance;

  public:
    IndexSearcherAnonymousInnerClass(
        std::shared_ptr<TestBooleanRewrites> outerInstance);

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<Query> original)  override;

  protected:
    std::shared_ptr<IndexSearcherAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexSearcherAnonymousInnerClass>(
          IndexSearcher::shared_from_this());
    }
  };

private:
  std::shared_ptr<Query> randomBooleanQuery();

  std::shared_ptr<Query> randomQuery();

  void assertEquals(std::shared_ptr<TopDocs> td1, std::shared_ptr<TopDocs> td2);

public:
  virtual void testDeduplicateShouldClauses() ;

  virtual void testDeduplicateMustClauses() ;

protected:
  std::shared_ptr<TestBooleanRewrites> shared_from_this()
  {
    return std::static_pointer_cast<TestBooleanRewrites>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
