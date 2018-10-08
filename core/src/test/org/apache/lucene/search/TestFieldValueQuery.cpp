using namespace std;

#include "TestFieldValueQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestFieldValueQuery::testRandom() 
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
        doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 1));
        doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv2", 1));
        doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv2", 2));
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
        make_shared<DocValuesFieldExistsQuery>(L"dv1"), false);
    assertSameMatches(
        searcher,
        make_shared<TermQuery>(make_shared<Term>(L"has_value", L"yes")),
        make_shared<DocValuesFieldExistsQuery>(L"dv2"), false);

    delete reader;
    delete dir;
  }
}

void TestFieldValueQuery::testApproximation() 
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
        doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 1));
        doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv2", 1));
        doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv2", 2));
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
    bq1->add(make_shared<DocValuesFieldExistsQuery>(L"dv1"), Occur::FILTER);
    assertSameMatches(searcher, ref->build(), bq1->build(), true);

    shared_ptr<BooleanQuery::Builder> bq2 =
        make_shared<BooleanQuery::Builder>();
    bq2->add(make_shared<TermQuery>(make_shared<Term>(L"f", L"yes")),
             Occur::MUST);
    bq2->add(make_shared<DocValuesFieldExistsQuery>(L"dv2"), Occur::FILTER);
    assertSameMatches(searcher, ref->build(), bq2->build(), true);

    delete reader;
    delete dir;
  }
}

void TestFieldValueQuery::testScore() 
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
        doc->push_back(make_shared<NumericDocValuesField>(L"dv1", 1));
        doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv2", 1));
        doc->push_back(make_shared<SortedNumericDocValuesField>(L"dv2", 2));
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
        make_shared<DocValuesFieldExistsQuery>(L"dv1"), boost);
    assertSameMatches(searcher, ref, q1, true);

    shared_ptr<Query> *const q2 = make_shared<BoostQuery>(
        make_shared<DocValuesFieldExistsQuery>(L"dv2"), boost);
    assertSameMatches(searcher, ref, q2, true);

    delete reader;
    delete dir;
  }
}

void TestFieldValueQuery::testMissingField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  iw->addDocument(make_shared<Document>());
  iw->commit();
  shared_ptr<IndexReader> *const reader = iw->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  delete iw;
  assertEquals(0,
               searcher->search(make_shared<DocValuesFieldExistsQuery>(L"f"), 1)
                   ->totalHits);
  delete reader;
  delete dir;
}

void TestFieldValueQuery::testAllDocsHaveField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"f", 1));
  iw->addDocument(doc);
  iw->commit();
  shared_ptr<IndexReader> *const reader = iw->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  delete iw;
  assertEquals(1,
               searcher->search(make_shared<DocValuesFieldExistsQuery>(L"f"), 1)
                   ->totalHits);
  delete reader;
  delete dir;
}

void TestFieldValueQuery::testFieldExistsButNoDocsHaveField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  // 1st segment has the field, but 2nd one does not
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<NumericDocValuesField>(L"f", 1));
  iw->addDocument(doc);
  iw->commit();
  iw->addDocument(make_shared<Document>());
  iw->commit();
  shared_ptr<IndexReader> *const reader = iw->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  delete iw;
  assertEquals(1,
               searcher->search(make_shared<DocValuesFieldExistsQuery>(L"f"), 1)
                   ->totalHits);
  delete reader;
  delete dir;
}

void TestFieldValueQuery::assertSameMatches(shared_ptr<IndexSearcher> searcher,
                                            shared_ptr<Query> q1,
                                            shared_ptr<Query> q2,
                                            bool scores) 
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