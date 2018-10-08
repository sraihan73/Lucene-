using namespace std;

#include "TestControlledRealTimeReopenThread.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexCommit = org::apache::lucene::index::IndexCommit;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexableField = org::apache::lucene::index::IndexableField;
using KeepOnlyLastCommitDeletionPolicy =
    org::apache::lucene::index::KeepOnlyLastCommitDeletionPolicy;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using Term = org::apache::lucene::index::Term;
using ThreadedIndexingAndSearchingTestCase =
    org::apache::lucene::index::ThreadedIndexingAndSearchingTestCase;
using Directory = org::apache::lucene::store::Directory;
using NRTCachingDirectory = org::apache::lucene::store::NRTCachingDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

void TestControlledRealTimeReopenThread::
    testControlledRealTimeReopenThread() 
{
  runTest(L"TestControlledRealTimeReopenThread");
}

shared_ptr<IndexSearcher>
TestControlledRealTimeReopenThread::getFinalSearcher() 
{
  if (VERBOSE) {
    wcout << L"TEST: finalSearcher maxGen=" << maxGen << endl;
  }
  nrtDeletesThread->waitForGeneration(maxGen);
  return nrtDeletes->acquire();
}

shared_ptr<Directory>
TestControlledRealTimeReopenThread::getDirectory(shared_ptr<Directory> in_)
{
  // Randomly swap in NRTCachingDir
  if (random()->nextBoolean()) {
    if (VERBOSE) {
      wcout << L"TEST: wrap NRTCachingDir" << endl;
    }

    return make_shared<NRTCachingDirectory>(in_, 5.0, 60.0);
  } else {
    return in_;
  }
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override protected void
// updateDocuments(org.apache.lucene.index.Term id, java.util.List<? extends
// Iterable<? extends org.apache.lucene.index.IndexableField>> docs) throws
// Exception
void TestControlledRealTimeReopenThread::updateDocuments(
    shared_ptr<Term> id, deque<T1> docs) 
{
  constexpr int64_t gen = genWriter->updateDocuments(id, docs);

  // Randomly verify the update "took":
  if (random()->nextInt(20) == 2) {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: verify updateDocuments " << id << L" gen=" << gen
            << endl;
    }
    nrtDeletesThread->waitForGeneration(gen);
    assertTrue(gen <= nrtDeletesThread->getSearchingGen());
    shared_ptr<IndexSearcher> *const s = nrtDeletes->acquire();
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: got deletes searcher=" << s << endl;
    }
    try {
      assertEquals(docs.size(),
                   s->search(make_shared<TermQuery>(id), 10)->totalHits);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nrtDeletes->release(s);
    }
  }

  lastGens->set(gen);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override protected void
// addDocuments(org.apache.lucene.index.Term id, java.util.List<? extends
// Iterable<? extends org.apache.lucene.index.IndexableField>> docs) throws
// Exception
void TestControlledRealTimeReopenThread::addDocuments(
    shared_ptr<Term> id, deque<T1> docs) 
{
  constexpr int64_t gen = genWriter->addDocuments(docs);
  // Randomly verify the add "took":
  if (random()->nextInt(20) == 2) {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: verify addDocuments " << id << L" gen=" << gen << endl;
    }
    nrtNoDeletesThread->waitForGeneration(gen);
    assertTrue(gen <= nrtNoDeletesThread->getSearchingGen());
    shared_ptr<IndexSearcher> *const s = nrtNoDeletes->acquire();
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: got noDeletes searcher=" << s << endl;
    }
    try {
      assertEquals(docs.size(),
                   s->search(make_shared<TermQuery>(id), 10)->totalHits);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nrtNoDeletes->release(s);
    }
  }
  lastGens->set(gen);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override protected void
// addDocument(org.apache.lucene.index.Term id, Iterable<? extends
// org.apache.lucene.index.IndexableField> doc) throws Exception
void TestControlledRealTimeReopenThread::addDocument(
    shared_ptr<Term> id, deque<T1> doc) 
{
  constexpr int64_t gen = genWriter->addDocument(doc);

  // Randomly verify the add "took":
  if (random()->nextInt(20) == 2) {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: verify addDocument " << id << L" gen=" << gen << endl;
    }
    nrtNoDeletesThread->waitForGeneration(gen);
    assertTrue(gen <= nrtNoDeletesThread->getSearchingGen());
    shared_ptr<IndexSearcher> *const s = nrtNoDeletes->acquire();
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: got noDeletes searcher=" << s << endl;
    }
    try {
      assertEquals(1, s->search(make_shared<TermQuery>(id), 10)->totalHits);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nrtNoDeletes->release(s);
    }
  }
  lastGens->set(gen);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override protected void
// updateDocument(org.apache.lucene.index.Term id, Iterable<? extends
// org.apache.lucene.index.IndexableField> doc) throws Exception
void TestControlledRealTimeReopenThread::updateDocument(
    shared_ptr<Term> id, deque<T1> doc) 
{
  constexpr int64_t gen = genWriter->updateDocument(id, doc);
  // Randomly verify the udpate "took":
  if (random()->nextInt(20) == 2) {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: verify updateDocument " << id << L" gen=" << gen
            << endl;
    }
    nrtDeletesThread->waitForGeneration(gen);
    assertTrue(gen <= nrtDeletesThread->getSearchingGen());
    shared_ptr<IndexSearcher> *const s = nrtDeletes->acquire();
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: got deletes searcher=" << s << endl;
    }
    try {
      assertEquals(1, s->search(make_shared<TermQuery>(id), 10)->totalHits);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nrtDeletes->release(s);
    }
  }
  lastGens->set(gen);
}

void TestControlledRealTimeReopenThread::deleteDocuments(
    shared_ptr<Term> id) 
{
  constexpr int64_t gen = genWriter->deleteDocuments({id});
  // randomly verify the delete "took":
  if (random()->nextInt(20) == 7) {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: verify deleteDocuments " << id << L" gen=" << gen
            << endl;
    }
    nrtDeletesThread->waitForGeneration(gen);
    assertTrue(gen <= nrtDeletesThread->getSearchingGen());
    shared_ptr<IndexSearcher> *const s = nrtDeletes->acquire();
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L": nrt: got deletes searcher=" << s << endl;
    }
    try {
      assertEquals(0, s->search(make_shared<TermQuery>(id), 10)->totalHits);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      nrtDeletes->release(s);
    }
  }
  lastGens->set(gen);
}

void TestControlledRealTimeReopenThread::doAfterWriter(
    shared_ptr<ExecutorService> es) 
{
  constexpr double minReopenSec = 0.01 + 0.05 * random()->nextDouble();
  constexpr double maxReopenSec =
      minReopenSec * (1.0 + 10 * random()->nextDouble());

  if (VERBOSE) {
    wcout << L"TEST: make SearcherManager maxReopenSec=" << maxReopenSec
          << L" minReopenSec=" << minReopenSec << endl;
  }

  genWriter = writer;

  shared_ptr<SearcherFactory> *const sf =
      make_shared<SearcherFactoryAnonymousInnerClass>(shared_from_this(), es);

  nrtNoDeletes = make_shared<SearcherManager>(writer, false, false, sf);
  nrtDeletes = make_shared<SearcherManager>(writer, sf);

  nrtDeletesThread = make_shared<
      ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>(
      genWriter, nrtDeletes, maxReopenSec, minReopenSec);
  nrtDeletesThread->setName(L"NRTDeletes Reopen Thread");
  nrtDeletesThread->setPriority(
      min(Thread::currentThread().getPriority() + 2, Thread::MAX_PRIORITY));
  nrtDeletesThread->setDaemon(true);
  nrtDeletesThread->start();

  nrtNoDeletesThread = make_shared<
      ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>(
      genWriter, nrtNoDeletes, maxReopenSec, minReopenSec);
  nrtNoDeletesThread->setName(L"NRTNoDeletes Reopen Thread");
  nrtNoDeletesThread->setPriority(
      min(Thread::currentThread().getPriority() + 2, Thread::MAX_PRIORITY));
  nrtNoDeletesThread->setDaemon(true);
  nrtNoDeletesThread->start();
}

TestControlledRealTimeReopenThread::SearcherFactoryAnonymousInnerClass::
    SearcherFactoryAnonymousInnerClass(
        shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        shared_ptr<ExecutorService> es)
{
  this->outerInstance = outerInstance;
  this->es = es;
}

shared_ptr<IndexSearcher> TestControlledRealTimeReopenThread::
    SearcherFactoryAnonymousInnerClass::LuceneTestCase::newSearcher(
        shared_ptr<IndexReader> r,
        shared_ptr<IndexReader> previous) 
{
  outerInstance->warmCalled = true;
  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r, es);
  s->search(make_shared<TermQuery>(make_shared<Term>(L"body", L"united")), 10);
  return s;
}

void TestControlledRealTimeReopenThread::doAfterIndexingThreadDone()
{
  optional<int64_t> gen = lastGens->get();
  if (gen) {
    addMaxGen(gen);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void TestControlledRealTimeReopenThread::addMaxGen(int64_t gen)
{
  maxGen = max(gen, maxGen);
}

void TestControlledRealTimeReopenThread::doSearching(
    shared_ptr<ExecutorService> es, int64_t stopTime) 
{
  runSearchThreads(stopTime);
}

shared_ptr<IndexSearcher>
TestControlledRealTimeReopenThread::getCurrentSearcher() 
{
  // Test doesn't assert deletions until the end, so we
  // can randomize whether dels must be applied
  shared_ptr<SearcherManager> *const nrt;
  if (random()->nextBoolean()) {
    nrt = nrtDeletes;
  } else {
    nrt = nrtNoDeletes;
  }

  return nrt->acquire();
}

void TestControlledRealTimeReopenThread::releaseSearcher(
    shared_ptr<IndexSearcher> s) 
{
  // NOTE: a bit iffy... technically you should release
  // against the same SearcherManager you acquired from... but
  // both impls just decRef the underlying reader so we
  // can get away w/ cheating:
  nrtNoDeletes->release(s);
}

void TestControlledRealTimeReopenThread::doClose() 
{
  assertTrue(warmCalled);
  if (VERBOSE) {
    wcout << L"TEST: now close SearcherManagers" << endl;
  }
  delete nrtDeletesThread;
  delete nrtDeletes;
  delete nrtNoDeletesThread;
  delete nrtNoDeletes;
}

void TestControlledRealTimeReopenThread::
    testThreadStarvationNoDeleteNRTReader() throw(IOException,
                                                  InterruptedException)
{
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<CountDownLatch> *const latch = make_shared<CountDownLatch>(1);
  shared_ptr<CountDownLatch> *const signal = make_shared<CountDownLatch>(1);

  shared_ptr<LatchedIndexWriter> writer =
      make_shared<LatchedIndexWriter>(d, conf, latch, signal);
  shared_ptr<SearcherManager> *const manager =
      make_shared<SearcherManager>(writer, false, false, nullptr);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"test", L"test", Field::Store::YES));
  writer->addDocument(doc);
  manager->maybeRefresh();
  shared_ptr<Thread> t = make_shared<ThreadAnonymousInnerClass>(
      shared_from_this(), latch, signal, writer, manager);
  t->start();
  writer->waitAfterUpdate =
      true; // wait in addDocument to let some reopens go through

  constexpr int64_t lastGen = writer->updateDocument(
      make_shared<Term>(L"foo", L"bar"),
      doc); // once this returns the doc is already reflected in the last reopen

  // We now eagerly resolve deletes so the manager should see it after update:
  assertTrue(manager->isSearcherCurrent());

  shared_ptr<IndexSearcher> searcher = manager->acquire();
  try {
    assertEquals(2, searcher->getIndexReader()->numDocs());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    manager->release(searcher);
  }
  shared_ptr<ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>
      *const thread = make_shared<
          ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>(
          writer, manager, 0.01, 0.01);
  thread->start(); // start reopening
  if (VERBOSE) {
    wcout << L"waiting now for generation " << lastGen << endl;
  }

  shared_ptr<AtomicBoolean> *const finished = make_shared<AtomicBoolean>(false);
  shared_ptr<Thread> waiter = make_shared<ThreadAnonymousInnerClass2>(
      shared_from_this(), lastGen, thread, finished);
  waiter->start();
  manager->maybeRefresh();
  waiter->join(1000);
  if (!finished->get()) {
    waiter->interrupt();
    fail(L"thread deadlocked on waitForGeneration");
  }
  delete thread;
  thread->join();
  delete writer;
  IOUtils::close({manager, d});
}

TestControlledRealTimeReopenThread::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        shared_ptr<CountDownLatch> latch, shared_ptr<CountDownLatch> signal,
        shared_ptr<org::apache::lucene::search::
                       TestControlledRealTimeReopenThread::LatchedIndexWriter>
            writer,
        shared_ptr<org::apache::lucene::search::SearcherManager> manager)
{
  this->outerInstance = outerInstance;
  this->latch = latch;
  this->signal = signal;
  this->writer = writer;
  this->manager = manager;
}

void TestControlledRealTimeReopenThread::ThreadAnonymousInnerClass::run()
{
  try {
    signal->await();
    manager->maybeRefresh();
    writer->deleteDocuments(
        {make_shared<TermQuery>(make_shared<Term>(L"foo", L"barista"))});
    manager
        ->maybeRefresh(); // kick off another reopen so we inc. the internal gen
  } catch (const runtime_error &e) {
    e.printStackTrace();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    latch->countDown(); // let the add below finish
  }
}

TestControlledRealTimeReopenThread::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(
        shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        int64_t lastGen,
        shared_ptr<org::apache::lucene::search::ControlledRealTimeReopenThread<
            std::shared_ptr<IndexSearcher>>>
            thread,
        shared_ptr<AtomicBoolean> finished)
{
  this->outerInstance = outerInstance;
  this->lastGen = lastGen;
  this->thread = thread;
  this->finished = finished;
}

void TestControlledRealTimeReopenThread::ThreadAnonymousInnerClass2::run()
{
  try {
    thread->waitForGeneration(lastGen);
  } catch (const InterruptedException &ie) {
    Thread::currentThread().interrupt();
    throw runtime_error(ie);
  }
  finished->set(true);
}

TestControlledRealTimeReopenThread::LatchedIndexWriter::LatchedIndexWriter(
    shared_ptr<Directory> d, shared_ptr<IndexWriterConfig> conf,
    shared_ptr<CountDownLatch> latch,
    shared_ptr<CountDownLatch> signal) 
    : org::apache::lucene::index::IndexWriter(d, conf)
{
  this->latch = latch;
  this->signal = signal;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public long
// updateDocument(org.apache.lucene.index.Term term, Iterable<? extends
// org.apache.lucene.index.IndexableField> doc) throws java.io.IOException
int64_t
TestControlledRealTimeReopenThread::LatchedIndexWriter::updateDocument(
    shared_ptr<Term> term, deque<T1> doc) 
{
  int64_t result = IndexWriter::updateDocument(term, doc);
  try {
    if (waitAfterUpdate) {
      signal->countDown();
      latch->await();
    }
  } catch (const InterruptedException &e) {
    throw make_shared<ThreadInterruptedException>(e);
  }
  return result;
}

void TestControlledRealTimeReopenThread::testEvilSearcherFactory() throw(
    runtime_error)
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  w->commit();

  shared_ptr<IndexReader> *const other = DirectoryReader::open(dir);

  shared_ptr<SearcherFactory> *const theEvilOne =
      make_shared<SearcherFactoryAnonymousInnerClass>(shared_from_this(),
                                                      other);

  expectThrows(IllegalStateException::typeid, [&]() {
    make_shared<SearcherManager>(w->w, false, false, theEvilOne);
  });

  delete w;
  delete other;
  delete dir;
}

TestControlledRealTimeReopenThread::SearcherFactoryAnonymousInnerClass::
    SearcherFactoryAnonymousInnerClass(
        shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        shared_ptr<IndexReader> other)
{
  this->outerInstance = outerInstance;
  this->other = other;
}

shared_ptr<IndexSearcher>
TestControlledRealTimeReopenThread::SearcherFactoryAnonymousInnerClass::
    LuceneTestCase::newSearcher(shared_ptr<IndexReader> ignored,
                                shared_ptr<IndexReader> previous)
{
  return LuceneTestCase::newSearcher(other);
}

void TestControlledRealTimeReopenThread::testListenerCalled() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(nullptr));
  shared_ptr<AtomicBoolean> *const afterRefreshCalled =
      make_shared<AtomicBoolean>(false);
  shared_ptr<SearcherManager> sm =
      make_shared<SearcherManager>(iw, make_shared<SearcherFactory>());
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

TestControlledRealTimeReopenThread::RefreshListenerAnonymousInnerClass::
    RefreshListenerAnonymousInnerClass(
        shared_ptr<TestControlledRealTimeReopenThread> outerInstance,
        shared_ptr<AtomicBoolean> afterRefreshCalled)
{
  this->outerInstance = outerInstance;
  this->afterRefreshCalled = afterRefreshCalled;
}

void TestControlledRealTimeReopenThread::RefreshListenerAnonymousInnerClass::
    beforeRefresh()
{
}

void TestControlledRealTimeReopenThread::RefreshListenerAnonymousInnerClass::
    afterRefresh(bool didRefresh)
{
  if (didRefresh) {
    afterRefreshCalled->set(true);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AwaitsFix(bugUrl =
// "https://issues.apache.org/jira/browse/LUCENE-5737") public void
// testCRTReopen() throws Exception
void TestControlledRealTimeReopenThread::testCRTReopen() 
{
  // test behaving badly

  // should be high enough
  int maxStaleSecs = 20;

  // build crap data just to store it.
  wstring s = L"        abcdefghijklmnopqrstuvwxyz     ";
  std::deque<wchar_t> chars = s.toCharArray();
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>(2048);
  for (int i = 0; i < 2048; i++) {
    builder->append(chars[random()->nextInt(chars.size())]);
  }
  wstring content = builder->toString();

  shared_ptr<SnapshotDeletionPolicy> *const sdp =
      make_shared<SnapshotDeletionPolicy>(
          make_shared<KeepOnlyLastCommitDeletionPolicy>());
  shared_ptr<Directory> *const dir = make_shared<NRTCachingDirectory>(
      newFSDirectory(createTempDir(L"nrt")), 5, 128);
  shared_ptr<IndexWriterConfig> config =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  config->setCommitOnClose(true);
  config->setIndexDeletionPolicy(sdp);
  config->setOpenMode(IndexWriterConfig::OpenMode::CREATE_OR_APPEND);
  shared_ptr<IndexWriter> *const iw = make_shared<IndexWriter>(dir, config);
  shared_ptr<SearcherManager> sm =
      make_shared<SearcherManager>(iw, make_shared<SearcherFactory>());
  shared_ptr<ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>
      controlledRealTimeReopenThread = make_shared<
          ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>(
          iw, sm, maxStaleSecs, 0);

  controlledRealTimeReopenThread->setDaemon(true);
  controlledRealTimeReopenThread->start();

  deque<std::shared_ptr<Thread>> commitThreads =
      deque<std::shared_ptr<Thread>>();

  for (int i = 0; i < 500; i++) {
    if (i > 0 && i % 50 == 0) {
      shared_ptr<Thread> commitThread = make_shared<Thread>([&]() {
        try {
          iw->commit();
          shared_ptr<IndexCommit> ic = sdp->snapshot();
          for (auto name : ic->getFileNames()) {
            // distribute, and backup
            // System.out.println(names);
            assertTrue(slowFileExists(dir, name));
          }
        } catch (const runtime_error &e) {
          throw runtime_error(e);
        }
      });
      commitThread->start();
      commitThreads.push_back(commitThread);
    }
    shared_ptr<Document> d = make_shared<Document>();
    d->push_back(make_shared<TextField>(L"count", to_wstring(i) + L"",
                                        Field::Store::NO));
    d->push_back(
        make_shared<TextField>(L"content", content, Field::Store::YES));
    int64_t start = System::currentTimeMillis();
    int64_t l = iw->addDocument(d);
    controlledRealTimeReopenThread->waitForGeneration(l);
    int64_t wait = System::currentTimeMillis() - start;
    assertTrue(L"waited too long for generation " + to_wstring(wait),
               wait < (maxStaleSecs * 1000));
    shared_ptr<IndexSearcher> searcher = sm->acquire();
    shared_ptr<TopDocs> td =
        searcher->search(make_shared<TermQuery>(
                             make_shared<Term>(L"count", to_wstring(i) + L"")),
                         10);
    sm->release(searcher);
    assertEquals(1, td->totalHits);
  }

  for (auto commitThread : commitThreads) {
    commitThread->join();
  }

  delete controlledRealTimeReopenThread;
  delete sm;
  delete iw;
  delete dir;
}

void TestControlledRealTimeReopenThread::testDeleteAll() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig());
  shared_ptr<SearcherManager> mgr =
      make_shared<SearcherManager>(w, make_shared<SearcherFactory>());
  nrtDeletesThread = make_shared<
      ControlledRealTimeReopenThread<std::shared_ptr<IndexSearcher>>>(
      w, mgr, 0.1, 0.01);
  nrtDeletesThread->setName(L"NRTDeletes Reopen Thread");
  nrtDeletesThread->setDaemon(true);
  nrtDeletesThread->start();

  int64_t gen1 = w->addDocument(make_shared<Document>());
  int64_t gen2 = w->deleteAll();
  nrtDeletesThread->waitForGeneration(gen2);
  IOUtils::close({nrtDeletesThread, nrtDeletes, w, dir});
}
} // namespace org::apache::lucene::search