using namespace std;

#include "TestSearcherManager.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using ConcurrentMergeScheduler =
    org::apache::lucene::index::ConcurrentMergeScheduler;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using FilterDirectoryReader = org::apache::lucene::index::FilterDirectoryReader;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using ThreadedIndexingAndSearchingTestCase =
    org::apache::lucene::index::ThreadedIndexingAndSearchingTestCase;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSearcherManager::testSearcherManager() 
{
  pruner = make_shared<SearcherLifetimeManager::PruneByAge>(
      TEST_NIGHTLY ? TestUtil::nextInt(random(), 1, 20) : 1);
  runTest(L"TestSearcherManager");
}

shared_ptr<IndexSearcher>
TestSearcherManager::getFinalSearcher() 
{
  if (!isNRT) {
    writer->commit();
  }
  assertTrue(mgr->maybeRefresh() || mgr->isSearcherCurrent());
  return mgr->acquire();
}

void TestSearcherManager::doAfterWriter(shared_ptr<ExecutorService> es) throw(
    runtime_error)
{
  shared_ptr<SearcherFactory> *const factory =
      make_shared<SearcherFactoryAnonymousInnerClass>(shared_from_this(), es);
  if (random()->nextBoolean()) {
    // TODO: can we randomize the applyAllDeletes?  But
    // somehow for final searcher we must apply
    // deletes...
    mgr = make_shared<SearcherManager>(writer, factory);
    isNRT = true;
  } else {
    // SearcherManager needs to see empty commit:
    writer->commit();
    mgr = make_shared<SearcherManager>(dir, factory);
    isNRT = false;
    assertMergedSegmentsWarmed = false;
  }

  lifetimeMGR = make_shared<SearcherLifetimeManager>();
}

TestSearcherManager::SearcherFactoryAnonymousInnerClass::
    SearcherFactoryAnonymousInnerClass(
        shared_ptr<TestSearcherManager> outerInstance,
        shared_ptr<ExecutorService> es)
{
  this->outerInstance = outerInstance;
  this->es = es;
}

shared_ptr<IndexSearcher>
TestSearcherManager::SearcherFactoryAnonymousInnerClass::LuceneTestCase::
    newSearcher(shared_ptr<IndexReader> r,
                shared_ptr<IndexReader> previous) 
{
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r, es);
  outerInstance->warmCalled = true;
  s->search(make_shared<TermQuery>(make_shared<Term>(L"body", L"united")), 10);
  return s;
}

void TestSearcherManager::doSearching(
    shared_ptr<ExecutorService> es,
    int64_t const stopTime) 
{

  shared_ptr<Thread> reopenThread =
      make_shared<ThreadAnonymousInnerClass>(shared_from_this(), stopTime);
  reopenThread->setDaemon(true);
  reopenThread->start();

  runSearchThreads(stopTime);

  reopenThread->join();
}

TestSearcherManager::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestSearcherManager> outerInstance, int64_t stopTime)
{
  this->outerInstance = outerInstance;
  this->stopTime = stopTime;
}

void TestSearcherManager::ThreadAnonymousInnerClass::run()
{
  try {
    if (VERBOSE) {
      wcout << L"[" << Thread::currentThread().getName()
            << L"]: launch reopen thread" << endl;
    }

    while (System::currentTimeMillis() < stopTime) {
      delay(TestUtil::nextInt(LuceneTestCase::random(), 1, 100));
      outerInstance->writer->commit();
      delay(TestUtil::nextInt(LuceneTestCase::random(), 1, 5));
      bool block = LuceneTestCase::random()->nextBoolean();
      if (block) {
        outerInstance->mgr->maybeRefreshBlocking();
        outerInstance->lifetimeMGR->prune(outerInstance->pruner);
      } else if (outerInstance->mgr->maybeRefresh()) {
        outerInstance->lifetimeMGR->prune(outerInstance->pruner);
      }
    }
  } catch (const runtime_error &t) {
    if (VERBOSE) {
      wcout << L"TEST: reopen thread hit exc" << endl;
      t.printStackTrace(System::out);
    }
    outerInstance->failed->set(true);
    throw runtime_error(t);
  }
}

shared_ptr<IndexSearcher>
TestSearcherManager::getCurrentSearcher() 
{
  if (random()->nextInt(10) == 7) {
    // NOTE: not best practice to call maybeRefresh
    // synchronous to your search threads, but still we
    // test as apps will presumably do this for
    // simplicity:
    if (mgr->maybeRefresh()) {
      lifetimeMGR->prune(pruner);
    }
  }

  shared_ptr<IndexSearcher> s = nullptr;

  {
    lock_guard<mutex> lock(pastSearchers);
    while (pastSearchers.size() != 0 && random()->nextDouble() < 0.25) {
      // 1/4 of the time pull an old searcher, ie, simulate
      // a user doing a follow-on action on a previous
      // search (drilling down/up, clicking next/prev page,
      // etc.)
      const optional<int64_t> token =
          pastSearchers[random()->nextInt(pastSearchers.size())];
      s = lifetimeMGR->acquire(token);
      if (s == nullptr) {
        // Searcher was pruned
        // C++ TODO: The Java deque 'remove(Object)' method is not
        // converted:
        pastSearchers.remove(token);
      } else {
        break;
      }
    }
  }

  if (s == nullptr) {
    s = mgr->acquire();
    if (s->getIndexReader()->numDocs() != 0) {
      optional<int64_t> token = lifetimeMGR->record(s);
      {
        lock_guard<mutex> lock(pastSearchers);
        if (!find(pastSearchers.begin(), pastSearchers.end(), token) !=
            pastSearchers.end()) {
          pastSearchers.push_back(token);
        }
      }
    }
  }

  return s;
}

void TestSearcherManager::releaseSearcher(shared_ptr<IndexSearcher> s) throw(
    runtime_error)
{
  s->getIndexReader()->decRef();
}

void TestSearcherManager::doClose() 
{
  assertTrue(warmCalled);
  if (VERBOSE) {
    wcout << L"TEST: now close SearcherManager" << endl;
  }
  delete mgr;
  delete lifetimeMGR;
}

void TestSearcherManager::testIntermediateClose() throw(IOException,
                                                        InterruptedException)
{
  shared_ptr<Directory> dir = newDirectory();
  // Test can deadlock if we use SMS:
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>()));
  writer->addDocument(make_shared<Document>());
  writer->commit();
  shared_ptr<CountDownLatch> *const awaitEnterWarm =
      make_shared<CountDownLatch>(1);
  shared_ptr<CountDownLatch> *const awaitClose = make_shared<CountDownLatch>(1);
  shared_ptr<AtomicBoolean> *const triedReopen =
      make_shared<AtomicBoolean>(false);
  shared_ptr<ExecutorService> *const es =
      random()->nextBoolean()
          ? nullptr
          : Executors::newCachedThreadPool(
                make_shared<NamedThreadFactory>(L"testIntermediateClose"));
  shared_ptr<SearcherFactory> *const factory =
      make_shared<SearcherFactoryAnonymousInnerClass2>(
          shared_from_this(), awaitEnterWarm, awaitClose, triedReopen, es);
  shared_ptr<SearcherManager> *const searcherManager =
      random()->nextBoolean()
          ? make_shared<SearcherManager>(dir, factory)
          : make_shared<SearcherManager>(writer, random()->nextBoolean(), false,
                                         factory);
  if (VERBOSE) {
    wcout << L"sm created" << endl;
  }
  shared_ptr<IndexSearcher> searcher = searcherManager->acquire();
  try {
    TestUtil::assertEquals(1, searcher->getIndexReader()->numDocs());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    searcherManager->release(searcher);
  }
  writer->addDocument(make_shared<Document>());
  writer->commit();
  shared_ptr<AtomicBoolean> *const success = make_shared<AtomicBoolean>(false);
  const std::deque<runtime_error> exc = std::deque<runtime_error>(1);
  shared_ptr<Thread> thread = make_shared<Thread>([&]() {
    try {
      triedReopen->set(true);
      if (VERBOSE) {
        wcout << L"NOW call maybeRefresh" << endl;
      }
      searcherManager->maybeRefresh();
      success->set(true);
    } catch (const AlreadyClosedException &e) {
      // expected
    } catch (const runtime_error &e) {
      if (VERBOSE) {
        wcout << L"FAIL: unexpected exc" << endl;
        e.printStackTrace(System::out);
      }
      exc[0] = e;
      // use success as the barrier here to make sure we see the write
      success->set(false);
    }
  });
  thread->start();
  if (VERBOSE) {
    wcout << L"THREAD started" << endl;
  }
  awaitEnterWarm->await();
  if (VERBOSE) {
    wcout << L"NOW call close" << endl;
  }
  delete searcherManager;
  awaitClose->countDown();
  thread->join();
  expectThrows(AlreadyClosedException::typeid,
               [&]() { searcherManager->acquire(); });
  assertFalse(success->get());
  assertTrue(triedReopen->get());
  assertNull(L"" + exc[0], exc[0]);
  delete writer;
  delete dir;
  if (es != nullptr) {
    es->shutdown();
    es->awaitTermination(1, TimeUnit::SECONDS);
  }
}

TestSearcherManager::SearcherFactoryAnonymousInnerClass2::
    SearcherFactoryAnonymousInnerClass2(
        shared_ptr<TestSearcherManager> outerInstance,
        shared_ptr<CountDownLatch> awaitEnterWarm,
        shared_ptr<CountDownLatch> awaitClose,
        shared_ptr<AtomicBoolean> triedReopen, shared_ptr<ExecutorService> es)
{
  this->outerInstance = outerInstance;
  this->awaitEnterWarm = awaitEnterWarm;
  this->awaitClose = awaitClose;
  this->triedReopen = triedReopen;
  this->es = es;
}

shared_ptr<IndexSearcher>
TestSearcherManager::SearcherFactoryAnonymousInnerClass2::LuceneTestCase::
    newSearcher(shared_ptr<IndexReader> r, shared_ptr<IndexReader> previous)
{
  try {
    if (triedReopen->get()) {
      awaitEnterWarm->countDown();
      awaitClose->await();
    }
  } catch (const InterruptedException &e) {
    //
  }
  return make_shared<IndexSearcher>(r, es);
}

void TestSearcherManager::testCloseTwice() 
{
  // test that we can close SM twice (per Closeable's contract).
  shared_ptr<Directory> dir = newDirectory();
  delete (
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr)));
  shared_ptr<SearcherManager> sm = make_shared<SearcherManager>(dir, nullptr);
  delete sm;
  delete sm;
  delete dir;
}

void TestSearcherManager::testReferenceDecrementIllegally() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>()));
  shared_ptr<SearcherManager> sm = make_shared<SearcherManager>(
      writer, false, false, make_shared<SearcherFactory>());
  writer->addDocument(make_shared<Document>());
  writer->commit();
  sm->maybeRefreshBlocking();

  shared_ptr<IndexSearcher> acquire = sm->acquire();
  shared_ptr<IndexSearcher> acquire2 = sm->acquire();
  sm->release(acquire);
  sm->release(acquire2);

  acquire = sm->acquire();
  acquire->getIndexReader()->decRef();
  sm->release(acquire);
  expectThrows(IllegalStateException::typeid, [&]() { sm->acquire(); });

  // sm.close(); -- already closed
  delete writer;
  delete dir;
}

void TestSearcherManager::testEnsureOpen() 
{
  shared_ptr<Directory> dir = newDirectory();
  delete (
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr)));
  shared_ptr<SearcherManager> sm = make_shared<SearcherManager>(dir, nullptr);
  shared_ptr<IndexSearcher> s = sm->acquire();
  delete sm;

  // this should succeed;
  sm->release(s);

  // this should fail
  expectThrows(AlreadyClosedException::typeid, [&]() { sm->acquire(); });

  // this should fail
  expectThrows(AlreadyClosedException::typeid, [&]() { sm->maybeRefresh(); });

  delete dir;
}

void TestSearcherManager::testListenerCalled() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  shared_ptr<AtomicBoolean> *const afterRefreshCalled =
      make_shared<AtomicBoolean>(false);
  shared_ptr<SearcherManager> sm = make_shared<SearcherManager>(
      iw, false, false, make_shared<SearcherFactory>());
  sm->addListener(make_shared<RefreshListenerAnonymousInnerClass>(
      shared_from_this(), afterRefreshCalled));
  iw->addDocument(make_shared<Document>());
  iw->commit();
  assertFalse(afterRefreshCalled->get());
  sm->maybeRefreshBlocking();
  assertTrue(afterRefreshCalled->get());
  delete sm;
  delete iw;
  delete dir;
}

TestSearcherManager::RefreshListenerAnonymousInnerClass::
    RefreshListenerAnonymousInnerClass(
        shared_ptr<TestSearcherManager> outerInstance,
        shared_ptr<AtomicBoolean> afterRefreshCalled)
{
  this->outerInstance = outerInstance;
  this->afterRefreshCalled = afterRefreshCalled;
}

void TestSearcherManager::RefreshListenerAnonymousInnerClass::beforeRefresh() {}

void TestSearcherManager::RefreshListenerAnonymousInnerClass::afterRefresh(
    bool didRefresh)
{
  if (didRefresh) {
    afterRefreshCalled->set(true);
  }
}

void TestSearcherManager::testEvilSearcherFactory() 
{
  shared_ptr<Random> *const random = TestSearcherManager::random();
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random, dir);
  w->commit();

  shared_ptr<IndexReader> *const other = DirectoryReader::open(dir);

  shared_ptr<SearcherFactory> *const theEvilOne =
      make_shared<SearcherFactoryAnonymousInnerClass3>(shared_from_this(),
                                                       other);

  expectThrows(IllegalStateException::typeid,
               [&]() { make_shared<SearcherManager>(dir, theEvilOne); });
  expectThrows(IllegalStateException::typeid, [&]() {
    make_shared<SearcherManager>(w->w, random->nextBoolean(), false,
                                 theEvilOne);
  });
  delete w;
  delete other;
  delete dir;
}

TestSearcherManager::SearcherFactoryAnonymousInnerClass3::
    SearcherFactoryAnonymousInnerClass3(
        shared_ptr<TestSearcherManager> outerInstance,
        shared_ptr<IndexReader> other)
{
  this->outerInstance = outerInstance;
  this->other = other;
}

shared_ptr<IndexSearcher>
TestSearcherManager::SearcherFactoryAnonymousInnerClass3::LuceneTestCase::
    newSearcher(shared_ptr<IndexReader> ignored,
                shared_ptr<IndexReader> previous)
{
  return LuceneTestCase::newSearcher(other);
}

void TestSearcherManager::testMaybeRefreshBlockingLock() 
{
  // make sure that maybeRefreshBlocking releases the lock, otherwise other
  // threads cannot obtain it.
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  delete w;

  shared_ptr<SearcherManager> *const sm =
      make_shared<SearcherManager>(dir, nullptr);

  shared_ptr<Thread> t =
      make_shared<ThreadAnonymousInnerClass2>(shared_from_this(), sm);
  t->start();
  t->join();

  // if maybeRefreshBlocking didn't release the lock, this will fail.
  assertTrue(L"failde to obtain the refreshLock!", sm->maybeRefresh());

  delete sm;
  delete dir;
}

TestSearcherManager::ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
    shared_ptr<TestSearcherManager> outerInstance,
    shared_ptr<org::apache::lucene::search::SearcherManager> sm)
{
  this->outerInstance = outerInstance;
  this->sm = sm;
}

void TestSearcherManager::ThreadAnonymousInnerClass2::run()
{
  try {
    // this used to not release the lock, preventing other threads from
    // obtaining it.
    sm->maybeRefreshBlocking();
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

TestSearcherManager::MyFilterLeafReader::MyFilterLeafReader(
    shared_ptr<LeafReader> in_)
    : org::apache::lucene::index::FilterLeafReader(in_)
{
}

shared_ptr<IndexReader::CacheHelper>
TestSearcherManager::MyFilterLeafReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

shared_ptr<IndexReader::CacheHelper>
TestSearcherManager::MyFilterLeafReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

TestSearcherManager::MyFilterDirectoryReader::MyFilterDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  FilterDirectoryReader(in_,
                        make_shared<SubReaderWrapperAnonymousInnerClass>());
}

TestSearcherManager::MyFilterDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::SubReaderWrapperAnonymousInnerClass()
{
}

shared_ptr<LeafReader> TestSearcherManager::MyFilterDirectoryReader::
    SubReaderWrapperAnonymousInnerClass::wrap(shared_ptr<LeafReader> reader)
{
  shared_ptr<FilterLeafReader> wrapped =
      make_shared<MyFilterLeafReader>(reader);
  TestUtil::assertEquals(reader, wrapped->getDelegate());
  return wrapped;
}

shared_ptr<DirectoryReader>
TestSearcherManager::MyFilterDirectoryReader::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<MyFilterDirectoryReader>(in_);
}

shared_ptr<IndexReader::CacheHelper>
TestSearcherManager::MyFilterDirectoryReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

void TestSearcherManager::testCustomDirectoryReader() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<DirectoryReader> nrtReader = w->getReader();

  shared_ptr<FilterDirectoryReader> reader =
      make_shared<MyFilterDirectoryReader>(nrtReader);
  TestUtil::assertEquals(nrtReader, reader->getDelegate());
  TestUtil::assertEquals(FilterDirectoryReader::unwrap(nrtReader),
                         FilterDirectoryReader::unwrap(reader));

  shared_ptr<SearcherManager> mgr =
      make_shared<SearcherManager>(reader, nullptr);
  for (int i = 0; i < 10; i++) {
    w->addDocument(make_shared<Document>());
    mgr->maybeRefresh();
    shared_ptr<IndexSearcher> s = mgr->acquire();
    try {
      assertTrue(std::dynamic_pointer_cast<MyFilterDirectoryReader>(
                     s->getIndexReader()) != nullptr);
      for (auto ctx : s->getIndexReader()->leaves()) {
        assertTrue(std::dynamic_pointer_cast<MyFilterLeafReader>(
                       ctx->reader()) != nullptr);
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      mgr->release(s);
    }
  }
  delete mgr;
  delete w;
  delete dir;
}

void TestSearcherManager::testPreviousReaderIsPassed() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriter> *const w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  w->addDocument(make_shared<Document>());
  // C++ TODO: Local classes are not converted by Java to C++ Converter:
  //      class MySearcherFactory extends SearcherFactory
  //    {
  //      IndexReader lastReader = nullptr;
  //      IndexReader lastPreviousReader = nullptr;
  //      int called = 0;
  //      @@Override public IndexSearcher newSearcher(IndexReader reader,
  //      IndexReader previousReader) throws IOException
  //      {
  //        called++;
  //        lastReader = reader;
  //        lastPreviousReader = previousReader;
  //        return super.newSearcher(reader, previousReader);
  //      }
  //    }

  shared_ptr<MySearcherFactory> factory = make_shared<MySearcherFactory>();
  shared_ptr<SearcherManager> *const sm =
      make_shared<SearcherManager>(w, random()->nextBoolean(), false, factory);
  TestUtil::assertEquals(1, factory->called);
  assertNull(factory->lastPreviousReader);
  assertNotNull(factory->lastReader);
  shared_ptr<IndexSearcher> acquire = sm->acquire();
  assertSame(factory->lastReader, acquire->getIndexReader());
  sm->release(acquire);

  shared_ptr<IndexReader> *const lastReader = factory->lastReader;
  // refresh
  w->addDocument(make_shared<Document>());
  assertTrue(sm->maybeRefresh());

  acquire = sm->acquire();
  assertSame(factory->lastReader, acquire->getIndexReader());
  sm->release(acquire);
  assertNotNull(factory->lastPreviousReader);
  assertSame(lastReader, factory->lastPreviousReader);
  assertNotSame(factory->lastReader, lastReader);
  TestUtil::assertEquals(2, factory->called);
  delete w;
  delete sm;
  delete dir;
}

void TestSearcherManager::testConcurrentIndexCloseSearchAndRefresh() throw(
    runtime_error)
{
  shared_ptr<Directory> *const dir = newFSDirectory(createTempDir());
  shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>> writerRef =
      make_shared<AtomicReference<std::shared_ptr<IndexWriter>>>();
  shared_ptr<MockAnalyzer> *const analyzer =
      make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(IndexWriter::MAX_TERM_LENGTH);
  writerRef->set(make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer)));

  shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>> mgrRef =
      make_shared<AtomicReference<std::shared_ptr<SearcherManager>>>();
  mgrRef->set(make_shared<SearcherManager>(writerRef->get(), nullptr));
  shared_ptr<AtomicBoolean> *const stop = make_shared<AtomicBoolean>();

  shared_ptr<Thread> indexThread = make_shared<ThreadAnonymousInnerClass>(
      shared_from_this(), dir, writerRef, analyzer, stop);

  shared_ptr<Thread> searchThread =
      make_shared<ThreadAnonymousInnerClass2>(shared_from_this(), mgrRef, stop);

  shared_ptr<Thread> refreshThread =
      make_shared<ThreadAnonymousInnerClass3>(shared_from_this(), mgrRef, stop);

  shared_ptr<Thread> closeThread = make_shared<ThreadAnonymousInnerClass4>(
      shared_from_this(), writerRef, mgrRef, stop);

  indexThread->start();
  searchThread->start();
  refreshThread->start();
  closeThread->start();

  indexThread->join();
  searchThread->join();
  refreshThread->join();
  closeThread->join();

  mgrRef->get()->close();
  writerRef->get()->close();
  delete dir;
}

TestSearcherManager::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestSearcherManager> outerInstance, shared_ptr<Directory> dir,
    shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>> writerRef,
    shared_ptr<MockAnalyzer> analyzer, shared_ptr<AtomicBoolean> stop)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
  this->writerRef = writerRef;
  this->analyzer = analyzer;
  this->stop = stop;
}

void TestSearcherManager::ThreadAnonymousInnerClass::run()
{
  try {
    shared_ptr<LineFileDocs> docs =
        make_shared<LineFileDocs>(LuceneTestCase::random());
    int64_t runTimeSec =
        TEST_NIGHTLY ? LuceneTestCase::atLeast(10) : LuceneTestCase::atLeast(2);
    int64_t endTime = System::nanoTime() + runTimeSec * 1000000000;
    while (System::nanoTime() < endTime) {
      shared_ptr<IndexWriter> w = writerRef->get();
      w->addDocument(docs->nextDoc());
      if (LuceneTestCase::random()->nextInt(1000) == 17) {
        if (LuceneTestCase::random()->nextBoolean()) {
          delete w;
        } else {
          w->rollback();
        }
        writerRef->set(make_shared<IndexWriter>(
            dir, LuceneTestCase::newIndexWriterConfig(analyzer)));
      }
    }
    delete docs;
    if (VERBOSE) {
      wcout << L"TEST: index count=" << writerRef->get().maxDoc() << endl;
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    stop->set(true);
  }
}

TestSearcherManager::ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
    shared_ptr<TestSearcherManager> outerInstance,
    shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>> mgrRef,
    shared_ptr<AtomicBoolean> stop)
{
  this->outerInstance = outerInstance;
  this->mgrRef = mgrRef;
  this->stop = stop;
}

void TestSearcherManager::ThreadAnonymousInnerClass2::run()
{
  try {
    int64_t totCount = 0;
    while (stop->get() == false) {
      shared_ptr<SearcherManager> mgr = mgrRef->get();
      if (mgr != nullptr) {
        shared_ptr<IndexSearcher> searcher;
        try {
          searcher = mgr->acquire();
        } catch (const AlreadyClosedException &ace) {
          // ok
          continue;
        }
        totCount += searcher->getIndexReader()->maxDoc();
        mgr->release(searcher);
      }
    }
    if (VERBOSE) {
      wcout << L"TEST: search totCount=" << totCount << endl;
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

TestSearcherManager::ThreadAnonymousInnerClass3::ThreadAnonymousInnerClass3(
    shared_ptr<TestSearcherManager> outerInstance,
    shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>> mgrRef,
    shared_ptr<AtomicBoolean> stop)
{
  this->outerInstance = outerInstance;
  this->mgrRef = mgrRef;
  this->stop = stop;
}

void TestSearcherManager::ThreadAnonymousInnerClass3::run()
{
  try {
    int refreshCount = 0;
    int aceCount = 0;
    while (stop->get() == false) {
      shared_ptr<SearcherManager> mgr = mgrRef->get();
      if (mgr != nullptr) {
        refreshCount++;
        try {
          mgr->maybeRefreshBlocking();
        } catch (const AlreadyClosedException &ace) {
          // ok
          aceCount++;
          continue;
        }
      }
    }
    if (VERBOSE) {
      wcout << L"TEST: refresh count=" << refreshCount << L" aceCount="
            << aceCount << endl;
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

TestSearcherManager::ThreadAnonymousInnerClass4::ThreadAnonymousInnerClass4(
    shared_ptr<TestSearcherManager> outerInstance,
    shared_ptr<AtomicReference<std::shared_ptr<IndexWriter>>> writerRef,
    shared_ptr<AtomicReference<std::shared_ptr<SearcherManager>>> mgrRef,
    shared_ptr<AtomicBoolean> stop)
{
  this->outerInstance = outerInstance;
  this->writerRef = writerRef;
  this->mgrRef = mgrRef;
  this->stop = stop;
}

void TestSearcherManager::ThreadAnonymousInnerClass4::run()
{
  try {
    int closeCount = 0;
    int aceCount = 0;
    while (stop->get() == false) {
      shared_ptr<SearcherManager> mgr = mgrRef->get();
      assert(mgr != nullptr);
      delete mgr;
      closeCount++;
      while (stop->get() == false) {
        try {
          mgrRef->set(make_shared<SearcherManager>(writerRef->get(), nullptr));
          break;
        } catch (const AlreadyClosedException &ace) {
          // ok
          aceCount++;
        }
      }
    }
    if (VERBOSE) {
      wcout << L"TEST: close count=" << closeCount << L" aceCount=" << aceCount
            << endl;
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}
} // namespace org::apache::lucene::search