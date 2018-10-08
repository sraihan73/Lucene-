#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class BulkScorer;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
class LeafCollector;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class ScorerSupplier;
}
namespace org::apache::lucene::search
{
class Matches;
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
using Bits = org::apache::lucene::util::Bits;

/**
 * A query that wraps another query and simply returns a constant score equal to
 * 1 for every document that matches the query.
 * It therefore simply strips of all scores and always returns 1.
 */
class ConstantScoreQuery final : public Query
{
  GET_CLASS_NAME(ConstantScoreQuery)
private:
  const std::shared_ptr<Query> query;

  /** Strips off scores from the passed in Query. The hits will get a constant
   * score of 1. */
public:
  ConstantScoreQuery(std::shared_ptr<Query> query);

  /** Returns the encapsulated query. */
  std::shared_ptr<Query> getQuery();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  /** We return this as our {@link BulkScorer} so that if the CSQ
   *  wraps a query with its own optimized top-level
   *  scorer (e.g. BooleanScorer) we can use that
   *  top-level scorer. */
protected:
  class ConstantBulkScorer : public BulkScorer
  {
    GET_CLASS_NAME(ConstantBulkScorer)
  public:
    const std::shared_ptr<BulkScorer> bulkScorer;
    const std::shared_ptr<Weight> weight;
    const float theScore;

    ConstantBulkScorer(std::shared_ptr<BulkScorer> bulkScorer,
                       std::shared_ptr<Weight> weight, float theScore);

    int score(std::shared_ptr<LeafCollector> collector,
              std::shared_ptr<Bits> acceptDocs, int min,
              int max)  override;

  private:
    std::shared_ptr<LeafCollector>
    wrapCollector(std::shared_ptr<LeafCollector> collector);

  private:
    class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
    {
      GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantBulkScorer> outerInstance;

    public:
      FilterLeafCollectorAnonymousInnerClass(
          std::shared_ptr<ConstantBulkScorer> outerInstance,
          std::shared_ptr<org::apache::lucene::search::LeafCollector>
              collector);

      void
      setScorer(std::shared_ptr<Scorer> scorer)  override;

    private:
      class FilterScorerAnonymousInnerClass : public FilterScorer
      {
        GET_CLASS_NAME(FilterScorerAnonymousInnerClass)
      private:
        std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> outerInstance;

      public:
        FilterScorerAnonymousInnerClass(
            std::shared_ptr<FilterLeafCollectorAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<org::apache::lucene::search::Scorer> scorer);

        float score()  override;

      protected:
        std::shared_ptr<FilterScorerAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterScorerAnonymousInnerClass>(
              FilterScorer::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
            FilterLeafCollector::shared_from_this());
      }
    };

  public:
    int64_t cost() override;

  protected:
    std::shared_ptr<ConstantBulkScorer> shared_from_this()
    {
      return std::static_pointer_cast<ConstantBulkScorer>(
          BulkScorer::shared_from_this());
    }
  };

public:
  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<ConstantScoreQuery> outerInstance;

    std::shared_ptr<org::apache::lucene::search::Weight> innerWeight;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<ConstantScoreQuery> outerInstance, float boost,
        std::shared_ptr<org::apache::lucene::search::Weight> innerWeight);

    std::shared_ptr<BulkScorer> bulkScorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    std::shared_ptr<ScorerSupplier> scorerSupplier(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ScorerSupplierAnonymousInnerClass : public ScorerSupplier
    {
      GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<org::apache::lucene::search::ScorerSupplier>
          innerScorerSupplier;

    public:
      ScorerSupplierAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<org::apache::lucene::search::ScorerSupplier>
              innerScorerSupplier);

      std::shared_ptr<Scorer>
      get(int64_t leadCost)  override;

    private:
      class FilterScorerAnonymousInnerClass : public FilterScorer
      {
        GET_CLASS_NAME(FilterScorerAnonymousInnerClass)
      private:
        std::shared_ptr<ScorerSupplierAnonymousInnerClass> outerInstance;

        std::shared_ptr<org::apache::lucene::search::Scorer> innerScorer;
        float score = 0;

      public:
        FilterScorerAnonymousInnerClass(
            std::shared_ptr<ScorerSupplierAnonymousInnerClass> outerInstance,
            std::shared_ptr<org::apache::lucene::search::Scorer> innerScorer,
            float score);

        float score()  override;
        std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
        getChildren() override;

      protected:
        std::shared_ptr<FilterScorerAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterScorerAnonymousInnerClass>(
              FilterScorer::shared_from_this());
        }
      };

    public:
      int64_t cost() override;

    protected:
      std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
            ScorerSupplier::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          ConstantScoreWeight::shared_from_this());
    }
  };

public:
  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

  virtual int hashCode();

protected:
  std::shared_ptr<ConstantScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<ConstantScoreQuery>(
        Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
