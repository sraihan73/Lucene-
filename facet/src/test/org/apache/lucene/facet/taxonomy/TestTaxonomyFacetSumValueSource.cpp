using namespace std;

#include "TestTaxonomyFacetSumValueSource.h"

namespace org::apache::lucene::facet::taxonomy
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
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
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using FunctionQuery = org::apache::lucene::queries::function::FunctionQuery;
using LongFieldSource =
    org::apache::lucene::queries::function::valuesource::LongFieldSource;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestTaxonomyFacetSumValueSource::testBasic() 
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

  // Reused across documents, to add the necessary facet
  // fields:
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 10));
  doc->push_back(make_shared<FacetField>(L"Author", L"Bob"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 20));
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 30));
  doc->push_back(make_shared<FacetField>(L"Author", L"Lisa"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 40));
  doc->push_back(make_shared<FacetField>(L"Author", L"Susan"));
  writer->addDocument(config->build(taxoWriter, doc));

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 45));
  doc->push_back(make_shared<FacetField>(L"Author", L"Frank"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());
  delete writer;

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);
  delete taxoWriter;

  // Aggregate the facet counts:
  shared_ptr<FacetsCollector> c = make_shared<FacetsCollector>();

  // MatchAllDocsQuery is for "browsing" (counts facets
  // for all non-deleted docs in the index); normally
  // you'd use a "normal" query and one of the
  // Facets.search utility methods:
  searcher->search(make_shared<MatchAllDocsQuery>(), c);

  shared_ptr<TaxonomyFacetSumValueSource> facets =
      make_shared<TaxonomyFacetSumValueSource>(
          taxoReader, make_shared<FacetsConfig>(), c,
          DoubleValuesSource::fromIntField(L"num"));

  // Retrieve & verify results:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=Author path=[] value=145.0 childCount=4\n  Lisa (50.0)\n  Frank "
      L"(45.0)\n  Susan (40.0)\n  Bob (10.0)\n",
      facets->getTopChildren(10, L"Author")->toString());

  delete taxoReader;
  delete searcher->getIndexReader();
  delete dir;
  delete taxoDir;
}

void TestTaxonomyFacetSumValueSource::testSparseFacets() 
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
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 10));
  doc->push_back(make_shared<FacetField>(L"a", L"foo1"));
  writer->addDocument(config->build(taxoWriter, doc));

  if (random()->nextBoolean()) {
    writer->commit();
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 20));
  doc->push_back(make_shared<FacetField>(L"a", L"foo2"));
  doc->push_back(make_shared<FacetField>(L"b", L"bar1"));
  writer->addDocument(config->build(taxoWriter, doc));

  if (random()->nextBoolean()) {
    writer->commit();
  }

  doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 30));
  doc->push_back(make_shared<FacetField>(L"a", L"foo3"));
  doc->push_back(make_shared<FacetField>(L"b", L"bar2"));
  doc->push_back(make_shared<FacetField>(L"c", L"baz1"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());
  delete writer;

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);
  delete taxoWriter;

  shared_ptr<FacetsCollector> c = make_shared<FacetsCollector>();
  searcher->search(make_shared<MatchAllDocsQuery>(), c);

  shared_ptr<TaxonomyFacetSumValueSource> facets =
      make_shared<TaxonomyFacetSumValueSource>(
          taxoReader, make_shared<FacetsConfig>(), c,
          DoubleValuesSource::fromIntField(L"num"));

  // Ask for top 10 labels for any dims that have counts:
  deque<std::shared_ptr<FacetResult>> results = facets->getAllDims(10);

  TestUtil::assertEquals(3, results.size());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"dim=a path=[] value=60.0 childCount=3\n  foo3 "
                         L"(30.0)\n  foo2 (20.0)\n  foo1 (10.0)\n",
                         results[0]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=b path=[] value=50.0 childCount=2\n  bar2 (30.0)\n  bar1 (20.0)\n",
      results[1]->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=c path=[] value=30.0 childCount=1\n  baz1 (30.0)\n",
      results[2]->toString());

  IOUtils::close({searcher->getIndexReader(), taxoReader, dir, taxoDir});
}

void TestTaxonomyFacetSumValueSource::testWrongIndexFieldName() throw(
    runtime_error)
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
  doc->push_back(make_shared<NumericDocValuesField>(L"num", 10));
  doc->push_back(make_shared<FacetField>(L"a", L"foo1"));
  writer->addDocument(config->build(taxoWriter, doc));

  // NRT open
  shared_ptr<IndexSearcher> searcher = newSearcher(writer->getReader());
  delete writer;

  // NRT open
  shared_ptr<TaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);
  delete taxoWriter;

  shared_ptr<FacetsCollector> c = make_shared<FacetsCollector>();
  searcher->search(make_shared<MatchAllDocsQuery>(), c);

  shared_ptr<TaxonomyFacetSumValueSource> facets =
      make_shared<TaxonomyFacetSumValueSource>(
          taxoReader, config, c, DoubleValuesSource::fromIntField(L"num"));

  // Ask for top 10 labels for any dims that have counts:
  deque<std::shared_ptr<FacetResult>> results = facets->getAllDims(10);
  assertTrue(results.empty());

  expectThrows(invalid_argument::typeid,
               [&]() { facets->getSpecificValue(L"a"); });

  expectThrows(invalid_argument::typeid,
               [&]() { facets->getTopChildren(10, L"a"); });

  IOUtils::close({searcher->getIndexReader(), taxoReader, dir, taxoDir});
}

void TestTaxonomyFacetSumValueSource::testSumScoreAggregator() throw(
    runtime_error)
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();

  for (int i = atLeast(30); i > 0; --i) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (random()->nextBoolean()) { // don't match all documents
      doc->push_back(make_shared<StringField>(L"f", L"v", Field::Store::NO));
    }
    doc->push_back(make_shared<FacetField>(L"dim", L"a"));
    iw->addDocument(config->build(taxoWriter, doc));
  }

  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>(true);
  shared_ptr<BoostQuery> csq = make_shared<BoostQuery>(
      make_shared<ConstantScoreQuery>(make_shared<MatchAllDocsQuery>()), 2.0f);

  shared_ptr<TopDocs> td = FacetsCollector::search(newSearcher(r), csq, 10, fc);

  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumValueSource>(
      taxoReader, config, fc, DoubleValuesSource::SCORES);

  int expected = static_cast<int>(td->getMaxScore() * td->totalHits);
  TestUtil::assertEquals(expected,
                         facets->getSpecificValue(L"dim", {L"a"})->intValue());

  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetSumValueSource::testNoScore() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  for (int i = 0; i < 4; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"price", (i + 1)));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<FacetField>(L"a", Integer::toString(i % 2)));
    iw->addDocument(config->build(taxoWriter, doc));
  }

  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
  newSearcher(r)->search(make_shared<MatchAllDocsQuery>(), sfc);
  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumValueSource>(
      taxoReader, config, sfc, DoubleValuesSource::fromLongField(L"price"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=a path=[] value=10.0 childCount=2\n  1 (6.0)\n  0 (4.0)\n",
      facets->getTopChildren(10, L"a")->toString());

  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetSumValueSource::testWithScore() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  for (int i = 0; i < 4; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"price", (i + 1)));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<FacetField>(L"a", Integer::toString(i % 2)));
    iw->addDocument(config->build(taxoWriter, doc));
  }

  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>(true);
  // score documents by their 'price' field - makes asserting the correct counts
  // for the categories easier
  shared_ptr<Query> q =
      make_shared<FunctionQuery>(make_shared<LongFieldSource>(L"price"));
  FacetsCollector::search(newSearcher(r), q, 10, fc);
  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumValueSource>(
      taxoReader, config, fc, DoubleValuesSource::SCORES);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=a path=[] value=10.0 childCount=2\n  1 (6.0)\n  0 (4.0)\n",
      facets->getTopChildren(10, L"a")->toString());

  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetSumValueSource::testRollupValues() 
{
  shared_ptr<Directory> indexDir = newDirectory();
  shared_ptr<Directory> taxoDir = newDirectory();

  shared_ptr<DirectoryTaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      indexDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<FacetsConfig> config = make_shared<FacetsConfig>();
  config->setHierarchical(L"a", true);
  // config.setRequireDimCount("a", true);

  for (int i = 0; i < 4; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"price", (i + 1)));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        make_shared<FacetField>(L"a", Integer::toString(i % 2), L"1"));
    iw->addDocument(config->build(taxoWriter, doc));
  }

  shared_ptr<DirectoryReader> r = DirectoryReader::open(iw);
  shared_ptr<DirectoryTaxonomyReader> taxoReader =
      make_shared<DirectoryTaxonomyReader>(taxoWriter);

  shared_ptr<FacetsCollector> sfc = make_shared<FacetsCollector>();
  newSearcher(r)->search(make_shared<MatchAllDocsQuery>(), sfc);
  shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumValueSource>(
      taxoReader, config, sfc, DoubleValuesSource::fromLongField(L"price"));

  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(
      L"dim=a path=[] value=10.0 childCount=2\n  1 (6.0)\n  0 (4.0)\n",
      facets->getTopChildren(10, L"a")->toString());

  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetSumValueSource::testCountAndSumScore() throw(
    runtime_error)
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

  shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>(true);
  FacetsCollector::search(newSearcher(r), make_shared<MatchAllDocsQuery>(), 10,
                          fc);

  shared_ptr<Facets> facets1 = getTaxonomyFacetCounts(taxoReader, config, fc);
  shared_ptr<Facets> facets2 = make_shared<TaxonomyFacetSumValueSource>(
      make_shared<DocValuesOrdinalsReader>(L"$b"), taxoReader, config, fc,
      DoubleValuesSource::SCORES);

  TestUtil::assertEquals(r->maxDoc(),
                         facets1->getTopChildren(10, L"a").value.intValue());
  assertEquals(r->maxDoc(),
               facets2->getTopChildren(10, L"b").value.doubleValue(), 1E-10);
  delete iw;
  IOUtils::close({taxoWriter, taxoReader, taxoDir, r, indexDir});
}

void TestTaxonomyFacetSumValueSource::testRandom() 
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
    testDoc->value = random()->nextFloat();
    doc->push_back(make_shared<FloatDocValuesField>(L"value", testDoc->value));
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
    shared_ptr<Facets> facets = make_shared<TaxonomyFacetSumValueSource>(
        tr, config, fc, DoubleValuesSource::fromFloatField(L"value"));

    // Slow, yet hopefully bug-free, faceting:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"})
    // java.util.Map<std::wstring,float>[] expectedValues = new
    // java.util.HashMap[numDims];
    std::deque<unordered_map<wstring, float>> expectedValues =
        std::deque<unordered_map>(numDims);
    for (int i = 0; i < numDims; i++) {
      expectedValues[i] = unordered_map<>();
    }

    for (auto doc : testDocs) {
      if (doc->content == searchToken) {
        for (int j = 0; j < numDims; j++) {
          if (doc->dims[j] != L"") {
            optional<float> v = expectedValues[j][doc->dims[j]];
            if (!v) {
              expectedValues[j].emplace(doc->dims[j], doc->value);
            } else {
              expectedValues[j].emplace(doc->dims[j], v + doc->value);
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
      double totValue = 0;
      for (auto ent : expectedValues[i]) {
        labelValues.push_back(
            make_shared<LabelAndValue>(ent.first, ent.second));
        totValue += ent.second;
      }
      sortLabelValues(labelValues);
      if (totValue > 0) {
        expected.push_back(make_shared<FacetResult>(
            L"dim" + to_wstring(i), std::deque<wstring>(0), totValue,
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

    if (VERBOSE) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << L"expected=\n" << expected.toString() << endl;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << L"actual=\n" << actual.toString() << endl;
    }

    assertFloatValuesEquals(expected, actual);
  }

  delete w;
  IOUtils::close({tw, searcher->getIndexReader(), tr, indexDir, taxoDir});
}
} // namespace org::apache::lucene::facet::taxonomy