using namespace std;

#include "TestTopFieldCollectorEarlyTermination.h"

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
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MockRandomMergePolicy = org::apache::lucene::index::MockRandomMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

shared_ptr<Document> TestTopFieldCollectorEarlyTermination::randomDocument()
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

void TestTopFieldCollectorEarlyTermination::createRandomIndex(
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
    iw->forceMerge(1);
  } else if (random()->nextBoolean()) {
    iw->forceMerge(FORCE_MERGE_MAX_SEGMENT_COUNT);
  }
  reader = iw->getReader();
  if (reader->numDocs() == 0) {
    iw->addDocument(make_shared<Document>());
    delete reader;
    reader = iw->getReader();
  }
}

void TestTopFieldCollectorEarlyTermination::closeIndex() 
{
  delete reader;
  delete iw;
  delete dir;
}

void TestTopFieldCollectorEarlyTermination::testEarlyTermination() throw(
    IOException)
{
  doTestEarlyTermination(false);
}

void TestTopFieldCollectorEarlyTermination::
    testEarlyTerminationWhenPaging() 
{
  doTestEarlyTermination(true);
}

void TestTopFieldCollectorEarlyTermination::doTestEarlyTermination(
    bool paging) 
{
  constexpr int iters = atLeast(8);
  for (int i = 0; i < iters; ++i) {
    createRandomIndex(false);
    int maxSegmentSize = 0;
    for (auto ctx : reader->leaves()) {
      maxSegmentSize = max(ctx->reader()->numDocs(), maxSegmentSize);
    }
    for (int j = 0; j < iters; ++j) {
      shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
      constexpr int numHits = TestUtil::nextInt(random(), 1, numDocs);
      shared_ptr<FieldDoc> after;
      if (paging) {
        assert(searcher->getIndexReader()->numDocs() > 0);
        shared_ptr<TopFieldDocs> td =
            searcher->search(make_shared<MatchAllDocsQuery>(), 10, sort);
        after = std::static_pointer_cast<FieldDoc>(
            td->scoreDocs[td->scoreDocs.size() - 1]);
      } else {
        after.reset();
      }
      constexpr bool fillFields = random()->nextBoolean();
      constexpr bool trackDocScores = random()->nextBoolean();
      constexpr bool trackMaxScore = random()->nextBoolean();
      shared_ptr<TopFieldCollector> *const collector1 =
          TopFieldCollector::create(sort, numHits, after, fillFields,
                                    trackDocScores, trackMaxScore, true);
      shared_ptr<TopFieldCollector> *const collector2 =
          TopFieldCollector::create(sort, numHits, after, fillFields,
                                    trackDocScores, trackMaxScore, false);

      shared_ptr<Query> *const query;
      if (random()->nextBoolean()) {
        query = make_shared<TermQuery>(
            make_shared<Term>(L"s", RandomPicks::randomFrom(random(), terms)));
      } else {
        query = make_shared<MatchAllDocsQuery>();
      }
      searcher->search(query, collector1);
      searcher->search(query, collector2);
      shared_ptr<TopDocs> td1 = collector1->topDocs();
      shared_ptr<TopDocs> td2 = collector2->topDocs();

      assertFalse(collector1->isEarlyTerminated());
      if (trackMaxScore == false && paging == false &&
          maxSegmentSize > numHits &&
          std::dynamic_pointer_cast<MatchAllDocsQuery>(query) != nullptr) {
        // Make sure that we sometimes early terminate
        assertTrue(collector2->isEarlyTerminated());
      }
      if (collector2->isEarlyTerminated()) {
        assertTrue(td2->totalHits >= td1->scoreDocs.size());
        assertTrue(td2->totalHits <= reader->maxDoc());
      } else {
        TestUtil::assertEquals(td2->totalHits, td1->totalHits);
      }
      assertTopDocsEquals(td1->scoreDocs, td2->scoreDocs);
    }
    closeIndex();
  }
}

void TestTopFieldCollectorEarlyTermination::testCanEarlyTerminate()
{
  assertTrue(TopFieldCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG)),
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG))));

  assertTrue(TopFieldCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG),
                        make_shared<SortField>(L"b", SortField::Type::STRING)),
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG),
          make_shared<SortField>(L"b", SortField::Type::STRING))));

  assertTrue(TopFieldCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG)),
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG),
          make_shared<SortField>(L"b", SortField::Type::STRING))));

  assertFalse(TopFieldCollector::canEarlyTerminate(
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG, true)),
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG, false))));

  assertFalse(TopFieldCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG),
                        make_shared<SortField>(L"b", SortField::Type::STRING)),
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG))));

  assertFalse(TopFieldCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG),
                        make_shared<SortField>(L"b", SortField::Type::STRING)),
      make_shared<Sort>(
          make_shared<SortField>(L"a", SortField::Type::LONG),
          make_shared<SortField>(L"c", SortField::Type::STRING))));

  assertFalse(TopFieldCollector::canEarlyTerminate(
      make_shared<Sort>(make_shared<SortField>(L"a", SortField::Type::LONG),
                        make_shared<SortField>(L"b", SortField::Type::STRING)),
      make_shared<Sort>(
          make_shared<SortField>(L"c", SortField::Type::LONG),
          make_shared<SortField>(L"b", SortField::Type::STRING))));
}

void TestTopFieldCollectorEarlyTermination::assertTopDocsEquals(
    std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs1,
    std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs2)
{
  TestUtil::assertEquals(scoreDocs1.size(), scoreDocs2.size());
  for (int i = 0; i < scoreDocs1.size(); ++i) {
    shared_ptr<ScoreDoc> *const scoreDoc1 = scoreDocs1[i];
    shared_ptr<ScoreDoc> *const scoreDoc2 = scoreDocs2[i];
    TestUtil::assertEquals(scoreDoc1->doc, scoreDoc2->doc);
    assertEquals(scoreDoc1->score, scoreDoc2->score, 0.0f);
  }
}
} // namespace org::apache::lucene::search