using namespace std;

#include "LocalReplicatorTest.h"

namespace org::apache::lucene::replicator
{
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;
const wstring LocalReplicatorTest::VERSION_ID = L"version";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before @Override public void setUp() throws Exception
void LocalReplicatorTest::setUp() 
{
  ReplicatorTestCase::setUp();
  sourceDir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  sourceWriter = make_shared<IndexWriter>(sourceDir, conf);
  replicator = make_shared<LocalReplicator>();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After @Override public void tearDown() throws Exception
void LocalReplicatorTest::tearDown() 
{
  delete sourceWriter;
  IOUtils::close({replicator, sourceDir});
  ReplicatorTestCase::tearDown();
}

shared_ptr<Revision>
LocalReplicatorTest::createRevision(int const id) 
{
  sourceWriter->addDocument(make_shared<Document>());
  sourceWriter->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass>(shared_from_this(), id))
          ->entrySet());
  sourceWriter->commit();
  return make_shared<IndexRevision>(sourceWriter);
}

LocalReplicatorTest::HashMapAnonymousInnerClass::HashMapAnonymousInnerClass(
    shared_ptr<LocalReplicatorTest> outerInstance, int id)
{
  this->outerInstance = outerInstance;
  this->id = id;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  this->put(VERSION_ID, Integer::toString(id, 16));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCheckForUpdateNoRevisions() throws
// Exception
void LocalReplicatorTest::testCheckForUpdateNoRevisions() 
{
  assertNull(replicator->checkForUpdate(L""));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testObtainFileAlreadyClosed() throws
// java.io.IOException
void LocalReplicatorTest::testObtainFileAlreadyClosed() 
{
  replicator->publish(createRevision(1));
  shared_ptr<SessionToken> res = replicator->checkForUpdate(L"");
  assertNotNull(res);
  assertEquals(1, res->sourceFiles.size());
  unordered_map::Entry<wstring, deque<std::shared_ptr<RevisionFile>>> entry =
      res->sourceFiles.begin()->next();
  delete replicator;
  expectThrows(AlreadyClosedException::typeid, [&]() {
    replicator->obtainFile(res->id, entry.getKey(),
                           entry.getValue()->get(0).fileName);
  });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPublishAlreadyClosed() throws
// java.io.IOException
void LocalReplicatorTest::testPublishAlreadyClosed() 
{
  delete replicator;
  expectThrows(AlreadyClosedException::typeid,
               [&]() { replicator->publish(createRevision(2)); });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testUpdateAlreadyClosed() throws
// java.io.IOException
void LocalReplicatorTest::testUpdateAlreadyClosed() 
{
  delete replicator;
  expectThrows(AlreadyClosedException::typeid,
               [&]() { replicator->checkForUpdate(L""); });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPublishSameRevision() throws
// java.io.IOException
void LocalReplicatorTest::testPublishSameRevision() 
{
  shared_ptr<Revision> rev = createRevision(1);
  replicator->publish(rev);
  shared_ptr<SessionToken> res = replicator->checkForUpdate(L"");
  assertNotNull(res);
  assertEquals(rev->getVersion(), res->version);
  replicator->release(res->id);
  replicator->publish(make_shared<IndexRevision>(sourceWriter));
  res = replicator->checkForUpdate(res->version);
  assertNull(res);

  // now make sure that publishing same revision doesn't leave revisions
  // "locked", i.e. that replicator releases revisions even when they are not
  // kept
  replicator->publish(createRevision(2));
  assertEquals(1, DirectoryReader::listCommits(sourceDir).size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPublishOlderRev() throws
// java.io.IOException
void LocalReplicatorTest::testPublishOlderRev() 
{
  replicator->publish(createRevision(1));
  shared_ptr<Revision> old = make_shared<IndexRevision>(sourceWriter);
  replicator->publish(createRevision(2));
  // should fail to publish an older revision
  expectThrows(invalid_argument::typeid, [&]() { replicator->publish(old); });
  assertEquals(1, DirectoryReader::listCommits(sourceDir).size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testObtainMissingFile() throws
// java.io.IOException
void LocalReplicatorTest::testObtainMissingFile() 
{
  replicator->publish(createRevision(1));
  shared_ptr<SessionToken> res = replicator->checkForUpdate(L"");
  try {
    replicator->obtainFile(res->id, res->sourceFiles.keySet().begin()->next(),
                           L"madeUpFile");
    fail(L"should have failed obtaining an unrecognized file");
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (FileNotFoundException | NoSuchFileException e) {
    // expected
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSessionExpiration() throws
// java.io.IOException, InterruptedException
void LocalReplicatorTest::testSessionExpiration() throw(IOException,
                                                        InterruptedException)
{
  replicator->publish(createRevision(1));
  shared_ptr<SessionToken> session = replicator->checkForUpdate(L"");
  replicator->setExpirationThreshold(5); // expire quickly
  delay(50);                             // sufficient for expiration
  // should fail to obtain a file for an expired session
  expectThrows(SessionExpiredException::typeid, [&]() {
    replicator->obtainFile(
        session->id, session->sourceFiles.keySet().begin()->next(),
        session->sourceFiles.values().begin()->next()->get(0).fileName);
  });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testUpdateToLatest() throws
// java.io.IOException
void LocalReplicatorTest::testUpdateToLatest() 
{
  replicator->publish(createRevision(1));
  shared_ptr<Revision> rev = createRevision(2);
  replicator->publish(rev);
  shared_ptr<SessionToken> res = replicator->checkForUpdate(L"");
  assertNotNull(res);
  assertEquals(0, rev->compareTo(res->version));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRevisionRelease() throws Exception
void LocalReplicatorTest::testRevisionRelease() 
{
  replicator->publish(createRevision(1));
  assertTrue(slowFileExists(sourceDir, IndexFileNames::SEGMENTS + L"_1"));
  replicator->publish(createRevision(2));
  // now the files of revision 1 can be deleted
  assertTrue(slowFileExists(sourceDir, IndexFileNames::SEGMENTS + L"_2"));
  assertFalse(L"segments_1 should not be found in index directory after "
              L"revision is released",
              slowFileExists(sourceDir, IndexFileNames::SEGMENTS + L"_1"));
}
} // namespace org::apache::lucene::replicator