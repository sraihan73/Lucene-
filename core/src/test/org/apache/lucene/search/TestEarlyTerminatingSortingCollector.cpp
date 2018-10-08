using namespace std;

#include "TestEarlyTerminatingSortingCollector.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MockRandomMergePolicy = org::apache::lucene::index::MockRandomMergePolicy;
using QueryTimeout = org::apache::lucene::index::QueryTimeout;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

shared_ptr<Document> TestEarlyTerminatingSortingCollector::randomDocument()
{
  shared_ptr<Document> *const doc = make_shared<Document>();
  doc->push_back(
      make_shared<NumericDocValuesField>(L"ndv1", random()->nextInt(10)));
  doc->push_back(
      make_shared<NumericDocValuesField>(L"ndv2", random()->nextInt(10)));
  doc->push_back(make_shared<StringField>(
      L"s", RandomPicks::randomFrom(random(), terms), Store::YES));
  return doc;
}

void TestEarlyTerminatingSortingCollector::createRandomIndex(
    bool singleSortedSegment) 
{
  dir = newDirectory();
  numDocs = atLeast(150);
  constexpr int numTerms = TestUtil::nextInt(random(), 1, numDocs / 5);
  shared_ptr<Set<wstring>> randomTerms = unordered_set<wstring>();
  while (randomTerms->size() < numTerms) {
    randomTerms->add(TestUtil::randomSimpleString(random()));
  }
  terms = deque<>(randomTerms);
  constexpr int64_t seed = random()->nextLong();
  shared_ptr<IndexWriterConfig> *const iwc = newIndexWriterConfig(
      make_shared<MockAnalyzer>(make_shared<Random>(seed)));
  if (std::dynamic_pointer_cast<MockRandomMergePolicy>(iwc->getMergePolicy()) !=
      nullptr) {
    // MockRandomMP randomly wraps the leaf readers which makes merging angry
    iwc->setMergePolicy(newTieredMergePolicy());
  }
  iwc->setMergeScheduler(
      make_shared<SerialMergeScheduler>()); // for reproducible tests
  iwc->setIndexSort(sort);
  iw = make_shared<RandomIndexWriter>(make_shared<Random>(seed), dir, iwc);
  iw->setDoRandomForceMerge(
      false); // don't do this, it may happen anyway with MockRandomMP
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> *const doc = randomDocument();
    iw->addDocument(doc);
    if (i == numDocs / 2 || (i != numDocs - 1 && random()->nextInt(8) == 0)) {
      iw->commit();
    }
    if (random()->nextInt(15) == 0) {
      const wstring term = RandomPicks::randomFrom(random(), terms);
      iw->deleteDocuments(make_shared<Term>(L"s", term));
    }
  }
  if (singleSortedSegment) {
    // because of deletions, there might still be a single flush segment in
    // the index, although want want a sorted segment so it needs to be merged
    iw->getReader()->close(); // refresh
    iw->addDocument(make_shared<Document>());
    iw->commit();
    iw->addDocument(make_shared<Document>());
    iw->forceMerge(1);
  } else if (random()->nextBoolean()) {
    iw->forceMerge(FORCE_MERGE_MAX_SEGMENT_COUNT);
  }
  reader = iw->getReader();
}

void TestEarlyTerminatingSortingCollector::closeIndex() 
{
  delete reader;
  delete iw;
  delete dir;
}

void TestEarlyTerminatingSortingCollector::testEarlyTermination() throw(
    IOException)
{
  constexpr int iters = atLeast(8);
  for (int i = 0; i < iters; ++i) {
    createRandomIndex(false);
    for (int j = 0; j < iters; ++j) {
      shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
      constexpr int numHits = TestUtil::nextInt(random(), 1, numDocs);
      shared_ptr<Sort> *const sort = make_shared<Sort>(
          make_shared<SortField>(L"ndv1", SortField::Type::LONG, false));
      constexpr bool fillFields = random()->nextBoolean();
      constexpr bool trackDocScores = random()->nextBoolean();
      constexpr bool trackMaxScore = random()->nextBoolean();
      shared_ptr<TopFieldCollector> *const collector1 =
          TopFieldCollector::create(sort, numHits, fillFields, trackDocScores,
                                    trackMaxScore, false);
      shared_ptr<TopFieldCollector> *const collector2 =
          TopFieldCollector::create(sort, numHits, fillFields, trackDocScores,
                                    trackMaxScore, false);

      shared_ptr<Query> *const query;
      if (random()->nextBoolean()) {
        query = make_shared<TermQuery>(
            make_shared<Term>(L"s", RandomPicks::randomFrom(random(), terms)));
      } else {
        query = make_shared<MatchAllDocsQuery>();
      }
      searcher->search(query, collector1);
      searcher->search(query, make_shared<EarlyTerminatingSortingCollector>(
                                  collector2, sort, numHits));
      assertTrue(collector1->getTotalHits() >= collector2->getTotalHits());
      assertTopDocsEquals(collector1->topDocs()->scoreDocs,
                          collector2->topDocs()->scoreDocs);
    }
    closeIndex();
  }
}

void TestEarlyTerminatingSortingCollector::testCanEarlyTerminate()
{
  assertTrue(EarlyTerminatingSortingCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG)),
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG))));

  assertTrue(EarlyTerminatingSortingCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG),
                        make_shared<SortField>(L"b", SortField::Type::STRING)),
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG),
          make_shared<SortField>(L"b", SortField::Type::STRING))));

  assertTrue(EarlyTerminatingSortingCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG)),
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG),
          make_shared<SortField>(L"b", SortField::Type::STRING))));

  assertFalse(EarlyTerminatingSortingCollector::canEarlyTerminate(
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG, true)),
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG, false))));

  assertFalse(EarlyTerminatingSortingCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG),
                        make_shared<SortField>(L"b", SortField::Type::STRING)),
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG))));

  assertFalse(EarlyTerminatingSortingCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG),
                        make_shared<SortField>(L"b", SortField::Type::STRING)),
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG),
          make_shared<SortField>(L"c", SortField::Type::STRING))));

  assertFalse(EarlyTerminatingSortingCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG),
                        make_shared<SortField>(L"b", SortField::Type::STRING)),
      make_shared<Sort>(
          make_shared<SortField>(L"c", SortField::Type::LONG),
          make_shared<SortField>(L"b", SortField::Type::STRING))));
}

void TestEarlyTerminatingSortingCollector::
    testEarlyTerminationDifferentSorter() 
{
  createRandomIndex(true);

  shared_ptr<Sort> sort = make_shared<Sort>(
      make_shared<SortField>(L"ndv2", SortField::Type::LONG, false));
  shared_ptr<Collector> c = make_shared<EarlyTerminatingSortingCollector>(
      TopFieldCollector::create(sort, 10, true, true, true), sort, 10);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  runtime_error e = expectThrows(IllegalStateException::typeid, [&]() {
    searcher->search(make_shared<MatchAllDocsQuery>(), c);
  });
  TestUtil::assertEquals(
      L"Cannot early terminate with sort order <long: \"ndv2\"> if segments "
      L"are sorted with <long: \"ndv1\">",
      e.what());
  closeIndex();
}

void TestEarlyTerminatingSortingCollector::assertTopDocsEquals(
    std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs1,
    std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs2)
{
  TestUtil::assertEquals(scoreDocs1.size(), scoreDocs2.size());
  for (int i = 0; i < scoreDocs1.size(); ++i) {
    shared_ptr<ScoreDoc> *const scoreDoc1 = scoreDocs1[i];
    shared_ptr<ScoreDoc> *const scoreDoc2 = scoreDocs2[i];
    TestUtil::assertEquals(scoreDoc1->doc, scoreDoc2->doc);
    assertEquals(scoreDoc1->score, scoreDoc2->score, 0.001f);
  }
}

bool TestEarlyTerminatingSortingCollector::TestTerminatedEarlySimpleCollector::
    collectedSomething()
{
  return collectedSomething_;
}

void TestEarlyTerminatingSortingCollector::TestTerminatedEarlySimpleCollector::
    collect(int doc) 
{
  collectedSomething_ = true;
}

bool TestEarlyTerminatingSortingCollector::TestTerminatedEarlySimpleCollector::
    needsScores()
{
  return false;
}

TestEarlyTerminatingSortingCollector::
    TestEarlyTerminatingSortingcollectorQueryTimeout::
        TestEarlyTerminatingSortingcollectorQueryTimeout(bool shouldExit)
    : shouldExit(shouldExit)
{
}

bool TestEarlyTerminatingSortingCollector::
    TestEarlyTerminatingSortingcollectorQueryTimeout::shouldExit()
{
  return shouldExit_;
}

void TestEarlyTerminatingSortingCollector::testTerminatedEarly() throw(
    IOException)
{
  constexpr int iters = atLeast(8);
  for (int i = 0; i < iters; ++i) {
    createRandomIndex(true);

    shared_ptr<IndexSearcher> *const searcher = make_shared<IndexSearcher>(
        reader); // future TODO: use newSearcher(reader);
    shared_ptr<Query> *const query =
        make_shared<MatchAllDocsQuery>(); // search for everything/anything

    shared_ptr<TestTerminatedEarlySimpleCollector> *const collector1 =
        make_shared<TestTerminatedEarlySimpleCollector>();
    searcher->search(query, collector1);

    shared_ptr<TestTerminatedEarlySimpleCollector> *const collector2 =
        make_shared<TestTerminatedEarlySimpleCollector>();
    shared_ptr<EarlyTerminatingSortingCollector> *const etsCollector =
        make_shared<EarlyTerminatingSortingCollector>(collector2, sort, 1);
    searcher->search(query, etsCollector);

    assertTrue(L"collector1=" +
                   StringHelper::toString(collector1->collectedSomething()) +
                   L" vs. collector2=" +
                   StringHelper::toString(collector2->collectedSomething()),
               collector1->collectedSomething() ==
                   collector2->collectedSomething());

    if (collector1->collectedSomething()) {
      // we collected something and since we modestly asked for just one
      // document we should have terminated early
      assertTrue(L"should have terminated early (searcher.reader=" +
                     searcher->reader + L")",
                 etsCollector->terminatedEarly());
    }
    closeIndex();
  }
}
} // namespace org::apache::lucene::search