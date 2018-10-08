using namespace std;

#include "BaseLockFactoryTestCase.h"

namespace org::apache::lucene::store
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using Constants = org::apache::lucene::util::Constants;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using PrintStreamInfoStream = org::apache::lucene::util::PrintStreamInfoStream;
using TestUtil = org::apache::lucene::util::TestUtil;

void BaseLockFactoryTestCase::testBasics() 
{
  shared_ptr<Path> tempPath = createTempDir();
  shared_ptr<Directory> dir = getDirectory(tempPath);

  shared_ptr<Lock> l = dir->obtainLock(L"commit");
  // shouldn't be able to get the lock twice
  expectThrows(LockObtainFailedException::typeid,
               [&]() { dir->obtainLock(L"commit"); });
  delete l;

  // Make sure we can obtain first one again:
  l = dir->obtainLock(L"commit");
  delete l;

  delete dir;
}

void BaseLockFactoryTestCase::testDoubleClose() 
{
  shared_ptr<Path> tempPath = createTempDir();
  shared_ptr<Directory> dir = getDirectory(tempPath);

  shared_ptr<Lock> l = dir->obtainLock(L"commit");
  delete l;
  delete l; // close again, should be no exception

  delete dir;
}

void BaseLockFactoryTestCase::testValidAfterAcquire() 
{
  shared_ptr<Path> tempPath = createTempDir();
  shared_ptr<Directory> dir = getDirectory(tempPath);
  shared_ptr<Lock> l = dir->obtainLock(L"commit");
  l->ensureValid(); // no exception
  delete l;
  delete dir;
}

void BaseLockFactoryTestCase::testInvalidAfterClose() 
{
  shared_ptr<Path> tempPath = createTempDir();
  shared_ptr<Directory> dir = getDirectory(tempPath);

  shared_ptr<Lock> l = dir->obtainLock(L"commit");
  delete l;

  expectThrows(AlreadyClosedException::typeid, [&]() { l->ensureValid(); });

  delete dir;
}

void BaseLockFactoryTestCase::testObtainConcurrently() throw(
    InterruptedException, IOException)
{
  shared_ptr<Path> tempPath = createTempDir();
  shared_ptr<Directory> *const directory = getDirectory(tempPath);
  shared_ptr<AtomicBoolean> *const running = make_shared<AtomicBoolean>(true);
  shared_ptr<AtomicInteger> *const atomicCounter =
      make_shared<AtomicInteger>(0);
  shared_ptr<ReentrantLock> *const assertingLock = make_shared<ReentrantLock>();
  int numThreads = 2 + random()->nextInt(10);
  constexpr int runs = atLeast(10000);
  shared_ptr<CyclicBarrier> barrier = make_shared<CyclicBarrier>(numThreads);
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), directory, running, atomicCounter, assertingLock,
        runs, barrier);
    threads[i]->start();
  }

  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
  }
  delete directory;
}

BaseLockFactoryTestCase::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<BaseLockFactoryTestCase> outerInstance,
    shared_ptr<org::apache::lucene::store::Directory> directory,
    shared_ptr<AtomicBoolean> running, shared_ptr<AtomicInteger> atomicCounter,
    shared_ptr<ReentrantLock> assertingLock, int runs,
    shared_ptr<CyclicBarrier> barrier)
{
  this->outerInstance = outerInstance;
  this->directory = directory;
  this->running = running;
  this->atomicCounter = atomicCounter;
  this->assertingLock = assertingLock;
  this->runs = runs;
  this->barrier = barrier;
}

void BaseLockFactoryTestCase::ThreadAnonymousInnerClass::run()
{
  try {
    barrier->await();
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
  while (running->get()) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (Lock lock =
    // directory.obtainLock("foo.lock"))
    {
      Lock lock = directory->obtainLock(L"foo.lock");
      try {
        assertFalse(assertingLock->isLocked());
        if (assertingLock->tryLock()) {
          assertingLock->unlock();
        } else {
          fail();
        }
        assert(lock != nullptr); // stupid compiler
      } catch (const IOException &ex) {
        //
      }
    }
    if (atomicCounter->incrementAndGet() > runs) {
      running->set(false);
    }
  }
}

void BaseLockFactoryTestCase::testStressLocks() 
{
  shared_ptr<Path> tempPath = createTempDir();
  assumeFalse(L"cannot handle buggy Files.delete",
              TestUtil::hasWindowsFS(tempPath));

  shared_ptr<Directory> dir = getDirectory(tempPath);

  // First create a 1 doc index:
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
               ->setOpenMode(OpenMode::CREATE));
  addDoc(w);
  delete w;

  shared_ptr<WriterThread> writer =
      make_shared<WriterThread>(shared_from_this(), 100, dir);
  shared_ptr<SearcherThread> searcher = make_shared<SearcherThread>(100, dir);
  writer->start();
  searcher->start();

  writer->join();
  searcher->join();

  assertTrue(L"IndexWriter hit unexpected exceptions", !writer->hitException);
  assertTrue(L"IndexSearcher hit unexpected exceptions",
             !searcher->hitException);

  delete dir;
}

void BaseLockFactoryTestCase::addDoc(shared_ptr<IndexWriter> writer) throw(
    IOException)
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  writer->addDocument(doc);
}

BaseLockFactoryTestCase::WriterThread::WriterThread(
    shared_ptr<BaseLockFactoryTestCase> outerInstance, int numIteration,
    shared_ptr<Directory> dir)
    : outerInstance(outerInstance)
{
  this->numIteration = numIteration;
  this->dir = dir;
}

wstring BaseLockFactoryTestCase::WriterThread::toString(
    shared_ptr<ByteArrayOutputStream> baos)
{
  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return baos->toString(L"UTF8");
  } catch (const UnsupportedEncodingException &uee) {
    // shouldn't happen
    throw runtime_error(uee);
  }
}

void BaseLockFactoryTestCase::WriterThread::run()
{
  shared_ptr<IndexWriter> writer = nullptr;
  shared_ptr<ByteArrayOutputStream> baos = make_shared<ByteArrayOutputStream>();
  for (int i = 0; i < this->numIteration; i++) {
    if (VERBOSE) {
      wcout << L"TEST: WriterThread iter=" << i << endl;
    }

    shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>(
        make_shared<MockAnalyzer>(LuceneTestCase::random()));

    // We only print the IW infoStream output on exc, below:
    shared_ptr<PrintStream> printStream;
    try {
      printStream = make_shared<PrintStream>(baos, true, L"UTF8");
    } catch (const UnsupportedEncodingException &uee) {
      // shouldn't happen
      throw runtime_error(uee);
    }

    iwc->setInfoStream(make_shared<PrintStreamInfoStream>(printStream));

    printStream->println(L"\nTEST: WriterThread iter=" + to_wstring(i));
    iwc->setOpenMode(OpenMode::APPEND);
    try {
      writer = make_shared<IndexWriter>(dir, iwc);

    } catch (const runtime_error &t) {
      if (Constants::WINDOWS &&
          std::dynamic_pointer_cast<AccessDeniedException>(t) != nullptr) {
        // LUCENE-6684: suppress this: on Windows, a file in the curious
        // "pending delete" state can cause this exc on IW init, where one
        // thread/process deleted an old segments_N, but the delete hasn't
        // finished yet because other threads/processes still have it open
        printStream->println(L"TEST: AccessDeniedException on init writer");
        t.printStackTrace(printStream);
      } else {
        hitException = true;
        wcout
            << L"Stress Test Index Writer: creation hit unexpected exception: "
            << t.what() << endl;
        t.printStackTrace(System::out);
        wcout << toString(baos) << endl;
      }
      break;
    }
    if (writer != nullptr) {
      try {
        outerInstance->addDoc(writer);
      } catch (const runtime_error &t) {
        hitException = true;
        wcout << L"Stress Test Index Writer: addDoc hit unexpected exception: "
              << t.what() << endl;
        t.printStackTrace(System::out);
        wcout << toString(baos) << endl;
        break;
      }
      try {
        delete writer;
      } catch (const runtime_error &t) {
        hitException = true;
        wcout << L"Stress Test Index Writer: close hit unexpected exception: "
              << t.what() << endl;
        t.printStackTrace(System::out);
        wcout << toString(baos) << endl;
        break;
      }
    }
  }
}

BaseLockFactoryTestCase::SearcherThread::SearcherThread(
    int numIteration, shared_ptr<Directory> dir)
{
  this->numIteration = numIteration;
  this->dir = dir;
}

void BaseLockFactoryTestCase::SearcherThread::run()
{
  shared_ptr<IndexReader> reader = nullptr;
  shared_ptr<IndexSearcher> searcher = nullptr;
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"content", L"aaa"));
  for (int i = 0; i < this->numIteration; i++) {
    try {
      reader = DirectoryReader::open(dir);
      searcher = LuceneTestCase::newSearcher(reader);
    } catch (const runtime_error &e) {
      hitException = true;
      wcout << L"Stress Test Index Searcher: create hit unexpected exception: "
            << e.what() << endl;
      e.printStackTrace(System::out);
      break;
    }
    try {
      searcher->search(query, 1000);
    } catch (const IOException &e) {
      hitException = true;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << L"Stress Test Index Searcher: search hit unexpected exception: "
            << e->toString() << endl;
      e->printStackTrace(System::out);
      break;
    }
    // System.out.println(hits.length() + " total results");
    try {
      delete reader;
    } catch (const IOException &e) {
      hitException = true;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << L"Stress Test Index Searcher: close hit unexpected exception: "
            << e->toString() << endl;
      e->printStackTrace(System::out);
      break;
    }
  }
}
} // namespace org::apache::lucene::store