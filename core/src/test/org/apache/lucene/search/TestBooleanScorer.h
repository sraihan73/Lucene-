#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
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
class Explanation;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class BulkScorer;
}
namespace org::apache::lucene::search
{
class LeafCollector;
}
namespace org::apache::lucene::util
{
class Bits;
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

class TestBooleanScorer : public LuceneTestCase
{
  GET_CLASS_NAME(TestBooleanScorer)
private:
  static const std::wstring FIELD;

public:
  virtual void testMethod() ;

  /** Throws UOE if Weight.scorer is called */
private:
  class CrazyMustUseBulkScorerQuery : public Query
  {
    GET_CLASS_NAME(CrazyMustUseBulkScorerQuery)

  public:
    std::wstring toString(const std::wstring &field) override;

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class WeightAnonymousInnerClass : public Weight
    {
      GET_CLASS_NAME(WeightAnonymousInnerClass)
    private:
      std::shared_ptr<CrazyMustUseBulkScorerQuery> outerInstance;

    public:
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      WeightAnonymousInnerClass(
          std::shared_ptr<CrazyMustUseBulkScorerQuery> outerInstance,
          std::shared_ptr<org::apache::lucene::search::TestBooleanScorer::
                              CrazyMustUseBulkScorerQuery>
              shared_from_this());

      void
      extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

      std::shared_ptr<Explanation>
      explain(std::shared_ptr<LeafReaderContext> context, int doc) override;

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) override;

      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

      std::shared_ptr<BulkScorer>
      bulkScorer(std::shared_ptr<LeafReaderContext> context) override;

    private:
      class BulkScorerAnonymousInnerClass : public BulkScorer
      {
        GET_CLASS_NAME(BulkScorerAnonymousInnerClass)
      private:
        std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

      public:
        BulkScorerAnonymousInnerClass(
            std::shared_ptr<WeightAnonymousInnerClass> outerInstance);

        int score(std::shared_ptr<LeafCollector> collector,
                  std::shared_ptr<Bits> acceptDocs, int min,
                  int max)  override;
        int64_t cost() override;

      protected:
        std::shared_ptr<BulkScorerAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<BulkScorerAnonymousInnerClass>(
              BulkScorer::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<WeightAnonymousInnerClass>(
            Weight::shared_from_this());
      }
    };

  public:
    bool equals(std::any obj) override;

    virtual int hashCode();

  protected:
    std::shared_ptr<CrazyMustUseBulkScorerQuery> shared_from_this()
    {
      return std::static_pointer_cast<CrazyMustUseBulkScorerQuery>(
          Query::shared_from_this());
    }
  };

  /** Make sure BooleanScorer can embed another
   *  BooleanScorer. */
public:
  virtual void testEmbeddedBooleanScorer() ;

  virtual void testOptimizeTopLevelClauseOrNull() ;

  virtual void testOptimizeProhibitedClauses() ;

  virtual void testSparseClauseOptimization() ;

protected:
  std::shared_ptr<TestBooleanScorer> shared_from_this()
  {
    return std::static_pointer_cast<TestBooleanScorer>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
