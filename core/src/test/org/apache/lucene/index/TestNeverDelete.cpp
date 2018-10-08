using namespace std;

#include "TestNeverDelete.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestNeverDelete::testIndexing() 
{
  shared_ptr<Path> *const tmpDir = createTempDir(L"TestNeverDelete");
  shared_ptr<BaseDirectoryWrapper> *const d = newFSDirectory(tmpDir);

  shared_ptr<RandomIndexWriter> *const w = make_shared<RandomIndexWriter>(
      random(), d,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setIndexDeletionPolicy(NoDeletionPolicy::INSTANCE));
  w->w->getConfig()->setMaxBufferedDocs(TestUtil::nextInt(random(), 5, 30));

  w->commit();
  std::deque<std::shared_ptr<Thread>> indexThreads(random()->nextInt(4));
  constexpr int64_t stopTime = System::currentTimeMillis() + atLeast(1000);
  for (int x = 0; x < indexThreads.size(); x++) {
    indexThreads[x] =
        make_shared<ThreadAnonymousInnerClass>(shared_from_this(), w, stopTime);
    indexThreads[x]->setName(L"Thread " + to_wstring(x));
    indexThreads[x]->start();
  }

  shared_ptr<Set<wstring>> *const allFiles = unordered_set<wstring>();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(d);
  while (System::currentTimeMillis() < stopTime) {
    shared_ptr<IndexCommit> *const ic = r->getIndexCommit();
    if (VERBOSE) {
      wcout << L"TEST: check files: " << ic->getFileNames() << endl;
    }
    allFiles->addAll(ic->getFileNames());
    // Make sure no old files were removed
    for (auto fileName : allFiles) {
      assertTrue(L"file " + fileName + L" does not exist",
                 slowFileExists(d, fileName));
    }
    shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
    if (r2 != nullptr) {
      r->close();
      r = r2;
    }
    delay(1);
  }
  r->close();

  for (auto t : indexThreads) {
    t->join();
  }
  delete w;
  delete d;
}

TestNeverDelete::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestNeverDelete> outerInstance,
    shared_ptr<org::apache::lucene::index::RandomIndexWriter> w,
    int64_t stopTime)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->stopTime = stopTime;
}

void TestNeverDelete::ThreadAnonymousInnerClass::run()
{
  try {
    int docCount = 0;
    while (System::currentTimeMillis() < stopTime) {
      shared_ptr<Document> *const doc = make_shared<Document>();
      doc->push_back(LuceneTestCase::newStringField(
          L"dc", L"" + to_wstring(docCount), Field::Store::YES));
      doc->push_back(LuceneTestCase::newTextField(
          L"field", L"here is some text", Field::Store::YES));
      w->addDocument(doc);

      if (docCount % 13 == 0) {
        w->commit();
      }
      docCount++;
    }
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::index