using namespace std;

#include "TestDocValuesNumbersQuery.h"

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
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocValuesNumbersQuery::testEquals()
{
  TestUtil::assertEquals(
      make_shared<DocValuesNumbersQuery>(L"field", 17LL, 42LL),
      make_shared<DocValuesNumbersQuery>(L"field", 17LL, 42LL));
  TestUtil::assertEquals(
      make_shared<DocValuesNumbersQuery>(L"field", 17LL, 42LL, 32416190071LL),
      make_shared<DocValuesNumbersQuery>(L"field", 17LL, 32416190071LL, 42LL));
  assertFalse(
      (make_shared<DocValuesNumbersQuery>(L"field", 42LL))
          ->equals(make_shared<DocValuesNumbersQuery>(L"field2", 42LL)));
  assertFalse((make_shared<DocValuesNumbersQuery>(L"field", 17LL, 42LL))
                  ->equals(make_shared<DocValuesNumbersQuery>(L"field", 17LL,
                                                              32416190071LL)));
}

void TestDocValuesNumbersQuery::testDuelTermsQuery() 
{
  constexpr int iters = atLeast(2);
  for (int iter = 0; iter < iters; ++iter) {
    const deque<int64_t> allNumbers = deque<int64_t>();
    constexpr int numNumbers =
        TestUtil::nextInt(random(), 1, 1 << TestUtil::nextInt(random(), 1, 10));
    for (int i = 0; i < numNumbers; ++i) {
      allNumbers.push_back(random()->nextLong());
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      const optional<int64_t> number =
          allNumbers[random()->nextInt(allNumbers.size())];
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(
          make_shared<StringField>(L"text", number.toString(), Store::NO));
      doc->push_back(make_shared<NumericDocValuesField>(L"long", number));
      doc->push_back(
          make_shared<SortedNumericDocValuesField>(L"twolongs", number));
      doc->push_back(
          make_shared<SortedNumericDocValuesField>(L"twolongs", number * 2));
      iw->addDocument(doc);
    }
    if (numNumbers > 1 && random()->nextBoolean()) {
      iw->deleteDocuments(make_shared<TermQuery>(
          make_shared<Term>(L"text", to_wstring(allNumbers[0]))));
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
      constexpr int numQueryNumbers = TestUtil::nextInt(
          random(), 1, 1 << TestUtil::nextInt(random(), 1, 8));
      shared_ptr<Set<int64_t>> queryNumbers = unordered_set<int64_t>();
      shared_ptr<Set<int64_t>> queryNumbersX2 = unordered_set<int64_t>();
      for (int j = 0; j < numQueryNumbers; ++j) {
        optional<int64_t> number =
            allNumbers[random()->nextInt(allNumbers.size())];
        queryNumbers->add(number);
        queryNumbersX2->add(2 * number);
      }
      shared_ptr<BooleanQuery::Builder> *const bq =
          make_shared<BooleanQuery::Builder>();
      for (shared_ptr<> : : optional<int64_t> number : queryNumbers) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        bq->add(make_shared<TermQuery>(
                    make_shared<Term>(L"text", number->toString())),
                Occur::SHOULD);
      }
      shared_ptr<Query> q1 = make_shared<BoostQuery>(
          make_shared<ConstantScoreQuery>(bq->build()), boost);

      shared_ptr<Query> q2 = make_shared<BoostQuery>(
          make_shared<DocValuesNumbersQuery>(L"long", queryNumbers), boost);
      assertSameMatches(searcher, q1, q2, true);

      shared_ptr<Query> q3 = make_shared<BoostQuery>(
          make_shared<DocValuesNumbersQuery>(L"twolongs", queryNumbers), boost);
      assertSameMatches(searcher, q1, q3, true);

      shared_ptr<Query> q4 = make_shared<BoostQuery>(
          make_shared<DocValuesNumbersQuery>(L"twolongs", queryNumbersX2),
          boost);
      assertSameMatches(searcher, q1, q4, true);
    }

    delete reader;
    delete dir;
  }
}

void TestDocValuesNumbersQuery::testApproximation() 
{
  constexpr int iters = atLeast(2);
  for (int iter = 0; iter < iters; ++iter) {
    const deque<int64_t> allNumbers = deque<int64_t>();
    constexpr int numNumbers =
        TestUtil::nextInt(random(), 1, 1 << TestUtil::nextInt(random(), 1, 10));
    for (int i = 0; i < numNumbers; ++i) {
      allNumbers.push_back(random()->nextLong());
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      const optional<int64_t> number =
          allNumbers[random()->nextInt(allNumbers.size())];
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(
          make_shared<StringField>(L"text", number.toString(), Store::NO));
      doc->push_back(make_shared<NumericDocValuesField>(L"long", number));
      iw->addDocument(doc);
    }
    if (numNumbers > 1 && random()->nextBoolean()) {
      iw->deleteDocuments(make_shared<TermQuery>(
          make_shared<Term>(L"text", to_wstring(allNumbers[0]))));
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
      constexpr int numQueryNumbers = TestUtil::nextInt(
          random(), 1, 1 << TestUtil::nextInt(random(), 1, 8));
      shared_ptr<Set<int64_t>> queryNumbers = unordered_set<int64_t>();
      for (int j = 0; j < numQueryNumbers; ++j) {
        queryNumbers->add(allNumbers[random()->nextInt(allNumbers.size())]);
      }
      shared_ptr<BooleanQuery::Builder> *const bq =
          make_shared<BooleanQuery::Builder>();
      for (shared_ptr<> : : optional<int64_t> number : queryNumbers) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        bq->add(make_shared<TermQuery>(
                    make_shared<Term>(L"text", number->toString())),
                Occur::SHOULD);
      }
      shared_ptr<Query> q1 = make_shared<BoostQuery>(
          make_shared<ConstantScoreQuery>(bq->build()), boost);
      shared_ptr<Query> *const q2 = make_shared<BoostQuery>(
          make_shared<DocValuesNumbersQuery>(L"long", queryNumbers), boost);

      shared_ptr<BooleanQuery::Builder> bq1 =
          make_shared<BooleanQuery::Builder>();
      bq1->add(q1, Occur::MUST);
      bq1->add(make_shared<TermQuery>(
                   make_shared<Term>(L"text", to_wstring(allNumbers[0]))),
               Occur::FILTER);

      shared_ptr<BooleanQuery::Builder> bq2 =
          make_shared<BooleanQuery::Builder>();
      bq2->add(q2, Occur::MUST);
      bq2->add(make_shared<TermQuery>(
                   make_shared<Term>(L"text", to_wstring(allNumbers[0]))),
               Occur::FILTER);

      assertSameMatches(searcher, bq1->build(), bq2->build(), true);
    }

    delete reader;
    delete dir;
  }
}

void TestDocValuesNumbersQuery::assertSameMatches(
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