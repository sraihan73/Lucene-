using namespace std;

#include "TestTaxonomyFacetCounts.h"

namespace org::apache::lucene::facet::taxonomy
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetResult = org::apache::lucene::facet::FacetResult;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using LabelAndValue = org::apache::lucene::facet::LabelAndValue;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using PerFieldSimilarityWrapper =
    org::apache::lucene::search::similarities::PerFieldSimilarityWrapper;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestTaxonomyFacetCounts::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // Writes facet ords to a separate directory from the
  // main index:
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setHierarchical(L"Publish Date", true);

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Bob"));
  doc->push_back(
      make_shared<FacetField>(L"Publish Date", L"2010", L"10", L"15"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  doc->push_back(
      make_shared<FacetField>(L"Publish Date", L"2010", L"10", L"20"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"2012", L"1", L"1"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Susan"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"2012", L"1", L"7"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"Author", L"Frank"));
  doc->push_back(make_shared<FacetField>(L"Publish Date", L"1999", L"5", L"5"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<Facets> facets = getAllFacets(
      FacetsConfig::DEFAULT_INDEX_FIELD_NAME, searcher, taxoReader, config);

  // Publish Date is hierarchical, so we should have loaded all 3 int[]:
  assertTrue(
      (std::static_pointer_cast<TaxonomyFacets>(facets))->siblingsLoaded());
  assertTrue(
      (std::static_pointer_cast<TaxonomyFacets>(facets))->childrenLoaded());

  // Retrieve & verify results:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Publish Date path=[] value=5 childCount=3\n  2010 (2)\n  2012 "
      L"(2)\n  1999 (1)\n",
      facets->getTopChildren(10, L"Publish Date")->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=Author path=[] value=5 childCount=4\n  Lisa "
                         L"(2)\n  Bob (1)\n  Susan (1)\n  Frank (1)\n",
                         facets->getTopChildren(10, L"Author")->toString());

  // Now user drills down on Publish Date/2010:
  shared_ptr<DrillDownQuery> q2 = make_shared<DrillDownQuery>(config);
  q2->add(L"Publish Date", {L"2010"});
  shared_ptr<FacetsCollector> c = make_shared<FacetsCollector>();
  searcher->search(q2, c);
  facets = make_shared<FastTaxonomyFacetCounts>(taxoReader, config, c);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Author path=[] value=2 childCount=2\n  Bob (1)\n  Lisa (1)\n",
      facets->getTopChildren(10, L"Author")->toString());

  TestUtil::assertEquals(1, facets->getSpecificValue(L"Author", {L"Lisa"}));

  assertNull(facets->getTopChildren(10, L"Non exitent dim"));

  // Smoke test PrintTaxonomyStats:
  shared_ptr<ByteArrayOutputStream> bos = make_shared<ByteArrayOutputStream>();
  PrintTaxonomyStats::printStats(
      taxoReader, make_shared<PrintStream>(bos, false, IOUtils::UTF_8), true);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring result = bos->toString(IOUtils::UTF_8);
  assertTrue(
      result.find(L"/Author: 4 immediate children; 5 total categories") !=
      wstring::npos);
  assertTrue(result.find(
                 L"/Publish Date: 3 immediate children; 12 total categories") !=
             wstring::npos);
  // Make sure at least a few nodes of the tree came out:
  assertTrue(result.find(L"  /1999") != wstring::npos);
  assertTrue(result.find(L"  /2012") != wstring::npos);
  assertTrue(result.find(L"      /20") != wstring::npos);

  delete writer;
  IOUtils::close(
      {taxoWriter, searcher->getIndexReader(), taxoReader, taxoDir, dir});
}

void TestTaxonomyFacetCounts::testSparseFacets() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // Writes facet ords to a separate directory from the
  // main index:
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"a", L"foo1"));
  writer->addDocument(config->build(taxoWriter, doc));

  if (random()->nextBoolean()) {
    writer->commit();
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"a", L"foo2"));
  doc->push_back(make_shared<FacetField>(L"b", L"bar1"));
  writer->addDocument(config->build(taxoWriter, doc));

  if (random()->nextBoolean()) {
    writer->commit();
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"a", L"foo3"));
  doc->push_back(make_shared<FacetField>(L"b", L"bar2"));
  doc->push_back(make_shared<FacetField>(L"c", L"baz1"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<Facets> facets = getAllFacets(
      FacetsConfig::DEFAULT_INDEX_FIELD_NAME, searcher, taxoReader, config);

  // Ask for top 10 labels for any dims that have counts:
  deque<std::shared_ptr<FacetResult>> results = facets->getAllDims(10);

  TestUtil::assertEquals(3, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=a path=[] value=3 childCount=3\n  foo1 (1)\n  "
                         L"foo2 (1)\n  foo3 (1)\n",
                         results[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=b path=[] value=2 childCount=2\n  bar1 (1)\n  bar2 (1)\n",
      results[1]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=c path=[] value=1 childCount=1\n  baz1 (1)\n",
                         results[2]->toString());

  delete writer;
  IOUtils::close(
      {taxoWriter, searcher->getIndexReader(), taxoReader, taxoDir, dir});
}

void TestTaxonomyFacetCounts::testWrongIndexFieldName() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  // Writes facet ords to a separate directory from the
  // main index:
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setIndexFieldName(L"a", L"$facets2");
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"a", L"foo1"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<FacetsCollector> c = make_shared<FacetsCollector>();
  searcher->search(make_shared<MatchAllDocsQuery>(), c);

  // Uses default $facets field:
  shared_ptr<Facets> facets;
  if (random()->nextBoolean()) {
    facets = make_shared<FastTaxonomyFacetCounts>(taxoReader, config, c);
  } else {
    shared_ptr<OrdinalsReader> ordsReader =
        make_shared<DocValuesOrdinalsReader>();
    if (random()->nextBoolean()) {
      ordsReader = make_shared<CachedOrdinalsReader>(ordsReader);
    }
    facets =
        make_shared<TaxonomyFacetCounts>(ordsReader, taxoReader, config, c);
  }

  // Ask for top 10 labels for any dims that have counts:
  deque<std::shared_ptr<FacetResult>> results = facets->getAllDims(10);
  assertTrue(results.empty());

  expectThrows(invalid_argument::typeid,
               [&]() { facets->getSpecificValue(L"a"); });

  expectThrows(invalid_argument::typeid,
               [&]() { facets->getTopChildren(10, L"a"); });

  delete writer;
  IOUtils::close(
      {taxoWriter, searcher->getIndexReader(), taxoReader, taxoDir, dir});
}

void TestTaxonomyFacetCounts::testReallyNoNormsForDrillDown() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setSimilarity(make_shared<PerFieldSimilarityWrapperAnonymousInnerClass>(
      shared_from_this()));
  shared_ptr<TaxonomyWriter> taxoWriter = make_shared<DirectoryTaxonomyWriter>(
      taxoDir, IndexWriterConfig::OpenMode::CREATE);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"text", Field::Store::NO));
  doc->push_back(make_shared<FacetField>(L"a", L"path"));
  writer->addDocument(config->build(taxoWriter, doc));
  delete writer;
  IOUtils::close({taxoWriter, dir, taxoDir});
}

TestTaxonomyFacetCounts::PerFieldSimilarityWrapperAnonymousInnerClass::
    PerFieldSimilarityWrapperAnonymousInnerClass(
        shared_ptr<TestTaxonomyFacetCounts> outerInstance)
{
  this->outerInstance = outerInstance;
  sim = make_shared<ClassicSimilarity>();
}

shared_ptr<Similarity>
TestTaxonomyFacetCounts::PerFieldSimilarityWrapperAnonymousInnerClass::get(
    const wstring &name)
{
  TestUtil::assertEquals(L"field", name);
  return sim;
}

void TestTaxonomyFacetCounts::testMultiValuedHierarchy() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setHierarchical(L"a", true);
  config->setMultiValued(L"a", true);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"text", Field::Store::NO));
  doc->push_back(make_shared<FacetField>(L"a", L"path", L"x"));
  doc->push_back(make_shared<FacetField>(L"a", L"path", L"y"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<Facets> facets = getAllFacets(
      FacetsConfig::DEFAULT_INDEX_FIELD_NAME, searcher, taxoReader, config);

  expectThrows(invalid_argument::typeid,
               [&]() { facets->getSpecificValue(L"a"); });

  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"a");
  TestUtil::assertEquals(1, result->labelValues.size());
  TestUtil::assertEquals(1, result->labelValues[0]->value.intValue());

  delete writer;
  IOUtils::close(
      {taxoWriter, searcher->getIndexReader(), taxoReader, dir, taxoDir});
}

void TestTaxonomyFacetCounts::testLabelWithDelimiter() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setMultiValued(L"dim", true);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"text", Field::Store::NO));
  doc->push_back(make_shared<FacetField>(L"dim", L"test\u001Fone"));
  doc->push_back(make_shared<FacetField>(L"dim", L"test\u001Etwo"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<Facets> facets = getAllFacets(
      FacetsConfig::DEFAULT_INDEX_FIELD_NAME, searcher, taxoReader, config);

  TestUtil::assertEquals(1,
                         facets->getSpecificValue(L"dim", {L"test\u001Fone"}));
  TestUtil::assertEquals(1,
                         facets->getSpecificValue(L"dim", {L"test\u001Etwo"}));

  // no hierarchy
  assertFalse(
      (std::static_pointer_cast<TaxonomyFacets>(facets))->siblingsLoaded());
  assertFalse(
      (std::static_pointer_cast<TaxonomyFacets>(facets))->childrenLoaded());

  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"dim");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=dim path=[] value=-1 childCount=2\n  "
                         L"test\u001Fone (1)\n  test\u001Etwo (1)\n",
                         result->toString());
  delete writer;
  IOUtils::close(
      {taxoWriter, searcher->getIndexReader(), taxoReader, dir, taxoDir});
}

void TestTaxonomyFacetCounts::testRequireDimCount() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setRequireDimCount(L"dim", true);

  config->setMultiValued(L"dim2", true);
  config->setRequireDimCount(L"dim2", true);

  config->setMultiValued(L"dim3", true);
  config->setHierarchical(L"dim3", true);
  config->setRequireDimCount(L"dim3", true);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"text", Field::Store::NO));
  doc->push_back(make_shared<FacetField>(L"dim", L"a"));
  doc->push_back(make_shared<FacetField>(L"dim2", L"a"));
  doc->push_back(make_shared<FacetField>(L"dim2", L"b"));
  doc->push_back(make_shared<FacetField>(L"dim3", L"a", L"b"));
  doc->push_back(make_shared<FacetField>(L"dim3", L"a", L"c"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);
  shared_ptr<Facets> facets = getAllFacets(
      FacetsConfig::DEFAULT_INDEX_FIELD_NAME, searcher, taxoReader, config);

  TestUtil::assertEquals(1, facets->getTopChildren(10, L"dim").value);
  TestUtil::assertEquals(1, facets->getTopChildren(10, L"dim2").value);
  TestUtil::assertEquals(1, facets->getTopChildren(10, L"dim3").value);
  expectThrows(invalid_argument::typeid,
               [&]() { facets->getSpecificValue(L"dim"); });
  TestUtil::assertEquals(1, facets->getSpecificValue(L"dim2"));
  TestUtil::assertEquals(1, facets->getSpecificValue(L"dim3"));
  delete writer;
  IOUtils::close(
      {taxoWriter, searcher->getIndexReader(), taxoReader, dir, taxoDir});
}

void TestTaxonomyFacetCounts::testManyFacetsInOneDocument() 
{
  assumeTrue(L"default Codec doesn't support huge BinaryDocValues",
             TestUtil::fieldSupportsHugeBinaryDocValues(
                 FacetsConfig::DEFAULT_INDEX_FIELD_NAME));
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir,
                                           IndexWriterConfig::OpenMode::CREATE);

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setMultiValued(L"dim", true);

  int numLabels = TestUtil::nextInt(random(), 40000, 100000);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"text", Field::Store::NO));
  for (int i = 0; i < numLabels; i++) {
    doc->push_back(make_shared<FacetField>(L"dim", L"" + to_wstring(i)));
  }
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<Facets> facets = getAllFacets(
      FacetsConfig::DEFAULT_INDEX_FIELD_NAME, searcher, taxoReader, config);

  shared_ptr<FacetResult> result =
      facets->getTopChildren(numeric_limits<int>::max(), L"dim");
  TestUtil::assertEquals(numLabels, result->labelValues.size());
  shared_ptr<Set<wstring>> allLabels = unordered_set<wstring>();
  for (auto labelValue : result->labelValues) {
    allLabels->add(labelValue->label);
    TestUtil::assertEquals(1, labelValue->value->intValue());
  }
  TestUtil::assertEquals(numLabels, allLabels->size());

  delete writer;
  IOUtils::close(
      {searcher->getIndexReader(), taxoWriter, taxoReader, dir, taxoDir});
}

void TestTaxonomyFacetCounts::testDetectHierarchicalField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<TaxonomyWriter> taxoWriter = make_shared<DirectoryTaxonomyWriter>(
      taxoDir, IndexWriterConfig::OpenMode::CREATE);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"text", Field::Store::NO));
  doc->push_back(make_shared<FacetField>(L"a", L"path", L"other"));
  expectThrows(invalid_argument::typeid,
               [&]() { config->build(taxoWriter, doc); });

  delete writer;
  IOUtils::close({taxoWriter, dir, taxoDir});
}

void TestTaxonomyFacetCounts::testDetectMultiValuedField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();
  shared_ptr<TaxonomyWriter> taxoWriter = make_shared<DirectoryTaxonomyWriter>(
      taxoDir, IndexWriterConfig::OpenMode::CREATE);
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"text", Field::Store::NO));
  doc->push_back(make_shared<FacetField>(L"a", L"path"));
  doc->push_back(make_shared<FacetField>(L"a", L"path2"));
  expectThrows(invalid_argument::typeid,
               [&]() { config->build(taxoWriter, doc); });

  delete writer;
  IOUtils::close({taxoWriter, dir, taxoDir});
}

void TestTaxonomyFacetCounts::testSeparateIndexedFields() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setIndexFieldName(L"b", L"$b");

  for (int i = atLeast(30); i > 0; --i) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"f", L"v", Field::Store::NO));
    doc->push_back(make_shared<FacetField>(L"a", L"1"));
    doc->push_back(make_shared<FacetField>(L"b", L"1"));
    iw->addDocument(config->build(taxoWriter, doc));
  }

  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
  newSearcher(r)->search(make_shared<MatchAllDocsQuery>(), sfc);
  shared_ptr<Facets> facets1 = getTaxonomyFacetCounts(taxoReader, config, sfc);
  shared_ptr<Facets> facets2 =
      getTaxonomyFacetCounts(taxoReader, config, sfc, L"$b");
  TestUtil::assertEquals(r->maxDoc(),
                         facets1->getTopChildren(10, L"a").value.intValue());
  TestUtil::assertEquals(r->maxDoc(),
                         facets2->getTopChildren(10, L"b").value.intValue());
  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetCounts::testCountRoot() 
{
  // LUCENE-4882: FacetsAccumulator threw NPE if a FacetRequest was defined on
  // CP.EMPTY
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  for (int i = atLeast(30); i > 0; --i) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<FacetField>(L"a", L"1"));
    doc->push_back(make_shared<FacetField>(L"b", L"1"));
    iw->addDocument(config->build(taxoWriter, doc));
  }

  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<Facets> facets =
      getAllFacets(FacetsConfig::DEFAULT_INDEX_FIELD_NAME, newSearcher(r),
                   taxoReader, config);

  for (auto result : facets->getAllDims(10)) {
    TestUtil::assertEquals(r->numDocs(), result->value->intValue());
  }

  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetCounts::testGetFacetResultsTwice() 
{
  // LUCENE-4893: counts were multiplied as many times as getFacetResults was
  // called.
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FacetField>(L"a", L"1"));
  doc->push_back(make_shared<FacetField>(L"b", L"1"));
  iw->addDocument(config->build(taxoWriter, doc));

  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<Facets> facets =
      getAllFacets(FacetsConfig::DEFAULT_INDEX_FIELD_NAME, newSearcher(r),
                   taxoReader, config);

  deque<std::shared_ptr<FacetResult>> res1 = facets->getAllDims(10);
  deque<std::shared_ptr<FacetResult>> res2 = facets->getAllDims(10);
  assertEquals(
      L"calling getFacetResults twice should return the .equals()=true result",
      res1, res2);

  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetCounts::testChildCount() 
{
  // LUCENE-4885: FacetResult.numValidDescendants was not set properly by
  // FacetsAccumulator
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<FacetField>(L"a", Integer::toString(i)));
    iw->addDocument(config->build(taxoWriter, doc));
  }

  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<Facets> facets =
      getAllFacets(FacetsConfig::DEFAULT_INDEX_FIELD_NAME, newSearcher(r),
                   taxoReader, config);

  TestUtil::assertEquals(10, facets->getTopChildren(2, L"a").childCount);

  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetCounts::indexTwoDocs(
    shared_ptr<TaxonomyWriter> taxoWriter, shared_ptr<IndexWriter> indexWriter,
    shared_ptr<FacetsConfig> config, bool withContent) 
{
  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (withContent) {
      doc->push_back(make_shared<StringField>(L"f", L"a", Field::Store::NO));
    }
    if (config != nullptr) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<FacetField>(L"A", Integer::toString(i)));
      indexWriter->addDocument(config->build(taxoWriter, doc));
    } else {
      indexWriter->addDocument(doc);
    }
  }

  indexWriter->commit();
}

void TestTaxonomyFacetCounts::testSegmentsWithoutCategoriesOrResults() throw(
    runtime_error)
{
  // tests the accumulator when there are segments with no results
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMergePolicy(NoMergePolicy::INSTANCE); // prevent merges
  shared_ptr<IndexWriter> indexWriter = make_shared<IndexWriter>(indexDir, iwc);

  shared_ptr<TaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  indexTwoDocs(taxoWriter, indexWriter, config,
               false); // 1st segment, no content, with categories
  indexTwoDocs(taxoWriter, indexWriter, nullptr,
               true); // 2nd segment, with content, no categories
  indexTwoDocs(taxoWriter, indexWriter, config, true); // 3rd segment ok
  indexTwoDocs(taxoWriter, indexWriter, nullptr,
               false); // 4th segment, no content, or categories
  indexTwoDocs(taxoWriter, indexWriter, nullptr,
               true); // 5th segment, with content, no categories
  indexTwoDocs(taxoWriter, indexWriter, config,
               true); // 6th segment, with content, with categories
  indexTwoDocs(taxoWriter, indexWriter, nullptr,
               true); // 7th segment, with content, no categories
  delete indexWriter;
  IOUtils::close({taxoWriter});

  shared_ptr<DirectoryReader> indexReader = DirectoryReader::open(indexDir);
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoDir);
  shared_ptr<IndexSearcher> indexSearcher = newSearcher(indexReader);

  // search for "f:a", only segments 1 and 3 should match results
  shared_ptr<Query> q = make_shared<TermQuery>(make_shared<Term>(L"f", L"a"));
  shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
  indexSearcher->search(q, sfc);
  shared_ptr<Facets> facets = getTaxonomyFacetCounts(taxoReader, config, sfc);
  shared_ptr<FacetResult> result = facets->getTopChildren(10, L"A");
  assertEquals(L"wrong number of children", 2, result->labelValues.size());
  for (auto labelValue : result->labelValues) {
    assertEquals(L"wrong weight for child " + labelValue->label, 2,
                 labelValue->value->intValue());
  }

  IOUtils::close({indexReader, taxoReader, indexDir, taxoDir});
}

void TestTaxonomyFacetCounts::testRandom() 
{
  std::deque<wstring> tokens = getRandomTokens(10);
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), indexDir);
  shared_ptr<DirectoryTaxonomyWriter> tw =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  int numDocs = atLeast(1000);
  int numDims = TestUtil::nextInt(random(), 1, 7);
  deque<std::shared_ptr<FacetTestCase::TestDoc>> testDocs =
      getRandomDocs(tokens, numDocs, numDims);
  for (auto testDoc : testDocs) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"content", testDoc->content, Field::Store::NO));
    for (int j = 0; j < numDims; j++) {
      if (testDoc->dims[j] != L"") {
        doc->push_back(
            make_shared<FacetField>(L"dim" + to_wstring(j), testDoc->dims[j]));
      }
    }
    w->addDocument(config->build(tw, doc));
  }

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(w->getReader());

  // NRT open
  shared_ptr<TaxonomyReader> tr = make_shared<DirectoryTaxonomyReader>(tw);

  int iters = atLeast(100);
  for (int iter = 0; iter < iters; iter++) {
    wstring searchToken = tokens[random()->nextInt(tokens.size())];
    if (VERBOSE) {
      wcout << L"\nTEST: iter content=" << searchToken << endl;
    }
    shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
    FacetsCollector::search(
        searcher,
        make_shared<TermQuery>(make_shared<Term>(L"content", searchToken)), 10,
        fc);
    shared_ptr<Facets> facets = getTaxonomyFacetCounts(tr, config, fc);

    // Slow, yet hopefully bug-free, faceting:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"})
    // java.util.Map<std::wstring,int>[] expectedCounts = new
    // java.util.HashMap[numDims];
    std::deque<unordered_map<wstring, int>> expectedCounts =
        std::deque<unordered_map>(numDims);
    for (int i = 0; i < numDims; i++) {
      expectedCounts[i] = unordered_map<>();
    }

    for (auto doc : testDocs) {
      if (doc->content == searchToken) {
        for (int j = 0; j < numDims; j++) {
          if (doc->dims[j] != L"") {
            optional<int> v = expectedCounts[j][doc->dims[j]];
            if (!v) {
              expectedCounts[j].emplace(doc->dims[j], 1);
            } else {
              expectedCounts[j].emplace(doc->dims[j], v.value() + 1);
            }
          }
        }
      }
    }

    deque<std::shared_ptr<FacetResult>> expected =
        deque<std::shared_ptr<FacetResult>>();
    for (int i = 0; i < numDims; i++) {
      deque<std::shared_ptr<LabelAndValue>> labelValues =
          deque<std::shared_ptr<LabelAndValue>>();
      int totCount = 0;
      for (auto ent : expectedCounts[i]) {
        labelValues.push_back(
            make_shared<LabelAndValue>(ent.first, ent.second));
        totCount += ent.second;
      }
      sortLabelValues(labelValues);
      if (totCount > 0) {
        expected.push_back(make_shared<FacetResult>(
            L"dim" + to_wstring(i), std::deque<wstring>(0), totCount,
            labelValues.toArray(std::deque<std::shared_ptr<LabelAndValue>>(
                labelValues.size())),
            labelValues.size()));
      }
    }

    // Sort by highest value, tie break by value:
    sortFacetResults(expected);

    deque<std::shared_ptr<FacetResult>> actual = facets->getAllDims(10);

    // Messy: fixup ties
    sortTies(actual);

    TestUtil::assertEquals(expected, actual);
  }

  delete w;
  IOUtils::close({tw, searcher->getIndexReader(), tr, indexDir, taxoDir});
}

shared_ptr<Facets> TestTaxonomyFacetCounts::getAllFacets(
    const wstring &indexFieldName, shared_ptr<IndexSearcher> searcher,
    shared_ptr<TaxonomyReader> taxoReader,
    shared_ptr<FacetsConfig> config) 
{
  if (random()->nextBoolean()) {
    // Aggregate the facet counts:
    shared_ptr<FacetsCollector> c = make_shared<FacetsCollector>();

    // MatchAllDocsQuery is for "browsing" (counts facets
    // for all non-deleted docs in the index); normally
    // you'd use a "normal" query, and use MultiCollector to
    // wrap collecting the "normal" hits and also facets:
    searcher->search(make_shared<MatchAllDocsQuery>(), c);

    return make_shared<FastTaxonomyFacetCounts>(taxoReader, config, c);
  } else {
    return make_shared<FastTaxonomyFacetCounts>(
        indexFieldName, searcher->getIndexReader(), taxoReader, config);
  }
}
} // namespace org::apache::lucene::facet::taxonomy