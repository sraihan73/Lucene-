using namespace std;

#include "TestTragicIndexWriterDeadlock.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestTragicIndexWriterDeadlock::testDeadlockExcNRTReaderCommit() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(
          iwc->getMergeScheduler()) != nullptr) {
    iwc->setMergeScheduler(
        make_shared<SuppressingConcurrentMergeSchedulerAnonymousInnerClass>(
            shared_from_this()));
  }
  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  shared_ptr<AtomicBoolean> *const done = make_shared<AtomicBoolean>();
  shared_ptr<Thread> commitThread = make_shared<ThreadAnonymousInnerClass>(
      shared_from_this(), w, startingGun, done);
  commitThread->start();
  shared_ptr<DirectoryReader> *const r0 = DirectoryReader::open(w);
  shared_ptr<Thread> nrtThread = make_shared<ThreadAnonymousInnerClass2>(
      shared_from_this(), startingGun, done, r0);
  nrtThread->start();
  dir->setRandomIOExceptionRate(.1);
  startingGun->countDown();
  commitThread->join();
  nrtThread->join();
  dir->setRandomIOExceptionRate(0.0);
  delete w;
  delete dir;
}

TestTragicIndexWriterDeadlock::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass::
        SuppressingConcurrentMergeSchedulerAnonymousInnerClass(
            shared_ptr<TestTragicIndexWriterDeadlock> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestTragicIndexWriterDeadlock::
    SuppressingConcurrentMergeSchedulerAnonymousInnerClass::isOK(
        runtime_error th)
{
  return true;
}

TestTragicIndexWriterDeadlock::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestTragicIndexWriterDeadlock> outerInstance,
        shared_ptr<org::apache::lucene::index::IndexWriter> w,
        shared_ptr<CountDownLatch> startingGun, shared_ptr<AtomicBoolean> done)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->startingGun = startingGun;
  this->done = done;
}

void TestTragicIndexWriterDeadlock::ThreadAnonymousInnerClass::run()
{
  try {
    startingGun->await();
    while (done->get() == false) {
      w->addDocument(make_shared<Document>());
      w->commit();
    }
  } catch (const runtime_error &t) {
    done->set(true);
    // System.out.println("commit exc:");
    // t.printStackTrace(System.out);
  }
}

TestTragicIndexWriterDeadlock::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(
        shared_ptr<TestTragicIndexWriterDeadlock> outerInstance,
        shared_ptr<CountDownLatch> startingGun, shared_ptr<AtomicBoolean> done,
        shared_ptr<org::apache::lucene::index::DirectoryReader> r0)
{
  this->outerInstance = outerInstance;
  this->startingGun = startingGun;
  this->done = done;
  this->r0 = r0;
}

void TestTragicIndexWriterDeadlock::ThreadAnonymousInnerClass2::run()
{
  shared_ptr<DirectoryReader> r = r0;
  try {
    try {
      startingGun->await();
      while (done->get() == false) {
        shared_ptr<DirectoryReader> oldReader = r;
        shared_ptr<DirectoryReader> r2 =
            DirectoryReader::openIfChanged(oldReader);
        if (r2 != nullptr) {
          r = r2;
          oldReader->decRef();
        }
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      r->close();
    }
  } catch (const runtime_error &t) {
    done->set(true);
    // System.out.println("nrt exc:");
    // t.printStackTrace(System.out);
  }
}

void TestTragicIndexWriterDeadlock::testDeadlockStalledMerges() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>();

  // so we merge every 2 segments:
  shared_ptr<LogMergePolicy> mp = make_shared<LogDocMergePolicy>();
  mp->setMergeFactor(2);
  iwc->setMergePolicy(mp);
  shared_ptr<CountDownLatch> done = make_shared<CountDownLatch>(1);
  shared_ptr<ConcurrentMergeScheduler> cms =
      make_shared<ConcurrentMergeSchedulerAnonymousInnerClass>(
          shared_from_this(), dir, done);

  // so we stall once the 2nd merge wants to run:
  cms->setMaxMergesAndThreads(1, 1);
  iwc->setMergeScheduler(cms);

  // so we write a segment every 2 indexed docs:
  iwc->setMaxBufferedDocs(2);

  shared_ptr<IndexWriter> *const w =
      make_shared<IndexWriterAnonymousInnerClass>(shared_from_this(), dir, iwc);

  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  // w writes first segment
  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  // w writes second segment, and kicks off merge, that takes forever
  // (done.await)
  w->addDocument(make_shared<Document>());
  w->addDocument(make_shared<Document>());
  // w writes third segment
  w->addDocument(make_shared<Document>());
  w->commit();
  // w writes fourth segment, and commit flushes and kicks off merge that stalls
  delete w;
  delete dir;
}

TestTragicIndexWriterDeadlock::ConcurrentMergeSchedulerAnonymousInnerClass::
    ConcurrentMergeSchedulerAnonymousInnerClass(
        shared_ptr<TestTragicIndexWriterDeadlock> outerInstance,
        shared_ptr<Directory> dir, shared_ptr<CountDownLatch> done)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
  this->done = done;
}

void TestTragicIndexWriterDeadlock::
    ConcurrentMergeSchedulerAnonymousInnerClass::doMerge(
        shared_ptr<IndexWriter> writer,
        shared_ptr<MergePolicy::OneMerge> merge) 
{
  // let merge takes forever, until commit thread is stalled
  try {
    done->await();
  } catch (const InterruptedException &ie) {
    Thread::currentThread().interrupt();
    throw runtime_error(ie);
  }
  outerInstance->super->doMerge(writer, merge);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void TestTragicIndexWriterDeadlock::
    ConcurrentMergeSchedulerAnonymousInnerClass::doStall()
{
  done->countDown();
  outerInstance->super->doStall();
}

void TestTragicIndexWriterDeadlock::
    ConcurrentMergeSchedulerAnonymousInnerClass::handleMergeException(
        shared_ptr<Directory> dir, runtime_error exc)
{
}

TestTragicIndexWriterDeadlock::IndexWriterAnonymousInnerClass::
    IndexWriterAnonymousInnerClass(
        shared_ptr<TestTragicIndexWriterDeadlock> outerInstance,
        shared_ptr<Directory> dir,
        shared_ptr<org::apache::lucene::index::IndexWriterConfig> iwc)
    : IndexWriter(dir, iwc)
{
  this->outerInstance = outerInstance;
}

void TestTragicIndexWriterDeadlock::IndexWriterAnonymousInnerClass::
    mergeSuccess(shared_ptr<MergePolicy::OneMerge> merge)
{
  // tragedy strikes!
  throw make_shared<OutOfMemoryError>();
}
} // namespace org::apache::lucene::index