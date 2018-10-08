using namespace std;

#include "DrillSideways.h"

namespace org::apache::lucene::facet
{
using SortedSetDocValuesFacetCounts =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetCounts;
using SortedSetDocValuesFacetField =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField;
using SortedSetDocValuesReaderState =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesReaderState;
using FastTaxonomyFacetCounts =
    org::apache::lucene::facet::taxonomy::FastTaxonomyFacetCounts;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using Collector = org::apache::lucene::search::Collector;
using CollectorManager = org::apache::lucene::search::CollectorManager;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using FilterCollector = org::apache::lucene::search::FilterCollector;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MultiCollector = org::apache::lucene::search::MultiCollector;
using MultiCollectorManager =
    org::apache::lucene::search::MultiCollectorManager;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using TopDocs = org::apache::lucene::search::TopDocs;
using TopFieldCollector = org::apache::lucene::search::TopFieldCollector;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using TopScoreDocCollector = org::apache::lucene::search::TopScoreDocCollector;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

DrillSideways::DrillSideways(shared_ptr<IndexSearcher> searcher,
                             shared_ptr<FacetsConfig> config,
                             shared_ptr<TaxonomyReader> taxoReader)
    : DrillSideways(searcher, config, taxoReader, nullptr)
{
}

DrillSideways::DrillSideways(shared_ptr<IndexSearcher> searcher,
                             shared_ptr<FacetsConfig> config,
                             shared_ptr<SortedSetDocValuesReaderState> state)
    : DrillSideways(searcher, config, nullptr, state)
{
}

DrillSideways::DrillSideways(shared_ptr<IndexSearcher> searcher,
                             shared_ptr<FacetsConfig> config,
                             shared_ptr<TaxonomyReader> taxoReader,
                             shared_ptr<SortedSetDocValuesReaderState> state)
    : DrillSideways(searcher, config, taxoReader, state, nullptr)
{
}

DrillSideways::DrillSideways(shared_ptr<IndexSearcher> searcher,
                             shared_ptr<FacetsConfig> config,
                             shared_ptr<TaxonomyReader> taxoReader,
                             shared_ptr<SortedSetDocValuesReaderState> state,
                             shared_ptr<ExecutorService> executor)
    : searcher(searcher), taxoReader(taxoReader), state(state), config(config),
      executor(executor)
{
}

shared_ptr<Facets> DrillSideways::buildFacetsResult(
    shared_ptr<FacetsCollector> drillDowns,
    std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
    std::deque<wstring> &drillSidewaysDims) 
{

  shared_ptr<Facets> drillDownFacets;
  unordered_map<wstring, std::shared_ptr<Facets>> drillSidewaysFacets =
      unordered_map<wstring, std::shared_ptr<Facets>>();

  if (taxoReader != nullptr) {
    drillDownFacets =
        make_shared<FastTaxonomyFacetCounts>(taxoReader, config, drillDowns);
    if (drillSideways.size() > 0) {
      for (int i = 0; i < drillSideways.size(); i++) {
        drillSidewaysFacets.emplace(drillSidewaysDims[i],
                                    make_shared<FastTaxonomyFacetCounts>(
                                        taxoReader, config, drillSideways[i]));
      }
    }
  } else {
    drillDownFacets =
        make_shared<SortedSetDocValuesFacetCounts>(state, drillDowns);
    if (drillSideways.size() > 0) {
      for (int i = 0; i < drillSideways.size(); i++) {
        drillSidewaysFacets.emplace(drillSidewaysDims[i],
                                    make_shared<SortedSetDocValuesFacetCounts>(
                                        state, drillSideways[i]));
      }
    }
  }

  if (drillSidewaysFacets.empty()) {
    return drillDownFacets;
  } else {
    return make_shared<MultiFacets>(drillSidewaysFacets, drillDownFacets);
  }
}

shared_ptr<DrillSidewaysResult>
DrillSideways::search(shared_ptr<DrillDownQuery> query,
                      shared_ptr<Collector> hitCollector) 
{

  unordered_map<wstring, int> drillDownDims = query->getDims();

  shared_ptr<FacetsCollector> drillDownCollector =
      make_shared<FacetsCollector>();

  if (drillDownDims.empty()) {
    // There are no drill-down dims, so there is no
    // drill-sideways to compute:
    searcher->search(query,
                     MultiCollector::wrap({hitCollector, drillDownCollector}));
    return make_shared<DrillSidewaysResult>(
        buildFacetsResult(drillDownCollector, nullptr, nullptr), nullptr);
  }

  shared_ptr<Query> baseQuery = query->getBaseQuery();
  if (baseQuery == nullptr) {
    // TODO: we could optimize this pure-browse case by
    // making a custom scorer instead:
    baseQuery = make_shared<MatchAllDocsQuery>();
  }
  std::deque<std::shared_ptr<Query>> drillDownQueries =
      query->getDrillDownQueries();

  std::deque<std::shared_ptr<FacetsCollector>> drillSidewaysCollectors(
      drillDownDims.size());
  for (int i = 0; i < drillSidewaysCollectors.size(); i++) {
    drillSidewaysCollectors[i] = make_shared<FacetsCollector>();
  }

  shared_ptr<DrillSidewaysQuery> dsq = make_shared<DrillSidewaysQuery>(
      baseQuery, drillDownCollector, drillSidewaysCollectors, drillDownQueries,
      scoreSubDocsAtOnce());
  if (hitCollector->needsScores() == false) {
    // this is a horrible hack in order to make sure IndexSearcher will not
    // attempt to cache the DrillSidewaysQuery
    hitCollector = make_shared<FilterCollectorAnonymousInnerClass>(
        shared_from_this(), hitCollector);
  }
  searcher->search(dsq, hitCollector);

  return make_shared<DrillSidewaysResult>(
      buildFacetsResult(drillDownCollector, drillSidewaysCollectors,
                        drillDownDims.keySet().toArray(
                            std::deque<wstring>(drillDownDims.size()))),
      nullptr);
}

DrillSideways::FilterCollectorAnonymousInnerClass::
    FilterCollectorAnonymousInnerClass(shared_ptr<DrillSideways> outerInstance,
                                       shared_ptr<Collector> hitCollector)
    : org::apache::lucene::search::FilterCollector(hitCollector)
{
  this->outerInstance = outerInstance;
}

bool DrillSideways::FilterCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

shared_ptr<DrillSidewaysResult>
DrillSideways::search(shared_ptr<DrillDownQuery> query,
                      shared_ptr<Query> filter, shared_ptr<FieldDoc> after,
                      int topN, shared_ptr<Sort> sort, bool doDocScores,
                      bool doMaxScore) 
{
  if (filter != nullptr) {
    query = make_shared<DrillDownQuery>(config, filter, query);
  }
  if (sort != nullptr) {
    int limit = searcher->getIndexReader()->maxDoc();
    if (limit == 0) {
      limit = 1; // the collector does not alow numHits = 0
    }
    constexpr int fTopN = min(topN, limit);

    if (executor !=
        nullptr) { // We have an executor, let use the multi-threaded version

      shared_ptr<CollectorManager<std::shared_ptr<TopFieldCollector>,
                                  std::shared_ptr<TopFieldDocs>>> *const
          collectorManager = make_shared<CollectorManagerAnonymousInnerClass>(
              shared_from_this(), after, topN, sort, doDocScores, doMaxScore,
              fTopN);
      shared_ptr<ConcurrentDrillSidewaysResult<std::shared_ptr<TopFieldDocs>>>
          r = search(query, collectorManager);
      return make_shared<DrillSidewaysResult>(r->facets, r->collectorResult);

    } else {

      shared_ptr<TopFieldCollector> *const hitCollector =
          TopFieldCollector::create(sort, fTopN, after, true, doDocScores,
                                    doMaxScore, true);
      shared_ptr<DrillSidewaysResult> r = search(query, hitCollector);
      return make_shared<DrillSidewaysResult>(r->facets,
                                              hitCollector->topDocs());
    }
  } else {
    return search(after, query, topN);
  }
}

DrillSideways::CollectorManagerAnonymousInnerClass::
    CollectorManagerAnonymousInnerClass(shared_ptr<DrillSideways> outerInstance,
                                        shared_ptr<FieldDoc> after, int topN,
                                        shared_ptr<Sort> sort, bool doDocScores,
                                        bool doMaxScore, int fTopN)
{
  this->outerInstance = outerInstance;
  this->after = after;
  this->topN = topN;
  this->sort = sort;
  this->doDocScores = doDocScores;
  this->doMaxScore = doMaxScore;
  this->fTopN = fTopN;
}

shared_ptr<TopFieldCollector>
DrillSideways::CollectorManagerAnonymousInnerClass::newCollector() throw(
    IOException)
{
  return TopFieldCollector::create(sort, fTopN, after, true, doDocScores,
                                   doMaxScore, true);
}

shared_ptr<TopFieldDocs>
DrillSideways::CollectorManagerAnonymousInnerClass::reduce(
    shared_ptr<deque<std::shared_ptr<TopFieldCollector>>>
        collectors) 
{
  std::deque<std::shared_ptr<TopFieldDocs>> topFieldDocs(collectors->size());
  int pos = 0;
  for (auto collector : collectors) {
    topFieldDocs[pos++] = collector->topDocs();
  }
  return TopDocs::merge(sort, topN, topFieldDocs);
}

shared_ptr<DrillSidewaysResult>
DrillSideways::search(shared_ptr<DrillDownQuery> query,
                      int topN) 
{
  return search(nullptr, query, topN);
}

shared_ptr<DrillSidewaysResult>
DrillSideways::search(shared_ptr<ScoreDoc> after,
                      shared_ptr<DrillDownQuery> query,
                      int topN) 
{
  int limit = searcher->getIndexReader()->maxDoc();
  if (limit == 0) {
    limit = 1; // the collector does not alow numHits = 0
  }
  constexpr int fTopN = min(topN, limit);

  if (executor !=
      nullptr) { // We have an executor, let use the multi-threaded version

    shared_ptr<CollectorManager<std::shared_ptr<TopScoreDocCollector>,
                                std::shared_ptr<TopDocs>>> *const
        collectorManager = make_shared<CollectorManagerAnonymousInnerClass2>(
            shared_from_this(), after, topN, fTopN);
    shared_ptr<ConcurrentDrillSidewaysResult<std::shared_ptr<TopDocs>>> r =
        search(query, collectorManager);
    return make_shared<DrillSidewaysResult>(r->facets, r->collectorResult);

  } else {

    shared_ptr<TopScoreDocCollector> hitCollector =
        TopScoreDocCollector::create(topN, after);
    shared_ptr<DrillSidewaysResult> r = search(query, hitCollector);
    return make_shared<DrillSidewaysResult>(r->facets, hitCollector->topDocs());
  }
}

DrillSideways::CollectorManagerAnonymousInnerClass2::
    CollectorManagerAnonymousInnerClass2(
        shared_ptr<DrillSideways> outerInstance, shared_ptr<ScoreDoc> after,
        int topN, int fTopN)
{
  this->outerInstance = outerInstance;
  this->after = after;
  this->topN = topN;
  this->fTopN = fTopN;
}

shared_ptr<TopScoreDocCollector>
DrillSideways::CollectorManagerAnonymousInnerClass2::newCollector() throw(
    IOException)
{
  return TopScoreDocCollector::create(fTopN, after);
}

shared_ptr<TopDocs> DrillSideways::CollectorManagerAnonymousInnerClass2::reduce(
    shared_ptr<deque<std::shared_ptr<TopScoreDocCollector>>>
        collectors) 
{
  std::deque<std::shared_ptr<TopDocs>> topDocs(collectors->size());
  int pos = 0;
  for (auto collector : collectors) {
    topDocs[pos++] = collector->topDocs();
  }
  return TopDocs::merge(topN, topDocs);
}

bool DrillSideways::scoreSubDocsAtOnce() { return false; }

DrillSideways::DrillSidewaysResult::DrillSidewaysResult(
    shared_ptr<Facets> facets, shared_ptr<TopDocs> hits)
    : facets(facets), hits(hits)
{
}

template <typename T1, typename T1>
DrillSideways::CallableCollector::CallableCollector(
    int pos, shared_ptr<IndexSearcher> searcher, shared_ptr<Query> query,
    shared_ptr<CollectorManager<T1>> collectorManager)
    : pos(pos), searcher(searcher), query(query),
      collectorManager(collectorManager)
{
}

shared_ptr<CallableResult>
DrillSideways::CallableCollector::call() 
{
  return make_shared<CallableResult>(pos,
                                     searcher->search(query, collectorManager));
}

DrillSideways::CallableResult::CallableResult(int pos, any result)
    : pos(pos), result(result)
{
}

shared_ptr<DrillDownQuery>
DrillSideways::getDrillDownQuery(shared_ptr<DrillDownQuery> query,
                                 std::deque<std::shared_ptr<Query>> &queries,
                                 const wstring &excludedDimension)
{
  shared_ptr<DrillDownQuery> *const ddl =
      make_shared<DrillDownQuery>(config, query->getBaseQuery());
  query->getDims().forEach([&](dim, pos) {
    if (!dim::equals(excludedDimension)) {
      ddl->add(dim, queries[pos]);
    }
  });
  return ddl->getDims().size() == queries.size() ? nullptr : ddl;
}

template <typename R, typename T1>
shared_ptr<ConcurrentDrillSidewaysResult<R>> DrillSideways::search(
    shared_ptr<DrillDownQuery> query,
    shared_ptr<CollectorManager<T1>> hitCollectorManager) 
{

  const unordered_map<wstring, int> drillDownDims = query->getDims();
  const deque<std::shared_ptr<CallableCollector>> callableCollectors =
      deque<std::shared_ptr<CallableCollector>>(drillDownDims.size() + 1);

  // Add the main DrillDownQuery
  callableCollectors.push_back(make_shared<CallableCollector>(
      -1, searcher, query,
      make_shared<MultiCollectorManager>(make_shared<FacetsCollectorManager>(),
                                         hitCollectorManager)));
  int i = 0;
  std::deque<std::shared_ptr<Query>> filters = query->getDrillDownQueries();
  for (auto dim : drillDownDims) {
    callableCollectors.push_back(make_shared<CallableCollector>(
        i++, searcher, getDrillDownQuery(query, filters, dim.first),
        make_shared<FacetsCollectorManager>()));
  }

  shared_ptr<FacetsCollector> *const mainFacetsCollector;
  std::deque<std::shared_ptr<FacetsCollector>> facetsCollectors(
      drillDownDims.size());
  constexpr R collectorResult;

  try {
    // Run the query pool
    const deque<Future<std::shared_ptr<CallableResult>>> futures =
        executor->invokeAll(callableCollectors);

    // Extract the results
    // C++ TODO: The following line could not be converted:
    final Object[] mainResults = (Object[])futures.get(0).get().result;
    mainFacetsCollector =
        any_cast<std::shared_ptr<FacetsCollector>>(mainResults[0]);
    collectorResult = any_cast<R>(mainResults[1]);
    for (i = 1; i < futures.size(); i++) {
      shared_ptr<CallableResult> *const result = futures[i]->get();
      facetsCollectors[result->pos] =
          std::static_pointer_cast<FacetsCollector>(result->result);
    }
    // Fill the null results with the mainFacetsCollector
    for (i = 0; i < facetsCollectors.size(); i++) {
      if (facetsCollectors[i] == nullptr) {
        facetsCollectors[i] = mainFacetsCollector;
      }
    }

  } catch (const InterruptedException &e) {
    throw make_shared<ThreadInterruptedException>(e);
  } catch (const ExecutionException &e) {
    throw runtime_error(e);
  }

  // build the facets and return the result
  return make_shared<ConcurrentDrillSidewaysResult<R>>(
      buildFacetsResult(mainFacetsCollector, facetsCollectors,
                        drillDownDims.keySet().toArray(
                            std::deque<wstring>(drillDownDims.size()))),
      nullptr, collectorResult);
}
} // namespace org::apache::lucene::facet