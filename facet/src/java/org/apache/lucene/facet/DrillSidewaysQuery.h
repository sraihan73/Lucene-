#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/search/Collector.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/BulkScorer.h"
#include  "core/src/java/org/apache/lucene/facet/DrillSidewaysScorer.h"
#include  "core/src/java/org/apache/lucene/facet/DocsAndCost.h"

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
namespace org::apache::lucene::facet
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Collector = org::apache::lucene::search::Collector;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

/** Only purpose is to punch through and return a
 *  DrillSidewaysScorer*/

// TODO change the way DrillSidewaysScorer is used, this query does not work
// with filter caching
class DrillSidewaysQuery : public Query
{
  GET_CLASS_NAME(DrillSidewaysQuery)
public:
  const std::shared_ptr<Query> baseQuery;
  const std::shared_ptr<Collector> drillDownCollector;
  std::deque<std::shared_ptr<Collector>> const drillSidewaysCollectors;
  std::deque<std::shared_ptr<Query>> const drillDownQueries;
  const bool scoreSubDocsAtOnce;

  DrillSidewaysQuery(
      std::shared_ptr<Query> baseQuery,
      std::shared_ptr<Collector> drillDownCollector,
      std::deque<std::shared_ptr<Collector>> &drillSidewaysCollectors,
      std::deque<std::shared_ptr<Query>> &drillDownQueries,
      bool scoreSubDocsAtOnce);

  std::wstring toString(const std::wstring &field) override;

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
    std::shared_ptr<DrillSidewaysQuery> outerInstance;

    std::shared_ptr<Weight> baseWeight;
    std::deque<std::shared_ptr<Weight>> drillDowns;

  public:
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    WeightAnonymousInnerClass(
        std::shared_ptr<DrillSidewaysQuery> outerInstance,
        std::shared_ptr<org::apache::lucene::facet::DrillSidewaysQuery>
            shared_from_this(),
        std::shared_ptr<Weight> baseWeight,
        std::deque<std::shared_ptr<Weight>> &drillDowns);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;
    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<BulkScorer> bulkScorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ComparatorAnonymousInnerClass
        : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
          public Comparator<std::shared_ptr<DrillSidewaysScorer::DocsAndCost>>
    {
      GET_CLASS_NAME(ComparatorAnonymousInnerClass)
    private:
      std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

    public:
      ComparatorAnonymousInnerClass(
          std::shared_ptr<WeightAnonymousInnerClass> outerInstance);

      int compare(std::shared_ptr<DocsAndCost> o1,
                  std::shared_ptr<DocsAndCost> o2);
    };

  protected:
    std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<WeightAnonymousInnerClass>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

  // TODO: these should do "deeper" equals/hash on the 2-D drillDownTerms array

public:
  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<DrillSidewaysQuery> other);

protected:
  std::shared_ptr<DrillSidewaysQuery> shared_from_this()
  {
    return std::static_pointer_cast<DrillSidewaysQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
