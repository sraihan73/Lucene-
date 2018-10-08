using namespace std;

#include "TestMultiThreadTermVectors.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using namespace org::apache::lucene::document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMultiThreadTermVectors::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                     ->setMergePolicy(newLogMergePolicy()));
  // writer.setNoCFSRatio(0.0);
  // writer.infoStream = System.out;
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setTokenized(false);
  customType->setStoreTermVectors(true);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> fld =
        newField(L"field", English::intToEnglish(i), customType);
    doc->push_back(fld);
    writer->addDocument(doc);
  }
  delete writer;
}

void TestMultiThreadTermVectors::tearDown() 
{
  delete directory;
  LuceneTestCase::tearDown();
}

void TestMultiThreadTermVectors::test() 
{

  shared_ptr<IndexReader> reader = nullptr;

  try {
    reader = DirectoryReader::open(directory);
    for (int i = 1; i <= numThreads; i++) {
      testTermPositionVectors(reader, i);
    }

  } catch (const IOException &ioe) {
    fail(ioe->getMessage());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      try {
        /** close the opened reader */
        delete reader;
      } catch (const IOException &ioe) {
        ioe->printStackTrace();
      }
    }
  }
}

void TestMultiThreadTermVectors::testTermPositionVectors(
    shared_ptr<IndexReader> reader, int threadCount) 
{
  std::deque<std::shared_ptr<MultiThreadTermVectorsReader>> mtr(threadCount);
  for (int i = 0; i < threadCount; i++) {
    mtr[i] = make_shared<MultiThreadTermVectorsReader>();
    mtr[i]->init(reader);
  }

  /** run until all threads finished */
  int threadsAlive = mtr.size();
  while (threadsAlive > 0) {
    // System.out.println("Threads alive");
    delay(10);
    threadsAlive = mtr.size();
    for (int i = 0; i < mtr.size(); i++) {
      if (mtr[i]->isAlive() == true) {
        break;
      }

      threadsAlive--;
    }
  }

  int64_t totalTime = 0LL;
  for (int i = 0; i < mtr.size(); i++) {
    totalTime += mtr[i]->timeElapsed;
    mtr[i].reset();
  }

  // System.out.println("threadcount: " + mtr.length + " average term deque
  // time: " + totalTime/mtr.length);
}

void MultiThreadTermVectorsReader::init(shared_ptr<IndexReader> reader)
{
  this->reader = reader;
  timeElapsed = 0;
  t = make_shared<Thread>(shared_from_this());
  t->start();
}

bool MultiThreadTermVectorsReader::isAlive()
{
  if (t == nullptr) {
    return false;
  }

  return t->isAlive();
}

void MultiThreadTermVectorsReader::run()
{
  try {
    // run the test 100 times
    for (int i = 0; i < runsToDo; i++) {
      testTermVectors();
    }
  } catch (const runtime_error &e) {
    e.printStackTrace();
  }
  return;
}

void MultiThreadTermVectorsReader::testTermVectors() 
{
  // check:
  int numDocs = reader->numDocs();
  int64_t start = 0LL;
  for (int docId = 0; docId < numDocs; docId++) {
    start = System::currentTimeMillis();
    shared_ptr<Fields> vectors = reader->getTermVectors(docId);
    timeElapsed += System::currentTimeMillis() - start;

    // verify vectors result
    verifyVectors(vectors, docId);

    start = System::currentTimeMillis();
    shared_ptr<Terms> deque = reader->getTermVectors(docId)->terms(L"field");
    timeElapsed += System::currentTimeMillis() - start;

    verifyVector(deque->begin(), docId);
  }
}

void MultiThreadTermVectorsReader::verifyVectors(shared_ptr<Fields> vectors,
                                                 int num) 
{
  for (auto field : vectors) {
    shared_ptr<Terms> terms = vectors->terms(field);
    assert(terms != nullptr);
    verifyVector(terms->begin(), num);
  }
}

void MultiThreadTermVectorsReader::verifyVector(shared_ptr<TermsEnum> deque,
                                                int num) 
{
  shared_ptr<StringBuilder> temp = make_shared<StringBuilder>();
  while (deque->next() != nullptr) {
    temp->append(deque->term()->utf8ToString());
  }
  if (!StringHelper::trim(English::intToEnglish(num))
           .equals(temp->toString()->trim())) {
    wcout << L"wrong term result" << endl;
  }
}
} // namespace org::apache::lucene::search