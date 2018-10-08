using namespace std;

#include "TestIndexWriterThreadsToSegments.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;

void TestIndexWriterThreadsToSegments::testSegmentCountOnFlushBasic() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  shared_ptr<CountDownLatch> *const startDone = make_shared<CountDownLatch>(2);
  shared_ptr<CountDownLatch> *const middleGun = make_shared<CountDownLatch>(1);
  shared_ptr<CountDownLatch> *const finalGun = make_shared<CountDownLatch>(1);
  std::deque<std::shared_ptr<Thread>> threads(2);
  for (int i = 0; i < threads.size(); i++) {
    constexpr int threadID = i;
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), w, startingGun, startDone, middleGun, finalGun,
        threadID);
    threads[i]->start();
  }

  startingGun->countDown();
  startDone->await();

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  TestUtil::assertEquals(2, r->numDocs());
  int numSegments = r->leaves().size();
  // 1 segment if the threads ran sequentially, else 2:
  assertTrue(numSegments <= 2);
  delete r;

  middleGun->countDown();
  threads[0]->join();

  finalGun->countDown();
  threads[1]->join();

  r = DirectoryReader::open(w);
  TestUtil::assertEquals(4, r->numDocs());
  // Both threads should have shared a single thread state since they did not
  // try to index concurrently:
  TestUtil::assertEquals(1 + numSegments, r->leaves().size());
  delete r;

  delete w;
  delete dir;
}

TestIndexWriterThreadsToSegments::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestIndexWriterThreadsToSegments> outerInstance,
        shared_ptr<org::apache::lucene::index::IndexWriter> w,
        shared_ptr<CountDownLatch> startingGun,
        shared_ptr<CountDownLatch> startDone,
        shared_ptr<CountDownLatch> middleGun,
        shared_ptr<CountDownLatch> finalGun, int threadID)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->startingGun = startingGun;
  this->startDone = startDone;
  this->middleGun = middleGun;
  this->finalGun = finalGun;
  this->threadID = threadID;
}

void TestIndexWriterThreadsToSegments::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(LuceneTestCase::newTextField(L"field", L"here is some text",
                                                Field::Store::NO));
    w->addDocument(doc);
    startDone->countDown();

    middleGun->await();
    if (threadID == 0) {
      w->addDocument(doc);
    } else {
      finalGun->await();
      w->addDocument(doc);
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

TestIndexWriterThreadsToSegments::CheckSegmentCount::CheckSegmentCount(
    shared_ptr<IndexWriter> w, shared_ptr<AtomicInteger> maxThreadCountPerIter,
    shared_ptr<AtomicInteger> indexingCount) 
    : w(w), maxThreadCountPerIter(maxThreadCountPerIter),
      indexingCount(indexingCount)
{
  r = DirectoryReader::open(w);
  TestUtil::assertEquals(0, r->leaves()->size());
  setNextIterThreadCount();
}

void TestIndexWriterThreadsToSegments::CheckSegmentCount::run()
{
  try {
    int oldSegmentCount = r->leaves()->size();
    shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
    assertNotNull(r2);
    r->close();
    r = r2;
    int maxExpectedSegments = oldSegmentCount + maxThreadCountPerIter->get();
    if (VERBOSE) {
      wcout << L"TEST: iter done; now verify oldSegCount=" << oldSegmentCount
            << L" newSegCount=" << r2->leaves()->size() << L" maxExpected="
            << maxExpectedSegments << endl;
    }
    // NOTE: it won't necessarily be ==, in case some threads were strangely
    // scheduled and never conflicted with one another (should be uncommon...?):
    assertTrue(r->leaves()->size() <= maxExpectedSegments);
    setNextIterThreadCount();
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TestIndexWriterThreadsToSegments::CheckSegmentCount::
    setNextIterThreadCount()
{
  indexingCount->set(0);
  maxThreadCountPerIter->set(
      TestUtil::nextInt(LuceneTestCase::random(), 1, MAX_THREADS_AT_ONCE));
  if (VERBOSE) {
    wcout << L"TEST: iter set maxThreadCount=" << maxThreadCountPerIter->get()
          << endl;
  }
}

TestIndexWriterThreadsToSegments::CheckSegmentCount::~CheckSegmentCount()
{
  r->close();
  r.reset();
}

void TestIndexWriterThreadsToSegments::testSegmentCountOnFlushRandom() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));

  // Never trigger flushes (so we only flush on getReader):
  iwc->setMaxBufferedDocs(100000000);
  iwc->setRAMBufferSizeMB(-1);

  // Never trigger merges (so we can simplistically count flushed segments):
  iwc->setMergePolicy(NoMergePolicy::INSTANCE);

  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);

  // How many threads are indexing in the current cycle:
  shared_ptr<AtomicInteger> *const indexingCount = make_shared<AtomicInteger>();

  // How many threads we will use on each cycle:
  shared_ptr<AtomicInteger> *const maxThreadCount =
      make_shared<AtomicInteger>();

  shared_ptr<CheckSegmentCount> checker =
      make_shared<CheckSegmentCount>(w, maxThreadCount, indexingCount);

  // We spin up 10 threads up front, but then in between flushes we limit how
  // many can run on each iteration
  constexpr int ITERS = TEST_NIGHTLY ? 300 : 10;
  std::deque<std::shared_ptr<Thread>> threads(MAX_THREADS_AT_ONCE);

  // We use this to stop all threads once they've indexed their docs in the
  // current iter, and pull a new NRT reader, and verify the segment count:
  shared_ptr<CyclicBarrier> *const barrier =
      make_shared<CyclicBarrier>(MAX_THREADS_AT_ONCE, checker);

  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), w, indexingCount, maxThreadCount, ITERS, barrier);
    threads[i]->start();
  }

  for (auto t : threads) {
    t->join();
  }

  IOUtils::close({checker, w, dir});
}

TestIndexWriterThreadsToSegments::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestIndexWriterThreadsToSegments> outerInstance,
        shared_ptr<org::apache::lucene::index::IndexWriter> w,
        shared_ptr<AtomicInteger> indexingCount,
        shared_ptr<AtomicInteger> maxThreadCount, int ITERS,
        shared_ptr<CyclicBarrier> barrier)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->indexingCount = indexingCount;
  this->maxThreadCount = maxThreadCount;
  this->ITERS = ITERS;
  this->barrier = barrier;
}

void TestIndexWriterThreadsToSegments::ThreadAnonymousInnerClass::run()
{
  try {
    for (int iter = 0; iter < ITERS; iter++) {
      if (indexingCount->incrementAndGet() <= maxThreadCount->get()) {
        if (VERBOSE) {
          wcout << L"TEST: " << Thread::currentThread().getName()
                << L": do index" << endl;
        }

        // We get to index on this cycle:
        shared_ptr<Document> doc = make_shared<Document>();
        doc->push_back(make_shared<TextField>(
            L"field",
            L"here is some text that is a bit longer than normal trivial text",
            Field::Store::NO));
        for (int j = 0; j < 200; j++) {
          w->addDocument(doc);
        }
      } else {
        // We lose: no indexing for us on this cycle
        if (VERBOSE) {
          wcout << L"TEST: " << Thread::currentThread().getName()
                << L": don't index" << endl;
        }
      }
      barrier->await();
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TestIndexWriterThreadsToSegments::testManyThreadsClose() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> r = random();
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(r, make_shared<MockAnalyzer>(r));
  iwc->setCommitOnClose(false);
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(r, dir, iwc);
  w->setDoRandomForceMerge(false);
  std::deque<std::shared_ptr<Thread>> threads(
      TestUtil::nextInt(random(), 4, 30));
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass2>(shared_from_this(), w,
                                                         startingGun);
    threads[i]->start();
  }

  startingGun->countDown();

  delay(100);
  try {
    delete w;
  } catch (const IllegalStateException &ise) {
    // OK but not required
  }
  for (auto t : threads) {
    t->join();
  }
  delete w;
  delete dir;
}

TestIndexWriterThreadsToSegments::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(
        shared_ptr<TestIndexWriterThreadsToSegments> outerInstance,
        shared_ptr<org::apache::lucene::index::RandomIndexWriter> w,
        shared_ptr<CountDownLatch> startingGun)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->startingGun = startingGun;
}

void TestIndexWriterThreadsToSegments::ThreadAnonymousInnerClass2::run()
{
  try {
    startingGun->await();
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<TextField>(
        L"field",
        L"here is some text that is a bit longer than normal trivial text",
        Field::Store::NO));
    for (int j = 0; j < 1000; j++) {
      w->addDocument(doc);
    }
  } catch (const AlreadyClosedException &ace) {
    // ok
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void TestIndexWriterThreadsToSegments::testDocsStuckInRAMForever() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setRAMBufferSizeMB(.2);
  shared_ptr<Codec> codec = TestUtil::getDefaultCodec();
  iwc->setCodec(codec);
  iwc->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  std::deque<std::shared_ptr<Thread>> threads(2);
  for (int i = 0; i < threads.size(); i++) {
    constexpr int threadID = i;
    threads[i] = make_shared<ThreadAnonymousInnerClass3>(shared_from_this(), w,
                                                         startingGun, threadID);
    threads[i]->start();
  }

  startingGun->countDown();
  for (auto t : threads) {
    t->join();
  }

  shared_ptr<Set<wstring>> segSeen = unordered_set<wstring>();
  int thread0Count = 0;
  int thread1Count = 0;

  // At this point the writer should have 2 thread states w/ docs; now we index
  // with only 1 thread until we see all 1000 thread0 & thread1 docs flushed. If
  // the writer incorrectly holds onto previously indexed docs forever then this
  // will run forever:
  int64_t counter = 0;
  int64_t checkAt = 100;
  while (thread0Count < 1000 || thread1Count < 1000) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(L"field", L"threadIDmain", Field::Store::NO));
    w->addDocument(doc);
    if (counter++ == checkAt) {
      for (auto fileName : dir->listAll()) {
        if (fileName.endsWith(L".si")) {
          wstring segName = IndexFileNames::parseSegmentName(fileName);
          if (segSeen->contains(segName) == false) {
            segSeen->add(segName);
            std::deque<char> id = readSegmentInfoID(dir, fileName);
            shared_ptr<SegmentInfo> si =
                TestUtil::getDefaultCodec()->segmentInfoFormat()->read(
                    dir, segName, id, IOContext::DEFAULT);
            si->setCodec(codec);
            shared_ptr<SegmentCommitInfo> sci =
                make_shared<SegmentCommitInfo>(si, 0, 0, -1, -1, -1);
            shared_ptr<SegmentReader> sr = make_shared<SegmentReader>(
                sci, Version::LATEST->major, IOContext::DEFAULT);
            try {
              thread0Count +=
                  sr->docFreq(make_shared<Term>(L"field", L"threadID0"));
              thread1Count +=
                  sr->docFreq(make_shared<Term>(L"field", L"threadID1"));
            }
            // C++ TODO: There is no native C++ equivalent to the exception
            // 'finally' clause:
            finally {
              delete sr;
            }
          }
        }
      }

      checkAt = static_cast<int64_t>(checkAt * 1.25);
      counter = 0;
    }
  }

  delete w;
  delete dir;
}

TestIndexWriterThreadsToSegments::ThreadAnonymousInnerClass3::
    ThreadAnonymousInnerClass3(
        shared_ptr<TestIndexWriterThreadsToSegments> outerInstance,
        shared_ptr<org::apache::lucene::index::IndexWriter> w,
        shared_ptr<CountDownLatch> startingGun, int threadID)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->startingGun = startingGun;
  this->threadID = threadID;
}

void TestIndexWriterThreadsToSegments::ThreadAnonymousInnerClass3::run()
{
  try {
    startingGun->await();
    for (int j = 0; j < 1000; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(LuceneTestCase::newStringField(
          L"field", L"threadID" + to_wstring(threadID), Field::Store::NO));
      w->addDocument(doc);
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

std::deque<char> TestIndexWriterThreadsToSegments::readSegmentInfoID(
    shared_ptr<Directory> dir, const wstring &file) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput in =
  // dir.openInput(file, org.apache.lucene.store.IOContext.DEFAULT))
  {
    org::apache::lucene::store::IndexInput in_ =
        dir->openInput(file, org::apache::lucene::store::IOContext::DEFAULT);
    in_->readInt();    // magic
    in_->readString(); // codec name
    in_->readInt();    // version
    std::deque<char> id(StringHelper::ID_LENGTH);
    in_->readBytes(id, 0, id.size());
    return id;
  }
}
} // namespace org::apache::lucene::index