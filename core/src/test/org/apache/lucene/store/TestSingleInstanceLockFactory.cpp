using namespace std;

#include "TestSingleInstanceLockFactory.h"

namespace org::apache::lucene::store
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;

shared_ptr<Directory> TestSingleInstanceLockFactory::getDirectory(
    shared_ptr<Path> path) 
{
  return newDirectory(random(), make_shared<SingleInstanceLockFactory>());
}

void TestSingleInstanceLockFactory::testDefaultRAMDirectory() 
{
  shared_ptr<RAMDirectory> dir = make_shared<RAMDirectory>();

  assertTrue(
      L"RAMDirectory did not use correct LockFactory: got " + dir->lockFactory,
      std::dynamic_pointer_cast<SingleInstanceLockFactory>(dir->lockFactory) !=
          nullptr);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  // Create a 2nd IndexWriter.  This should fail:
  expectThrows(IOException::typeid, [&]() {
    make_shared<IndexWriter>(dir, (make_shared<IndexWriterConfig>(
                                       make_shared<MockAnalyzer>(random())))
                                      ->setOpenMode(OpenMode::APPEND));
  });

  delete writer;
}
} // namespace org::apache::lucene::store