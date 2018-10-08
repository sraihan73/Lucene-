using namespace std;

#include "TestStressIndexing.h"

namespace org::apache::lucene::index
{
using namespace org::apache::lucene::util;
using namespace org::apache::lucene::store;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using namespace org::apache::lucene::document;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using namespace org::apache::lucene::search;
int TestStressIndexing::TimedThread::RUN_TIME_MSEC = atLeast(1000);

TestStressIndexing::TimedThread::TimedThread(
    std::deque<std::shared_ptr<TimedThread>> &threads)
{
  this->allThreads = threads;
}

void TestStressIndexing::TimedThread::run()
{
  constexpr int64_t stopTime = System::currentTimeMillis() + RUN_TIME_MSEC;

  count = 0;

  try {
    do {
      if (anyErrors()) {
        break;
      }
      doWork();
      count++;
    } while (System::currentTimeMillis() < stopTime);
  } catch (const runtime_error &e) {
    wcout << Thread::currentThread() << L": exc" << endl;
    e.printStackTrace(System::out);
    failed = true;
  }
}

bool TestStressIndexing::TimedThread::anyErrors()
{
  for (int i = 0; i < allThreads.size(); i++) {
    if (allThreads[i] != nullptr && allThreads[i]->failed) {
      return true;
    }
  }
  return false;
}

TestStressIndexing::IndexerThread::IndexerThread(
    shared_ptr<TestStressIndexing> outerInstance,
    shared_ptr<IndexWriter> writer,
    std::deque<std::shared_ptr<TimedThread>> &threads)
    : TimedThread(threads), outerInstance(outerInstance)
{
  this->writer = writer;
}

void TestStressIndexing::IndexerThread::doWork() 
{
  // Add 10 docs:
  for (int j = 0; j < 10; j++) {
    shared_ptr<Document> d = make_shared<Document>();
    int n = random()->nextInt();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(
        newStringField(L"id", Integer::toString(nextID++), Field::Store::YES));
    d->push_back(
        newTextField(L"contents", English::intToEnglish(n), Field::Store::NO));
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

TestStressIndexing::SearcherThread::SearcherThread(
    shared_ptr<Directory> directory,
    std::deque<std::shared_ptr<TimedThread>> &threads)
    : TimedThread(threads)
{
  this->directory = directory;
}

void TestStressIndexing::SearcherThread::doWork() 
{
  for (int i = 0; i < 100; i++) {
    shared_ptr<IndexReader> ir = DirectoryReader::open(directory);
    shared_ptr<IndexSearcher> is = newSearcher(ir);
    delete ir;
  }
  count += 100;
}

void TestStressIndexing::runStressTest(
    shared_ptr<Directory> directory,
    shared_ptr<MergeScheduler> mergeScheduler) 
{
  shared_ptr<IndexWriter> modifier = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setOpenMode(OpenMode::CREATE)
                     ->setMaxBufferedDocs(10)
                     ->setMergeScheduler(mergeScheduler));
  modifier->commit();

  std::deque<std::shared_ptr<TimedThread>> threads(4);
  int numThread = 0;

  // One modifier that writes 10 docs then removes 5, over
  // and over:
  shared_ptr<IndexerThread> indexerThread =
      make_shared<IndexerThread>(shared_from_this(), modifier, threads);
  threads[numThread++] = indexerThread;
  indexerThread->start();

  shared_ptr<IndexerThread> indexerThread2 =
      make_shared<IndexerThread>(shared_from_this(), modifier, threads);
  threads[numThread++] = indexerThread2;
  indexerThread2->start();

  // Two searchers that constantly just re-instantiate the
  // searcher:
  shared_ptr<SearcherThread> searcherThread1 =
      make_shared<SearcherThread>(directory, threads);
  threads[numThread++] = searcherThread1;
  searcherThread1->start();

  shared_ptr<SearcherThread> searcherThread2 =
      make_shared<SearcherThread>(directory, threads);
  threads[numThread++] = searcherThread2;
  searcherThread2->start();

  for (int i = 0; i < numThread; i++) {
    threads[i]->join();
  }

  delete modifier;

  for (int i = 0; i < numThread; i++) {
    assertTrue(!threads[i]->failed);
  }

  // System.out.println("    Writer: " + indexerThread.count + " iterations");
  // System.out.println("Searcher 1: " + searcherThread1.count + " searchers
  // created"); System.out.println("Searcher 2: " + searcherThread2.count + "
  // searchers created");
}

void TestStressIndexing::testStressIndexAndSearching() 
{
  shared_ptr<Directory> directory = newMaybeVirusCheckingDirectory();
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(directory) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(directory))
        ->setAssertNoUnrefencedFilesOnClose(true);
  }

  runStressTest(directory, make_shared<ConcurrentMergeScheduler>());
  delete directory;
}
} // namespace org::apache::lucene::index