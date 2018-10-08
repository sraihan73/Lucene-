using namespace std;

#include "TestPersistentSnapshotDeletionPolicy.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before @Override public void setUp() throws Exception
void TestPersistentSnapshotDeletionPolicy::setUp() 
{
  TestSnapshotDeletionPolicy::setUp();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After @Override public void tearDown() throws Exception
void TestPersistentSnapshotDeletionPolicy::tearDown() 
{
  TestSnapshotDeletionPolicy::tearDown();
}

shared_ptr<SnapshotDeletionPolicy>
TestPersistentSnapshotDeletionPolicy::getDeletionPolicy(
    shared_ptr<Directory> dir) 
{
  return make_shared<PersistentSnapshotDeletionPolicy>(
      make_shared<KeepOnlyLastCommitDeletionPolicy>(), dir, OpenMode::CREATE);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExistingSnapshots() throws Exception
void TestPersistentSnapshotDeletionPolicy::testExistingSnapshots() throw(
    runtime_error)
{
  int numSnapshots = 3;
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, getConfig(random(), getDeletionPolicy(dir)));
  shared_ptr<PersistentSnapshotDeletionPolicy> psdp =
      std::static_pointer_cast<PersistentSnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  assertNull(psdp->getLastSaveFile());
  prepareIndexAndSnapshots(psdp, writer, numSnapshots);
  assertNotNull(psdp->getLastSaveFile());
  delete writer;

  // Make sure only 1 save file exists:
  int count = 0;
  for (auto file : dir->listAll()) {
    if (file.startsWith(PersistentSnapshotDeletionPolicy::SNAPSHOTS_PREFIX)) {
      count++;
    }
  }
  assertEquals(1, count);

  // Make sure we fsync:
  dir->crash();
  dir->clearCrash();

  // Re-initialize and verify snapshots were persisted
  psdp = make_shared<PersistentSnapshotDeletionPolicy>(
      make_shared<KeepOnlyLastCommitDeletionPolicy>(), dir, OpenMode::APPEND);

  writer = make_shared<IndexWriter>(dir, getConfig(random(), psdp));
  psdp = std::static_pointer_cast<PersistentSnapshotDeletionPolicy>(
      writer->getConfig()->getIndexDeletionPolicy());

  assertEquals(numSnapshots, psdp->getSnapshots().size());
  assertEquals(numSnapshots, psdp->getSnapshotCount());
  assertSnapshotExists(dir, psdp, numSnapshots, false);

  writer->addDocument(make_shared<Document>());
  writer->commit();
  snapshots.push_back(psdp->snapshot());
  assertEquals(numSnapshots + 1, psdp->getSnapshots().size());
  assertEquals(numSnapshots + 1, psdp->getSnapshotCount());
  assertSnapshotExists(dir, psdp, numSnapshots + 1, false);

  delete writer;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoSnapshotInfos() throws Exception
void TestPersistentSnapshotDeletionPolicy::testNoSnapshotInfos() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  make_shared<PersistentSnapshotDeletionPolicy>(
      make_shared<KeepOnlyLastCommitDeletionPolicy>(), dir, OpenMode::CREATE);
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMissingSnapshots() throws Exception
void TestPersistentSnapshotDeletionPolicy::testMissingSnapshots() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();

  expectThrows(IllegalStateException::typeid, [&]() {
    make_shared<PersistentSnapshotDeletionPolicy>(
        make_shared<KeepOnlyLastCommitDeletionPolicy>(), dir, OpenMode::APPEND);
  });

  delete dir;
}

void TestPersistentSnapshotDeletionPolicy::testExceptionDuringSave() throw(
    runtime_error)
{
  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();
  dir->failOn(make_shared<FailureAnonymousInnerClass>(shared_from_this(), dir));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir,
      getConfig(random(), make_shared<PersistentSnapshotDeletionPolicy>(
                              make_shared<KeepOnlyLastCommitDeletionPolicy>(),
                              dir, OpenMode::CREATE_OR_APPEND)));
  writer->addDocument(make_shared<Document>());
  writer->commit();

  shared_ptr<PersistentSnapshotDeletionPolicy> psdp =
      std::static_pointer_cast<PersistentSnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  try {
    psdp->snapshot();
  } catch (const IOException &ioe) {
    if (ioe->getMessage().equals(L"now fail on purpose")) {
      // ok
    } else {
      throw ioe;
    }
  }
  assertEquals(0, psdp->getSnapshotCount());
  delete writer;
  assertEquals(1, DirectoryReader::listCommits(dir).size());
  delete dir;
}

TestPersistentSnapshotDeletionPolicy::FailureAnonymousInnerClass::
    FailureAnonymousInnerClass(
        shared_ptr<TestPersistentSnapshotDeletionPolicy> outerInstance,
        shared_ptr<MockDirectoryWrapper> dir)
{
  this->outerInstance = outerInstance;
  this->dir = dir;
}

void TestPersistentSnapshotDeletionPolicy::FailureAnonymousInnerClass::eval(
    shared_ptr<MockDirectoryWrapper> dir) 
{
  std::deque<std::shared_ptr<StackTraceElement>> trace =
      Thread::currentThread().getStackTrace();
  for (int i = 0; i < trace.size(); i++) {
    if (PersistentSnapshotDeletionPolicy::typeid->getName().equals(
            trace[i]->getClassName()) &&
        L"persist" == trace[i]->getMethodName()) {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(L"now fail on purpose");
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSnapshotRelease() throws Exception
void TestPersistentSnapshotDeletionPolicy::testSnapshotRelease() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, getConfig(random(), getDeletionPolicy(dir)));
  shared_ptr<PersistentSnapshotDeletionPolicy> psdp =
      std::static_pointer_cast<PersistentSnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  prepareIndexAndSnapshots(psdp, writer, 1);
  delete writer;

  psdp->release(snapshots[0]);

  psdp = make_shared<PersistentSnapshotDeletionPolicy>(
      make_shared<KeepOnlyLastCommitDeletionPolicy>(), dir, OpenMode::APPEND);
  assertEquals(L"Should have no snapshots !", 0, psdp->getSnapshotCount());
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSnapshotReleaseByGeneration() throws
// Exception
void TestPersistentSnapshotDeletionPolicy::
    testSnapshotReleaseByGeneration() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, getConfig(random(), getDeletionPolicy(dir)));
  shared_ptr<PersistentSnapshotDeletionPolicy> psdp =
      std::static_pointer_cast<PersistentSnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  prepareIndexAndSnapshots(psdp, writer, 1);
  delete writer;

  psdp->release(snapshots[0]->getGeneration());

  psdp = make_shared<PersistentSnapshotDeletionPolicy>(
      make_shared<KeepOnlyLastCommitDeletionPolicy>(), dir, OpenMode::APPEND);
  assertEquals(L"Should have no snapshots !", 0, psdp->getSnapshotCount());
  delete dir;
}
} // namespace org::apache::lucene::index