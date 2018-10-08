#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionQuery.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/queries/CustomScoreProvider.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/ChildScorer.h"

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
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using FunctionQuery = org::apache::lucene::queries::function::FunctionQuery;
using Explanation = org::apache::lucene::search::Explanation;
using FilterScorer = org::apache::lucene::search::FilterScorer;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

/**
 * Query that sets document score as a programmatic function of several (sub)
 * scores: <ol> <li>the score of its subQuery (any query)</li> <li>(optional)
 * the score of its {@link FunctionQuery} (or queries).</li>
 * </ol>
 * Subclasses can modify the computation by overriding {@link
 * #getCustomScoreProvider}.
 *
 * @lucene.experimental
 *
 * Clients should instead use FunctionScoreQuery.  For simple multiplicative
 * boosts, use
 * {@link FunctionScoreQuery#boostByValue(Query, DoubleValuesSource)}.  For more
 * complex custom scores, use the lucene-expressions library <pre>
 *   SimpleBindings bindings = new SimpleBindings();
 *   bindings.add("score", DoubleValuesSource.SCORES);
 *   bindings.add("boost1", DoubleValuesSource.fromIntField("myboostfield"));
 *   bindings.add("boost2",
 * DoubleValuesSource.fromIntField("myotherboostfield")); Expression expr =
 * JavascriptCompiler.compile("score * (boost1 + ln(boost2))");
 *   FunctionScoreQuery q = new FunctionScoreQuery(inputQuery,
 * expr.getDoubleValuesSource(bindings));
 * </pre>
 *
 * @deprecated use {@link org.apache.lucene.queries.function.FunctionScoreQuery}
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class CustomScoreQuery extends
// org.apache.lucene.search.Query implements Cloneable
class CustomScoreQuery : public Query, public Cloneable
{

private:
  std::shared_ptr<Query> subQuery;
  std::deque<std::shared_ptr<Query>>
      scoringQueries; // never null (empty array if there are no valSrcQueries).

  /**
   * Create a CustomScoreQuery over input subQuery.
   * @param subQuery the sub query whose scored is being customized. Must not be
   * null.
   */
public:
  CustomScoreQuery(std::shared_ptr<Query> subQuery);

  /**
   * Create a CustomScoreQuery over input subQuery and a {@link
   * org.apache.lucene.queries.function.FunctionQuery}.
   * @param subQuery the sub query whose score is being customized. Must not be
   * null.
   * @param scoringQuery a value source query whose scores are used in the
   * custom score computation.  This parameter is optional - it can be null.
   */
  CustomScoreQuery(std::shared_ptr<Query> subQuery,
                   std::shared_ptr<FunctionQuery> scoringQuery);

  /**
   * Create a CustomScoreQuery over input subQuery and a {@link
   * org.apache.lucene.queries.function.FunctionQuery}.
   * @param subQuery the sub query whose score is being customized. Must not be
   * null.
   * @param scoringQueries value source queries whose scores are used in the
   * custom score computation.  This parameter is optional - it can be null or
   * even an empty array.
   */
  CustomScoreQuery(std::shared_ptr<Query> subQuery,
                   std::deque<FunctionQuery> &scoringQueries);

  /*(non-Javadoc) @see
   * org.apache.lucene.search.Query#rewrite(org.apache.lucene.index.IndexReader)
   */
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  /*(non-Javadoc) @see org.apache.lucene.search.Query#clone() */
  std::shared_ptr<CustomScoreQuery> clone() override;

  /* (non-Javadoc) @see
   * org.apache.lucene.search.Query#toString(java.lang.std::wstring) */
  std::wstring toString(const std::wstring &field) override;

  /** Returns true if <code>o</code> is equal to this. */
  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<CustomScoreQuery> other);

  /** Returns a hash code value for this object. */
public:
  virtual int hashCode();

  /**
   * Returns a {@link CustomScoreProvider} that calculates the custom scores
   * for the given {@link IndexReader}. The default implementation returns a
   * default implementation as specified in the docs of {@link
   * CustomScoreProvider}.
   * @since 2.9.2
   */
protected:
  virtual std::shared_ptr<CustomScoreProvider> getCustomScoreProvider(
      std::shared_ptr<LeafReaderContext> context) ;

  //=========================== W E I G H T ============================

private:
  class CustomWeight : public Weight
  {
    GET_CLASS_NAME(CustomWeight)
  private:
    std::shared_ptr<CustomScoreQuery> outerInstance;

  public:
    const std::shared_ptr<Weight> subQueryWeight;
    std::deque<std::shared_ptr<Weight>> const valSrcWeights;
    const float queryWeight;

    CustomWeight(std::shared_ptr<CustomScoreQuery> outerInstance,
                 std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

  private:
    std::shared_ptr<Explanation>
    doExplain(std::shared_ptr<LeafReaderContext> info,
              int doc) ;

  protected:
    std::shared_ptr<CustomWeight> shared_from_this()
    {
      return std::static_pointer_cast<CustomWeight>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

  //=========================== S C O R E R ============================

  /**
   * A scorer that applies a (callback) function on scores of the subQuery.
   */
private:
  class CustomScorer : public FilterScorer
  {
    GET_CLASS_NAME(CustomScorer)
  private:
    const float qWeight;
    const std::shared_ptr<Scorer> subQueryScorer;
    std::deque<std::shared_ptr<Scorer>> const valSrcScorers;
    const std::shared_ptr<CustomScoreProvider> provider;
    std::deque<float> const vScores; // reused in score() to avoid allocating
                                      // this array for each doc
    int valSrcDocID = -1;             // we lazily advance subscorers.

    // constructor
    CustomScorer(std::shared_ptr<CustomScoreProvider> provider,
                 std::shared_ptr<CustomWeight> w, float qWeight,
                 std::shared_ptr<Scorer> subQueryScorer,
                 std::deque<std::shared_ptr<Scorer>> &valSrcScorers);

  public:
    float score()  override;

    std::shared_ptr<std::deque<std::shared_ptr<Scorer::ChildScorer>>>
    getChildren() override;

  protected:
    std::shared_ptr<CustomScorer> shared_from_this()
    {
      return std::static_pointer_cast<CustomScorer>(
          org.apache.lucene.search.FilterScorer::shared_from_this());
    }
  };

public:
  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  /** The sub-query that CustomScoreQuery wraps, affecting both the score and
   * which documents match. */
  virtual std::shared_ptr<Query> getSubQuery();

  /** The scoring queries that only affect the score of CustomScoreQuery. */
  virtual std::deque<std::shared_ptr<Query>> getScoringQueries();

  /**
   * A short name of this query, used in {@link #toString(std::wstring)}.
   */
  virtual std::wstring name();

protected:
  std::shared_ptr<CustomScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<CustomScoreQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/
