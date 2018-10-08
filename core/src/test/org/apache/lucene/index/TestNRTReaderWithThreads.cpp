using namespace std;

#include "TestNRTReaderWithThreads.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestNRTReaderWithThreads::testIndexing() 
{
  shared_ptr<Directory> mainDir = newDirectory();
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(mainDir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(mainDir))
        ->setAssertNoDeleteOpenFile(true);
  }
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      mainDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setMaxBufferedDocs(10)
                   ->setMergePolicy(newLogMergePolicy(false, 2)));
  shared_ptr<IndexReader> reader = writer->getReader(); // start pooling readers
  delete reader;
  std::deque<std::shared_ptr<RunThread>> indexThreads(4);
  for (int x = 0; x < indexThreads.size(); x++) {
    indexThreads[x] = make_shared<RunThread>(shared_from_this(), x % 2, writer);
    indexThreads[x]->setName(L"Thread " + to_wstring(x));
    indexThreads[x]->start();
  }
  int64_t startTime = System::currentTimeMillis();
  int64_t duration = 1000;
  while ((System::currentTimeMillis() - startTime) < duration) {
    delay(100);
  }
  for (int x = 0; x < indexThreads.size(); x++) {
    indexThreads[x]->run_ = false;
    assertNull(L"Exception thrown: " + indexThreads[x]->ex,
               indexThreads[x]->ex);
  }
  int delCount = 0;
  int addCount = 0;
  for (int x = 0; x < indexThreads.size(); x++) {
    indexThreads[x]->join();
    addCount += indexThreads[x]->addCount;
    delCount += indexThreads[x]->delCount;
  }
  for (int x = 0; x < indexThreads.size(); x++) {
    assertNull(L"Exception thrown: " + indexThreads[x]->ex,
               indexThreads[x]->ex);
  }
  // System.out.println("addCount:"+addCount);
  // System.out.println("delCount:"+delCount);
  delete writer;
  delete mainDir;
}

TestNRTReaderWithThreads::RunThread::RunThread(
    shared_ptr<TestNRTReaderWithThreads> outerInstance, int type,
    shared_ptr<IndexWriter> writer)
    : outerInstance(outerInstance)
{
  this->type = type;
  this->writer = writer;
}

void TestNRTReaderWithThreads::RunThread::run()
{
  try {
    while (run_) {
      // int n = random.nextInt(2);
      if (type == 0) {
        int i = outerInstance->seq->addAndGet(1);
        shared_ptr<Document> doc = DocHelper::createDocument(i, L"index1", 10);
        writer->addDocument(doc);
        addCount++;
      } else if (type == 1) {
        // we may or may not delete because the term may not exist,
        // however we're opening and closing the reader rapidly
        shared_ptr<IndexReader> reader = writer->getReader();
        int id = r->nextInt(outerInstance->seq->intValue());
        // C++ TODO: There is no native C++ equivalent to 'toString':
        shared_ptr<Term> term = make_shared<Term>(L"id", Integer::toString(id));
        int count = TestIndexWriterReader::count(term, reader);
        writer->deleteDocuments({term});
        delete reader;
        delCount += count;
      }
    }
  } catch (const runtime_error &ex) {
    ex.printStackTrace(System::out);
    this->ex = ex;
    run_ = false;
  }
}
} // namespace org::apache::lucene::index