using namespace std;

#include "TestTieredMergePolicy.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<MergePolicy> TestTieredMergePolicy::mergePolicy()
{
  return newTieredMergePolicy();
}

void TestTieredMergePolicy::testForceMergeDeletes() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<TieredMergePolicy> tmp = newTieredMergePolicy();
  conf->setMergePolicy(tmp);
  conf->setMaxBufferedDocs(4);
  tmp->setMaxMergeAtOnce(100);
  tmp->setSegmentsPerTier(100);
  tmp->setForceMergeDeletesPctAllowed(30.0);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, conf);
  for (int i = 0; i < 80; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"content", L"aaa " + (i % 4), Field::Store::NO));
    w->addDocument(doc);
  }
  TestUtil::assertEquals(80, w->maxDoc());
  TestUtil::assertEquals(80, w->numDocs());

  if (VERBOSE) {
    wcout << L"\nTEST: delete docs" << endl;
  }
  w->deleteDocuments({make_shared<Term>(L"content", L"0")});
  w->forceMergeDeletes();

  TestUtil::assertEquals(80, w->maxDoc());
  TestUtil::assertEquals(60, w->numDocs());

  if (VERBOSE) {
    wcout << L"\nTEST: forceMergeDeletes2" << endl;
  }
  (std::static_pointer_cast<TieredMergePolicy>(
       w->getConfig()->getMergePolicy()))
      ->setForceMergeDeletesPctAllowed(10.0);
  w->forceMergeDeletes();
  TestUtil::assertEquals(60, w->maxDoc());
  TestUtil::assertEquals(60, w->numDocs());
  delete w;
  delete dir;
}

void TestTieredMergePolicy::testPartialMerge() 
{
  int num = atLeast(10);
  for (int iter = 0; iter < num; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriterConfig> conf =
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
    conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
    shared_ptr<TieredMergePolicy> tmp = newTieredMergePolicy();
    conf->setMergePolicy(tmp);
    conf->setMaxBufferedDocs(2);
    tmp->setMaxMergeAtOnce(3);
    tmp->setSegmentsPerTier(6);

    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, conf);
    int maxCount = 0;
    constexpr int numDocs = TestUtil::nextInt(random(), 20, 100);
    for (int i = 0; i < numDocs; i++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(
          newTextField(L"content", L"aaa " + (i % 4), Field::Store::NO));
      w->addDocument(doc);
      int count = w->getSegmentCount();
      maxCount = max(count, maxCount);
      assertTrue(L"count=" + to_wstring(count) + L" maxCount=" +
                     to_wstring(maxCount),
                 count >= maxCount - 3);
    }

    w->flush(true, true);

    int segmentCount = w->getSegmentCount();
    int targetCount = TestUtil::nextInt(random(), 1, segmentCount);
    if (VERBOSE) {
      wcout << L"TEST: merge to " << targetCount << L" segs (current count="
            << segmentCount << L")" << endl;
    }
    w->forceMerge(targetCount);
    TestUtil::assertEquals(targetCount, w->getSegmentCount());

    delete w;
    delete dir;
  }
}

void TestTieredMergePolicy::testForceMergeDeletesMaxSegSize() throw(
    runtime_error)
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriterConfig> *const conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<TieredMergePolicy> *const tmp = make_shared<TieredMergePolicy>();
  tmp->setMaxMergedSegmentMB(0.01);
  tmp->setForceMergeDeletesPctAllowed(0.0);
  conf->setMergePolicy(tmp);

  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, conf);

  constexpr int numDocs = atLeast(200);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", L"" + to_wstring(i), Field::Store::NO));
    doc->push_back(
        newTextField(L"content", L"aaa " + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
  }

  w->forceMerge(1);
  shared_ptr<IndexReader> r = w->getReader();
  TestUtil::assertEquals(numDocs, r->maxDoc());
  TestUtil::assertEquals(numDocs, r->numDocs());
  delete r;

  if (VERBOSE) {
    wcout << L"\nTEST: delete doc" << endl;
  }

  w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(42 + 17))});

  r = w->getReader();
  TestUtil::assertEquals(numDocs, r->maxDoc());
  TestUtil::assertEquals(numDocs - 1, r->numDocs());
  delete r;

  w->forceMergeDeletes();

  r = w->getReader();
  TestUtil::assertEquals(numDocs - 1, r->maxDoc());
  TestUtil::assertEquals(numDocs - 1, r->numDocs());
  delete r;

  delete w;

  delete dir;
}

void TestTieredMergePolicy::testSetters()
{
  shared_ptr<TieredMergePolicy> *const tmp = make_shared<TieredMergePolicy>();

  tmp->setMaxMergedSegmentMB(0.5);
  assertEquals(0.5, tmp->getMaxMergedSegmentMB(), EPSILON);

  tmp->setMaxMergedSegmentMB(numeric_limits<double>::infinity());
  assertEquals(numeric_limits<int64_t>::max() / 1024 / 1024.0,
               tmp->getMaxMergedSegmentMB(),
               EPSILON * numeric_limits<int64_t>::max());

  tmp->setMaxMergedSegmentMB(numeric_limits<int64_t>::max() / 1024 / 1024.0);
  assertEquals(numeric_limits<int64_t>::max() / 1024 / 1024.0,
               tmp->getMaxMergedSegmentMB(),
               EPSILON * numeric_limits<int64_t>::max());

  expectThrows(invalid_argument::typeid,
               [&]() { tmp->setMaxMergedSegmentMB(-2.0); });

  tmp->setFloorSegmentMB(2.0);
  assertEquals(2.0, tmp->getFloorSegmentMB(), EPSILON);

  tmp->setFloorSegmentMB(numeric_limits<double>::infinity());
  assertEquals(numeric_limits<int64_t>::max() / 1024 / 1024.0,
               tmp->getFloorSegmentMB(),
               EPSILON * numeric_limits<int64_t>::max());

  tmp->setFloorSegmentMB(numeric_limits<int64_t>::max() / 1024 / 1024.0);
  assertEquals(numeric_limits<int64_t>::max() / 1024 / 1024.0,
               tmp->getFloorSegmentMB(),
               EPSILON * numeric_limits<int64_t>::max());

  expectThrows(invalid_argument::typeid,
               [&]() { tmp->setFloorSegmentMB(-2.0); });

  tmp->setMaxCFSSegmentSizeMB(2.0);
  assertEquals(2.0, tmp->getMaxCFSSegmentSizeMB(), EPSILON);

  tmp->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());
  assertEquals(numeric_limits<int64_t>::max() / 1024 / 1024.0,
               tmp->getMaxCFSSegmentSizeMB(),
               EPSILON * numeric_limits<int64_t>::max());

  tmp->setMaxCFSSegmentSizeMB(numeric_limits<int64_t>::max() / 1024 / 1024.0);
  assertEquals(numeric_limits<int64_t>::max() / 1024 / 1024.0,
               tmp->getMaxCFSSegmentSizeMB(),
               EPSILON * numeric_limits<int64_t>::max());

  expectThrows(invalid_argument::typeid,
               [&]() { tmp->setMaxCFSSegmentSizeMB(-2.0); });

  // TODO: Add more checks for other non-double setters!
}

void TestTieredMergePolicy::testUnbalancedMergeSelection() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<TieredMergePolicy> tmp =
      std::static_pointer_cast<TieredMergePolicy>(iwc->getMergePolicy());
  tmp->setFloorSegmentMB(0.00001);
  // We need stable sizes for each segment:
  iwc->setCodec(TestUtil::getDefaultCodec());
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  iwc->setMaxBufferedDocs(100);
  iwc->setRAMBufferSizeMB(-1);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 15000 * RANDOM_MULTIPLIER; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newTextField(L"id", random()->nextLong() + L"" + random()->nextLong(),
                     Field::Store::YES));
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> r = DirectoryReader::open(w);

  // Make sure TMP always merged equal-number-of-docs segments:
  for (auto ctx : r->leaves()) {
    int numDocs = ctx->reader()->numDocs();
    assertTrue(L"got numDocs=" + to_wstring(numDocs),
               numDocs == 100 || numDocs == 1000 || numDocs == 10000);
  }
  delete r;
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index