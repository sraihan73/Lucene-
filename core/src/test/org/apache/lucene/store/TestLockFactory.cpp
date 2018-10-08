using namespace std;

#include "TestLockFactory.h"

namespace org::apache::lucene::store
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestLockFactory::testCustomLockFactory() 
{
  shared_ptr<MockLockFactory> lf = make_shared<MockLockFactory>();
  shared_ptr<Directory> dir = make_shared<MockDirectoryWrapper>(
      random(), make_shared<RAMDirectory>(lf));

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  // add 100 documents (so that commit lock is used)
  for (int i = 0; i < 100; i++) {
    addDoc(writer);
  }

  // Both write lock and commit lock should have been created:
  assertEquals(L"# of unique locks created (after instantiating IndexWriter)",
               1, lf->locksCreated.size());
  delete writer;
}

void TestLockFactory::testRAMDirectoryNoLocking() 
{
  shared_ptr<MockDirectoryWrapper> dir = make_shared<MockDirectoryWrapper>(
      random(), make_shared<RAMDirectory>(NoLockFactory::INSTANCE));

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
  writer->commit(); // required so the second open succeed
  // Create a 2nd IndexWriter.  This is normally not allowed but it should run
  // through since we're not using any locks:
  shared_ptr<IndexWriter> writer2 = nullptr;
  try {
    writer2 = make_shared<IndexWriter>(
        dir,
        (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
            ->setOpenMode(IndexWriterConfig::OpenMode::APPEND));
  } catch (const runtime_error &e) {
    e.printStackTrace(System::out);
    fail(L"Should not have hit an IOException with no locking");
  }

  delete writer;
  if (writer2 != nullptr) {
    delete writer2;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Lock>
TestLockFactory::MockLockFactory::obtainLock(shared_ptr<Directory> dir,
                                             const wstring &lockName)
{
  shared_ptr<Lock> lock = make_shared<MockLock>();
  locksCreated.emplace(lockName, lock);
  return lock;
}

TestLockFactory::MockLockFactory::MockLock::~MockLock()
{
  // do nothing
}

void TestLockFactory::MockLockFactory::MockLock::ensureValid() throw(
    IOException)
{
  // do nothing
}

void TestLockFactory::addDoc(shared_ptr<IndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", L"aaa", Field::Store::NO));
  writer->addDocument(doc);
}
} // namespace org::apache::lucene::store