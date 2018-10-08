using namespace std;

#include "BoostingQueryTest.h"

namespace org::apache::lucene::queries
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using QueryUtils = org::apache::lucene::search::QueryUtils;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using BooleanSimilarity =
    org::apache::lucene::search::similarities::BooleanSimilarity;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void BoostingQueryTest::testBoostingQueryEquals()
{
  shared_ptr<TermQuery> q1 =
      make_shared<TermQuery>(make_shared<Term>(L"subject:", L"java"));
  shared_ptr<TermQuery> q2 =
      make_shared<TermQuery>(make_shared<Term>(L"subject:", L"java"));
  assertEquals(L"Two TermQueries with same attributes should be equal", q1, q2);
  shared_ptr<BoostingQuery> bq1 = make_shared<BoostingQuery>(q1, q2, 0.1f);
  QueryUtils::check(bq1);
  shared_ptr<BoostingQuery> bq2 = make_shared<BoostingQuery>(q1, q2, 0.1f);
  assertEquals(L"BoostingQuery with same attributes is not equal", bq1, bq2);
}

void BoostingQueryTest::testRewrite() 
{
  shared_ptr<IndexReader> reader = make_shared<MultiReader>();
  shared_ptr<BoostingQuery> q = make_shared<BoostingQuery>(
      (make_shared<BooleanQuery::Builder>())->build(),
      make_shared<MatchAllDocsQuery>(), 3);
  shared_ptr<Query> rewritten =
      (make_shared<IndexSearcher>(reader))->rewrite(q);
  shared_ptr<Query> expectedRewritten = make_shared<BoostingQuery>(
      make_shared<MatchNoDocsQuery>(), make_shared<MatchAllDocsQuery>(), 3);
  assertEquals(expectedRewritten, rewritten);
  assertSame(rewritten, rewritten->rewrite(reader));
}

void BoostingQueryTest::testQueryScore() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  std::deque<wstring> docs = {L"foo bar", L"foo", L"foobar", L"foo baz"};
  shared_ptr<FieldType> ft = make_shared<FieldType>();
  ft->setStored(false);
  ft->setIndexOptions(IndexOptions::DOCS);
  ft->setOmitNorms(true);
  for (int i = 0; i < docs.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::YES));
    doc->push_back(make_shared<Field>(L"field", docs[i], ft));
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  s->setSimilarity(make_shared<BooleanSimilarity>());
  shared_ptr<BoostingQuery> query = make_shared<BoostingQuery>(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
      make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")), 0.1f);
  shared_ptr<TopDocs> search = s->search(query, 10);
  assertEquals(3LL, search->totalHits);
  assertEquals(3, search->scoreDocs.size());
  assertEquals(1.0f, search->scoreDocs[0]->score, 0.0);
  assertEquals(1.0f, search->scoreDocs[1]->score, 0.0);
  assertEquals(0.1f, search->scoreDocs[2]->score, 0.0);
  QueryUtils::check(random(), query, s);
  IOUtils::close({r, w, dir, analyzer});
}
} // namespace org::apache::lucene::queries