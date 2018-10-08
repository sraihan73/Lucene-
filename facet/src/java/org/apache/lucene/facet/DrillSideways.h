#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include  "core/src/java/org/apache/lucene/facet/sortedset/SortedSetDocValuesReaderState.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/Facets.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"
#include  "core/src/java/org/apache/lucene/search/Collector.h"
#include  "core/src/java/org/apache/lucene/facet/DrillDownQuery.h"
#include  "core/src/java/org/apache/lucene/facet/DrillSidewaysResult.h"
#include  "core/src/java/org/apache/lucene/search/FieldDoc.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/search/TopFieldCollector.h"
#include  "core/src/java/org/apache/lucene/search/TopFieldDocs.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/search/TopScoreDocCollector.h"
#include  "core/src/java/org/apache/lucene/facet/CallableResult.h"
namespace org::apache::lucene::search
{
template <typename Ctypename T>
class CollectorManager;
}
namespace org::apache::lucene::facet
{
template <typename R>
class ConcurrentDrillSidewaysResult;
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
namespace org::apache::lucene::facet
{

using SortedSetDocValuesReaderState =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesReaderState;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using Collector = org::apache::lucene::search::Collector;
using CollectorManager = org::apache::lucene::search::CollectorManager;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using TopDocs = org::apache::lucene::search::TopDocs;

/**
 * Computes drill down and sideways counts for the provided
 * {@link DrillDownQuery}.  Drill sideways counts include
 * alternative values/aggregates for the drill-down
 * dimensions so that a dimension does not disappear after
 * the user drills down into it.
 * <p> Use one of the static search
 * methods to do the search, and then get the hits and facet
 * results from the returned {@link DrillSidewaysResult}.
 * <p><b>NOTE</b>: this allocates one {@link
 * FacetsCollector} for each drill-down, plus one.  If your
 * index has high number of facet labels then this will
 * multiply your memory usage.
 *
 * @lucene.experimental
 */
class DrillSideways : public std::enable_shared_from_this<DrillSideways>
{
  GET_CLASS_NAME(DrillSideways)

  /**
   * {@link IndexSearcher} passed to constructor.
   */
protected:
  const std::shared_ptr<IndexSearcher> searcher;

  /**
   * {@link TaxonomyReader} passed to constructor.
   */
  const std::shared_ptr<TaxonomyReader> taxoReader;

  /**
   * {@link SortedSetDocValuesReaderState} passed to
   * constructor; can be null.
   */
  const std::shared_ptr<SortedSetDocValuesReaderState> state;

  /**
   * {@link FacetsConfig} passed to constructor.
   */
  const std::shared_ptr<FacetsConfig> config;

  // These are only used for multi-threaded search
private:
  const std::shared_ptr<ExecutorService> executor;

  /**
   * Create a new {@code DrillSideways} instance.
   */
public:
  DrillSideways(std::shared_ptr<IndexSearcher> searcher,
                std::shared_ptr<FacetsConfig> config,
                std::shared_ptr<TaxonomyReader> taxoReader);

  /**
   * Create a new {@code DrillSideways} instance, assuming the categories were
   * indexed with {@link SortedSetDocValuesFacetField}.
   */
  DrillSideways(std::shared_ptr<IndexSearcher> searcher,
                std::shared_ptr<FacetsConfig> config,
                std::shared_ptr<SortedSetDocValuesReaderState> state);

  /**
   * Create a new {@code DrillSideways} instance, where some
   * dimensions were indexed with {@link
   * SortedSetDocValuesFacetField} and others were indexed
   * with {@link FacetField}.
   */
  DrillSideways(std::shared_ptr<IndexSearcher> searcher,
                std::shared_ptr<FacetsConfig> config,
                std::shared_ptr<TaxonomyReader> taxoReader,
                std::shared_ptr<SortedSetDocValuesReaderState> state);

  /**
   * Create a new {@code DrillSideways} instance, where some
   * dimensions were indexed with {@link
   * SortedSetDocValuesFacetField} and others were indexed
   * with {@link FacetField}.
   * <p>
   * Use this constructor to use the concurrent implementation and/or the
   * CollectorManager
   */
  DrillSideways(std::shared_ptr<IndexSearcher> searcher,
                std::shared_ptr<FacetsConfig> config,
                std::shared_ptr<TaxonomyReader> taxoReader,
                std::shared_ptr<SortedSetDocValuesReaderState> state,
                std::shared_ptr<ExecutorService> executor);

  /**
   * Subclass can override to customize per-dim Facets
   * impl.
   */
protected:
  virtual std::shared_ptr<Facets> buildFacetsResult(
      std::shared_ptr<FacetsCollector> drillDowns,
      std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
      std::deque<std::wstring> &drillSidewaysDims) ;

  /**
   * Search, collecting hits with a {@link Collector}, and
   * computing drill down and sideways counts.
   */
public:
  virtual std::shared_ptr<DrillSidewaysResult>
  search(std::shared_ptr<DrillDownQuery> query,
         std::shared_ptr<Collector> hitCollector) ;

private:
  class FilterCollectorAnonymousInnerClass : public FilterCollector
  {
    GET_CLASS_NAME(FilterCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<DrillSideways> outerInstance;

  public:
    FilterCollectorAnonymousInnerClass(
        std::shared_ptr<DrillSideways> outerInstance,
        std::shared_ptr<Collector> hitCollector);

    bool needsScores() override;

  protected:
    std::shared_ptr<FilterCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterCollectorAnonymousInnerClass>(
          org.apache.lucene.search.FilterCollector::shared_from_this());
    }
  };

  /**
   * Search, sorting by {@link Sort}, and computing
   * drill down and sideways counts.
   */
public:
  virtual std::shared_ptr<DrillSidewaysResult>
  search(std::shared_ptr<DrillDownQuery> query, std::shared_ptr<Query> filter,
         std::shared_ptr<FieldDoc> after, int topN, std::shared_ptr<Sort> sort,
         bool doDocScores, bool doMaxScore) ;

private:
  class CollectorManagerAnonymousInnerClass
      : public std::enable_shared_from_this<
            CollectorManagerAnonymousInnerClass>,
        public CollectorManager<std::shared_ptr<TopFieldCollector>,
                                std::shared_ptr<TopFieldDocs>>
  {
    GET_CLASS_NAME(CollectorManagerAnonymousInnerClass)
  private:
    std::shared_ptr<DrillSideways> outerInstance;

    std::shared_ptr<FieldDoc> after;
    int topN = 0;
    std::shared_ptr<Sort> sort;
    bool doDocScores = false;
    bool doMaxScore = false;
    int fTopN = 0;

  public:
    CollectorManagerAnonymousInnerClass(
        std::shared_ptr<DrillSideways> outerInstance,
        std::shared_ptr<FieldDoc> after, int topN, std::shared_ptr<Sort> sort,
        bool doDocScores, bool doMaxScore, int fTopN);

    std::shared_ptr<TopFieldCollector> newCollector() ;

    std::shared_ptr<TopFieldDocs>
    reduce(std::shared_ptr<std::deque<std::shared_ptr<TopFieldCollector>>>
               collectors) ;
  };

  /**
   * Search, sorting by score, and computing
   * drill down and sideways counts.
   */
public:
  virtual std::shared_ptr<DrillSidewaysResult>
  search(std::shared_ptr<DrillDownQuery> query, int topN) ;

  /**
   * Search, sorting by score, and computing
   * drill down and sideways counts.
   */
  virtual std::shared_ptr<DrillSidewaysResult>
  search(std::shared_ptr<ScoreDoc> after, std::shared_ptr<DrillDownQuery> query,
         int topN) ;

private:
  class CollectorManagerAnonymousInnerClass2
      : public std::enable_shared_from_this<
            CollectorManagerAnonymousInnerClass2>,
        public CollectorManager<std::shared_ptr<TopScoreDocCollector>,
                                std::shared_ptr<TopDocs>>
  {
    GET_CLASS_NAME(CollectorManagerAnonymousInnerClass2)
  private:
    std::shared_ptr<DrillSideways> outerInstance;

    std::shared_ptr<ScoreDoc> after;
    int topN = 0;
    int fTopN = 0;

  public:
    CollectorManagerAnonymousInnerClass2(
        std::shared_ptr<DrillSideways> outerInstance,
        std::shared_ptr<ScoreDoc> after, int topN, int fTopN);

    std::shared_ptr<TopScoreDocCollector> newCollector() ;

    std::shared_ptr<TopDocs>
    reduce(std::shared_ptr<std::deque<std::shared_ptr<TopScoreDocCollector>>>
               collectors) ;
  };

  /**
   * Override this and return true if your collector
   * (e.g., {@code ToParentBlockJoinCollector}) expects all
   * sub-scorers to be positioned on the document being
   * collected.  This will cause some performance loss;
   * default is false.
   */
protected:
  virtual bool scoreSubDocsAtOnce();

  /**
   * Result of a drill sideways search, including the
   * {@link Facets} and {@link TopDocs}.
   */
public:
  class DrillSidewaysResult
      : public std::enable_shared_from_this<DrillSidewaysResult>
  {
    GET_CLASS_NAME(DrillSidewaysResult)
    /**
     * Combined drill down and sideways results.
     */
  public:
    const std::shared_ptr<Facets> facets;

    /**
     * Hits.
     */
    const std::shared_ptr<TopDocs> hits;

    /**
     * Sole constructor.
     */
    DrillSidewaysResult(std::shared_ptr<Facets> facets,
                        std::shared_ptr<TopDocs> hits);
  };

private:
  class CallableCollector
      : public std::enable_shared_from_this<CallableCollector>,
        public Callable<std::shared_ptr<CallableResult>>
  {
    GET_CLASS_NAME(CallableCollector)

  private:
    const int pos;
    const std::shared_ptr<IndexSearcher> searcher;
    const std::shared_ptr<Query> query;
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: private final org.apache.lucene.search.CollectorManager<?,
    // ?> collectorManager;
    const std::shared_ptr < CollectorManager < ?, ? >> collectorManager;

    template <typename T1, typename T1>
    CallableCollector(int pos, std::shared_ptr<IndexSearcher> searcher,
                      std::shared_ptr<Query> query,
                      std::shared_ptr<CollectorManager<T1>> collectorManager);

  public:
    std::shared_ptr<CallableResult> call()  override;
  };

private:
  class CallableResult : public std::enable_shared_from_this<CallableResult>
  {
    GET_CLASS_NAME(CallableResult)

  private:
    const int pos;
    const std::any result;

    CallableResult(int pos, std::any result);
  };

private:
  std::shared_ptr<DrillDownQuery>
  getDrillDownQuery(std::shared_ptr<DrillDownQuery> query,
                    std::deque<std::shared_ptr<Query>> &queries,
                    const std::wstring &excludedDimension);

  /** Runs a search, using a {@link CollectorManager} to gather and merge search
   * results */
public:
  template <typename R, typename T1>
  std::shared_ptr<ConcurrentDrillSidewaysResult<R>>
  search(std::shared_ptr<DrillDownQuery> query,
         std::shared_ptr<CollectorManager<T1>>
             hitCollectorManager) ;

  /**
   * Result of a concurrent drill sideways search, including the
   * {@link Facets} and {@link TopDocs}.
   */
public:
  template <typename R>
  class ConcurrentDrillSidewaysResult : public DrillSidewaysResult
  {
    GET_CLASS_NAME(ConcurrentDrillSidewaysResult)

    /** The merged search results */
  public:
    const R collectorResult;

    /**
     * Sole constructor.
     */
    ConcurrentDrillSidewaysResult(std::shared_ptr<Facets> facets,
                                  std::shared_ptr<TopDocs> hits,
                                  R collectorResult)
        : DrillSidewaysResult(facets, hits), collectorResult(collectorResult)
    {
    }

  protected:
    std::shared_ptr<ConcurrentDrillSidewaysResult> shared_from_this()
    {
      return std::static_pointer_cast<ConcurrentDrillSidewaysResult>(
          DrillSidewaysResult::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/facet/
