using namespace std;

#include "TestDrillDownQuery.h"

namespace org::apache::lucene::facet
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::index::IndexReader> TestDrillDownQuery::reader;
shared_ptr<
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader>
    TestDrillDownQuery::taxo;
shared_ptr<org::apache::lucene::store::Directory> TestDrillDownQuery::dir;
shared_ptr<org::apache::lucene::store::Directory> TestDrillDownQuery::taxoDir;
shared_ptr<FacetsConfig> TestDrillDownQuery::config;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClassDrillDownQueryTest()
// throws Exception
void TestDrillDownQuery::afterClassDrillDownQueryTest() 
{
  IOUtils::close({reader, taxo, dir, taxoDir});
  reader.reset();
  taxo.reset();
  dir.reset();
  taxoDir.reset();
  config.reset();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void
// beforeClassDrillDownQueryTest() throws Exception
void TestDrillDownQuery::beforeClassDrillDownQueryTest() 
{
  dir = newDirectory();
  shared_ptr<Random> r = random();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      r, dir,
      newIndexWriterConfig(
          make_shared<MockAnalyzer>(r, MockTokenizer::KEYWORD, false)));

  taxoDir = newDirectory();
  shared_ptr<TaxonomyWriter> taxoWriter =
      make_shared<DirectoryTaxonomyWriter>(taxoDir);
  config = make_shared<FacetsConfig>();

  // Randomize the per-dim config:
  config->setHierarchical(L"a", random()->nextBoolean());
  config->setMultiValued(L"a", random()->nextBoolean());
  if (random()->nextBoolean()) {
    config->setIndexFieldName(L"a", L"$a");
  }
  config->setRequireDimCount(L"a", true);

  config->setHierarchical(L"b", random()->nextBoolean());
  config->setMultiValued(L"b", random()->nextBoolean());
  if (random()->nextBoolean()) {
    config->setIndexFieldName(L"b", L"$b");
  }
  config->setRequireDimCount(L"b", true);

  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (i % 2 == 0) { // 50
      doc->push_back(
          make_shared<TextField>(L"content", L"foo", Field::Store::NO));
    }
    if (i % 3 == 0) { // 33
      doc->push_back(
          make_shared<TextField>(L"content", L"bar", Field::Store::NO));
    }
    if (i % 4 == 0) { // 25
      if (r->nextBoolean()) {
        doc->push_back(make_shared<FacetField>(L"a", L"1"));
      } else {
        doc->push_back(make_shared<FacetField>(L"a", L"2"));
      }
    }
    if (i % 5 == 0) { // 20
      doc->push_back(make_shared<FacetField>(L"b", L"1"));
    }
    writer->addDocument(config->build(taxoWriter, doc));
  }

  delete taxoWriter;
  reader = writer->getReader();
  delete writer;

  taxo = make_shared<DirectoryTaxonomyReader>(taxoDir);
}

void TestDrillDownQuery::testAndOrs() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // test (a/1 OR a/2) AND b/1
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);
  q->add(L"a", {L"1"});
  q->add(L"a", {L"2"});
  q->add(L"b", {L"1"});
  shared_ptr<TopDocs> docs = searcher->search(q, 100);
  assertEquals(5, docs->totalHits);
}

void TestDrillDownQuery::testQuery() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // Making sure the query yields 25 documents with the facet "a"
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);
  q->add(L"a");
  QueryUtils::check(q);
  shared_ptr<TopDocs> docs = searcher->search(q, 100);
  assertEquals(25, docs->totalHits);

  // Making sure the query yields 5 documents with the facet "b" and the
  // previous (facet "a") query as a base query
  shared_ptr<DrillDownQuery> q2 = make_shared<DrillDownQuery>(config, q);
  q2->add(L"b");
  docs = searcher->search(q2, 100);
  assertEquals(5, docs->totalHits);

  // Making sure that a query of both facet "a" and facet "b" yields 5 results
  shared_ptr<DrillDownQuery> q3 = make_shared<DrillDownQuery>(config);
  q3->add(L"a");
  q3->add(L"b");
  docs = searcher->search(q3, 100);

  assertEquals(5, docs->totalHits);
  // Check that content:foo (which yields 50% results) and facet/b (which yields
  // 20%) would gather together 10 results (10%..)
  shared_ptr<Query> fooQuery =
      make_shared<TermQuery>(make_shared<Term>(L"content", L"foo"));
  shared_ptr<DrillDownQuery> q4 = make_shared<DrillDownQuery>(config, fooQuery);
  q4->add(L"b");
  docs = searcher->search(q4, 100);
  assertEquals(10, docs->totalHits);
}

void TestDrillDownQuery::testQueryImplicitDefaultParams() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // Create the base query to start with
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);
  q->add(L"a");

  // Making sure the query yields 5 documents with the facet "b" and the
  // previous (facet "a") query as a base query
  shared_ptr<DrillDownQuery> q2 = make_shared<DrillDownQuery>(config, q);
  q2->add(L"b");
  shared_ptr<TopDocs> docs = searcher->search(q2, 100);
  assertEquals(5, docs->totalHits);

  // Check that content:foo (which yields 50% results) and facet/b (which yields
  // 20%) would gather together 10 results (10%..)
  shared_ptr<Query> fooQuery =
      make_shared<TermQuery>(make_shared<Term>(L"content", L"foo"));
  shared_ptr<DrillDownQuery> q4 = make_shared<DrillDownQuery>(config, fooQuery);
  q4->add(L"b");
  docs = searcher->search(q4, 100);
  assertEquals(10, docs->totalHits);
}

void TestDrillDownQuery::testZeroLimit() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);
  q->add(L"b", {L"1"});
  int limit = 0;
  shared_ptr<FacetsCollector> facetCollector = make_shared<FacetsCollector>();
  FacetsCollector::search(searcher, q, limit, facetCollector);
  shared_ptr<Facets> facets = getTaxonomyFacetCounts(
      taxo, config, facetCollector, config->getDimConfig(L"b")->indexFieldName);
  assertNotNull(facets->getTopChildren(10, L"b"));
}

void TestDrillDownQuery::testScoring() 
{
  // verify that drill-down queries do not modify scores
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  std::deque<float> scores(reader->maxDoc());

  shared_ptr<Query> q =
      make_shared<TermQuery>(make_shared<Term>(L"content", L"foo"));
  shared_ptr<TopDocs> docs = searcher->search(
      q, reader->maxDoc()); // fetch all available docs to this query
  for (auto sd : docs->scoreDocs) {
    scores[sd->doc] = sd->score;
  }

  // create a drill-down query with category "a", scores should not change
  shared_ptr<DrillDownQuery> q2 = make_shared<DrillDownQuery>(config, q);
  q2->add(L"a");
  docs = searcher->search(
      q2, reader->maxDoc()); // fetch all available docs to this query
  for (auto sd : docs->scoreDocs) {
    assertEquals(L"score of doc=" + to_wstring(sd->doc) + L" modified",
                 scores[sd->doc], sd->score, 0.0f);
  }
}

void TestDrillDownQuery::testScoringNoBaseQuery() 
{
  // verify that drill-down queries (with no base query) returns 0.0 score
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config);
  q->add(L"a");
  shared_ptr<TopDocs> docs = searcher->search(
      q, reader->maxDoc()); // fetch all available docs to this query
  for (auto sd : docs->scoreDocs) {
    assertEquals(0.0f, sd->score, 0.0f);
  }
}

void TestDrillDownQuery::testTermNonDefault()
{
  wstring aField = config->getDimConfig(L"a")->indexFieldName;
  shared_ptr<Term> termA = DrillDownQuery::term(aField, L"a");
  assertEquals(make_shared<Term>(aField, L"a"), termA);

  wstring bField = config->getDimConfig(L"b")->indexFieldName;
  shared_ptr<Term> termB = DrillDownQuery::term(bField, L"b");
  assertEquals(make_shared<Term>(bField, L"b"), termB);
}

void TestDrillDownQuery::testClone() 
{
  shared_ptr<DrillDownQuery> q =
      make_shared<DrillDownQuery>(config, make_shared<MatchAllDocsQuery>());
  q->add(L"a");

  shared_ptr<DrillDownQuery> clone = q->clone();
  clone->add(L"b");

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertFalse(L"query wasn't cloned: source=" + q + L" clone=" + clone,
              q->toString() == clone->toString());
}

void TestDrillDownQuery::testNoDrillDown() 
{
  shared_ptr<Query> base = make_shared<MatchAllDocsQuery>();
  shared_ptr<DrillDownQuery> q = make_shared<DrillDownQuery>(config, base);
  shared_ptr<Query> rewrite = q->rewrite(reader)->rewrite(reader);
  assertEquals(base, rewrite);
}
} // namespace org::apache::lucene::facet