using namespace std;

#include "TestParallelDrillSideways.h"

namespace org::apache::lucene::facet
{
using SortedSetDocValuesReaderState =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesReaderState;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<java::util::concurrent::ExecutorService>
    TestParallelDrillSideways::executor;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void prepareExecutor()
void TestParallelDrillSideways::prepareExecutor()
{
  executor = Executors::newCachedThreadPool(
      make_shared<NamedThreadFactory>(L"TestParallelDrillSideways"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void shutdownExecutor()
void TestParallelDrillSideways::shutdownExecutor()
{
  executor->shutdown();
  executor.reset();
}

shared_ptr<DrillSideways> TestParallelDrillSideways::getNewDrillSideways(
    shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> config,
    shared_ptr<SortedSetDocValuesReaderState> state)
{
  return make_shared<DrillSideways>(searcher, config, nullptr, state, executor);
}

shared_ptr<DrillSideways> TestParallelDrillSideways::getNewDrillSideways(
    shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> config,
    shared_ptr<TaxonomyReader> taxoReader)
{
  return make_shared<DrillSideways>(searcher, config, taxoReader, nullptr,
                                    executor);
}

shared_ptr<DrillSideways>
TestParallelDrillSideways::getNewDrillSidewaysScoreSubdocsAtOnce(
    shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> config,
    shared_ptr<TaxonomyReader> taxoReader)
{
  return make_shared<DrillSidewaysAnonymousInnerClass>(
      shared_from_this(), searcher, config, taxoReader, executor);
}

TestParallelDrillSideways::DrillSidewaysAnonymousInnerClass::
    DrillSidewaysAnonymousInnerClass(
        shared_ptr<TestParallelDrillSideways> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<org::apache::lucene::facet::FacetsConfig> config,
        shared_ptr<TaxonomyReader> taxoReader,
        shared_ptr<ExecutorService> executor)
    : DrillSideways(searcher, config, taxoReader, nullptr, executor)
{
  this->outerInstance = outerInstance;
}

bool TestParallelDrillSideways::DrillSidewaysAnonymousInnerClass::
    scoreSubDocsAtOnce()
{
  return true;
}

shared_ptr<DrillSideways>
TestParallelDrillSideways::getNewDrillSidewaysBuildFacetsResult(
    shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> config,
    shared_ptr<TaxonomyReader> taxoReader)
{
  return make_shared<DrillSidewaysAnonymousInnerClass2>(
      shared_from_this(), searcher, config, taxoReader, executor);
}

TestParallelDrillSideways::DrillSidewaysAnonymousInnerClass2::
    DrillSidewaysAnonymousInnerClass2(
        shared_ptr<TestParallelDrillSideways> outerInstance,
        shared_ptr<IndexSearcher> searcher,
        shared_ptr<org::apache::lucene::facet::FacetsConfig> config,
        shared_ptr<TaxonomyReader> taxoReader,
        shared_ptr<ExecutorService> executor)
    : DrillSideways(searcher, config, taxoReader, nullptr, executor)
{
  this->outerInstance = outerInstance;
  this->config = config;
  this->taxoReader = taxoReader;
}

shared_ptr<Facets>
TestParallelDrillSideways::DrillSidewaysAnonymousInnerClass2::buildFacetsResult(
    shared_ptr<FacetsCollector> drillDowns,
    std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
    std::deque<wstring> &drillSidewaysDims) 
{
  unordered_map<wstring, std::shared_ptr<Facets>> drillSidewaysFacets =
      unordered_map<wstring, std::shared_ptr<Facets>>();
  shared_ptr<Facets> drillDownFacets =
      outerInstance->getTaxonomyFacetCounts(taxoReader, config, drillDowns);
  if (drillSideways.size() > 0) {
    for (int i = 0; i < drillSideways.size(); i++) {
      drillSidewaysFacets.emplace(drillSidewaysDims[i],
                                  outerInstance->getTaxonomyFacetCounts(
                                      taxoReader, config, drillSideways[i]));
    }
  }

  if (drillSidewaysFacets.empty()) {
    return drillDownFacets;
  } else {
    return make_shared<MultiFacets>(drillSidewaysFacets, drillDownFacets);
  }
}
} // namespace org::apache::lucene::facet