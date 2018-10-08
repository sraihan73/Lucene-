using namespace std;

#include "TestDocValuesQueries.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocValuesQueries::testDuelPointRangeSortedNumericRangeQuery() throw(
    IOException)
{
  doTestDuelPointRangeNumericRangeQuery(true, 1);
}

void TestDocValuesQueries::
    testDuelPointRangeMultivaluedSortedNumericRangeQuery() 
{
  doTestDuelPointRangeNumericRangeQuery(true, 3);
}

void TestDocValuesQueries::testDuelPointRangeNumericRangeQuery() throw(
    IOException)
{
  doTestDuelPointRangeNumericRangeQuery(false, 1);
}

void TestDocValuesQueries::doTestDuelPointRangeNumericRangeQuery(
    bool sortedNumeric, int maxValuesPerDoc) 
{
  constexpr int iters = atLeast(10);
  for (int iter = 0; iter < iters; ++iter) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      constexpr int numValues = TestUtil::nextInt(random(), 0, maxValuesPerDoc);
      for (int j = 0; j < numValues; ++j) {
        constexpr int64_t value = TestUtil::nextLong(random(), -100, 10000);
        if (sortedNumeric) {
          doc->push_back(
              make_shared<SortedNumericDocValuesField>(L"dv", value));
        } else {
          doc->push_back(make_shared<NumericDocValuesField>(L"dv", value));
        }
        doc->push_back(make_shared<LongPoint>(L"idx", value));
      }
      iw->addDocument(doc);
    }
    if (random()->nextBoolean()) {
      iw->deleteDocuments(LongPoint::newRangeQuery(L"idx", 0LL, 10LL));
    }
    shared_ptr<IndexReader> *const reader = iw->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader, false);
    delete iw;

    for (int i = 0; i < 100; ++i) {
      constexpr int64_t min = random()->nextBoolean()
                                    ? numeric_limits<int64_t>::min()
                                    : TestUtil::nextLong(random(), -100, 10000);
      constexpr int64_t max = random()->nextBoolean()
                                    ? numeric_limits<int64_t>::max()
                                    : TestUtil::nextLong(random(), -100, 10000);
      shared_ptr<Query> *const q1 = LongPoint::newRangeQuery(L"idx", min, max);
      shared_ptr<Query> *const q2;
      if (sortedNumeric) {
        q2 = SortedNumericDocValuesField::newSlowRangeQuery(L"dv", min, max);
      } else {
        q2 = NumericDocValuesField::newSlowRangeQuery(L"dv", min, max);
      }
      assertSameMatches(searcher, q1, q2, false);
    }

    delete reader;
    delete dir;
  }
}

void TestDocValuesQueries::doTestDuelPointRangeSortedRangeQuery(
    bool sortedSet, int maxValuesPerDoc) 
{
  constexpr int iters = atLeast(10);
  for (int iter = 0; iter < iters; ++iter) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> iw =
        make_shared<RandomIndexWriter>(random(), dir);
    constexpr int numDocs = atLeast(100);
    for (int i = 0; i < numDocs; ++i) {
      shared_ptr<Document> doc = make_shared<Document>();
      constexpr int numValues = TestUtil::nextInt(random(), 0, maxValuesPerDoc);
      for (int j = 0; j < numValues; ++j) {
        constexpr int64_t value = TestUtil::nextLong(random(), -100, 10000);
        std::deque<char> encoded(Long::BYTES);
        LongPoint::encodeDimension(value, encoded, 0);
        if (sortedSet) {
          doc->push_back(make_shared<SortedSetDocValuesField>(
              L"dv", make_shared<BytesRef>(encoded)));
        } else {
          doc->push_back(make_shared<SortedDocValuesField>(
              L"dv", make_shared<BytesRef>(encoded)));
        }
        doc->push_back(make_shared<LongPoint>(L"idx", value));
      }
      iw->addDocument(doc);
    }
    if (random()->nextBoolean()) {
      iw->deleteDocuments(LongPoint::newRangeQuery(L"idx", 0LL, 10LL));
    }
    shared_ptr<IndexReader> *const reader = iw->getReader();
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader, false);
    delete iw;

    for (int i = 0; i < 100; ++i) {
      int64_t min = random()->nextBoolean()
                          ? numeric_limits<int64_t>::min()
                          : TestUtil::nextLong(random(), -100, 10000);
      int64_t max = random()->nextBoolean()
                          ? numeric_limits<int64_t>::max()
                          : TestUtil::nextLong(random(), -100, 10000);
      std::deque<char> encodedMin(Long::BYTES);
      std::deque<char> encodedMax(Long::BYTES);
      LongPoint::encodeDimension(min, encodedMin, 0);
      LongPoint::encodeDimension(max, encodedMax, 0);
      bool includeMin = true;
      bool includeMax = true;
      if (random()->nextBoolean()) {
        includeMin = false;
        min++;
      }
      if (random()->nextBoolean()) {
        includeMax = false;
        max--;
      }
      shared_ptr<Query> *const q1 = LongPoint::newRangeQuery(L"idx", min, max);
      shared_ptr<Query> *const q2;
      if (sortedSet) {
        q2 = SortedSetDocValuesField::newSlowRangeQuery(
            L"dv",
            min == numeric_limits<int64_t>::min() && random()->nextBoolean()
                ? nullptr
                : make_shared<BytesRef>(encodedMin),
            max == numeric_limits<int64_t>::max() && random()->nextBoolean()
                ? nullptr
                : make_shared<BytesRef>(encodedMax),
            includeMin, includeMax);
      } else {
        q2 = SortedDocValuesField::newSlowRangeQuery(
            L"dv",
            min == numeric_limits<int64_t>::min() && random()->nextBoolean()
                ? nullptr
                : make_shared<BytesRef>(encodedMin),
            max == numeric_limits<int64_t>::max() && random()->nextBoolean()
                ? nullptr
                : make_shared<BytesRef>(encodedMax),
            includeMin, includeMax);
      }
      assertSameMatches(searcher, q1, q2, false);
    }

    delete reader;
    delete dir;
  }
}

void TestDocValuesQueries::testDuelPointRangeSortedSetRangeQuery() throw(
    IOException)
{
  doTestDuelPointRangeSortedRangeQuery(true, 1);
}

void TestDocValuesQueries::
    testDuelPointRangeMultivaluedSortedSetRangeQuery() 
{
  doTestDuelPointRangeSortedRangeQuery(true, 3);
}

void TestDocValuesQueries::testDuelPointRangeSortedRangeQuery() throw(
    IOException)
{
  doTestDuelPointRangeSortedRangeQuery(false, 1);
}

void TestDocValuesQueries::assertSameMatches(shared_ptr<IndexSearcher> searcher,
                                             shared_ptr<Query> q1,
                                             shared_ptr<Query> q2,
                                             bool scores) 
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

void TestDocValuesQueries::testEquals()
{
  shared_ptr<Query> q1 =
      SortedNumericDocValuesField::newSlowRangeQuery(L"foo", 3, 5);
  QueryUtils::checkEqual(
      q1, SortedNumericDocValuesField::newSlowRangeQuery(L"foo", 3, 5));
  QueryUtils::checkUnequal(
      q1, SortedNumericDocValuesField::newSlowRangeQuery(L"foo", 3, 6));
  QueryUtils::checkUnequal(
      q1, SortedNumericDocValuesField::newSlowRangeQuery(L"foo", 4, 5));
  QueryUtils::checkUnequal(
      q1, SortedNumericDocValuesField::newSlowRangeQuery(L"bar", 3, 5));

  shared_ptr<Query> q2 = SortedSetDocValuesField::newSlowRangeQuery(
      L"foo", make_shared<BytesRef>(L"bar"), make_shared<BytesRef>(L"baz"),
      true, true);
  QueryUtils::checkEqual(q2, SortedSetDocValuesField::newSlowRangeQuery(
                                 L"foo", make_shared<BytesRef>(L"bar"),
                                 make_shared<BytesRef>(L"baz"), true, true));
  QueryUtils::checkUnequal(q2, SortedSetDocValuesField::newSlowRangeQuery(
                                   L"foo", make_shared<BytesRef>(L"baz"),
                                   make_shared<BytesRef>(L"baz"), true, true));
  QueryUtils::checkUnequal(q2, SortedSetDocValuesField::newSlowRangeQuery(
                                   L"foo", make_shared<BytesRef>(L"bar"),
                                   make_shared<BytesRef>(L"bar"), true, true));
  QueryUtils::checkUnequal(q2, SortedSetDocValuesField::newSlowRangeQuery(
                                   L"quux", make_shared<BytesRef>(L"bar"),
                                   make_shared<BytesRef>(L"baz"), true, true));
}

void TestDocValuesQueries::testToString()
{
  shared_ptr<Query> q1 =
      SortedNumericDocValuesField::newSlowRangeQuery(L"foo", 3, 5);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo:[3 TO 5]", q1->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"[3 TO 5]", q1->toString(L"foo"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo:[3 TO 5]", q1->toString(L"bar"));

  shared_ptr<Query> q2 = SortedSetDocValuesField::newSlowRangeQuery(
      L"foo", make_shared<BytesRef>(L"bar"), make_shared<BytesRef>(L"baz"),
      true, true);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo:[[62 61 72] TO [62 61 7a]]", q2->toString());
  q2 = SortedSetDocValuesField::newSlowRangeQuery(
      L"foo", make_shared<BytesRef>(L"bar"), make_shared<BytesRef>(L"baz"),
      false, true);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo:{[62 61 72] TO [62 61 7a]]", q2->toString());
  q2 = SortedSetDocValuesField::newSlowRangeQuery(
      L"foo", make_shared<BytesRef>(L"bar"), make_shared<BytesRef>(L"baz"),
      false, false);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo:{[62 61 72] TO [62 61 7a]}", q2->toString());
  q2 = SortedSetDocValuesField::newSlowRangeQuery(
      L"foo", make_shared<BytesRef>(L"bar"), nullptr, true, true);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo:[[62 61 72] TO *}", q2->toString());
  q2 = SortedSetDocValuesField::newSlowRangeQuery(
      L"foo", nullptr, make_shared<BytesRef>(L"baz"), true, true);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo:{* TO [62 61 7a]]", q2->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"{* TO [62 61 7a]]", q2->toString(L"foo"));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"foo:{* TO [62 61 7a]]", q2->toString(L"bar"));
}

void TestDocValuesQueries::testMissingField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  iw->addDocument(make_shared<Document>());
  shared_ptr<IndexReader> reader = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  for (shared_ptr<Query> query : Arrays::asList(
           NumericDocValuesField::newSlowRangeQuery(L"foo", 2, 4),
           SortedNumericDocValuesField::newSlowRangeQuery(L"foo", 2, 4),
           SortedDocValuesField::newSlowRangeQuery(
               L"foo", make_shared<BytesRef>(L"abc"),
               make_shared<BytesRef>(L"bcd"), random()->nextBoolean(),
               random()->nextBoolean()),
           SortedSetDocValuesField::newSlowRangeQuery(
               L"foo", make_shared<BytesRef>(L"abc"),
               make_shared<BytesRef>(L"bcd"), random()->nextBoolean(),
               random()->nextBoolean()))) {
    shared_ptr<Weight> w = searcher->createWeight(searcher->rewrite(query),
                                                  random()->nextBoolean(), 1);
    assertNull(w->scorer(searcher->getIndexReader()->leaves()[0]));
  }
  delete reader;
  delete dir;
}

void TestDocValuesQueries::testSortedNumericNPE() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  std::deque<double> nums = {-1.7147449030215377E-208, -1.6887024655302576E-11,
                              1.534911516604164E113,    0.0,
                              2.6947996404505155E-166,  -2.649722021970773E306,
                              6.138239235731689E-198,   2.3967090122610808E111};
  for (int i = 0; i < nums.size(); ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<SortedNumericDocValuesField>(
        L"dv", NumericUtils::doubleToSortableLong(nums[i])));
    iw->addDocument(doc);
  }
  iw->commit();
  shared_ptr<IndexReader> *const reader = iw->getReader();
  shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
  delete iw;

  constexpr int64_t lo =
      NumericUtils::doubleToSortableLong(8.701032080293731E-226);
  constexpr int64_t hi =
      NumericUtils::doubleToSortableLong(2.0801416404385346E-41);

  shared_ptr<Query> query =
      SortedNumericDocValuesField::newSlowRangeQuery(L"dv", lo, hi);
  // TODO: assert expected matches
  searcher->search(query, searcher->reader->maxDoc(), Sort::INDEXORDER);

  // swap order, should still work
  query = SortedNumericDocValuesField::newSlowRangeQuery(L"dv", hi, lo);
  // TODO: assert expected matches
  searcher->search(query, searcher->reader->maxDoc(), Sort::INDEXORDER);

  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::search