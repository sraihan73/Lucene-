using namespace std;

#include "TestTransactions.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using English = org::apache::lucene::util::English;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
bool TestTransactions::doFail = false;

TestTransactions::RandomFailure::RandomFailure(
    shared_ptr<TestTransactions> outerInstance)
    : outerInstance(outerInstance)
{
}

void TestTransactions::RandomFailure::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  if (TestTransactions::doFail &&
      LuceneTestCase::random()->nextInt() % 10 <= 3) {
    if (VERBOSE) {
      wcout << Thread::currentThread().getName()
            << L" TEST: now fail on purpose" << endl;
      // C++ TODO: This exception's constructor requires an argument:
      // ORIGINAL LINE: new Throwable().printStackTrace(System.out);
      (runtime_error())->printStackTrace(System::out);
    }
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"now failing randomly but on purpose");
  }
}

float TestTransactions::TimedThread::RUN_TIME_MSEC =
    LuceneTestCase::atLeast(500);

TestTransactions::TimedThread::TimedThread(
    std::deque<std::shared_ptr<TimedThread>> &threads)
{
  this->allThreads = threads;
}

void TestTransactions::TimedThread::run()
{
  constexpr int64_t stopTime =
      System::currentTimeMillis() + static_cast<int64_t>(RUN_TIME_MSEC);

  try {
    do {
      if (anyErrors()) {
        break;
      }
      doWork();
    } while (System::currentTimeMillis() < stopTime);
  } catch (const runtime_error &e) {
    wcout << Thread::currentThread() << L": exc" << endl;
    e.printStackTrace(System::out);
    failed = true;
  }
}

bool TestTransactions::TimedThread::anyErrors()
{
  for (int i = 0; i < allThreads.size(); i++) {
    if (allThreads[i] != nullptr && allThreads[i]->failed) {
      return true;
    }
  }
  return false;
}

TestTransactions::IndexerThread::IndexerThread(
    shared_ptr<TestTransactions> outerInstance, any lock,
    shared_ptr<Directory> dir1, shared_ptr<Directory> dir2,
    std::deque<std::shared_ptr<TimedThread>> &threads)
    : TimedThread(threads), outerInstance(outerInstance)
{
  this->lock = lock;
  this->dir1 = dir1;
  this->dir2 = dir2;
}

void TestTransactions::IndexerThread::doWork() 
{

  shared_ptr<IndexWriter> writer1 = make_shared<IndexWriter>(
      dir1, LuceneTestCase::newIndexWriterConfig(
                make_shared<MockAnalyzer>(LuceneTestCase::random()))
                ->setMaxBufferedDocs(3)
                ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
                ->setMergePolicy(LuceneTestCase::newLogMergePolicy(2)));
  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       writer1->getConfig()->getMergeScheduler()))
      ->setSuppressExceptions();

  // Intentionally use different params so flush/merge
  // happen @ different times
  shared_ptr<IndexWriter> writer2 = make_shared<IndexWriter>(
      dir2, LuceneTestCase::newIndexWriterConfig(
                make_shared<MockAnalyzer>(LuceneTestCase::random()))
                ->setMaxBufferedDocs(2)
                ->setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
                ->setMergePolicy(LuceneTestCase::newLogMergePolicy(3)));
  (std::static_pointer_cast<ConcurrentMergeScheduler>(
       writer2->getConfig()->getMergeScheduler()))
      ->setSuppressExceptions();

  update(writer1);
  update(writer2);

  TestTransactions::doFail = true;
  try {
    {
      lock_guard<mutex> lock(lock);
      try {
        writer1->prepareCommit();
      } catch (const runtime_error &t) {
        // release resources
        try {
          writer1->rollback();
        } catch (const runtime_error &ignore) {
        }
        try {
          writer2->rollback();
        } catch (const runtime_error &ignore) {
        }
        return;
      }
      try {
        writer2->prepareCommit();
      } catch (const runtime_error &t) {
        // release resources
        try {
          writer1->rollback();
        } catch (const runtime_error &ignore) {
        }
        try {
          writer2->rollback();
        } catch (const runtime_error &ignore) {
        }
        return;
      }

      writer1->commit();
      writer2->commit();
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    TestTransactions::doFail = false;
  }

  delete writer1;
  delete writer2;
}

void TestTransactions::IndexerThread::update(
    shared_ptr<IndexWriter> writer) 
{
  // Add 10 docs:
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(StringField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  for (int j = 0; j < 10; j++) {
    shared_ptr<Document> d = make_shared<Document>();
    int n = LuceneTestCase::random()->nextInt();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(LuceneTestCase::newField(L"id", Integer::toString(nextID++),
                                          customType));
    d->push_back(LuceneTestCase::newTextField(
        L"contents", English::intToEnglish(n), Field::Store::NO));
    writer->addDocument(d);
  }

  // Delete 5 docs:
  int deleteID = nextID - 1;
  for (int j = 0; j < 5; j++) {
    writer->deleteDocuments(
        {make_shared<Term>(L"id", L"" + to_wstring(deleteID))});
    deleteID -= 2;
  }
}

TestTransactions::SearcherThread::SearcherThread(
    any lock, shared_ptr<Directory> dir1, shared_ptr<Directory> dir2,
    std::deque<std::shared_ptr<TimedThread>> &threads)
    : TimedThread(threads)
{
  this->lock = lock;
  this->dir1 = dir1;
  this->dir2 = dir2;
}

void TestTransactions::SearcherThread::doWork() 
{
  shared_ptr<IndexReader> r1 = nullptr, r2 = nullptr;
  {
    lock_guard<mutex> lock(lock);
    try {
      r1 = DirectoryReader::open(dir1);
      r2 = DirectoryReader::open(dir2);
    } catch (const runtime_error &e) {
      // can be rethrown as RuntimeException if it happens during a close
      // listener
      if (!e.what()->contains(L"on purpose")) {
        throw e;
      }
      // release resources
      IOUtils::closeWhileHandlingException({r1, r2});
      return;
    }
  }
  if (r1->numDocs() != r2->numDocs()) {
    throw runtime_error(L"doc counts differ: r1=" + to_wstring(r1->numDocs()) +
                        L" r2=" + to_wstring(r2->numDocs()));
  }
  IOUtils::closeWhileHandlingException({r1, r2});
}

void TestTransactions::initIndex(shared_ptr<Directory> dir) 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  for (int j = 0; j < 7; j++) {
    shared_ptr<Document> d = make_shared<Document>();
    int n = random()->nextInt();
    d->push_back(
        newTextField(L"contents", English::intToEnglish(n), Field::Store::NO));
    writer->addDocument(d);
  }
  delete writer;
}

void TestTransactions::testTransactions() 
{
  // we cant use non-ramdir on windows, because this test needs to double-write.
  shared_ptr<MockDirectoryWrapper> dir1 =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  shared_ptr<MockDirectoryWrapper> dir2 =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  dir1->failOn(make_shared<RandomFailure>(shared_from_this()));
  dir2->failOn(make_shared<RandomFailure>(shared_from_this()));
  dir1->setFailOnOpenInput(false);
  dir2->setFailOnOpenInput(false);

  // We throw exceptions in deleteFile, which creates
  // leftover files:
  dir1->setAssertNoUnrefencedFilesOnClose(false);
  dir2->setAssertNoUnrefencedFilesOnClose(false);

  initIndex(dir1);
  initIndex(dir2);

  std::deque<std::shared_ptr<TimedThread>> threads(3);
  int numThread = 0;

  shared_ptr<IndexerThread> indexerThread = make_shared<IndexerThread>(
      shared_from_this(), shared_from_this(), dir1, dir2, threads);
  threads[numThread++] = indexerThread;
  indexerThread->start();

  shared_ptr<SearcherThread> searcherThread1 =
      make_shared<SearcherThread>(shared_from_this(), dir1, dir2, threads);
  threads[numThread++] = searcherThread1;
  searcherThread1->start();

  shared_ptr<SearcherThread> searcherThread2 =
      make_shared<SearcherThread>(shared_from_this(), dir1, dir2, threads);
  threads[numThread++] = searcherThread2;
  searcherThread2->start();

  for (int i = 0; i < numThread; i++) {
    threads[i]->join();
  }

  for (int i = 0; i < numThread; i++) {
    assertTrue(!threads[i]->failed);
  }
  delete dir1;
  delete dir2;
}
} // namespace org::apache::lucene::index