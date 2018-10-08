#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class Weight;
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

/** This class only tests some basic functionality in CSQ, the main parts are
 * mostly tested by MultiTermQuery tests, explanations seems to be tested in
 * TestExplanations! */
class TestConstantScoreQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestConstantScoreQuery)

public:
  virtual void testCSQ() ;

private:
  void checkHits(std::shared_ptr<IndexSearcher> searcher,
                 std::shared_ptr<Query> q, float const expectedScore,
                 std::type_info const innerScorerClass) ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestConstantScoreQuery> outerInstance;

    float expectedScore = 0;
    std::type_info innerScorerClass;
    std::deque<int> count;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestConstantScoreQuery> outerInstance,
        float expectedScore, std::type_info innerScorerClass,
        std::deque<int> &count);

  private:
    std::shared_ptr<Scorer> scorer;

  public:
    void setScorer(std::shared_ptr<Scorer> scorer) override;

    void collect(int doc)  override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testWrapped2Times() ;

  // a query for which other queries don't have special rewrite rules
private:
  class QueryWrapper : public Query
  {
    GET_CLASS_NAME(QueryWrapper)

  private:
    const std::shared_ptr<Query> in_;

  public:
    QueryWrapper(std::shared_ptr<Query> in_);

    std::wstring toString(const std::wstring &field) override;

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

    bool equals(std::any other) override;

    virtual int hashCode();

  protected:
    std::shared_ptr<QueryWrapper> shared_from_this()
    {
      return std::static_pointer_cast<QueryWrapper>(Query::shared_from_this());
    }
  };

public:
  virtual void testConstantScoreQueryAndFilter() ;

  virtual void testPropagatesApproximations() ;

  virtual void testExtractTerms() ;

protected:
  std::shared_ptr<TestConstantScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestConstantScoreQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
