#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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
namespace org::apache::lucene::queries
{

using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

/**
 * The BoostingQuery class can be used to effectively demote results that match
 * a given query. Unlike the "NOT" clause, this still selects documents that
 * contain undesirable terms, but reduces their overall score:
 *
 *     Query balancedQuery = new BoostingQuery(positiveQuery, negativeQuery,
 * 0.01f); In this scenario the positiveQuery contains the mandatory, desirable
 * criteria which is used to select all matching documents, and the
 * negativeQuery contains the undesirable elements which are simply used to
 * lessen the scores. Documents that match the negativeQuery have their score
 * multiplied by the supplied "boost" parameter, so this should be less than 1
 * to achieve a demoting effect
 *
 * This code was originally made available here:
 *   <a
 * href="http://marc.theaimsgroup.com/?l=lucene-user&amp;m=108058407130459&amp;w=2">http://marc.theaimsgroup.com/?l=lucene-user&amp;m=108058407130459&amp;w=2</a>
 * and is documented here:
 * http://wiki.apache.org/lucene-java/CommunityContributions
 *
 * @deprecated Use {@link
 * org.apache.lucene.queries.function.FunctionScoreQuery#boostByQuery(Query,
 * Query, float)}
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class BoostingQuery extends
// org.apache.lucene.search.Query
class BoostingQuery : public Query
{
private:
  const float contextBoost;             // the amount to boost by
  const std::shared_ptr<Query> match;   // query to match
  const std::shared_ptr<Query> context; // boost when matches too

public:
  BoostingQuery(std::shared_ptr<Query> match, std::shared_ptr<Query> context,
                float boost);

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
    std::shared_ptr<BoostingQuery> outerInstance;

    float boost = 0;
    std::shared_ptr<Weight> matchWeight;
    std::shared_ptr<Weight> contextWeight;

  public:
    WeightAnonymousInnerClass(std::shared_ptr<BoostingQuery> outerInstance,
                              float boost, std::shared_ptr<Weight> matchWeight,
                              std::shared_ptr<Weight> contextWeight);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class FilterScorerAnonymousInnerClass : public FilterScorer
    {
      GET_CLASS_NAME(FilterScorerAnonymousInnerClass)
    private:
      std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<TwoPhaseIterator> contextTwoPhase;
      std::shared_ptr<DocIdSetIterator> contextApproximation;

    public:
      FilterScorerAnonymousInnerClass(
          std::shared_ptr<WeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<TwoPhaseIterator> contextTwoPhase,
          std::shared_ptr<DocIdSetIterator> contextApproximation);

      float score()  override;

    protected:
      std::shared_ptr<FilterScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterScorerAnonymousInnerClass>(
            org.apache.lucene.search.FilterScorer::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<WeightAnonymousInnerClass>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

public:
  virtual std::shared_ptr<Query> getMatch();

  virtual std::shared_ptr<Query> getContext();

  virtual float getBoost();

  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<BoostingQuery> other);

public:
  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<BoostingQuery> shared_from_this()
  {
    return std::static_pointer_cast<BoostingQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/
