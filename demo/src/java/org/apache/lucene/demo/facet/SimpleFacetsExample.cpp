using namespace std;

#include "SimpleFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using Document = org::apache::lucene::document::Document;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using DrillSidewaysResult =
    org::apache::lucene::facet::DrillSideways::DrillSidewaysResult;
using DrillSideways = org::apache::lucene::facet::DrillSideways;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using FastTaxonomyFacetCounts =
    org::apache::lucene::facet::taxonomy::FastTaxonomyFacetCounts;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

SimpleFacetsExample::SimpleFacetsExample()
{
  config->setHierarchical(L"Publish Date", true);
}

void SimpleFacetsExample::index() 
{
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      indexDir,
      (make_shared<IndexWriterConfig>(make_shared<WhitespaceAnalyzer>()))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE));

  // Writes facet ords to a separate directory from the main index
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Bob"));
  doc->push_back(
      make_shared<FacetField>(L"Publish Date", L"2010", L"10", L"15"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  doc->push_back(
      make_shared<FacetField>(L"Publish Date", L"2010", L"10", L"20"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"2012", L"1", L"1"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Susan"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"2012", L"1", L"7"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Frank"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"1999", L"5", L"5"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  delete indexWriter;
  delete taxoWriter;
}

deque<std::shared_ptr<FacetResult>>
SimpleFacetsExample::facetsWithSearch() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  // MatchAllDocsQuery is for "browsing" (counts facets
  // for all non-deleted docs in the index); normally
  // you'd use a "normal" query:
  FacetsCollector::search(searcher, make_shared<MatchAllDocsQuery>(), 10, fc);

  // Retrieve results
  deque<std::shared_ptr<FacetResult>> results =
      deque<std::shared_ptr<FacetResult>>();

  // Count both "Publish Date" and "Author" dimensions
  shared_ptr<Facets> facets =
      make_shared<FastTaxonomyFacetCounts>(taxoReader, config, fc);
  results.push_back(facets->getTopChildren(10, L"Author"));
  results.push_back(facets->getTopChildren(10, L"Publish Date"));

  indexReader->close();
  delete taxoReader;

  return results;
}

deque<std::shared_ptr<FacetResult>>
SimpleFacetsExample::facetsOnly() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();

  // MatchAllDocsQuery is for "browsing" (counts facets
  // for all non-deleted docs in the index); normally
  // you'd use a "normal" query:
  searcher->search(make_shared<MatchAllDocsQuery>(), fc);

  // Retrieve results
  deque<std::shared_ptr<FacetResult>> results =
      deque<std::shared_ptr<FacetResult>>();

  // Count both "Publish Date" and "Author" dimensions
  shared_ptr<Facets> facets =
      make_shared<FastTaxonomyFacetCounts>(taxoReader, config, fc);

  results.push_back(facets->getTopChildren(10, L"Author"));
  results.push_back(facets->getTopChildren(10, L"Publish Date"));

  indexReader->close();
  delete taxoReader;

  return results;
}

shared_ptr<FacetResult> SimpleFacetsExample::drillDown() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);

  // Passing no baseQuery means we drill down on all
  // documents ("browse only"):
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);

  // Now user drills down on Publish Date/2010:
  q->add(L"Publish Date", {L"2010"});
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  FacetsCollector::search(searcher, q, 10, fc);

  // Retrieve results
  shared_ptr<Facets> facets =
      make_shared<FastTaxonomyFacetCounts>(taxoReader, config, fc);
  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"Author");

  indexReader->close();
  delete taxoReader;

  return result;
}

deque<std::shared_ptr<FacetResult>>
SimpleFacetsExample::drillSideways() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);

  // Passing no baseQuery means we drill down on all
  // documents ("browse only"):
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);

  // Now user drills down on Publish Date/2010:
  q->add(L"Publish Date", {L"2010"});

  shared_ptr<DrillSideways> ds =
      make_shared<DrillSideways>(searcher, config, taxoReader);
  shared_ptr<DrillSidewaysResult> result = ds->search(q, 10);

  // Retrieve results
  deque<std::shared_ptr<FacetResult>> facets = result->facets->getAllDims(10);

  indexReader->close();
  delete taxoReader;

  return facets;
}

deque<std::shared_ptr<FacetResult>>
SimpleFacetsExample::runFacetOnly() 
{
  index();
  return facetsOnly();
}

deque<std::shared_ptr<FacetResult>>
SimpleFacetsExample::runSearch() 
{
  index();
  return facetsWithSearch();
}

shared_ptr<FacetResult> SimpleFacetsExample::runDrillDown() 
{
  index();
  return drillDown();
}

deque<std::shared_ptr<FacetResult>>
SimpleFacetsExample::runDrillSideways() 
{
  index();
  return drillSideways();
}

void SimpleFacetsExample::main(std::deque<wstring> &args) 
{
  wcout << L"Facet counting example:" << endl;
  wcout << L"-----------------------" << endl;
  shared_ptr<SimpleFacetsExample> example = make_shared<SimpleFacetsExample>();
  deque<std::shared_ptr<FacetResult>> results1 = example->runFacetOnly();
  wcout << L"Author: " << results1[0] << endl;
  wcout << L"Publish Date: " << results1[1] << endl;

  wcout << L"Facet counting example (combined facets and search):" << endl;
  wcout << L"-----------------------" << endl;
  deque<std::shared_ptr<FacetResult>> results = example->runSearch();
  wcout << L"Author: " << results[0] << endl;
  wcout << L"Publish Date: " << results[1] << endl;

  wcout << L"Facet drill-down example (Publish Date/2010):" << endl;
  wcout << L"---------------------------------------------" << endl;
  wcout << L"Author: " << example->runDrillDown() << endl;

  wcout << L"Facet drill-sideways example (Publish Date/2010):" << endl;
  wcout << L"---------------------------------------------" << endl;
  for (auto result : example->runDrillSideways()) {
    wcout << result << endl;
  }
}
} // namespace org::apache::lucene::demo::facet