#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::search
{
class PhraseQuery;
}
namespace org::apache::lucene::search
{
class FixedScoreQuery;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class Query;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestQueryRescorer : public LuceneTestCase
{
  GET_CLASS_NAME(TestQueryRescorer)

private:
  std::shared_ptr<IndexSearcher> getSearcher(std::shared_ptr<IndexReader> r);

public:
  static std::shared_ptr<IndexWriterConfig> newIndexWriterConfig();

  virtual void testBasic() ;

  // Test LUCENE-5682
  virtual void testNullScorerTermQuery() ;

  virtual void testCustomCombine() ;

private:
  class QueryRescorerAnonymousInnerClass : public QueryRescorer
  {
    GET_CLASS_NAME(QueryRescorerAnonymousInnerClass)
  private:
    std::shared_ptr<TestQueryRescorer> outerInstance;

  public:
    QueryRescorerAnonymousInnerClass(
        std::shared_ptr<TestQueryRescorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::PhraseQuery> pq);

  protected:
    float combine(float firstPassScore, bool secondPassMatches,
                  float secondPassScore) override;

  protected:
    std::shared_ptr<QueryRescorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<QueryRescorerAnonymousInnerClass>(
          QueryRescorer::shared_from_this());
    }
  };

public:
  virtual void testExplain() ;

private:
  class QueryRescorerAnonymousInnerClass2 : public QueryRescorer
  {
    GET_CLASS_NAME(QueryRescorerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestQueryRescorer> outerInstance;

  public:
    QueryRescorerAnonymousInnerClass2(
        std::shared_ptr<TestQueryRescorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::PhraseQuery> pq);

  protected:
    float combine(float firstPassScore, bool secondPassMatches,
                  float secondPassScore) override;

  protected:
    std::shared_ptr<QueryRescorerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<QueryRescorerAnonymousInnerClass2>(
          QueryRescorer::shared_from_this());
    }
  };

public:
  virtual void testMissingSecondPassScore() ;

  virtual void testRandom() ;

private:
  class QueryRescorerAnonymousInnerClass3 : public QueryRescorer
  {
    GET_CLASS_NAME(QueryRescorerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestQueryRescorer> outerInstance;

  public:
    QueryRescorerAnonymousInnerClass3(
        std::shared_ptr<TestQueryRescorer> outerInstance,
        std::shared_ptr<FixedScoreQuery> new) new;

  protected:
    float combine(float firstPassScore, bool secondPassMatches,
                  float secondPassScore) override;

  protected:
    std::shared_ptr<QueryRescorerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<QueryRescorerAnonymousInnerClass3>(
          QueryRescorer::shared_from_this());
    }
  };

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<int>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestQueryRescorer> outerInstance;

    std::deque<int> idToNum;
    std::shared_ptr<IndexReader> r;
    int reverseInt = 0;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TestQueryRescorer> outerInstance,
        std::deque<int> &idToNum, std::shared_ptr<IndexReader> r,
        int reverseInt);

    int compare(std::optional<int> &a, std::optional<int> &b);
  };

  /** Just assigns score == idToNum[doc("id")] for each doc. */
private:
  class FixedScoreQuery : public Query
  {
    GET_CLASS_NAME(FixedScoreQuery)
  private:
    std::deque<int> const idToNum;
    const bool reverse;

  public:
    FixedScoreQuery(std::deque<int> &idToNum, bool reverse);

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class WeightAnonymousInnerClass : public Weight
    {
      GET_CLASS_NAME(WeightAnonymousInnerClass)
    private:
      std::shared_ptr<FixedScoreQuery> outerInstance;

    public:
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      WeightAnonymousInnerClass(
          std::shared_ptr<FixedScoreQuery> outerInstance,
          std::shared_ptr<
              org::apache::lucene::search::TestQueryRescorer::FixedScoreQuery>
              shared_from_this());

      void
      extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    private:
      class ScorerAnonymousInnerClass : public Scorer
      {
        GET_CLASS_NAME(ScorerAnonymousInnerClass)
      private:
        std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

        std::shared_ptr<LeafReaderContext> context;

      public:
        ScorerAnonymousInnerClass(
            std::shared_ptr<WeightAnonymousInnerClass> outerInstance,
            std::shared_ptr<LeafReaderContext> context);

        int docID = 0;

        int docID() override;

        std::shared_ptr<DocIdSetIterator> iterator() override;

      private:
        class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
        {
          GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
        private:
          std::shared_ptr<ScorerAnonymousInnerClass> outerInstance;

        public:
          DocIdSetIteratorAnonymousInnerClass(
              std::shared_ptr<ScorerAnonymousInnerClass> outerInstance);

          int docID() override;

          int64_t cost() override;

          int nextDoc() override;

          int advance(int target) override;

        protected:
          std::shared_ptr<DocIdSetIteratorAnonymousInnerClass>
          shared_from_this()
          {
            return std::static_pointer_cast<
                DocIdSetIteratorAnonymousInnerClass>(
                DocIdSetIterator::shared_from_this());
          }
        };

      public:
        float score()  override;

      protected:
        std::shared_ptr<ScorerAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<ScorerAnonymousInnerClass>(
              Scorer::shared_from_this());
        }
      };

    public:
      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

      std::shared_ptr<Explanation>
      explain(std::shared_ptr<LeafReaderContext> context,
              int doc)  override;

    protected:
      std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<WeightAnonymousInnerClass>(
            Weight::shared_from_this());
      }
    };

  public:
    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any other) override;

  private:
    bool equalsTo(std::shared_ptr<FixedScoreQuery> other);

  public:
    virtual int hashCode();

    std::shared_ptr<Query> clone() override;

  protected:
    std::shared_ptr<FixedScoreQuery> shared_from_this()
    {
      return std::static_pointer_cast<FixedScoreQuery>(
          Query::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestQueryRescorer> shared_from_this()
  {
    return std::static_pointer_cast<TestQueryRescorer>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
