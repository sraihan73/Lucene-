using namespace std;

#include "ExpressionAggregationFacetsExample.h"

namespace org::apache::lucene::demo::facet
{
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using TextField = org::apache::lucene::document::TextField;
using Expression = org::apache::lucene::expressions::Expression;
using SimpleBindings = org::apache::lucene::expressions::SimpleBindings;
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetResult = org::apache::lucene::facet::FacetResult;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using TaxonomyFacetSumValueSource =
    org::apache::lucene::facet::taxonomy::TaxonomyFacetSumValueSource;
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
using SortField = org::apache::lucene::search::SortField;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

ExpressionAggregationFacetsExample::ExpressionAggregationFacetsExample() {}

void ExpressionAggregationFacetsExample::index() 
{
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(
      indexDir,
      (make_shared<IndexWriterConfig>(make_shared<WhitespaceAnalyzer>()))
          ->setOpenMode(IndexWriterConfig::OpenMode::CREATE));

  // Writes facet ords to a separate directory from the main index
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"c", L"foo bar", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 5LL));
  doc->push_back(make_shared<FacetField>(L"A", L"B"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"c", L"foo foo bar", Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"popularity", 3LL));
  doc->push_back(make_shared<FacetField>(L"A", L"C"));
  indexWriter->addDocument(config->build(taxoWriter, doc));

  delete indexWriter;
  delete taxoWriter;
}

shared_ptr<FacetResult>
ExpressionAggregationFacetsExample::search() 
{
  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(indexReader);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);

  // Aggregate categories by an expression that combines the document's score
  // and its popularity field
  shared_ptr<Expression> expr =
      JavascriptCompiler::compile(L"_score * sqrt(popularity)");
  shared_ptr<SimpleBindings> bindings = make_shared<SimpleBindings>();
  bindings->add(make_shared<SortField>(
      L"_score", SortField::Type::SCORE)); // the score of the document
  bindings->add(make_shared<SortField>(
      L"popularity",
      SortField::Type::LONG)); // the value of the 'popularity' field

  // Aggregates the facet values
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>(true);

  // MatchAllDocsQuery is for "browsing" (counts facets
  // for all non-deleted docs in the index); normally
  // you'd use a "normal" query:
  FacetsCollector::search(searcher, make_shared<MatchAllDocsQuery>(), 10, fc);

  // Retrieve results
  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumValueSource>(
      taxoReader, config, fc, expr->getDoubleValuesSource(bindings));
  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"A");

  indexReader->close();
  delete taxoReader;

  return result;
}

shared_ptr<FacetResult>
ExpressionAggregationFacetsExample::runSearch() throw(IOException,
                                                      ParseException)
{
  index();
  return search();
}

void ExpressionAggregationFacetsExample::main(std::deque<wstring> &args) throw(
    runtime_error)
{
  wcout << L"Facet counting example:" << endl;
  wcout << L"-----------------------" << endl;
  shared_ptr<FacetResult> result =
      (make_shared<ExpressionAggregationFacetsExample>())->runSearch();
  wcout << result << endl;
}
} // namespace org::apache::lucene::demo::facet