using namespace std;

#include "SimpleSortedSetFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using Document = org::apache::lucene::document::Document;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using DefaultSortedSetDocValuesReaderState =
    org::apache::lucene::facet::sortedset::DefaultSortedSetDocValuesReaderState;
using SortedSetDocValuesFacetCounts =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetCounts;
using SortedSetDocValuesFacetField =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField;
using SortedSetDocValuesReaderState =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesReaderState;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

SimpleSortedSetFacetsExample::SimpleSortedSetFacetsExample() {}

void SimpleSortedSetFacetsExample::index() 
{
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      indexDir,
      (make_shared<IndexWriterConfig>(make_shared<WhitespaceAnalyzer>()))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"Author", L"Bob"));
  doc->push_back(
      make_shared<SortedSetDocValuesFacetField>(L"Publish Year", L"2010"));
  indexWriter->addDocument(config->build(doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"Author", L"Lisa"));
  doc->push_back(
      make_shared<SortedSetDocValuesFacetField>(L"Publish Year", L"2010"));
  indexWriter->addDocument(config->build(doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesFacetField>(L"Author", L"Lisa"));
  doc->push_back(
      make_shared<SortedSetDocValuesFacetField>(L"Publish Year", L"2012"));
  indexWriter->addDocument(config->build(doc));

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedSetDocValuesFacetField>(L"Author", L"Susan"));
  doc->push_back(
      make_shared<SortedSetDocValuesFacetField>(L"Publish Year", L"2012"));
  indexWriter->addDocument(config->build(doc));

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<SortedSetDocValuesFacetField>(L"Author", L"Frank"));
  doc->push_back(
      make_shared<SortedSetDocValuesFacetField>(L"Publish Year", L"1999"));
  indexWriter->addDocument(config->build(doc));

  delete indexWriter;
}

deque<std::shared_ptr<FacetResult>>
SimpleSortedSetFacetsExample::search() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<SortedSetDocValuesReaderState> state =
      make_shared<DefaultSortedSetDocValuesReaderState>(indexReader);

  // Aggregatses the facet counts
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  // MatchAllDocsQuery is for "browsing" (counts facets
  // for all non-deleted docs in the index); normally
  // you'd use a "normal" query:
  FacetsCollector::search(searcher, make_shared<MatchAllDocsQuery>(), 10, fc);

  // Retrieve results
  shared_ptr<Facets> facets =
      make_shared<SortedSetDocValuesFacetCounts>(state, fc);

  deque<std::shared_ptr<FacetResult>> results =
      deque<std::shared_ptr<FacetResult>>();
  results.push_back(facets->getTopChildren(10, L"Author"));
  results.push_back(facets->getTopChildren(10, L"Publish Year"));
  indexReader->close();

  return results;
}

shared_ptr<FacetResult>
SimpleSortedSetFacetsExample::drillDown() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<SortedSetDocValuesReaderState> state =
      make_shared<DefaultSortedSetDocValuesReaderState>(indexReader);

  // Now user drills down on Publish Year/2010:
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);
  q->add(L"Publish Year", {L"2010"});
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  FacetsCollector::search(searcher, q, 10, fc);

  // Retrieve results
  shared_ptr<Facets> facets =
      make_shared<SortedSetDocValuesFacetCounts>(state, fc);
  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"Author");
  indexReader->close();

  return result;
}

deque<std::shared_ptr<FacetResult>>
SimpleSortedSetFacetsExample::runSearch() 
{
  index();
  return search();
}

shared_ptr<FacetResult>
SimpleSortedSetFacetsExample::runDrillDown() 
{
  index();
  return drillDown();
}

void SimpleSortedSetFacetsExample::main(std::deque<wstring> &args) throw(
    runtime_error)
{
  wcout << L"Facet counting example:" << endl;
  wcout << L"-----------------------" << endl;
  shared_ptr<SimpleSortedSetFacetsExample> example =
      make_shared<SimpleSortedSetFacetsExample>();
  deque<std::shared_ptr<FacetResult>> results = example->runSearch();
  wcout << L"Author: " << results[0] << endl;
  wcout << L"Publish Year: " << results[0] << endl;

  wcout << L"\n" << endl;
  wcout << L"Facet drill-down example (Publish Year/2010):" << endl;
  wcout << L"---------------------------------------------" << endl;
  wcout << L"Author: " << example->runDrillDown() << endl;
}
} // namespace org::apache::lucene::demo::facet