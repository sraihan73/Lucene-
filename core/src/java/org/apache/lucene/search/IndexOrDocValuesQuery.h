#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Matches.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/BulkScorer.h"
#include  "core/src/java/org/apache/lucene/search/ScorerSupplier.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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

/**
 * A query that uses either an index structure (points or terms) or doc values
 * in order to run a query, depending which one is more efficient. This is
 * typically useful for range queries, whose {@link Weight#scorer} is costly
 * to create since it usually needs to sort large lists of doc ids. For
 * instance, for a field that both indexed {@link LongPoint}s and
 * {@link SortedNumericDocValuesField}s with the same values, an efficient
 * range query could be created by doing:
 * <pre class="prettyprint">
 *   std::wstring field;
 *   long minValue, maxValue;
 *   Query pointQuery = LongPoint.newRangeQuery(field, minValue, maxValue);
 *   Query dvQuery = SortedNumericDocValuesField.newSlowRangeQuery(field,
 * minValue, maxValue); Query query = new IndexOrDocValuesQuery(pointQuery,
 * dvQuery);
 * </pre>
 * The above query will be efficient as it will use points in the case that they
 * perform better, ie. when we need a good lead iterator that will be almost
 * entirely consumed; and doc values otherwise, ie. in the case that another
 * part of the query is already leading iteration but we still need the ability
 * to verify that some documents match.
 * <p><b>NOTE</b>This query currently only works well with point range/exact
 * queries and their equivalent doc values queries.
 * @lucene.experimental
 */
class IndexOrDocValuesQuery final : public Query
{
  GET_CLASS_NAME(IndexOrDocValuesQuery)

private:
  const std::shared_ptr<Query> indexQuery, dvQuery;

  /**
   * Create an {@link IndexOrDocValuesQuery}. Both provided queries must match
   * the same documents and give the same scores.
   * @param indexQuery a query that has a good iterator but whose scorer may be
   * costly to create
   * @param dvQuery a query whose scorer is cheap to create that can quickly
   * check whether a given document matches
   */
public:
  IndexOrDocValuesQuery(std::shared_ptr<Query> indexQuery,
                        std::shared_ptr<Query> dvQuery);

  /** Return the wrapped query that may be costly to initialize but has a good
   *  iterator. */
  std::shared_ptr<Query> getIndexQuery();

  /** Return the wrapped query that may be slow at identifying all matching
   *  documents, but which is cheap to initialize and can efficiently
   *  verify that some documents match. */
  std::shared_ptr<Query> getRandomAccessQuery();

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any obj) override;

  virtual int hashCode();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class WeightAnonymousInnerClass : public Weight
  {
    GET_CLASS_NAME(WeightAnonymousInnerClass)
  private:
    std::shared_ptr<IndexOrDocValuesQuery> outerInstance;

    std::shared_ptr<org::apache::lucene::search::Weight> indexWeight;
    std::shared_ptr<org::apache::lucene::search::Weight> dvWeight;

  public:
    WeightAnonymousInnerClass(
        std::shared_ptr<IndexOrDocValuesQuery> outerInstance,
        std::shared_ptr<org::apache::lucene::search::Weight> indexWeight,
        std::shared_ptr<org::apache::lucene::search::Weight> dvWeight);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<BulkScorer> bulkScorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    std::shared_ptr<ScorerSupplier> scorerSupplier(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ScorerSupplierAnonymousInnerClass : public ScorerSupplier
    {
      GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
    private:
      std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<org::apache::lucene::search::ScorerSupplier>
          indexScorerSupplier;
      std::shared_ptr<org::apache::lucene::search::ScorerSupplier>
          dvScorerSupplier;

    public:
      ScorerSupplierAnonymousInnerClass(
          std::shared_ptr<WeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<org::apache::lucene::search::ScorerSupplier>
              indexScorerSupplier,
          std::shared_ptr<org::apache::lucene::search::ScorerSupplier>
              dvScorerSupplier);

      std::shared_ptr<Scorer>
      get(int64_t leadCost)  override;

      int64_t cost() override;

    protected:
      std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
            ScorerSupplier::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<WeightAnonymousInnerClass>(
          Weight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<IndexOrDocValuesQuery> shared_from_this()
  {
    return std::static_pointer_cast<IndexOrDocValuesQuery>(
        Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
