using namespace std;

#include "RangeFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using Document = org::apache::lucene::document::Document;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using DrillSideways = org::apache::lucene::facet::DrillSideways;
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LongRange = org::apache::lucene::facet::range::LongRange;
using LongRangeFacetCounts =
    org::apache::lucene::facet::range::LongRangeFacetCounts;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

RangeFacetsExample::RangeFacetsExample() {}

void RangeFacetsExample::index() 
{
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      indexDir,
      (make_shared<IndexWriterConfig>(make_shared<WhitespaceAnalyzer>()))
          ->setOpenMode(OpenMode::CREATE));

  // Add documents with a fake timestamp, 1000 sec before
  // "now", 2000 sec before "now", ...:
  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int64_t then = nowSec - i * 1000;
    // Add as doc values field, so we can compute range facets:
    doc->push_back(make_shared<NumericDocValuesField>(L"timestamp", then));
    // Add as numeric field so we can drill-down:
    doc->push_back(make_shared<LongPoint>(L"timestamp", then));
    indexWriter->addDocument(doc);
  }

  // Open near-real-time searcher
  searcher = make_shared<IndexSearcher>(DirectoryReader::open(indexWriter));
  delete indexWriter;
}

shared_ptr<FacetsConfig> RangeFacetsExample::getConfig()
{
  return make_shared<FacetsConfig>();
}

shared_ptr<FacetResult> RangeFacetsExample::search() 
{

  // Aggregates the facet counts
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  // MatchAllDocsQuery is for "browsing" (counts facets
  // for all non-deleted docs in the index); normally
  // you'd use a "normal" query:
  FacetsCollector::search(searcher, make_shared<MatchAllDocsQuery>(), 10, fc);

  shared_ptr<Facets> facets = make_shared<LongRangeFacetCounts>(
      L"timestamp", fc, PAST_HOUR, PAST_SIX_HOURS, PAST_DAY);
  return facets->getTopChildren(10, L"timestamp");
}

shared_ptr<TopDocs>
RangeFacetsExample::drillDown(shared_ptr<LongRange> range) 
{

  // Passing no baseQuery means we drill down on all
  // documents ("browse only"):
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(getConfig());

  q->add(L"timestamp",
         LongPoint::newRangeQuery(L"timestamp", range->min, range->max));
  return searcher->search(q, 10);
}

shared_ptr<DrillSideways::DrillSidewaysResult>
RangeFacetsExample::drillSideways(shared_ptr<LongRange> range) throw(
    IOException)
{
  // Passing no baseQuery means we drill down on all
  // documents ("browse only"):
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(getConfig());
  q->add(L"timestamp",
         LongPoint::newRangeQuery(L"timestamp", range->min, range->max));

  // DrillSideways only handles taxonomy and sorted set drill facets by default;
  // to do range facets we must subclass and override the buildFacetsResult
  // method.
  shared_ptr<DrillSideways::DrillSidewaysResult> result =
      make_shared<DrillSidewaysAnonymousInnerClass>(shared_from_this(),
                                                    searcher, getConfig())
          .search(q, 10);

  return result;
}

RangeFacetsExample::DrillSidewaysAnonymousInnerClass::
    DrillSidewaysAnonymousInnerClass(
        shared_ptr<RangeFacetsExample> outerInstance,
        shared_ptr<IndexSearcher> searcher, shared_ptr<FacetsConfig> getConfig)
    : org::apache::lucene::facet::DrillSideways(searcher, getConfig, nullptr,
                                                nullptr)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Facets>
RangeFacetsExample::DrillSidewaysAnonymousInnerClass::buildFacetsResult(
    shared_ptr<FacetsCollector> drillDowns,
    std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
    std::deque<wstring> &drillSidewaysDims) 
{
  // If we had other dims we would also compute their drill-down or
  // drill-sideways facets here:
  assert(drillSidewaysDims[0] == L"timestamp");
  return make_shared<LongRangeFacetCounts>(
      L"timestamp", drillSideways[0], outerInstance->PAST_HOUR,
      outerInstance->PAST_SIX_HOURS, outerInstance->PAST_DAY);
}

RangeFacetsExample::~RangeFacetsExample()
{
  delete searcher->getIndexReader();
  delete indexDir;
}

void RangeFacetsExample::main(std::deque<wstring> &args) 
{
  shared_ptr<RangeFacetsExample> example = make_shared<RangeFacetsExample>();
  example->index();

  wcout << L"Facet counting example:" << endl;
  wcout << L"-----------------------" << endl;
  wcout << example->search() << endl;

  wcout << L"\n" << endl;
  wcout << L"Facet drill-down example (timestamp/Past six hours):" << endl;
  wcout << L"---------------------------------------------" << endl;
  shared_ptr<TopDocs> hits = example->drillDown(example->PAST_SIX_HOURS);
  wcout << hits->totalHits << L" totalHits" << endl;

  wcout << L"\n" << endl;
  wcout << L"Facet drill-sideways example (timestamp/Past six hours):" << endl;
  wcout << L"---------------------------------------------" << endl;
  shared_ptr<DrillSideways::DrillSidewaysResult> sideways =
      example->drillSideways(example->PAST_SIX_HOURS);
  wcout << sideways->hits->totalHits << L" totalHits" << endl;
  wcout << sideways->facets->getTopChildren(10, L"timestamp") << endl;

  delete example;
}
} // namespace org::apache::lucene::demo::facet