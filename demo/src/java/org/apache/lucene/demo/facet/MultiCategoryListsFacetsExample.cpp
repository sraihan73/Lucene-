using namespace std;

#include "MultiCategoryListsFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using Document = org::apache::lucene::document::Document;
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

MultiCategoryListsFacetsExample::MultiCategoryListsFacetsExample()
{
  config->setIndexFieldName(L"Author", L"author");
  config->setIndexFieldName(L"Publish Date", L"pubdate");
  config->setHierarchical(L"Publish Date", true);
}

void MultiCategoryListsFacetsExample::index() 
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
MultiCategoryListsFacetsExample::search() 
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
  shared_ptr<Facets> author =
      make_shared<FastTaxonomyFacetCounts>(L"author", taxoReader, config, fc);
  results.push_back(author->getTopChildren(10, L"Author"));

  shared_ptr<Facets> pubDate =
      make_shared<FastTaxonomyFacetCounts>(L"pubdate", taxoReader, config, fc);
  results.push_back(pubDate->getTopChildren(10, L"Publish Date"));

  indexReader->close();
  delete taxoReader;

  return results;
}

deque<std::shared_ptr<FacetResult>>
MultiCategoryListsFacetsExample::runSearch() 
{
  index();
  return search();
}

void MultiCategoryListsFacetsExample::main(std::deque<wstring> &args) throw(
    runtime_error)
{
  wcout << L"Facet counting over multiple category lists example:" << endl;
  wcout << L"-----------------------" << endl;
  deque<std::shared_ptr<FacetResult>> results =
      (make_shared<MultiCategoryListsFacetsExample>())->runSearch();
  wcout << L"Author: " << results[0] << endl;
  wcout << L"Publish Date: " << results[1] << endl;
}
} // namespace org::apache::lucene::demo::facet