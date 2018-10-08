using namespace std;

#include "TestNormsFieldExistsQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestNormsFieldExistsQuery::testRandom() 
{
  constexpr int iters = atLeast(10);
  for (int iter = 0; iter < iters; ++iter) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      constexpr bool hasValue = random()->nextBoolean();
      if (hasValue) {
        doc->push_back(make_shared<TextField>(L"text1", L"value", Store::NO));
        doc->push_back(
            make_shared<StringField>(L"has_value", L"yes", Store::NO));
      }
      doc->push_back(make_shared<StringField>(
          L"f", random()->nextBoolean() ? L"yes" : L"no", Store::NO));
      iw->addDocument(doc);
    }
    if (random()->nextBoolean()) {
      iw->deleteDocuments(
          make_shared<TermQuery>(make_shared<Term>(L"f", L"no")));
    }
    iw->commit();
    shared_ptr<IndexReader> *const reader = iw->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    delete iw;

    assertSameMatches(
        searcher,
        make_shared<TermQuery>(make_shared<Term>(L"has_value", L"yes")),
        make_shared<NormsFieldExistsQuery>(L"text1"), false);

    delete reader;
    delete dir;
  }
}

void TestNormsFieldExistsQuery::testApproximation() 
{
  constexpr int iters = atLeast(10);
  for (int iter = 0; iter < iters; ++iter) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      constexpr bool hasValue = random()->nextBoolean();
      if (hasValue) {
        doc->push_back(make_shared<TextField>(L"text1", L"value", Store::NO));
        doc->push_back(
            make_shared<StringField>(L"has_value", L"yes", Store::NO));
      }
      doc->push_back(make_shared<StringField>(
          L"f", random()->nextBoolean() ? L"yes" : L"no", Store::NO));
      iw->addDocument(doc);
    }
    if (random()->nextBoolean()) {
      iw->deleteDocuments(
          make_shared<TermQuery>(make_shared<Term>(L"f", L"no")));
    }
    iw->commit();
    shared_ptr<IndexReader> *const reader = iw->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    delete iw;

    shared_ptr<BooleanQuery::Builder> ref =
        make_shared<BooleanQuery::Builder>();
    ref->add(make_shared<TermQuery>(make_shared<Term>(L"f", L"yes")),
             Occur::MUST);
    ref->add(make_shared<TermQuery>(make_shared<Term>(L"has_value", L"yes")),
             Occur::FILTER);

    shared_ptr<BooleanQuery::Builder> bq1 =
        make_shared<BooleanQuery::Builder>();
    bq1->add(make_shared<TermQuery>(make_shared<Term>(L"f", L"yes")),
             Occur::MUST);
    bq1->add(make_shared<NormsFieldExistsQuery>(L"text1"), Occur::FILTER);
    assertSameMatches(searcher, ref->build(), bq1->build(), true);

    delete reader;
    delete dir;
  }
}

void TestNormsFieldExistsQuery::testScore() 
{
  constexpr int iters = atLeast(10);
  for (int iter = 0; iter < iters; ++iter) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      constexpr bool hasValue = random()->nextBoolean();
      if (hasValue) {
        doc->push_back(make_shared<TextField>(L"text1", L"value", Store::NO));
        doc->push_back(
            make_shared<StringField>(L"has_value", L"yes", Store::NO));
      }
      doc->push_back(make_shared<StringField>(
          L"f", random()->nextBoolean() ? L"yes" : L"no", Store::NO));
      iw->addDocument(doc);
    }
    if (random()->nextBoolean()) {
      iw->deleteDocuments(
          make_shared<TermQuery>(make_shared<Term>(L"f", L"no")));
    }
    iw->commit();
    shared_ptr<IndexReader> *const reader = iw->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    delete iw;

    constexpr float boost = random()->nextFloat() * 10;
    shared_ptr<Query> *const ref = make_shared<BoostQuery>(
        make_shared<ConstantScoreQuery>(
            make_shared<TermQuery>(make_shared<Term>(L"has_value", L"yes"))),
        boost);

    shared_ptr<Query> *const q1 = make_shared<BoostQuery>(
        make_shared<NormsFieldExistsQuery>(L"text1"), boost);
    assertSameMatches(searcher, ref, q1, true);

    delete reader;
    delete dir;
  }
}

void TestNormsFieldExistsQuery::testMissingField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  iw->addDocument(make_shared<Document>());
  iw->commit();
  shared_ptr<IndexReader> *const reader = iw->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  delete iw;
  assertEquals(
      0,
      searcher->search(make_shared<NormsFieldExistsQuery>(L"f"), 1)->totalHits);
  delete reader;
  delete dir;
}

void TestNormsFieldExistsQuery::testAllDocsHaveField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"f", L"value", Store::NO));
  iw->addDocument(doc);
  iw->commit();
  shared_ptr<IndexReader> *const reader = iw->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  delete iw;
  assertEquals(
      1,
      searcher->search(make_shared<NormsFieldExistsQuery>(L"f"), 1)->totalHits);
  delete reader;
  delete dir;
}

void TestNormsFieldExistsQuery::testFieldExistsButNoDocsHaveField() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  // 1st segment has the field, but 2nd one does not
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"f", L"value", Store::NO));
  iw->addDocument(doc);
  iw->commit();
  iw->addDocument(make_shared<Document>());
  iw->commit();
  shared_ptr<IndexReader> *const reader = iw->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  delete iw;
  assertEquals(
      1,
      searcher->search(make_shared<NormsFieldExistsQuery>(L"f"), 1)->totalHits);
  delete reader;
  delete dir;
}

void TestNormsFieldExistsQuery::assertSameMatches(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q1,
    shared_ptr<Query> q2, bool scores) 
{
  constexpr int maxDoc = searcher->getIndexReader()->maxDoc();
  shared_ptr<TopDocs> *const td1 =
      searcher->search(q1, maxDoc, scores ? Sort::RELEVANCE : Sort::INDEXORDER);
  shared_ptr<TopDocs> *const td2 =
      searcher->search(q2, maxDoc, scores ? Sort::RELEVANCE : Sort::INDEXORDER);
  assertEquals(td1->totalHits, td2->totalHits);
  for (int i = 0; i < td1->scoreDocs.size(); ++i) {
    assertEquals(td1->scoreDocs[i]->doc, td2->scoreDocs[i]->doc);
    if (scores) {
      assertEquals(td1->scoreDocs[i]->score, td2->scoreDocs[i]->score, 10e-7);
    }
  }
}
} // namespace org::apache::lucene::search