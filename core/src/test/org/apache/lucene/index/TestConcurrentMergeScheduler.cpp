using namespace std;

#include "TestConcurrentMergeScheduler.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

TestConcurrentMergeScheduler::FailOnlyOnFlush::FailOnlyOnFlush(
    shared_ptr<TestConcurrentMergeScheduler> outerInstance)
    : outerInstance(outerInstance)
{
}

void TestConcurrentMergeScheduler::FailOnlyOnFlush::setDoFail()
{
  this->doFail = true;
  hitExc = false;
}

void TestConcurrentMergeScheduler::FailOnlyOnFlush::clearDoFail()
{
  this->doFail = false;
}

void TestConcurrentMergeScheduler::FailOnlyOnFlush::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (doFail && outerInstance->isTestThread()) {
    bool isDoFlush = false;
    bool isClose = false;
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: StackTraceElement[] trace = new
    // Exception().getStackTrace();
    std::deque<std::shared_ptr<StackTraceElement>> trace =
        (runtime_error())->getStackTrace();
    for (int i = 0; i < trace.size(); i++) {
      if (isDoFlush && isClose) {
        break;
      }
      if (L"flush" == trace[i]->getMethodName()) {
        isDoFlush = true;
      }
      if (L"close" == trace[i]->getMethodName()) {
        isClose = true;
      }
    }
    if (isDoFlush && !isClose && LuceneTestCase::random()->nextBoolean()) {
      hitExc = true;
      throw make_shared<IOException>(Thread::currentThread().getName() +
                                     L": now failing during flush");
    }
  }
}

void TestConcurrentMergeScheduler::testFlushExceptions() 
{
  shared_ptr<MockDirectoryWrapper> directory = newMockDirectory();
  shared_ptr<FailOnlyOnFlush> failure =
      make_shared<FailOnlyOnFlush>(shared_from_this());
  directory->failOn(failure);
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(2);
  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(
          iwc->getMergeScheduler()) != nullptr) {
    iwc->setMergeScheduler(
        make_shared<SuppressingConcurrentMergeSchedulerAnonymousInnerClass>(
            shared_from_this()));
  }
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::YES);
  doc->push_back(idField);

  for (int i = 0; i < 10; i++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << i << endl;
    }

    for (int j = 0; j < 20; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      idField->setStringValue(Integer::toString(i * 20 + j));
      writer->addDocument(doc);
    }

    // must cycle here because sometimes the merge flushes
    // the doc we just added and so there's nothing to
    // flush, and we don't hit the exception
    while (true) {
      writer->addDocument(doc);
      failure->setDoFail();
      try {
        writer->flush(true, true);
        if (failure->hitExc) {
          fail(L"failed to hit IOException");
        }
      } catch (const IOException &ioe) {
        if (VERBOSE) {
          ioe->printStackTrace(System::out);
        }
        failure->clearDoFail();
        assertTrue(writer->isClosed());
        // Abort should have closed the deleter:
        assertTrue(writer->deleter->isClosed());
        goto outerBreak;
      }
    }
  outerContinue:;
  }
outerBreak:

  assertFalse(DirectoryReader::indexExists(directory));
  delete directory;
}

TestConcurrentMergeScheduler::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass::
        SuppressingConcurrentMergeSchedulerAnonymousInnerClass(
            shared_ptr<TestConcurrentMergeScheduler> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestConcurrentMergeScheduler::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass::isOK(
        runtime_error th)
{
  return std::dynamic_pointer_cast<AlreadyClosedException>(th) != nullptr ||
         (std::dynamic_pointer_cast<IllegalStateException>(th) != nullptr &&
          th.what()->contains(L"this writer hit an unrecoverable error"));
}

void TestConcurrentMergeScheduler::testDeleteMerging() 
{
  shared_ptr<Directory> directory = newDirectory();

  shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
  // Force degenerate merging so we can get a mix of
  // merging of segments with and without deletes at the
  // start:
  mp->setMinMergeDocs(1000);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setMergePolicy(mp));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::YES);
  doc->push_back(idField);
  for (int i = 0; i < 10; i++) {
    if (VERBOSE) {
      wcout << L"\nTEST: cycle" << endl;
    }
    for (int j = 0; j < 100; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      idField->setStringValue(Integer::toString(i * 100 + j));
      writer->addDocument(doc);
    }

    int delID = i;
    while (delID < 100 * (1 + i)) {
      if (VERBOSE) {
        wcout << L"TEST: del " << delID << endl;
      }
      writer->deleteDocuments(
          {make_shared<Term>(L"id", L"" + to_wstring(delID))});
      delID += 10;
    }

    writer->commit();
  }

  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  // Verify that we did not lose any deletes...
  TestUtil::assertEquals(450, reader->numDocs());
  delete reader;
  delete directory;
}

void TestConcurrentMergeScheduler::testNoExtraFiles() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setMaxBufferedDocs(2));

  for (int iter = 0; iter < 7; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }

    for (int j = 0; j < 21; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(newTextField(L"content", L"a b c", Field::Store::NO));
      writer->addDocument(doc);
    }

    delete writer;
    TestIndexWriter::assertNoUnreferencedFiles(directory, L"testNoExtraFiles");

    // Reopen
    writer = make_shared<IndexWriter>(
        directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                       ->setOpenMode(OpenMode::APPEND)
                       ->setMaxBufferedDocs(2));
  }

  delete writer;

  delete directory;
}

void TestConcurrentMergeScheduler::testNoWaitClose() 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField = newStringField(L"id", L"", Field::Store::YES);
  doc->push_back(idField);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setMaxBufferedDocs(2)
                     ->setMergePolicy(newLogMergePolicy(100))
                     ->setCommitOnClose(false));

  int numIters = TEST_NIGHTLY ? 10 : 3;
  for (int iter = 0; iter < numIters; iter++) {

    for (int j = 0; j < 201; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      idField->setStringValue(Integer::toString(iter * 201 + j));
      writer->addDocument(doc);
    }

    int delID = iter * 201;
    for (int j = 0; j < 20; j++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      writer->deleteDocuments(
          {make_shared<Term>(L"id", Integer::toString(delID))});
      delID += 5;
    }

    // Force a bunch of merge threads to kick off so we
    // stress out aborting them on close:
    (std::static_pointer_cast<LogMergePolicy>(
         writer->getConfig()->getMergePolicy()))
        ->setMergeFactor(3);
    writer->addDocument(doc);

    try {
      writer->commit();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete writer;
    }

    shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
    TestUtil::assertEquals((1 + iter) * 182, reader->numDocs());
    delete reader;

    // Reopen
    writer = make_shared<IndexWriter>(
        directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                       ->setOpenMode(OpenMode::APPEND)
                       ->setMergePolicy(newLogMergePolicy(100))
                       ->setMaxBufferedDocs(2)
                       ->setCommitOnClose(false));
  }
  delete writer;

  delete directory;
}

void TestConcurrentMergeScheduler::testMaxMergeCount() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setCommitOnClose(false);

  constexpr int maxMergeCount = TestUtil::nextInt(random(), 1, 5);
  constexpr int maxMergeThreads = TestUtil::nextInt(random(), 1, maxMergeCount);
  shared_ptr<CountDownLatch> *const enoughMergesWaiting =
      make_shared<CountDownLatch>(maxMergeCount);
  shared_ptr<AtomicInteger> *const runningMergeCount =
      make_shared<AtomicInteger>(0);
  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();

  if (VERBOSE) {
    wcout << L"TEST: maxMergeCount=" << maxMergeCount << L" maxMergeThreads="
          << maxMergeThreads << endl;
  }

  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeSchedulerAnonymousInnerClass>(
          shared_from_this(), maxMergeCount, enoughMergesWaiting,
          runningMergeCount, failed);
  cms->setMaxMergesAndThreads(maxMergeCount, maxMergeThreads);
  iwc->setMergeScheduler(cms);
  iwc->setMaxBufferedDocs(2);

  shared_ptr<TieredMergePolicy> tmp = make_shared<TieredMergePolicy>();
  iwc->setMergePolicy(tmp);
  tmp->setMaxMergeAtOnce(2);
  tmp->setSegmentsPerTier(2);

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newField(L"field", L"field", TextField::TYPE_NOT_STORED));
  while (enoughMergesWaiting->getCount() != 0 && !failed->get()) {
    for (int i = 0; i < 10; i++) {
      w->addDocument(doc);
    }
  }
  try {
    w->commit();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete w;
  }
  delete dir;
}

TestConcurrentMergeScheduler::ConcurrentMergeSchedulerAnonymousInnerClass::
    ConcurrentMergeSchedulerAnonymousInnerClass(
        shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        int maxMergeCount, shared_ptr<CountDownLatch> enoughMergesWaiting,
        shared_ptr<AtomicInteger> runningMergeCount,
        shared_ptr<AtomicBoolean> failed)
{
  this->outerInstance = outerInstance;
  this->maxMergeCount = maxMergeCount;
  this->enoughMergesWaiting = enoughMergesWaiting;
  this->runningMergeCount = runningMergeCount;
  this->failed = failed;
}

void TestConcurrentMergeScheduler::ConcurrentMergeSchedulerAnonymousInnerClass::
    doMerge(shared_ptr<IndexWriter> writer,
            shared_ptr<MergePolicy::OneMerge> merge) 
{
  try {
    // Stall all incoming merges until we see
    // maxMergeCount:
    int count = runningMergeCount->incrementAndGet();
    try {
      assertTrue(L"count=" + to_wstring(count) + L" vs maxMergeCount=" +
                     to_wstring(maxMergeCount),
                 count <= maxMergeCount);
      enoughMergesWaiting->countDown();

      // Stall this merge until we see exactly
      // maxMergeCount merges waiting
      while (true) {
        if (enoughMergesWaiting->await(10, TimeUnit::MILLISECONDS) ||
            failed->get()) {
          break;
        }
      }
      // Then sleep a bit to give a chance for the bug
      // (too many pending merges) to appear:
      delay(20);
      outerInstance->super->doMerge(writer, merge);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      runningMergeCount->decrementAndGet();
    }
  } catch (const runtime_error &t) {
    failed->set(true);
    writer->mergeFinish(merge);
    throw runtime_error(t);
  }
}

TestConcurrentMergeScheduler::TrackingCMS::TrackingCMS(
    shared_ptr<CountDownLatch> atLeastOneMerge)
{
  setMaxMergesAndThreads(5, 5);
  this->atLeastOneMerge = atLeastOneMerge;
}

void TestConcurrentMergeScheduler::TrackingCMS::doMerge(
    shared_ptr<IndexWriter> writer,
    shared_ptr<MergePolicy::OneMerge> merge) 
{
  totMergedBytes += merge->totalBytesSize();
  atLeastOneMerge->countDown();
  ConcurrentMergeScheduler::doMerge(writer, merge);
}

void TestConcurrentMergeScheduler::testTotalBytesSize() 
{
  shared_ptr<Directory> d = newDirectory();
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(d) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(d))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(5);
  shared_ptr<CountDownLatch> atLeastOneMerge = make_shared<CountDownLatch>(1);
  iwc->setMergeScheduler(make_shared<TrackingCMS>(atLeastOneMerge));
  if (TestUtil::getPostingsFormat(L"id") == L"SimpleText") {
    // no
    iwc->setCodec(
        TestUtil::alwaysPostingsFormat(TestUtil::getDefaultPostingsFormat()));
  }
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(d, iwc);
  for (int i = 0; i < 1000; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<StringField>(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);

    if (random()->nextBoolean()) {
      w->deleteDocuments(
          {make_shared<Term>(L"id", L"" + random()->nextInt(i + 1))});
    }
  }
  atLeastOneMerge->await();
  assertTrue((std::static_pointer_cast<TrackingCMS>(
                  w->getConfig()->getMergeScheduler()))
                 ->totMergedBytes != 0);
  delete w;
  delete d;
}

void TestConcurrentMergeScheduler::testInvalidMaxMergeCountAndThreads() throw(
    runtime_error)
{
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeScheduler>();
  expectThrows(invalid_argument::typeid, [&]() {
    cms->setMaxMergesAndThreads(
        ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS, 3);
  });
  expectThrows(invalid_argument::typeid, [&]() {
    cms->setMaxMergesAndThreads(
        3, ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS);
  });
}

void TestConcurrentMergeScheduler::testLiveMaxMergeCount() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<TieredMergePolicy> tmp = make_shared<TieredMergePolicy>();
  tmp->setSegmentsPerTier(1000);
  tmp->setMaxMergeAtOnce(1000);
  tmp->setMaxMergeAtOnceExplicit(10);
  iwc->setMergePolicy(tmp);
  iwc->setMaxBufferedDocs(2);
  iwc->setRAMBufferSizeMB(-1);

  shared_ptr<AtomicInteger> *const maxRunningMergeCount =
      make_shared<AtomicInteger>();

  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeSchedulerAnonymousInnerClass>(
          shared_from_this(), maxRunningMergeCount);

  TestUtil::assertEquals(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      cms->getMaxMergeCount());
  TestUtil::assertEquals(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      cms->getMaxThreadCount());

  cms->setMaxMergesAndThreads(5, 3);

  iwc->setMergeScheduler(cms);

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(d, iwc);
  // Makes 100 segments
  for (int i = 0; i < 200; i++) {
    w->addDocument(make_shared<Document>());
  }

  // No merges should have run so far, because TMP has high segmentsPerTier:
  TestUtil::assertEquals(0, maxRunningMergeCount->get());
  w->forceMerge(1);

  // At most 5 merge threads should have launched at once:
  assertTrue(L"maxRunningMergeCount=" + maxRunningMergeCount,
             maxRunningMergeCount->get() <= 5);
  maxRunningMergeCount->set(0);

  // Makes another 100 segments
  for (int i = 0; i < 200; i++) {
    w->addDocument(make_shared<Document>());
  }

  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       w->getConfig()->getMergeScheduler()))
      ->setMaxMergesAndThreads(1, 1);
  w->forceMerge(1);

  // At most 1 merge thread should have launched at once:
  TestUtil::assertEquals(1, maxRunningMergeCount->get());

  delete w;
  delete d;
}

TestConcurrentMergeScheduler::ConcurrentMergeSchedulerAnonymousInnerClass::
    ConcurrentMergeSchedulerAnonymousInnerClass(
        shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        shared_ptr<AtomicInteger> maxRunningMergeCount)
{
  this->outerInstance = outerInstance;
  this->maxRunningMergeCount = maxRunningMergeCount;
  runningMergeCount = make_shared<AtomicInteger>();
}

void TestConcurrentMergeScheduler::ConcurrentMergeSchedulerAnonymousInnerClass::
    doMerge(shared_ptr<IndexWriter> writer,
            shared_ptr<MergePolicy::OneMerge> merge) 
{
  int count = runningMergeCount->incrementAndGet();
  // evil?
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    if (count > maxRunningMergeCount->get()) {
      maxRunningMergeCount->set(count);
    }
  }
  try {
    outerInstance->super->doMerge(writer, merge);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    runningMergeCount->decrementAndGet();
  }
}

void TestConcurrentMergeScheduler::testMaybeStallCalled() 
{
  shared_ptr<AtomicBoolean> *const wasCalled = make_shared<AtomicBoolean>();
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMergeScheduler(
      make_shared<ConcurrentMergeSchedulerAnonymousInnerClass2>(
          shared_from_this(), wasCalled));
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  w->addDocument(make_shared<Document>());
  w->forceMerge(1);
  assertTrue(wasCalled->get());

  delete w;
  delete dir;
}

TestConcurrentMergeScheduler::ConcurrentMergeSchedulerAnonymousInnerClass2::
    ConcurrentMergeSchedulerAnonymousInnerClass2(
        shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        shared_ptr<AtomicBoolean> wasCalled)
{
  this->outerInstance = outerInstance;
  this->wasCalled = wasCalled;
}

bool TestConcurrentMergeScheduler::
    ConcurrentMergeSchedulerAnonymousInnerClass2::maybeStall(
        shared_ptr<IndexWriter> writer)
{
  wasCalled->set(true);
  return true;
}

void TestConcurrentMergeScheduler::testHangDuringRollback() 
{
  shared_ptr<Directory> dir = newMockDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(2);
  shared_ptr<LogDocMergePolicy> mp = make_shared<LogDocMergePolicy>();
  iwc->setMergePolicy(mp);
  mp->setMergeFactor(2);
  shared_ptr<CountDownLatch> *const mergeStart = make_shared<CountDownLatch>(1);
  shared_ptr<CountDownLatch> *const mergeFinish =
      make_shared<CountDownLatch>(1);
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeSchedulerAnonymousInnerClass3>(
          shared_from_this(), mergeStart, mergeFinish);
  cms->setMaxMergesAndThreads(1, 1);
  iwc->setMergeScheduler(cms);

  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);

  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  // flush

  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  // flush + merge

  // Wait for merge to kick off
  mergeStart->await();

  make_shared<ThreadAnonymousInnerClass>(shared_from_this(), mergeFinish, w)
      .start();

  while (w->numDocs() != 8) {
    delay(10);
  }

  w->rollback();
  delete dir;
}

TestConcurrentMergeScheduler::ConcurrentMergeSchedulerAnonymousInnerClass3::
    ConcurrentMergeSchedulerAnonymousInnerClass3(
        shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        shared_ptr<CountDownLatch> mergeStart,
        shared_ptr<CountDownLatch> mergeFinish)
{
  this->outerInstance = outerInstance;
  this->mergeStart = mergeStart;
  this->mergeFinish = mergeFinish;
}

void TestConcurrentMergeScheduler::
    ConcurrentMergeSchedulerAnonymousInnerClass3::doMerge(
        shared_ptr<IndexWriter> writer,
        shared_ptr<MergePolicy::OneMerge> merge) 
{
  mergeStart->countDown();
  try {
    mergeFinish->await();
  } catch (const InterruptedException &ie) {
    throw runtime_error(ie);
  }
  outerInstance->super->doMerge(writer, merge);
}

TestConcurrentMergeScheduler::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        shared_ptr<CountDownLatch> mergeFinish,
        shared_ptr<org::apache::lucene::index::IndexWriter> w)
{
  this->outerInstance = outerInstance;
  this->mergeFinish = mergeFinish;
  this->w = w;
}

void TestConcurrentMergeScheduler::ThreadAnonymousInnerClass::run()
{
  try {
    w->addDocument(make_shared<Document>());
    w->addDocument(make_shared<Document>());
    // flush

    w->addDocument(make_shared<Document>());
    // W/o the fix for LUCENE-6094 we would hang forever here:
    w->addDocument(make_shared<Document>());
    // flush + merge

    // Now allow first merge to finish:
    mergeFinish->countDown();

  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TestConcurrentMergeScheduler::testDynamicDefaults() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeScheduler>();
  TestUtil::assertEquals(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      cms->getMaxMergeCount());
  TestUtil::assertEquals(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      cms->getMaxThreadCount());
  iwc->setMergeScheduler(cms);
  iwc->setMaxBufferedDocs(2);
  shared_ptr<LogMergePolicy> lmp = newLogMergePolicy();
  lmp->setMergeFactor(2);
  iwc->setMergePolicy(lmp);

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  // flush

  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  // flush + merge

  // CMS should have now set true values:
  assertTrue(cms->getMaxMergeCount() !=
             ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS);
  assertTrue(cms->getMaxThreadCount() !=
             ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS);
  delete w;
  delete dir;
}

void TestConcurrentMergeScheduler::testResetToAutoDefault() 
{
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeScheduler>();
  TestUtil::assertEquals(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      cms->getMaxMergeCount());
  TestUtil::assertEquals(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      cms->getMaxThreadCount());
  cms->setMaxMergesAndThreads(4, 3);
  TestUtil::assertEquals(4, cms->getMaxMergeCount());
  TestUtil::assertEquals(3, cms->getMaxThreadCount());

  expectThrows(invalid_argument::typeid, [&]() {
    cms->setMaxMergesAndThreads(
        ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS, 4);
  });

  expectThrows(invalid_argument::typeid, [&]() {
    cms->setMaxMergesAndThreads(
        4, ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS);
  });

  cms->setMaxMergesAndThreads(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS);
  TestUtil::assertEquals(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      cms->getMaxMergeCount());
  TestUtil::assertEquals(
      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS,
      cms->getMaxThreadCount());
}

void TestConcurrentMergeScheduler::testSpinningDefaults() 
{
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeScheduler>();
  cms->setDefaultMaxMergesAndThreads(true);
  TestUtil::assertEquals(1, cms->getMaxThreadCount());
  TestUtil::assertEquals(6, cms->getMaxMergeCount());
}

void TestConcurrentMergeScheduler::testAutoIOThrottleGetter() throw(
    runtime_error)
{
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeScheduler>();
  cms->disableAutoIOThrottle();
  assertFalse(cms->getAutoIOThrottle());
  cms->enableAutoIOThrottle();
  assertTrue(cms->getAutoIOThrottle());
}

void TestConcurrentMergeScheduler::testNonSpinningDefaults() throw(
    runtime_error)
{
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeScheduler>();
  cms->setDefaultMaxMergesAndThreads(false);
  int threadCount = cms->getMaxThreadCount();
  assertTrue(threadCount >= 1);
  assertTrue(threadCount <= 4);
  TestUtil::assertEquals(5 + threadCount, cms->getMaxMergeCount());
}

void TestConcurrentMergeScheduler::testNoStallMergeThreads() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc->setMergePolicy(NoMergePolicy::INSTANCE);
  iwc->setMaxBufferedDocs(2);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
  for (int i = 0; i < 1000; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"field", L"" + to_wstring(i), Field::Store::YES));
    w->addDocument(doc);
  }
  delete w;

  iwc = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<AtomicBoolean> failed = make_shared<AtomicBoolean>();
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeSchedulerAnonymousInnerClass4>(
          shared_from_this(), failed);
  cms->setMaxMergesAndThreads(2, 1);
  iwc->setMergeScheduler(cms);
  iwc->setMaxBufferedDocs(2);

  w = make_shared<IndexWriter>(dir, iwc);
  w->forceMerge(1);
  delete w;
  delete dir;

  assertFalse(failed->get());
}

TestConcurrentMergeScheduler::ConcurrentMergeSchedulerAnonymousInnerClass4::
    ConcurrentMergeSchedulerAnonymousInnerClass4(
        shared_ptr<TestConcurrentMergeScheduler> outerInstance,
        shared_ptr<AtomicBoolean> failed)
{
  this->outerInstance = outerInstance;
  this->failed = failed;
}

void TestConcurrentMergeScheduler::
    ConcurrentMergeSchedulerAnonymousInnerClass4::doStall()
{
  if (Thread::currentThread().getName()->startsWith(L"Lucene Merge Thread")) {
    failed->set(true);
  }
  outerInstance->super->doStall();
}
} // namespace org::apache::lucene::index