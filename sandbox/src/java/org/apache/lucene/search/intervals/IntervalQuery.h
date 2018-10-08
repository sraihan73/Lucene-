#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/intervals/IntervalsSource.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
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

namespace org::apache::lucene::search::intervals
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using Similarity = org::apache::lucene::search::similarities::Similarity;

/**
 * A query that retrieves documents containing intervals returned from an
 * {@link IntervalsSource}
 *
 * Static constructor functions for various different sources can be found in
 * the
 * {@link Intervals} class
 */
class IntervalQuery final : public Query
{
  GET_CLASS_NAME(IntervalQuery)

private:
  const std::wstring field;
  const std::shared_ptr<IntervalsSource> intervalsSource;

  /**
   * Create a new IntervalQuery
   * @param field             the field to query
   * @param intervalsSource   an {@link IntervalsSource} to retrieve intervals
   * from
   */
public:
  IntervalQuery(const std::wstring &field,
                std::shared_ptr<IntervalsSource> intervalsSource);

  std::wstring getField();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  std::shared_ptr<Similarity::SimWeight>
  buildSimScorer(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost) ;

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

private:
  class IntervalWeight : public Weight
  {
    GET_CLASS_NAME(IntervalWeight)
  private:
    std::shared_ptr<IntervalQuery> outerInstance;

  public:
    const std::shared_ptr<Similarity::SimWeight> simWeight;
    const std::shared_ptr<Similarity> similarity;
    const bool needsScores;

    IntervalWeight(std::shared_ptr<IntervalQuery> outerInstance,
                   std::shared_ptr<Query> query,
                   std::shared_ptr<Similarity::SimWeight> simWeight,
                   std::shared_ptr<Similarity> similarity, bool needsScores);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<IntervalWeight> shared_from_this()
    {
      return std::static_pointer_cast<IntervalWeight>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<IntervalQuery> shared_from_this()
  {
    return std::static_pointer_cast<IntervalQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/intervals/
