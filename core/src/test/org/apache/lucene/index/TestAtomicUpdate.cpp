using namespace std;

#include "TestAtomicUpdate.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using namespace org::apache::lucene::document;
using namespace org::apache::lucene::store;
using namespace org::apache::lucene::util;
float TestAtomicUpdate::TimedThread::RUN_TIME_MSEC = atLeast(500);

TestAtomicUpdate::TimedThread::TimedThread(
    std::deque<std::shared_ptr<TimedThread>> &threads)
{
  this->allThreads = threads;
}

void TestAtomicUpdate::TimedThread::run()
{
  constexpr int64_t stopTime =
      System::currentTimeMillis() + static_cast<int64_t>(RUN_TIME_MSEC);

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
    wcout << Thread::currentThread().getName() << L": exc" << endl;
    e.printStackTrace(System::out);
    failed = true;
  }
}

bool TestAtomicUpdate::TimedThread::anyErrors()
{
  for (int i = 0; i < allThreads.size(); i++) {
    if (allThreads[i] != nullptr && allThreads[i]->failed) {
      return true;
    }
  }
  return false;
}

TestAtomicUpdate::IndexerThread::IndexerThread(
    shared_ptr<IndexWriter> writer,
    std::deque<std::shared_ptr<TimedThread>> &threads)
    : TimedThread(threads)
{
  this->writer = writer;
}

void TestAtomicUpdate::IndexerThread::doWork() 
{
  // Update all 100 docs...
  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                          Field::Store::YES));
    d->push_back(make_shared<TextField>(
        L"contents", English::intToEnglish(i + 10 * count), Field::Store::NO));
    d->push_back(make_shared<IntPoint>(L"doc", i));
    d->push_back(make_shared<IntPoint>(L"doc2d", i, i));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->updateDocument(make_shared<Term>(L"id", Integer::toString(i)), d);
  }
}

TestAtomicUpdate::SearcherThread::SearcherThread(
    shared_ptr<Directory> directory,
    std::deque<std::shared_ptr<TimedThread>> &threads)
    : TimedThread(threads)
{
  this->directory = directory;
}

void TestAtomicUpdate::SearcherThread::doWork() 
{
  shared_ptr<IndexReader> r = DirectoryReader::open(directory);
  assertEquals(100, r->numDocs());
  delete r;
}

void TestAtomicUpdate::runTest(shared_ptr<Directory> directory) throw(
    runtime_error)
{

  std::deque<std::shared_ptr<TimedThread>> threads(4);

  shared_ptr<IndexWriterConfig> conf =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMaxBufferedDocs(7);
  (std::static_pointer_cast<TieredMergePolicy>(conf->getMergePolicy()))
      ->setMaxMergeAtOnce(3);
  shared_ptr<IndexWriter> writer =
      RandomIndexWriter::mockIndexWriter(directory, conf, random());

  // Establish a base index of 100 docs:
  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(
        newStringField(L"id", Integer::toString(i), Field::Store::YES));
    d->push_back(
        newTextField(L"contents", English::intToEnglish(i), Field::Store::NO));
    if ((i - 1) % 7 == 0) {
      writer->commit();
    }
    writer->addDocument(d);
  }
  writer->commit();

  shared_ptr<IndexReader> r = DirectoryReader::open(directory);
  assertEquals(100, r->numDocs());
  delete r;

  shared_ptr<IndexerThread> indexerThread =
      make_shared<IndexerThread>(writer, threads);
  threads[0] = indexerThread;
  indexerThread->start();

  shared_ptr<IndexerThread> indexerThread2 =
      make_shared<IndexerThread>(writer, threads);
  threads[1] = indexerThread2;
  indexerThread2->start();

  shared_ptr<SearcherThread> searcherThread1 =
      make_shared<SearcherThread>(directory, threads);
  threads[2] = searcherThread1;
  searcherThread1->start();

  shared_ptr<SearcherThread> searcherThread2 =
      make_shared<SearcherThread>(directory, threads);
  threads[3] = searcherThread2;
  searcherThread2->start();

  indexerThread->join();
  indexerThread2->join();
  searcherThread1->join();
  searcherThread2->join();

  delete writer;

  assertTrue(L"hit unexpected exception in indexer", !indexerThread->failed);
  assertTrue(L"hit unexpected exception in indexer2", !indexerThread2->failed);
  assertTrue(L"hit unexpected exception in search1", !searcherThread1->failed);
  assertTrue(L"hit unexpected exception in search2", !searcherThread2->failed);
  // System.out.println("    Writer: " + indexerThread.count + " iterations");
  // System.out.println("Searcher 1: " + searcherThread1.count + " searchers
  // created"); System.out.println("Searcher 2: " + searcherThread2.count + "
  // searchers created");
}

void TestAtomicUpdate::testAtomicUpdates() 
{
  shared_ptr<Directory> directory;

  // First in a RAM directory:
  directory =
      make_shared<MockDirectoryWrapper>(random(), make_shared<RAMDirectory>());
  runTest(directory);
  delete directory;

  // Second in an FSDirectory:
  shared_ptr<Path> dirPath = createTempDir(L"lucene.test.atomic");
  directory = newFSDirectory(dirPath);
  runTest(directory);
  delete directory;
}
} // namespace org::apache::lucene::index