using namespace std;

#include "TestCoveringQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using MultiReader = org::apache::lucene::index::MultiReader;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestCoveringQuery::testEquals()
{
  shared_ptr<TermQuery> tq1 =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar"));
  shared_ptr<TermQuery> tq2 =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux"));
  shared_ptr<LongValuesSource> vs = LongValuesSource::fromLongField(L"field");

  shared_ptr<CoveringQuery> q1 =
      make_shared<CoveringQuery>(Arrays::asList(tq1, tq2), vs);
  shared_ptr<CoveringQuery> q2 =
      make_shared<CoveringQuery>(Arrays::asList(tq1, tq2), vs);
  QueryUtils::checkEqual(q1, q2);

  // order does not matter
  shared_ptr<CoveringQuery> q3 =
      make_shared<CoveringQuery>(Arrays::asList(tq2, tq1), vs);
  QueryUtils::checkEqual(q1, q3);

  // values source matters
  shared_ptr<CoveringQuery> q4 = make_shared<CoveringQuery>(
      Arrays::asList(tq2, tq1),
      LongValuesSource::fromLongField(L"other_field"));
  QueryUtils::checkUnequal(q1, q4);

  // duplicates matter
  shared_ptr<CoveringQuery> q5 =
      make_shared<CoveringQuery>(Arrays::asList(tq1, tq1, tq2), vs);
  shared_ptr<CoveringQuery> q6 =
      make_shared<CoveringQuery>(Arrays::asList(tq1, tq2, tq2), vs);
  QueryUtils::checkUnequal(q5, q6);

  // query matters
  shared_ptr<CoveringQuery> q7 =
      make_shared<CoveringQuery>(Arrays::asList(tq1), vs);
  shared_ptr<CoveringQuery> q8 =
      make_shared<CoveringQuery>(Arrays::asList(tq2), vs);
  QueryUtils::checkUnequal(q7, q8);
}

void TestCoveringQuery::testRewrite() 
{
  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(L"foo", L"bar");
  shared_ptr<TermQuery> tq =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar"));
  shared_ptr<LongValuesSource> vs = LongValuesSource::fromIntField(L"field");
  assertEquals(make_shared<CoveringQuery>(Collections::singleton(tq), vs),
               (make_shared<CoveringQuery>(Collections::singleton(pq), vs))
                   ->rewrite(make_shared<MultiReader>()));
}

void TestCoveringQuery::testToString()
{
  shared_ptr<TermQuery> tq1 =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar"));
  shared_ptr<TermQuery> tq2 =
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux"));
  shared_ptr<LongValuesSource> vs = LongValuesSource::fromIntField(L"field");
  shared_ptr<CoveringQuery> q =
      make_shared<CoveringQuery>(Arrays::asList(tq1, tq2), vs);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"CoveringQuery(queries=[foo:bar, foo:quux], "
               L"minimumNumberMatch=long(field))",
               q->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"CoveringQuery(queries=[bar, quux], minimumNumberMatch=long(field))",
      q->toString(L"foo"));
}

void TestCoveringQuery::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  int numDocs = atLeast(200);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    if (random()->nextBoolean()) {
      doc->push_back(make_shared<StringField>(L"field", L"A", Store::NO));
    }
    if (random()->nextBoolean()) {
      doc->push_back(make_shared<StringField>(L"field", L"B", Store::NO));
    }
    if (random()->nextDouble() > 0.9) {
      doc->push_back(make_shared<StringField>(L"field", L"C", Store::NO));
    }
    if (random()->nextDouble() > 0.1) {
      doc->push_back(make_shared<StringField>(L"field", L"D", Store::NO));
    }
    doc->push_back(
        make_shared<NumericDocValuesField>(L"min_match", random()->nextInt(6)));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(r);
  delete w;

  int iters = atLeast(10);
  for (int iter = 0; iter < iters; ++iter) {
    deque<std::shared_ptr<Query>> queries = deque<std::shared_ptr<Query>>();
    if (random()->nextBoolean()) {
      queries.push_back(
          make_shared<TermQuery>(make_shared<Term>(L"field", L"A")));
    }
    if (random()->nextBoolean()) {
      queries.push_back(
          make_shared<TermQuery>(make_shared<Term>(L"field", L"B")));
    }
    if (random()->nextBoolean()) {
      queries.push_back(
          make_shared<TermQuery>(make_shared<Term>(L"field", L"C")));
    }
    if (random()->nextBoolean()) {
      queries.push_back(
          make_shared<TermQuery>(make_shared<Term>(L"field", L"D")));
    }
    if (random()->nextBoolean()) {
      queries.push_back(
          make_shared<TermQuery>(make_shared<Term>(L"field", L"E")));
    }

    shared_ptr<Query> q = make_shared<CoveringQuery>(
        queries, LongValuesSource::fromLongField(L"min_match"));
    QueryUtils::check(random(), q, searcher);

    for (int i = 1; i < 4; ++i) {
      shared_ptr<BooleanQuery::Builder> builder =
          (make_shared<BooleanQuery::Builder>())
              ->setMinimumNumberShouldMatch(i);
      for (auto query : queries) {
        builder->add(query, Occur::SHOULD);
      }
      shared_ptr<Query> q1 = builder->build();
      shared_ptr<Query> q2 =
          make_shared<CoveringQuery>(queries, LongValuesSource::constant(i));
      assertEquals(searcher->count(q1), searcher->count(q2));
    }

    shared_ptr<Query> filtered =
        (make_shared<BooleanQuery::Builder>())
            ->add(q, Occur::MUST)
            ->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"A")),
                  Occur::MUST)
            ->build();
    QueryUtils::check(random(), filtered, searcher);
  }

  delete r;
  delete dir;
}
} // namespace org::apache::lucene::search