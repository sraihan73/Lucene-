using namespace std;

#include "TestMultipleIndexFields.h"

namespace org::apache::lucene::facet
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::Test;
std::deque<std::shared_ptr<FacetField>> const
    TestMultipleIndexFields::CATEGORIES =
        std::deque<std::shared_ptr<FacetField>>{
            make_shared<FacetField>(L"Author", L"Mark Twain"),
            make_shared<FacetField>(L"Author", L"Stephen King"),
            make_shared<FacetField>(L"Author", L"Kurt Vonnegut"),
            make_shared<FacetField>(L"Band", L"Rock & Pop", L"The Beatles"),
            make_shared<FacetField>(L"Band", L"Punk", L"The Ramones"),
            make_shared<FacetField>(L"Band", L"Rock & Pop", L"U2"),
            make_shared<FacetField>(L"Band", L"Rock & Pop", L"REM"),
            make_shared<FacetField>(L"Band", L"Rock & Pop",
                                    L"Dave Matthews Band"),
            make_shared<FacetField>(L"Composer", L"Bach")};

shared_ptr<FacetsConfig> TestMultipleIndexFields::getConfig()
{
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setHierarchical(L"Band", true);
  return config;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDefault() throws Exception
void TestMultipleIndexFields::testDefault() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // create and open an index writer
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), indexDir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(
          random(), MockTokenizer::WHITESPACE, false)));
  // create and open a taxonomy writer
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir, OpenMode::CREATE);
  shared_ptr<FacetsConfig> config = getConfig();

  seedIndex(tw, iw, config);

  shared_ptr<IndexReader> ir = iw->getReader();
  tw->commit();

  // prepare index reader and taxonomy.
  shared_ptr<TaxonomyReader> tr = make_shared<DirectoryTaxonomyReader>(taxoDir);

  // prepare searcher to search against
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);

  shared_ptr<FacetsCollector> sfc = performSearch(tr, ir, searcher);

  // Obtain facets results and hand-test them
  assertCorrectResults(getTaxonomyFacetCounts(tr, config, sfc));

  assertOrdinalsExist(L"$facets", ir);

  delete iw;
  IOUtils::close({tr, ir, tw, indexDir, taxoDir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCustom() throws Exception
void TestMultipleIndexFields::testCustom() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // create and open an index writer
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), indexDir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(
          random(), MockTokenizer::WHITESPACE, false)));
  // create and open a taxonomy writer
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir, OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = getConfig();
  config->setIndexFieldName(L"Author", L"$author");
  seedIndex(tw, iw, config);

  shared_ptr<IndexReader> ir = iw->getReader();
  tw->commit();

  // prepare index reader and taxonomy.
  shared_ptr<TaxonomyReader> tr = make_shared<DirectoryTaxonomyReader>(taxoDir);

  // prepare searcher to search against
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);

  shared_ptr<FacetsCollector> sfc = performSearch(tr, ir, searcher);

  unordered_map<wstring, std::shared_ptr<Facets>> facetsMap =
      unordered_map<wstring, std::shared_ptr<Facets>>();
  facetsMap.emplace(L"Author",
                    getTaxonomyFacetCounts(tr, config, sfc, L"$author"));
  shared_ptr<Facets> facets = make_shared<MultiFacets>(
      facetsMap, getTaxonomyFacetCounts(tr, config, sfc));

  // Obtain facets results and hand-test them
  assertCorrectResults(facets);

  assertOrdinalsExist(L"$facets", ir);
  assertOrdinalsExist(L"$author", ir);

  delete iw;
  IOUtils::close({tr, ir, tw, indexDir, taxoDir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testTwoCustomsSameField() throws Exception
void TestMultipleIndexFields::testTwoCustomsSameField() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // create and open an index writer
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), indexDir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(
          random(), MockTokenizer::WHITESPACE, false)));
  // create and open a taxonomy writer
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir, OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = getConfig();
  config->setIndexFieldName(L"Band", L"$music");
  config->setIndexFieldName(L"Composer", L"$music");
  seedIndex(tw, iw, config);

  shared_ptr<IndexReader> ir = iw->getReader();
  tw->commit();

  // prepare index reader and taxonomy.
  shared_ptr<TaxonomyReader> tr = make_shared<DirectoryTaxonomyReader>(taxoDir);

  // prepare searcher to search against
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);

  shared_ptr<FacetsCollector> sfc = performSearch(tr, ir, searcher);

  unordered_map<wstring, std::shared_ptr<Facets>> facetsMap =
      unordered_map<wstring, std::shared_ptr<Facets>>();
  shared_ptr<Facets> facets2 =
      getTaxonomyFacetCounts(tr, config, sfc, L"$music");
  facetsMap.emplace(L"Band", facets2);
  facetsMap.emplace(L"Composer", facets2);
  shared_ptr<Facets> facets = make_shared<MultiFacets>(
      facetsMap, getTaxonomyFacetCounts(tr, config, sfc));

  // Obtain facets results and hand-test them
  assertCorrectResults(facets);

  assertOrdinalsExist(L"$facets", ir);
  assertOrdinalsExist(L"$music", ir);
  assertOrdinalsExist(L"$music", ir);

  delete iw;
  IOUtils::close({tr, ir, tw, indexDir, taxoDir});
}

void TestMultipleIndexFields::assertOrdinalsExist(
    const wstring &field, shared_ptr<IndexReader> ir) 
{
  for (auto context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    if (r->getBinaryDocValues(field) != nullptr) {
      return; // not all segments must have this DocValues
    }
  }
  fail(L"no ordinals found for " + field);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testDifferentFieldsAndText() throws
// Exception
void TestMultipleIndexFields::testDifferentFieldsAndText() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // create and open an index writer
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), indexDir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(
          random(), MockTokenizer::WHITESPACE, false)));
  // create and open a taxonomy writer
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir, OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = getConfig();
  config->setIndexFieldName(L"Band", L"$bands");
  config->setIndexFieldName(L"Composer", L"$composers");
  seedIndex(tw, iw, config);

  shared_ptr<IndexReader> ir = iw->getReader();
  tw->commit();

  // prepare index reader and taxonomy.
  shared_ptr<TaxonomyReader> tr = make_shared<DirectoryTaxonomyReader>(taxoDir);

  // prepare searcher to search against
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);

  shared_ptr<FacetsCollector> sfc = performSearch(tr, ir, searcher);

  unordered_map<wstring, std::shared_ptr<Facets>> facetsMap =
      unordered_map<wstring, std::shared_ptr<Facets>>();
  facetsMap.emplace(L"Band",
                    getTaxonomyFacetCounts(tr, config, sfc, L"$bands"));
  facetsMap.emplace(L"Composer",
                    getTaxonomyFacetCounts(tr, config, sfc, L"$composers"));
  shared_ptr<Facets> facets = make_shared<MultiFacets>(
      facetsMap, getTaxonomyFacetCounts(tr, config, sfc));

  // Obtain facets results and hand-test them
  assertCorrectResults(facets);
  assertOrdinalsExist(L"$facets", ir);
  assertOrdinalsExist(L"$bands", ir);
  assertOrdinalsExist(L"$composers", ir);

  delete iw;
  IOUtils::close({tr, ir, tw, indexDir, taxoDir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSomeSameSomeDifferent() throws Exception
void TestMultipleIndexFields::testSomeSameSomeDifferent() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // create and open an index writer
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), indexDir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(
          random(), MockTokenizer::WHITESPACE, false)));
  // create and open a taxonomy writer
  shared_ptr<TaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir, OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = getConfig();
  config->setIndexFieldName(L"Band", L"$music");
  config->setIndexFieldName(L"Composer", L"$music");
  config->setIndexFieldName(L"Author", L"$literature");
  seedIndex(tw, iw, config);

  shared_ptr<IndexReader> ir = iw->getReader();
  tw->commit();

  // prepare index reader and taxonomy.
  shared_ptr<TaxonomyReader> tr = make_shared<DirectoryTaxonomyReader>(taxoDir);

  // prepare searcher to search against
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);

  shared_ptr<FacetsCollector> sfc = performSearch(tr, ir, searcher);

  unordered_map<wstring, std::shared_ptr<Facets>> facetsMap =
      unordered_map<wstring, std::shared_ptr<Facets>>();
  shared_ptr<Facets> facets2 =
      getTaxonomyFacetCounts(tr, config, sfc, L"$music");
  facetsMap.emplace(L"Band", facets2);
  facetsMap.emplace(L"Composer", facets2);
  facetsMap.emplace(L"Author",
                    getTaxonomyFacetCounts(tr, config, sfc, L"$literature"));
  shared_ptr<Facets> facets = make_shared<MultiFacets>(
      facetsMap, getTaxonomyFacetCounts(tr, config, sfc));

  // Obtain facets results and hand-test them
  assertCorrectResults(facets);
  assertOrdinalsExist(L"$music", ir);
  assertOrdinalsExist(L"$literature", ir);

  delete iw;
  IOUtils::close({tr, ir, iw, tw, indexDir, taxoDir});
}

void TestMultipleIndexFields::assertCorrectResults(
    shared_ptr<Facets> facets) 
{
  assertEquals(5, facets->getSpecificValue(L"Band"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"dim=Band path=[] value=5 childCount=2\n  Rock & Pop (4)\n  Punk (1)\n",
      facets->getTopChildren(10, L"Band")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"dim=Band path=[Rock & Pop] value=4 childCount=4\n  The Beatles (1)\n  "
      L"U2 (1)\n  REM (1)\n  Dave Matthews Band (1)\n",
      facets->getTopChildren(10, L"Band", {L"Rock & Pop"})->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"dim=Author path=[] value=3 childCount=3\n  Mark Twain (1)\n  "
               L"Stephen King (1)\n  Kurt Vonnegut (1)\n",
               facets->getTopChildren(10, L"Author")->toString());
}

shared_ptr<FacetsCollector> TestMultipleIndexFields::performSearch(
    shared_ptr<TaxonomyReader> tr, shared_ptr<IndexReader> ir,
    shared_ptr<IndexSearcher> searcher) 
{
  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
  FacetsCollector::search(searcher, make_shared<MatchAllDocsQuery>(), 10, fc);
  return fc;
}

void TestMultipleIndexFields::seedIndex(
    shared_ptr<TaxonomyWriter> tw, shared_ptr<RandomIndexWriter> iw,
    shared_ptr<FacetsConfig> config) 
{
  for (auto ff : CATEGORIES) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(ff);
    doc->push_back(
        make_shared<TextField>(L"content", L"alpha", Field::Store::YES));
    iw->addDocument(config->build(tw, doc));
  }
}
} // namespace org::apache::lucene::facet