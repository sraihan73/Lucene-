using namespace std;

#include "AssociationsFacetsExample.h"

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
using FloatAssociationFacetField =
    org::apache::lucene::facet::taxonomy::FloatAssociationFacetField;
using IntAssociationFacetField =
    org::apache::lucene::facet::taxonomy::IntAssociationFacetField;
using TaxonomyFacetSumFloatAssociations =
    org::apache::lucene::facet::taxonomy::TaxonomyFacetSumFloatAssociations;
using TaxonomyFacetSumIntAssociations =
    org::apache::lucene::facet::taxonomy::TaxonomyFacetSumIntAssociations;
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

AssociationsFacetsExample::AssociationsFacetsExample()
    : config(make_shared<FacetsConfig>())
{
  config->setMultiValued(L"tags", true);
  config->setIndexFieldName(L"tags", L"$tags");
  config->setMultiValued(L"genre", true);
  config->setIndexFieldName(L"genre", L"$genre");
}

void AssociationsFacetsExample::index() 
{
  shared_ptr<IndexWriterConfig> iwc =
      (make_shared<IndexWriterConfig>(make_shared<WhitespaceAnalyzer>()))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE);
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(indexDir, iwc);

  // Writes facet ords to a separate directory from the main index
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);

  shared_ptr<Document> doc = make_shared<Document>();
  // 3 occurrences for tag 'lucene'
  doc->push_back(make_shared<IntAssociationFacetField>(3, L"tags", L"lucene"));
  // 87% confidence level of genre 'computing'
  doc->push_back(
      make_shared<FloatAssociationFacetField>(0.87f, L"genre", L"computing"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  // 1 occurrence for tag 'lucene'
  doc->push_back(make_shared<IntAssociationFacetField>(1, L"tags", L"lucene"));
  // 2 occurrence for tag 'solr'
  doc->push_back(make_shared<IntAssociationFacetField>(2, L"tags", L"solr"));
  // 75% confidence level of genre 'computing'
  doc->push_back(
      make_shared<FloatAssociationFacetField>(0.75f, L"genre", L"computing"));
  // 34% confidence level of genre 'software'
  doc->push_back(
      make_shared<FloatAssociationFacetField>(0.34f, L"genre", L"software"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  delete indexWriter;
  delete taxoWriter;
}

deque<std::shared_ptr<FacetResult>>
AssociationsFacetsExample::sumAssociations() 
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

  shared_ptr<Facets> tags = make_shared<TaxonomyFacetSumIntAssociations>(
      L"$tags", taxoReader, config, fc);
  shared_ptr<Facets> genre = make_shared<TaxonomyFacetSumFloatAssociations>(
      L"$genre", taxoReader, config, fc);

  // Retrieve results
  deque<std::shared_ptr<FacetResult>> results =
      deque<std::shared_ptr<FacetResult>>();
  results.push_back(tags->getTopChildren(10, L"tags"));
  results.push_back(genre->getTopChildren(10, L"genre"));

  indexReader->close();
  delete taxoReader;

  return results;
}

shared_ptr<FacetResult>
AssociationsFacetsExample::drillDown() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);

  // Passing no baseQuery means we drill down on all
  // documents ("browse only"):
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);

  // Now user drills down on Publish Date/2010:
  q->add(L"tags", {L"solr"});
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  FacetsCollector::search(searcher, q, 10, fc);

  // Retrieve results
  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumFloatAssociations>(
      L"$genre", taxoReader, config, fc);
  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"genre");

  indexReader->close();
  delete taxoReader;

  return result;
}

deque<std::shared_ptr<FacetResult>>
AssociationsFacetsExample::runSumAssociations() 
{
  index();
  return sumAssociations();
}

shared_ptr<FacetResult>
AssociationsFacetsExample::runDrillDown() 
{
  index();
  return drillDown();
}

void AssociationsFacetsExample::main(std::deque<wstring> &args) throw(
    runtime_error)
{
  wcout << L"Sum associations example:" << endl;
  wcout << L"-------------------------" << endl;
  deque<std::shared_ptr<FacetResult>> results =
      (make_shared<AssociationsFacetsExample>())->runSumAssociations();
  wcout << L"tags: " << results[0] << endl;
  wcout << L"genre: " << results[1] << endl;
}
} // namespace org::apache::lucene::demo::facet