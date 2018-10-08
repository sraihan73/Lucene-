#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::queries::function
{
class ValueSource;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
class Explanation;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::search
{
class ChildScorer;
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
using Explanation = org::apache::lucene::search::Explanation;
using FilterScorer = org::apache::lucene::search::FilterScorer;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

/**
 * Query that is boosted by a ValueSource
 *
 * @deprecated Use {@link FunctionScoreQuery#boostByValue(Query,
 * DoubleValuesSource)}
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public final class BoostedQuery extends
// org.apache.lucene.search.Query
class BoostedQuery final : public Query
{
private:
  const std::shared_ptr<Query> q;
  const std::shared_ptr<ValueSource> boostVal; // optional, can be null

public:
  BoostedQuery(std::shared_ptr<Query> subQuery,
               std::shared_ptr<ValueSource> boostVal);

  std::shared_ptr<Query> getQuery();
  std::shared_ptr<ValueSource> getValueSource();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class BoostedWeight : public Weight
  {
    GET_CLASS_NAME(BoostedWeight)
  private:
    std::shared_ptr<BoostedQuery> outerInstance;

  public:
    std::shared_ptr<Weight> qWeight;
    std::unordered_map fcontext;

    BoostedWeight(std::shared_ptr<BoostedQuery> outerInstance,
                  std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                  float boost) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> readerContext,
            int doc)  override;

  protected:
    std::shared_ptr<BoostedWeight> shared_from_this()
    {
      return std::static_pointer_cast<BoostedWeight>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

private:
  class CustomScorer : public FilterScorer
  {
    GET_CLASS_NAME(CustomScorer)
  private:
    std::shared_ptr<BoostedQuery> outerInstance;

    const std::shared_ptr<BoostedQuery::BoostedWeight> weight;
    const std::shared_ptr<FunctionValues> vals;
    const std::shared_ptr<LeafReaderContext> readerContext;

    CustomScorer(std::shared_ptr<BoostedQuery> outerInstance,
                 std::shared_ptr<LeafReaderContext> readerContext,
                 std::shared_ptr<BoostedQuery::BoostedWeight> w,
                 std::shared_ptr<Scorer> scorer,
                 std::shared_ptr<ValueSource> vs) ;

  public:
    float score()  override;

    std::shared_ptr<std::deque<std::shared_ptr<Scorer::ChildScorer>>>
    getChildren() override;

    virtual std::shared_ptr<Explanation> explain(int doc) ;

  protected:
    std::shared_ptr<CustomScorer> shared_from_this()
    {
      return std::static_pointer_cast<CustomScorer>(
          org.apache.lucene.search.FilterScorer::shared_from_this());
    }
  };

public:
  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<BoostedQuery> other);

public:
  virtual int hashCode();

protected:
  std::shared_ptr<BoostedQuery> shared_from_this()
  {
    return std::static_pointer_cast<BoostedQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function
