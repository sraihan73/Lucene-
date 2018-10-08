using namespace std;

#include "TestSnapshotDeletionPolicy.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using IndexInput = org::apache::lucene::store::IndexInput;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using org::junit::Test;
const wstring TestSnapshotDeletionPolicy::INDEX_PATH = L"test.snapshots";

shared_ptr<IndexWriterConfig>
TestSnapshotDeletionPolicy::getConfig(shared_ptr<Random> random,
                                      shared_ptr<IndexDeletionPolicy> dp)
{
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  if (dp != nullptr) {
    conf->setIndexDeletionPolicy(dp);
  }
  return conf;
}

void TestSnapshotDeletionPolicy::checkSnapshotExists(
    shared_ptr<Directory> dir, shared_ptr<IndexCommit> c) 
{
  wstring segFileName = c->getSegmentsFileName();
  assertTrue(L"segments file not found in directory: " + segFileName,
             slowFileExists(dir, segFileName));
}

void TestSnapshotDeletionPolicy::checkMaxDoc(
    shared_ptr<IndexCommit> commit, int expectedMaxDoc) 
{
  shared_ptr<IndexReader> reader = DirectoryReader::open(commit);
  try {
    assertEquals(expectedMaxDoc, reader->maxDoc());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete reader;
  }
}

void TestSnapshotDeletionPolicy::prepareIndexAndSnapshots(
    shared_ptr<SnapshotDeletionPolicy> sdp, shared_ptr<IndexWriter> writer,
    int numSnapshots) 
{
  for (int i = 0; i < numSnapshots; i++) {
    // create dummy document to trigger commit.
    writer->addDocument(make_shared<Document>());
    writer->commit();
    snapshots.push_back(sdp->snapshot());
  }
}

shared_ptr<SnapshotDeletionPolicy>
TestSnapshotDeletionPolicy::getDeletionPolicy() 
{
  return make_shared<SnapshotDeletionPolicy>(
      make_shared<KeepOnlyLastCommitDeletionPolicy>());
}

void TestSnapshotDeletionPolicy::assertSnapshotExists(
    shared_ptr<Directory> dir, shared_ptr<SnapshotDeletionPolicy> sdp,
    int numSnapshots, bool checkIndexCommitSame) 
{
  for (int i = 0; i < numSnapshots; i++) {
    shared_ptr<IndexCommit> snapshot = snapshots[i];
    checkMaxDoc(snapshot, i + 1);
    checkSnapshotExists(dir, snapshot);
    if (checkIndexCommitSame) {
      assertSame(snapshot, sdp->getIndexCommit(snapshot->getGeneration()));
    } else {
      assertEquals(
          snapshot->getGeneration(),
          sdp->getIndexCommit(snapshot->getGeneration())->getGeneration());
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSnapshotDeletionPolicy() throws
// Exception
void TestSnapshotDeletionPolicy::testSnapshotDeletionPolicy() throw(
    runtime_error)
{
  shared_ptr<Directory> fsDir = newDirectory();
  runTest(random(), fsDir);
  delete fsDir;
}

void TestSnapshotDeletionPolicy::runTest(
    shared_ptr<Random> random, shared_ptr<Directory> dir) 
{
  // Run for ~1 seconds
  constexpr int64_t stopTime = System::currentTimeMillis() + 1000;

  shared_ptr<SnapshotDeletionPolicy> dp = getDeletionPolicy();
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
               ->setIndexDeletionPolicy(dp)
               ->setMaxBufferedDocs(2));

  // Verify we catch misuse:
  expectThrows(IllegalStateException::typeid, [&]() { dp->snapshot(); });

  writer->commit();

  shared_ptr<Thread> *const t = make_shared<ThreadAnonymousInnerClass>(
      shared_from_this(), stopTime, writer);

  t->start();

  // While the above indexing thread is running, take many
  // backups:
  do {
    backupIndex(dir, dp);
    delay(20);
  } while (t->isAlive());

  t->join();

  // Add one more document to force writer to commit a
  // final segment, so deletion policy has a chance to
  // delete again:
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  doc->push_back(newField(L"content", L"aaa", customType));
  writer->addDocument(doc);

  // Make sure we don't have any leftover files in the
  // directory:
  delete writer;
  TestIndexWriter::assertNoUnreferencedFiles(
      dir, L"some files were not deleted but should have been");
}

TestSnapshotDeletionPolicy::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<TestSnapshotDeletionPolicy> outerInstance,
        int64_t stopTime,
        shared_ptr<org::apache::lucene::index::IndexWriter> writer)
{
  this->outerInstance = outerInstance;
  this->stopTime = stopTime;
  this->writer = writer;
}

void TestSnapshotDeletionPolicy::ThreadAnonymousInnerClass::run()
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorPositions(true);
  customType->setStoreTermVectorOffsets(true);
  doc->push_back(LuceneTestCase::newField(L"content", L"aaa", customType));
  do {
    for (int i = 0; i < 27; i++) {
      try {
        writer->addDocument(doc);
      } catch (const runtime_error &t) {
        t.printStackTrace(System::out);
        fail(L"addDocument failed");
      }
      if (i % 2 == 0) {
        try {
          writer->commit();
        } catch (const runtime_error &e) {
          throw runtime_error(e);
        }
      }
    }
    try {
      delay(1);
    } catch (const InterruptedException &ie) {
      throw make_shared<ThreadInterruptedException>(ie);
    }
  } while (System::currentTimeMillis() < stopTime);
}

void TestSnapshotDeletionPolicy::backupIndex(
    shared_ptr<Directory> dir,
    shared_ptr<SnapshotDeletionPolicy> dp) 
{
  // To backup an index we first take a snapshot:
  shared_ptr<IndexCommit> snapshot = dp->snapshot();
  try {
    copyFiles(dir, snapshot);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // Make sure to release the snapshot, otherwise these
    // files will never be deleted during this IndexWriter
    // session:
    dp->release(snapshot);
  }
}

void TestSnapshotDeletionPolicy::copyFiles(
    shared_ptr<Directory> dir, shared_ptr<IndexCommit> cp) 
{

  // While we hold the snapshot, and nomatter how long
  // we take to do the backup, the IndexWriter will
  // never delete the files in the snapshot:
  shared_ptr<deque<wstring>> files = cp->getFileNames();
  for (auto fileName : files) {
    // NOTE: in a real backup you would not use
    // readFile; you would need to use something else
    // that copies the file to a backup location.  This
    // could even be a spawned shell process (eg "tar",
    // "zip") that takes the deque of files and builds a
    // backup.
    readFile(dir, fileName);
  }
}

void TestSnapshotDeletionPolicy::readFile(
    shared_ptr<Directory> dir, const wstring &name) 
{
  shared_ptr<IndexInput> input = dir->openInput(name, newIOContext(random()));
  try {
    int64_t size = dir->fileLength(name);
    int64_t bytesLeft = size;
    while (bytesLeft > 0) {
      constexpr int numToRead;
      if (bytesLeft < buffer.size()) {
        numToRead = static_cast<int>(bytesLeft);
      } else {
        numToRead = buffer.size();
      }
      input->readBytes(buffer, 0, numToRead, false);
      bytesLeft -= numToRead;
    }
    // Don't do this in your real backups!  This is just
    // to force a backup to take a somewhat long time, to
    // make sure we are exercising the fact that the
    // IndexWriter should not delete this file even when I
    // take my time reading it.
    delay(1);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete input;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicSnapshots() throws Exception
void TestSnapshotDeletionPolicy::testBasicSnapshots() 
{
  int numSnapshots = 3;

  // Create 3 snapshots: snapshot0, snapshot1, snapshot2
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, getConfig(random(), getDeletionPolicy()));
  shared_ptr<SnapshotDeletionPolicy> sdp =
      std::static_pointer_cast<SnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  prepareIndexAndSnapshots(sdp, writer, numSnapshots);
  delete writer;

  assertEquals(numSnapshots, sdp->getSnapshots().size());
  assertEquals(numSnapshots, sdp->getSnapshotCount());
  assertSnapshotExists(dir, sdp, numSnapshots, true);

  // open a reader on a snapshot - should succeed.
  DirectoryReader::open(snapshots[0])->close();

  // open a new IndexWriter w/ no snapshots to keep and assert that all
  // snapshots are gone.
  sdp = getDeletionPolicy();
  writer = make_shared<IndexWriter>(dir, getConfig(random(), sdp));
  writer->deleteUnusedFiles();
  delete writer;
  assertEquals(L"no snapshots should exist", 1,
               DirectoryReader::listCommits(dir).size());
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiThreadedSnapshotting() throws
// Exception
void TestSnapshotDeletionPolicy::testMultiThreadedSnapshotting() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> *const writer =
      make_shared<IndexWriter>(dir, getConfig(random(), getDeletionPolicy()));
  shared_ptr<SnapshotDeletionPolicy> *const sdp =
      std::static_pointer_cast<SnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());

  std::deque<std::shared_ptr<Thread>> threads(10);
  std::deque<std::shared_ptr<IndexCommit>> snapshots(threads.size());
  shared_ptr<CountDownLatch> *const startingGun =
      make_shared<CountDownLatch>(1);
  for (int i = 0; i < threads.size(); i++) {
    constexpr int finalI = i;
    threads[i] = make_shared<ThreadAnonymousInnerClass2>(
        shared_from_this(), writer, sdp, snapshots, startingGun, finalI);
    threads[i]->setName(L"t" + to_wstring(i));
  }

  for (auto t : threads) {
    t->start();
  }

  startingGun->countDown();

  for (auto t : threads) {
    t->join();
  }

  // Do one last commit, so that after we release all snapshots, we stay w/ one
  // commit
  writer->addDocument(make_shared<Document>());
  writer->commit();

  for (int i = 0; i < threads.size(); i++) {
    sdp->release(snapshots[i]);
    writer->deleteUnusedFiles();
  }
  assertEquals(1, DirectoryReader::listCommits(dir).size());
  delete writer;
  delete dir;
}

TestSnapshotDeletionPolicy::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(
        shared_ptr<TestSnapshotDeletionPolicy> outerInstance,
        shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        shared_ptr<org::apache::lucene::index::SnapshotDeletionPolicy> sdp,
        deque<std::shared_ptr<org::apache::lucene::index::IndexCommit>>
            &snapshots,
        shared_ptr<CountDownLatch> startingGun, int finalI)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
  this->sdp = sdp;
  this->snapshots = snapshots;
  this->startingGun = startingGun;
  this->finalI = finalI;
}

void TestSnapshotDeletionPolicy::ThreadAnonymousInnerClass2::run()
{
  try {
    startingGun->await();
    writer->addDocument(make_shared<Document>());
    writer->commit();
    snapshots[finalI] = sdp->snapshot();
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRollbackToOldSnapshot() throws Exception
void TestSnapshotDeletionPolicy::testRollbackToOldSnapshot() throw(
    runtime_error)
{
  int numSnapshots = 2;
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<SnapshotDeletionPolicy> sdp = getDeletionPolicy();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, getConfig(random(), sdp));
  prepareIndexAndSnapshots(sdp, writer, numSnapshots);
  delete writer;

  // now open the writer on "snapshot0" - make sure it succeeds
  writer = make_shared<IndexWriter>(
      dir, getConfig(random(), sdp)->setIndexCommit(snapshots[0]));
  // this does the actual rollback
  writer->commit();
  writer->deleteUnusedFiles();
  assertSnapshotExists(dir, sdp, numSnapshots - 1, false);
  delete writer;

  // but 'snapshot1' files will still exist (need to release snapshot before
  // they can be deleted).
  wstring segFileName = snapshots[1]->getSegmentsFileName();
  assertTrue(L"snapshot files should exist in the directory: " + segFileName,
             slowFileExists(dir, segFileName));

  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReleaseSnapshot() throws Exception
void TestSnapshotDeletionPolicy::testReleaseSnapshot() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, getConfig(random(), getDeletionPolicy()));
  shared_ptr<SnapshotDeletionPolicy> sdp =
      std::static_pointer_cast<SnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  prepareIndexAndSnapshots(sdp, writer, 1);

  // Create another commit - we must do that, because otherwise the "snapshot"
  // files will still remain in the index, since it's the last commit.
  writer->addDocument(make_shared<Document>());
  writer->commit();

  // Release
  wstring segFileName = snapshots[0]->getSegmentsFileName();
  sdp->release(snapshots[0]);
  writer->deleteUnusedFiles();
  delete writer;
  assertFalse(L"segments file should not be found in dirctory: " + segFileName,
              slowFileExists(dir, segFileName));
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSnapshotLastCommitTwice() throws
// Exception
void TestSnapshotDeletionPolicy::testSnapshotLastCommitTwice() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, getConfig(random(), getDeletionPolicy()));
  shared_ptr<SnapshotDeletionPolicy> sdp =
      std::static_pointer_cast<SnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  writer->addDocument(make_shared<Document>());
  writer->commit();

  shared_ptr<IndexCommit> s1 = sdp->snapshot();
  shared_ptr<IndexCommit> s2 = sdp->snapshot();
  assertSame(s1, s2); // should be the same instance

  // create another commit
  writer->addDocument(make_shared<Document>());
  writer->commit();

  // release "s1" should not delete "s2"
  sdp->release(s1);
  writer->deleteUnusedFiles();
  checkSnapshotExists(dir, s2);

  delete writer;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMissingCommits() throws Exception
void TestSnapshotDeletionPolicy::testMissingCommits() 
{
  // Tests the behavior of SDP when commits that are given at ctor are missing
  // on onInit().
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, getConfig(random(), getDeletionPolicy()));
  shared_ptr<SnapshotDeletionPolicy> sdp =
      std::static_pointer_cast<SnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  writer->addDocument(make_shared<Document>());
  writer->commit();
  shared_ptr<IndexCommit> s1 = sdp->snapshot();

  // create another commit, not snapshotted.
  writer->addDocument(make_shared<Document>());
  delete writer;

  // open a new writer w/ KeepOnlyLastCommit policy, so it will delete "s1"
  // commit.
  delete (make_shared<IndexWriter>(dir, getConfig(random(), nullptr)));

  assertFalse(L"snapshotted commit should not exist",
              slowFileExists(dir, s1->getSegmentsFileName()));
  delete dir;
}
} // namespace org::apache::lucene::index