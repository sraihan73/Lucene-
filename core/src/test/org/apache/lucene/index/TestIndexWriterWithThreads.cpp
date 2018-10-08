using namespace std;

#include "TestIndexWriterWithThreads.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using LockObtainFailedException =
    org::apache::lucene::store::LockObtainFailedException;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using org::apache::lucene::util::LuceneTestCase::Slow;

TestIndexWriterWithThreads::IndexerThread::IndexerThread(
    shared_ptr<IndexWriter> writer, bool noErrors,
    shared_ptr<CyclicBarrier> syncStart)
    : syncStart(syncStart)
{
  this->writer = writer;
  this->noErrors = noErrors;
}

void TestIndexWriterWithThreads::IndexerThread::run()
{
  try {
    syncStart->await();
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (BrokenBarrierException | InterruptedException e) {
    error = e;
    throw runtime_error(e);
  }

  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);

  doc->push_back(LuceneTestCase::newField(
      L"field", L"aaa bbb ccc ddd eee fff ggg hhh iii jjj", customType));
  doc->push_back(make_shared<NumericDocValuesField>(L"dv", 5));

  int idUpto = 0;
  int fullCount = 0;

  do {
    try {
      writer->updateDocument(
          make_shared<Term>(L"id", L"" + to_wstring(idUpto++)), doc);
      addCount++;
    } catch (const IOException &ioe) {
      if (VERBOSE) {
        wcout << L"TEST: expected exc:" << endl;
        ioe->printStackTrace(System::out);
      }
      // System.out.println(Thread.currentThread().getName() + ": hit exc");
      // ioe.printStackTrace(System.out);
      if (ioe->getMessage()->startsWith(L"fake disk full at") ||
          ioe->getMessage().equals(L"now failing on purpose")) {
        diskFull = true;
        try {
          delay(1);
        } catch (const InterruptedException &ie) {
          throw make_shared<ThreadInterruptedException>(ie);
        }
        if (fullCount++ >= 5) {
          break;
        }
      } else {
        if (noErrors) {
          wcout << Thread::currentThread().getName()
                << L": ERROR: unexpected IOException:" << endl;
          ioe->printStackTrace(System::out);
          error = ioe;
        }
        break;
      }
    } catch (const AlreadyClosedException &ace) {
      // OK: abort closes the writer
      break;
    } catch (const runtime_error &t) {
      if (noErrors) {
        wcout << Thread::currentThread().getName()
              << L": ERROR: unexpected Throwable:" << endl;
        t.printStackTrace(System::out);
        error = t;
      }
      break;
    }
  } while (true);
}

void TestIndexWriterWithThreads::testImmediateDiskFullWithThreads() throw(
    runtime_error)
{

  int NUM_THREADS = 3;
  constexpr int numIterations = TEST_NIGHTLY ? 10 : 3;
  for (int iter = 0; iter < numIterations; iter++) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }
    shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMaxBufferedDocs(2)
                 ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
                 ->setMergePolicy(newLogMergePolicy(4))
                 ->setCommitOnClose(false));
    (std::static_pointer_cast<ConcurrentMergeScheduler>(
         writer->getConfig()->getMergeScheduler()))
        ->setSuppressExceptions();
    dir->setMaxSizeInBytes(4 * 1024 + 20 * iter);

    shared_ptr<CyclicBarrier> syncStart =
        make_shared<CyclicBarrier>(NUM_THREADS + 1);
    std::deque<std::shared_ptr<IndexerThread>> threads(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
      threads[i] = make_shared<IndexerThread>(writer, true, syncStart);
      threads[i]->start();
    }
    syncStart->await();

    for (int i = 0; i < NUM_THREADS; i++) {
      // Without fix for LUCENE-1130: one of the
      // threads will hang
      threads[i]->join();
      assertTrue(L"hit unexpected Throwable", threads[i]->error == nullptr);
    }

    // Make sure once disk space is avail again, we can
    // cleanly close:
    dir->setMaxSizeInBytes(0);
    try {
      writer->commit();
    } catch (const AlreadyClosedException &ace) {
      // OK: abort closes the writer
      assertTrue(writer->deleter->isClosed());
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete writer;
    }
    delete dir;
  }
}

void TestIndexWriterWithThreads::testCloseWithThreads() 
{
  int NUM_THREADS = 3;
  int numIterations = TEST_NIGHTLY ? 7 : 3;
  for (int iter = 0; iter < numIterations; iter++) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }
    shared_ptr<Directory> dir = newDirectory();

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMaxBufferedDocs(10)
                 ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
                 ->setMergePolicy(newLogMergePolicy(4))
                 ->setCommitOnClose(false));
    (std::static_pointer_cast<ConcurrentMergeScheduler>(
         writer->getConfig()->getMergeScheduler()))
        ->setSuppressExceptions();

    shared_ptr<CyclicBarrier> syncStart =
        make_shared<CyclicBarrier>(NUM_THREADS + 1);
    std::deque<std::shared_ptr<IndexerThread>> threads(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
      threads[i] = make_shared<IndexerThread>(writer, false, syncStart);
      threads[i]->start();
    }
    syncStart->await();

    bool done = false;
    while (!done) {
      delay(100);
      for (int i = 0; i < NUM_THREADS; i++) {
        // only stop when at least one thread has added a doc
        if (threads[i]->addCount > 0) {
          done = true;
          break;
        } else if (!threads[i]->isAlive()) {
          fail(L"thread failed before indexing a single document");
        }
      }
    }

    if (VERBOSE) {
      wcout << L"\nTEST: now close" << endl;
    }
    try {
      writer->commit();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete writer;
    }

    // Make sure threads that are adding docs are not hung:
    for (int i = 0; i < NUM_THREADS; i++) {
      // Without fix for LUCENE-1130: one of the
      // threads will hang
      threads[i]->join();

      // [DW] this is unreachable once join() returns a thread cannot be alive.
      if (threads[i]->isAlive()) {
        fail(L"thread seems to be hung");
      }
    }

    // Quick test to make sure index is not corrupt:
    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    shared_ptr<PostingsEnum> tdocs = TestUtil::docs(
        random(), reader, L"field", make_shared<BytesRef>(L"aaa"), nullptr, 0);
    int count = 0;
    while (tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      count++;
    }
    assertTrue(count > 0);
    delete reader;

    delete dir;
  }
}

void TestIndexWriterWithThreads::_testMultipleThreadsFailure(
    shared_ptr<MockDirectoryWrapper::Failure> failure) 
{

  int NUM_THREADS = 3;

  for (int iter = 0; iter < 2; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }
    shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMaxBufferedDocs(2)
                 ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
                 ->setMergePolicy(newLogMergePolicy(4))
                 ->setCommitOnClose(false));
    (std::static_pointer_cast<ConcurrentMergeScheduler>(
         writer->getConfig()->getMergeScheduler()))
        ->setSuppressExceptions();

    shared_ptr<CyclicBarrier> syncStart =
        make_shared<CyclicBarrier>(NUM_THREADS + 1);
    std::deque<std::shared_ptr<IndexerThread>> threads(NUM_THREADS);
    for (int i = 0; i < NUM_THREADS; i++) {
      threads[i] = make_shared<IndexerThread>(writer, true, syncStart);
      threads[i]->start();
    }
    syncStart->await();

    dir->failOn(failure);
    failure->setDoFail();

    for (int i = 0; i < NUM_THREADS; i++) {
      threads[i]->join();
      assertTrue(L"hit unexpected Throwable", threads[i]->error == nullptr);
    }

    bool success = false;
    try {
      writer->commit();
      delete writer;
      success = true;
    } catch (const AlreadyClosedException &ace) {
      // OK: abort closes the writer
      assertTrue(writer->deleter->isClosed());
    } catch (const IOException &ioe) {
      writer->rollback();
      failure->clearDoFail();
    }
    if (VERBOSE) {
      wcout << L"TEST: success=" << success << endl;
    }

    if (success) {
      shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
      shared_ptr<Bits> *const delDocs = MultiFields::getLiveDocs(reader);
      for (int j = 0; j < reader->maxDoc(); j++) {
        if (delDocs == nullptr || !delDocs->get(j)) {
          reader->document(j);
          reader->getTermVectors(j);
        }
      }
      delete reader;
    }

    delete dir;
  }
}

void TestIndexWriterWithThreads::_testSingleThreadFailure(
    shared_ptr<MockDirectoryWrapper::Failure> failure) 
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(2)
          ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
          ->setCommitOnClose(false);

  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(
          iwc->getMergeScheduler()) != nullptr) {
    iwc->setMergeScheduler(
        make_shared<SuppressingConcurrentMergeSchedulerAnonymousInnerClass>(
            shared_from_this()));
  }

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  doc->push_back(newField(L"field", L"aaa bbb ccc ddd eee fff ggg hhh iii jjj",
                          customType));

  for (int i = 0; i < 6; i++) {
    writer->addDocument(doc);
  }

  dir->failOn(failure);
  failure->setDoFail();
  expectThrows(IOException::typeid, [&]() {
    writer->addDocument(doc);
    writer->addDocument(doc);
    writer->commit();
  });

  failure->clearDoFail();
  expectThrows(AlreadyClosedException::typeid, [&]() {
    writer->addDocument(doc);
    writer->commit();
    writer->close();
  });

  assertTrue(writer->deleter->isClosed());
  delete dir;
}

TestIndexWriterWithThreads::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass::
        SuppressingConcurrentMergeSchedulerAnonymousInnerClass(
            shared_ptr<TestIndexWriterWithThreads> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestIndexWriterWithThreads::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass::isOK(
        runtime_error th)
{
  return std::dynamic_pointer_cast<AlreadyClosedException>(th) != nullptr ||
         (std::dynamic_pointer_cast<IllegalStateException>(th) != nullptr &&
          th.what()->contains(L"this writer hit an unrecoverable error"));
}

TestIndexWriterWithThreads::FailOnlyOnAbortOrFlush::FailOnlyOnAbortOrFlush(
    bool onlyOnce)
{
  this->onlyOnce = onlyOnce;
}

void TestIndexWriterWithThreads::FailOnlyOnAbortOrFlush::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{

  // Since we throw exc during abort, eg when IW is
  // attempting to delete files, we will leave
  // leftovers:
  dir->setAssertNoUnrefencedFilesOnClose(false);

  if (doFail) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: StackTraceElement[] trace = new
    // Exception().getStackTrace();
    std::deque<std::shared_ptr<StackTraceElement>> trace =
        (runtime_error())->getStackTrace();
    bool sawAbortOrFlushDoc = false;
    bool sawClose = false;
    bool sawMerge = false;
    for (int i = 0; i < trace.size(); i++) {
      if (sawAbortOrFlushDoc && sawMerge && sawClose) {
        break;
      }
      if (L"abort" == trace[i]->getMethodName() ||
          L"finishDocument" == trace[i]->getMethodName()) {
        sawAbortOrFlushDoc = true;
      }
      if (L"merge" == trace[i]->getMethodName()) {
        sawMerge = true;
      }
      if (L"close" == trace[i]->getMethodName()) {
        sawClose = true;
      }
    }
    if (sawAbortOrFlushDoc && !sawClose && !sawMerge) {
      if (onlyOnce) {
        doFail = false;
      }
      // System.out.println(Thread.currentThread().getName() + ": now fail");
      // new Throwable().printStackTrace(System.out);
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"now failing on purpose");
    }
  }
}

void TestIndexWriterWithThreads::testIOExceptionDuringAbort() 
{
  _testSingleThreadFailure(make_shared<FailOnlyOnAbortOrFlush>(false));
}

void TestIndexWriterWithThreads::testIOExceptionDuringAbortOnlyOnce() throw(
    IOException)
{
  _testSingleThreadFailure(make_shared<FailOnlyOnAbortOrFlush>(true));
}

void TestIndexWriterWithThreads::testIOExceptionDuringAbortWithThreads() throw(
    runtime_error)
{
  _testMultipleThreadsFailure(make_shared<FailOnlyOnAbortOrFlush>(false));
}

void TestIndexWriterWithThreads::
    testIOExceptionDuringAbortWithThreadsOnlyOnce() 
{
  _testMultipleThreadsFailure(make_shared<FailOnlyOnAbortOrFlush>(true));
}

TestIndexWriterWithThreads::FailOnlyInWriteSegment::FailOnlyInWriteSegment(
    bool onlyOnce)
{
  this->onlyOnce = onlyOnce;
}

void TestIndexWriterWithThreads::FailOnlyInWriteSegment::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (doFail) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: StackTraceElement[] trace = new
    // Exception().getStackTrace();
    std::deque<std::shared_ptr<StackTraceElement>> trace =
        (runtime_error())->getStackTrace();
    for (int i = 0; i < trace.size(); i++) {
      if (L"flush" == trace[i]->getMethodName() &&
          DefaultIndexingChain::typeid->getName().equals(
              trace[i]->getClassName())) {
        if (onlyOnce) {
          doFail = false;
        }
        // System.out.println(Thread.currentThread().getName() + ": NOW FAIL:
        // onlyOnce=" + onlyOnce); new Throwable().printStackTrace(System.out);
        // C++ TODO: The following line could not be converted:
        throw java.io.IOException(L"now failing on purpose");
      }
    }
  }
}

void TestIndexWriterWithThreads::testIOExceptionDuringWriteSegment() throw(
    IOException)
{
  _testSingleThreadFailure(make_shared<FailOnlyInWriteSegment>(false));
}

void TestIndexWriterWithThreads::
    testIOExceptionDuringWriteSegmentOnlyOnce() 
{
  _testSingleThreadFailure(make_shared<FailOnlyInWriteSegment>(true));
}

void TestIndexWriterWithThreads::
    testIOExceptionDuringWriteSegmentWithThreads() 
{
  _testMultipleThreadsFailure(make_shared<FailOnlyInWriteSegment>(false));
}

void TestIndexWriterWithThreads::
    testIOExceptionDuringWriteSegmentWithThreadsOnlyOnce() 
{
  _testMultipleThreadsFailure(make_shared<FailOnlyInWriteSegment>(true));
}

void TestIndexWriterWithThreads::
    testOpenTwoIndexWritersOnDifferentThreads() throw(IOException,
                                                      InterruptedException)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (final org.apache.lucene.store.Directory dir
  // = newDirectory())
  {
    shared_ptr<org::apache::lucene::store::Directory> *const dir =
        newDirectory();
    shared_ptr<CyclicBarrier> syncStart = make_shared<CyclicBarrier>(2);
    shared_ptr<DelayedIndexAndCloseRunnable> thread1 =
        make_shared<DelayedIndexAndCloseRunnable>(dir, syncStart);
    shared_ptr<DelayedIndexAndCloseRunnable> thread2 =
        make_shared<DelayedIndexAndCloseRunnable>(dir, syncStart);
    thread1->start();
    thread2->start();
    thread1->join();
    thread2->join();

    if (std::dynamic_pointer_cast<LockObtainFailedException>(
            thread1->failure) != nullptr ||
        std::dynamic_pointer_cast<LockObtainFailedException>(
            thread2->failure) != nullptr) {
      // We only care about the situation when the two writers succeeded.
      return;
    }

    assertFalse(L"Failed due to: " + thread1->failure, thread1->failed);
    assertFalse(L"Failed due to: " + thread2->failure, thread2->failed);

    // now verify that we have two documents in the index
    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    assertEquals(L"IndexReader should have one document per thread running", 2,
                 reader->numDocs());

    delete reader;
  }
}

TestIndexWriterWithThreads::DelayedIndexAndCloseRunnable::
    DelayedIndexAndCloseRunnable(shared_ptr<Directory> dir,
                                 shared_ptr<CyclicBarrier> syncStart)
    : dir(dir)
{
  this->syncStart = syncStart;
}

void TestIndexWriterWithThreads::DelayedIndexAndCloseRunnable::run()
{
  try {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> field =
        LuceneTestCase::newTextField(L"field", L"testData", Field::Store::YES);
    doc->push_back(field);

    syncStart->await();
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, LuceneTestCase::newIndexWriterConfig(
                 make_shared<MockAnalyzer>(LuceneTestCase::random())));
    writer->addDocument(doc);
    delete writer;
  } catch (const runtime_error &e) {
    failed = true;
    failure = e;
  }
}

void TestIndexWriterWithThreads::testRollbackAndCommitWithThreads() throw(
    runtime_error)
{
  shared_ptr<BaseDirectoryWrapper> *const d = newDirectory();

  constexpr int threadCount = TestUtil::nextInt(random(), 2, 6);

  shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>> *const writerRef =
      make_shared<AtomicReference<std::shared_ptr<IndexWriter>>>();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));

  writerRef->set(make_shared<IndexWriter>(d, newIndexWriterConfig(analyzer)));
  // Make initial commit so the test doesn't trip "corrupt first commit" when
  // virus checker refuses to delete partial segments_N file:
  writerRef->get().commit();
  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random());
  std::deque<std::shared_ptr<Thread>> threads(threadCount);
  constexpr int iters = atLeast(100);
  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();
  shared_ptr<Lock> *const rollbackLock = make_shared<ReentrantLock>();
  shared_ptr<Lock> *const commitLock = make_shared<ReentrantLock>();
  for (int threadID = 0; threadID < threadCount; threadID++) {
    threads[threadID] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), d, writerRef, docs, iters, failed, rollbackLock,
        commitLock);
    threads[threadID]->start();
  }

  for (int threadID = 0; threadID < threadCount; threadID++) {
    threads[threadID]->join();
  }

  assertTrue(!failed->get());
  writerRef->get()->close();
  delete d;
}

TestIndexWriterWithThreads::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestIndexWriterWithThreads> outerInstance,
        shared_ptr<BaseDirectoryWrapper> d,
        shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>> writerRef,
        shared_ptr<LineFileDocs> docs, int iters,
        shared_ptr<AtomicBoolean> failed, shared_ptr<Lock> rollbackLock,
        shared_ptr<Lock> commitLock)
{
  this->outerInstance = outerInstance;
  this->d = d;
  this->writerRef = writerRef;
  this->docs = docs;
  this->iters = iters;
  this->failed = failed;
  this->rollbackLock = rollbackLock;
  this->commitLock = commitLock;
}

void TestIndexWriterWithThreads::ThreadAnonymousInnerClass::run()
{
  for (int iter = 0; iter < iters && !failed->get(); iter++) {
    // final int x = random().nextInt(5);
    constexpr int x = LuceneTestCase::random()->nextInt(3);
    try {
      switch (x) {
      case 0:
        rollbackLock->lock();
        if (VERBOSE) {
          wcout << L"\nTEST: " << Thread::currentThread().getName()
                << L": now rollback" << endl;
        }
        try {
          writerRef->get().rollback();
          if (VERBOSE) {
            wcout << L"TEST: " << Thread::currentThread().getName()
                  << L": rollback done; now open new writer" << endl;
          }
          writerRef->set(make_shared<IndexWriter>(
              d, LuceneTestCase::newIndexWriterConfig(
                     make_shared<MockAnalyzer>(LuceneTestCase::random()))));
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          rollbackLock->unlock();
        }
        break;
      case 1:
        commitLock->lock();
        if (VERBOSE) {
          wcout << L"\nTEST: " << Thread::currentThread().getName()
                << L": now commit" << endl;
        }
        try {
          if (LuceneTestCase::random()->nextBoolean()) {
            writerRef->get().prepareCommit();
          }
          writerRef->get().commit();
        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (AlreadyClosedException | NullPointerException ace) {
          // ok
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          commitLock->unlock();
        }
        break;
      case 2:
        if (VERBOSE) {
          wcout << L"\nTEST: " << Thread::currentThread().getName()
                << L": now add" << endl;
        }
        try {
          writerRef->get().addDocument(docs->nextDoc());
        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (AlreadyClosedException | NullPointerException |
               AssertionError ace) {
          // ok
        }
        break;
      }
    } catch (const runtime_error &t) {
      failed->set(true);
      throw runtime_error(t);
    }
  }
}

void TestIndexWriterWithThreads::testUpdateSingleDocWithThreads() throw(
    runtime_error)
{
  stressUpdateSingleDocWithThreads(false, rarely());
}

void TestIndexWriterWithThreads::testSoftUpdateSingleDocWithThreads() throw(
    runtime_error)
{
  stressUpdateSingleDocWithThreads(true, rarely());
}

void TestIndexWriterWithThreads::stressUpdateSingleDocWithThreads(
    bool useSoftDeletes, bool forceMerge) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); RandomIndexWriter writer = new RandomIndexWriter(random(),
  // dir,
  // newIndexWriterConfig().setMaxBufferedDocs(-1).setRAMBufferSizeMB(0.00001),
  // useSoftDeletes))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    RandomIndexWriter writer = RandomIndexWriter(
        random(), dir,
        newIndexWriterConfig()->setMaxBufferedDocs(-1)->setRAMBufferSizeMB(
            0.00001),
        useSoftDeletes);
    std::deque<std::shared_ptr<Thread>> threads(3 + random()->nextInt(3));
    shared_ptr<AtomicInteger> done = make_shared<AtomicInteger>(0);
    shared_ptr<CyclicBarrier> barrier =
        make_shared<CyclicBarrier>(threads.size() + 1);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::NO));
    writer->updateDocument(make_shared<Term>(L"id", L"1"), doc);
    int itersPerThread = 100 + random()->nextInt(2000);
    for (int i = 0; i < threads.size(); i++) {
      threads[i] = make_shared<Thread>([&]() {
        try {
          barrier->await();
          for (int iters = 0; iters < itersPerThread; iters++) {
            shared_ptr<Document> d = make_shared<Document>();
            d->push_back(
                make_shared<StringField>(L"id", L"1", Field::Store::NO));
            writer->updateDocument(make_shared<Term>(L"id", L"1"), d);
          }
        } catch (const runtime_error &e) {
          throw make_shared<AssertionError>(e);
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          done->incrementAndGet();
        }
      });
      threads[i]->start();
    }
    shared_ptr<DirectoryReader> open = DirectoryReader::open(writer->w);
    TestUtil::assertEquals(open->numDocs(), 1);
    barrier->await();
    try {
      do {
        if (forceMerge && random()->nextBoolean()) {
          writer->forceMerge(1);
        }
        shared_ptr<DirectoryReader> newReader =
            DirectoryReader::openIfChanged(open);
        if (newReader != nullptr) {
          open->close();
          open = newReader;
        }
        TestUtil::assertEquals(open->numDocs(), 1);
      } while (done->get() < threads.size());
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      open->close();
      for (int i = 0; i < threads.size(); i++) {
        threads[i]->join();
      }
    }
  }
}
} // namespace org::apache::lucene::index