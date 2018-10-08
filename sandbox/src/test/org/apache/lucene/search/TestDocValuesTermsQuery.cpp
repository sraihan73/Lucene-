using namespace std;

#include "TestDocValuesTermsQuery.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocValuesTermsQuery::testEquals()
{
  TestUtil::assertEquals(make_shared<DocValuesTermsQuery>(L"foo", L"bar"),
                         make_shared<DocValuesTermsQuery>(L"foo", L"bar"));
  TestUtil::assertEquals(
      make_shared<DocValuesTermsQuery>(L"foo", L"bar"),
      make_shared<DocValuesTermsQuery>(L"foo", L"bar", L"bar"));
  TestUtil::assertEquals(
      make_shared<DocValuesTermsQuery>(L"foo", L"bar", L"baz"),
      make_shared<DocValuesTermsQuery>(L"foo", L"baz", L"bar"));
  assertFalse((make_shared<DocValuesTermsQuery>(L"foo", L"bar"))
                  ->equals(make_shared<DocValuesTermsQuery>(L"foo2", L"bar")));
  assertFalse((make_shared<DocValuesTermsQuery>(L"foo", L"bar"))
                  ->equals(make_shared<DocValuesTermsQuery>(L"foo", L"baz")));
}

void TestDocValuesTermsQuery::testDuelTermsQuery() 
{
  constexpr int iters = atLeast(2);
  for (int iter = 0; iter < iters; ++iter) {
    const deque<std::shared_ptr<Term>> allTerms =
        deque<std::shared_ptr<Term>>();
    constexpr int numTerms =
        TestUtil::nextInt(random(), 1, 1 << TestUtil::nextInt(random(), 1, 10));
    for (int i = 0; i < numTerms; ++i) {
      const wstring value = TestUtil::randomAnalysisString(random(), 10, true);
      allTerms.push_back(make_shared<Term>(L"f", value));
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      shared_ptr<Term> *const term =
          allTerms[random()->nextInt(allTerms.size())];
      doc->push_back(
          make_shared<StringField>(term->field(), term->text(), Store::NO));
      doc->push_back(make_shared<SortedDocValuesField>(
          term->field(), make_shared<BytesRef>(term->text())));
      iw->addDocument(doc);
    }
    if (numTerms > 1 && random()->nextBoolean()) {
      iw->deleteDocuments(make_shared<TermQuery>(allTerms[0]));
    }
    iw->commit();
    shared_ptr<IndexReader> *const reader = iw->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    delete iw;

    if (reader->numDocs() == 0) {
      // may occasionally happen if all documents got the same term
      IOUtils::close({reader, dir});
      continue;
    }

    for (int i = 0; i < 100; ++i) {
      constexpr float boost = random()->nextFloat() * 10;
      constexpr int numQueryTerms = TestUtil::nextInt(
          random(), 1, 1 << TestUtil::nextInt(random(), 1, 8));
      deque<std::shared_ptr<Term>> queryTerms =
          deque<std::shared_ptr<Term>>();
      for (int j = 0; j < numQueryTerms; ++j) {
        queryTerms.push_back(allTerms[random()->nextInt(allTerms.size())]);
      }
      shared_ptr<BooleanQuery::Builder> *const bq =
          make_shared<BooleanQuery::Builder>();
      for (auto term : queryTerms) {
        bq->add(make_shared<TermQuery>(term), Occur::SHOULD);
      }
      shared_ptr<Query> q1 = make_shared<BoostQuery>(
          make_shared<ConstantScoreQuery>(bq->build()), boost);
      deque<wstring> bytesTerms = deque<wstring>();
      for (auto term : queryTerms) {
        bytesTerms.push_back(term->text());
      }
      shared_ptr<Query> *const q2 = make_shared<BoostQuery>(
          make_shared<DocValuesTermsQuery>(
              L"f", bytesTerms.toArray(std::deque<wstring>(0))),
          boost);
      assertSameMatches(searcher, q1, q2, true);
    }

    delete reader;
    delete dir;
  }
}

void TestDocValuesTermsQuery::testApproximation() 
{
  constexpr int iters = atLeast(2);
  for (int iter = 0; iter < iters; ++iter) {
    const deque<std::shared_ptr<Term>> allTerms =
        deque<std::shared_ptr<Term>>();
    constexpr int numTerms =
        TestUtil::nextInt(random(), 1, 1 << TestUtil::nextInt(random(), 1, 10));
    for (int i = 0; i < numTerms; ++i) {
      const wstring value = TestUtil::randomAnalysisString(random(), 10, true);
      allTerms.push_back(make_shared<Term>(L"f", value));
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      shared_ptr<Term> *const term =
          allTerms[random()->nextInt(allTerms.size())];
      doc->push_back(
          make_shared<StringField>(term->field(), term->text(), Store::NO));
      doc->push_back(make_shared<SortedDocValuesField>(
          term->field(), make_shared<BytesRef>(term->text())));
      iw->addDocument(doc);
    }
    if (numTerms > 1 && random()->nextBoolean()) {
      iw->deleteDocuments(make_shared<TermQuery>(allTerms[0]));
    }
    iw->commit();
    shared_ptr<IndexReader> *const reader = iw->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    delete iw;

    if (reader->numDocs() == 0) {
      // may occasionally happen if all documents got the same term
      IOUtils::close({reader, dir});
      continue;
    }

    for (int i = 0; i < 100; ++i) {
      constexpr float boost = random()->nextFloat() * 10;
      constexpr int numQueryTerms = TestUtil::nextInt(
          random(), 1, 1 << TestUtil::nextInt(random(), 1, 8));
      deque<std::shared_ptr<Term>> queryTerms =
          deque<std::shared_ptr<Term>>();
      for (int j = 0; j < numQueryTerms; ++j) {
        queryTerms.push_back(allTerms[random()->nextInt(allTerms.size())]);
      }
      shared_ptr<BooleanQuery::Builder> *const bq =
          make_shared<BooleanQuery::Builder>();
      for (auto term : queryTerms) {
        bq->add(make_shared<TermQuery>(term), Occur::SHOULD);
      }
      shared_ptr<Query> q1 = make_shared<BoostQuery>(
          make_shared<ConstantScoreQuery>(bq->build()), boost);
      deque<wstring> bytesTerms = deque<wstring>();
      for (auto term : queryTerms) {
        bytesTerms.push_back(term->text());
      }
      shared_ptr<Query> *const q2 = make_shared<BoostQuery>(
          make_shared<DocValuesTermsQuery>(
              L"f", bytesTerms.toArray(std::deque<wstring>(0))),
          boost);

      shared_ptr<BooleanQuery::Builder> bq1 =
          make_shared<BooleanQuery::Builder>();
      bq1->add(q1, Occur::MUST);
      bq1->add(make_shared<TermQuery>(allTerms[0]), Occur::FILTER);

      shared_ptr<BooleanQuery::Builder> bq2 =
          make_shared<BooleanQuery::Builder>();
      bq2->add(q2, Occur::MUST);
      bq2->add(make_shared<TermQuery>(allTerms[0]), Occur::FILTER);

      assertSameMatches(searcher, bq1->build(), bq2->build(), true);
    }

    delete reader;
    delete dir;
  }
}

void TestDocValuesTermsQuery::assertSameMatches(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q1,
    shared_ptr<Query> q2, bool scores) 
{
  constexpr int maxDoc = searcher->getIndexReader()->maxDoc();
  shared_ptr<TopDocs> *const td1 =
      searcher->search(q1, maxDoc, scores ? Sort::RELEVANCE : Sort::INDEXORDER);
  shared_ptr<TopDocs> *const td2 =
      searcher->search(q2, maxDoc, scores ? Sort::RELEVANCE : Sort::INDEXORDER);
  TestUtil::assertEquals(td1->totalHits, td2->totalHits);
  for (int i = 0; i < td1->scoreDocs.size(); ++i) {
    TestUtil::assertEquals(td1->scoreDocs[i]->doc, td2->scoreDocs[i]->doc);
    if (scores) {
      assertEquals(td1->scoreDocs[i]->score, td2->scoreDocs[i]->score, 10e-7);
    }
  }
}
} // namespace org::apache::lucene::search