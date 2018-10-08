#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::search
{
class DoubleValuesSource;
}
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
class IndexReader;
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
class Matches;
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
class DoubleValues;
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

namespace org::apache::lucene::queries::function
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Matches = org::apache::lucene::search::Matches;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

/**
 * A query that wraps another query, and uses a DoubleValuesSource to
 * replace or modify the wrapped query's score
 *
 * If the DoubleValuesSource doesn't return a value for a particular document,
 * then that document will be given a score of 0.
 */
class FunctionScoreQuery final : public Query
{
  GET_CLASS_NAME(FunctionScoreQuery)

private:
  const std::shared_ptr<Query> in_;
  const std::shared_ptr<DoubleValuesSource> source;

  /**
   * Create a new FunctionScoreQuery
   * @param in      the query to wrap
   * @param source  a source of scores
   */
public:
  FunctionScoreQuery(std::shared_ptr<Query> in_,
                     std::shared_ptr<DoubleValuesSource> source);

  /**
   * @return the wrapped Query
   */
  std::shared_ptr<Query> getWrappedQuery();

  /**
   * Returns a FunctionScoreQuery where the scores of a wrapped query are
   * multiplied by the value of a DoubleValuesSource.
   *
   * If the source has no value for a particular document, the score for that
   * document is preserved as-is.
   *
   * @param in    the query to boost
   * @param boost a {@link DoubleValuesSource} containing the boost values
   */
  static std::shared_ptr<FunctionScoreQuery>
  boostByValue(std::shared_ptr<Query> in_,
               std::shared_ptr<DoubleValuesSource> boost);

  /**
   * Returns a FunctionScoreQuery where the scores of a wrapped query are
   * multiplied by a boost factor if the document being scored also matches a
   * separate boosting query.
   *
   * Documents that do not match the boosting query have their scores preserved.
   *
   * This may be used to 'demote' documents that match the boosting query, by
   * passing in a boostValue between 0 and 1.
   *
   * @param in          the query to boost
   * @param boostMatch  the boosting query
   * @param boostValue  the amount to boost documents which match the boosting
   * query
   */
  static std::shared_ptr<FunctionScoreQuery>
  boostByQuery(std::shared_ptr<Query> in_, std::shared_ptr<Query> boostMatch,
               float boostValue);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

private:
  class FunctionScoreWeight : public Weight
  {
    GET_CLASS_NAME(FunctionScoreWeight)

  public:
    const std::shared_ptr<Weight> inner;
    const std::shared_ptr<DoubleValuesSource> valueSource;
    const float boost;

    FunctionScoreWeight(std::shared_ptr<Query> query,
                        std::shared_ptr<Weight> inner,
                        std::shared_ptr<DoubleValuesSource> valueSource,
                        float boost);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

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
      std::shared_ptr<FunctionScoreWeight> outerInstance;

      std::shared_ptr<DoubleValues> scores;

    public:
      FilterScorerAnonymousInnerClass(
          std::shared_ptr<FunctionScoreWeight> outerInstance,
          std::shared_ptr<Scorer> in_, std::shared_ptr<DoubleValues> scores);

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
    std::shared_ptr<FunctionScoreWeight> shared_from_this()
    {
      return std::static_pointer_cast<FunctionScoreWeight>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

private:
  class MultiplicativeBoostValuesSource : public DoubleValuesSource
  {
    GET_CLASS_NAME(MultiplicativeBoostValuesSource)

  private:
    const std::shared_ptr<DoubleValuesSource> boost;

    MultiplicativeBoostValuesSource(std::shared_ptr<DoubleValuesSource> boost);

  public:
    std::shared_ptr<DoubleValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class DoubleValuesAnonymousInnerClass : public DoubleValues
    {
      GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
    private:
      std::shared_ptr<MultiplicativeBoostValuesSource> outerInstance;

      std::shared_ptr<DoubleValues> scores;
      std::shared_ptr<DoubleValues> in_;

    public:
      DoubleValuesAnonymousInnerClass(
          std::shared_ptr<MultiplicativeBoostValuesSource> outerInstance,
          std::shared_ptr<DoubleValues> scores,
          std::shared_ptr<DoubleValues> in_);

      double doubleValue()  override;

      bool advanceExact(int doc)  override;

    protected:
      std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
            org.apache.lucene.search.DoubleValues::shared_from_this());
      }
    };

  public:
    bool needsScores() override;

    std::shared_ptr<DoubleValuesSource>
    rewrite(std::shared_ptr<IndexSearcher> reader)  override;

    virtual bool equals(std::any o);

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
            std::shared_ptr<Explanation> scoreExplanation) 
        override;

    virtual int hashCode();

    virtual std::wstring toString();

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<MultiplicativeBoostValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<MultiplicativeBoostValuesSource>(
          org.apache.lucene.search.DoubleValuesSource::shared_from_this());
    }
  };

private:
  class QueryBoostValuesSource : public DoubleValuesSource
  {
    GET_CLASS_NAME(QueryBoostValuesSource)

  private:
    const std::shared_ptr<DoubleValuesSource> query;
    const float boost;

  public:
    QueryBoostValuesSource(std::shared_ptr<DoubleValuesSource> query,
                           float boost);

    std::shared_ptr<DoubleValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class DoubleValuesAnonymousInnerClass : public DoubleValues
    {
      GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
    private:
      std::shared_ptr<QueryBoostValuesSource> outerInstance;

      std::shared_ptr<DoubleValues> in_;

    public:
      DoubleValuesAnonymousInnerClass(
          std::shared_ptr<QueryBoostValuesSource> outerInstance,
          std::shared_ptr<DoubleValues> in_);

      double doubleValue() override;

      bool advanceExact(int doc)  override;

    protected:
      std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
            org.apache.lucene.search.DoubleValues::shared_from_this());
      }
    };

  public:
    bool needsScores() override;

    std::shared_ptr<DoubleValuesSource>
    rewrite(std::shared_ptr<IndexSearcher> reader)  override;

    virtual bool equals(std::any o);

    virtual int hashCode();

    virtual std::wstring toString();

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
            std::shared_ptr<Explanation> scoreExplanation) 
        override;

  protected:
    std::shared_ptr<QueryBoostValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<QueryBoostValuesSource>(
          org.apache.lucene.search.DoubleValuesSource::shared_from_this());
    }
  };

protected:
  std::shared_ptr<FunctionScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<FunctionScoreQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function
