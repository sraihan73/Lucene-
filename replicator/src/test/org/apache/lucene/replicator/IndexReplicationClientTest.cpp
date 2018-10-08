using namespace std;

#include "IndexReplicationClientTest.h"

namespace org::apache::lucene::replicator
{
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using ReplicationHandler =
    org::apache::lucene::replicator::ReplicationClient::ReplicationHandler;
using SourceDirectoryFactory =
    org::apache::lucene::replicator::ReplicationClient::SourceDirectoryFactory;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;

IndexReplicationClientTest::IndexReadyCallback::IndexReadyCallback(
    shared_ptr<Directory> indexDir) 
    : indexDir(indexDir)
{
  if (DirectoryReader::indexExists(indexDir)) {
    reader = DirectoryReader::open(indexDir);
    lastGeneration = reader->getIndexCommit()->getGeneration();
  }
}

optional<bool>
IndexReplicationClientTest::IndexReadyCallback::call() 
{
  if (reader == nullptr) {
    reader = DirectoryReader::open(indexDir);
    lastGeneration = reader->getIndexCommit()->getGeneration();
  } else {
    shared_ptr<DirectoryReader> newReader =
        DirectoryReader::openIfChanged(reader);
    assertNotNull(
        L"should not have reached here if no changes were made to the index",
        newReader);
    int64_t newGeneration = newReader->getIndexCommit()->getGeneration();
    assertTrue(L"expected newer generation; current=" +
                   to_wstring(lastGeneration) + L" new=" +
                   to_wstring(newGeneration),
               newGeneration > lastGeneration);
    reader->close();
    reader = newReader;
    lastGeneration = newGeneration;
    TestUtil::checkIndex(indexDir);
  }
  return nullopt;
}

IndexReplicationClientTest::IndexReadyCallback::~IndexReadyCallback()
{
  IOUtils::close({reader});
}

const wstring IndexReplicationClientTest::VERSION_ID = L"version";

void IndexReplicationClientTest::assertHandlerRevision(
    int expectedID, shared_ptr<Directory> dir) 
{
  // loop as long as client is alive. test-framework will terminate us if
  // there's a serious bug, e.g. client doesn't really update. otherwise,
  // introducing timeouts is not good, can easily lead to false positives.
  while (client->isUpdateThreadAlive()) {
    // give client a chance to update
    try {
      delay(100);
    } catch (const InterruptedException &e) {
      throw make_shared<ThreadInterruptedException>(e);
    }

    try {
      shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
      try {
        // C++ TODO: Only single-argument parse and valueOf methods are
        // converted: ORIGINAL LINE: int handlerID =
        // Integer.parseInt(reader.getIndexCommit().getUserData().get(VERSION_ID),
        // 16);
        int handlerID = Integer::valueOf(
            reader->getIndexCommit()->getUserData()[VERSION_ID], 16);
        if (expectedID == handlerID) {
          return;
        } else if (VERBOSE) {
          wcout << L"expectedID=" << expectedID << L" actual=" << handlerID
                << L" generation=" << reader->getIndexCommit()->getGeneration()
                << endl;
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        reader->close();
      }
    } catch (const runtime_error &e) {
      // we can hit IndexNotFoundException or e.g. EOFException (on
      // segments_N) because it is being copied at the same time it is read by
      // DirectoryReader.open().
    }
  }
}

shared_ptr<Revision>
IndexReplicationClientTest::createRevision(int const id) 
{
  publishWriter->addDocument(make_shared<Document>());
  publishWriter->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass>(shared_from_this(), id))
          ->entrySet());
  publishWriter->commit();
  return make_shared<IndexRevision>(publishWriter);
}

IndexReplicationClientTest::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<IndexReplicationClientTest> outerInstance, int id)
{
  this->outerInstance = outerInstance;
  this->id = id;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  this->put(VERSION_ID, Integer::toString(id, 16));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void IndexReplicationClientTest::setUp() 
{
  ReplicatorTestCase::setUp();
  publishDir = newMockDirectory();
  handlerDir = newMockDirectory();
  sourceDirFactory = make_shared<PerSessionDirectoryFactory>(
      createTempDir(L"replicationClientTest"));
  replicator = make_shared<LocalReplicator>();
  callback = make_shared<IndexReadyCallback>(handlerDir);
  handler = make_shared<IndexReplicationHandler>(handlerDir, callback);
  client =
      make_shared<ReplicationClient>(replicator, handler, sourceDirFactory);

  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  publishWriter = make_shared<IndexWriter>(publishDir, conf);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After @Override public void tearDown() throws Exception
void IndexReplicationClientTest::tearDown() 
{
  delete publishWriter;
  IOUtils::close({client, callback, replicator, publishDir, handlerDir});
  ReplicatorTestCase::tearDown();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoUpdateThread() throws Exception
void IndexReplicationClientTest::testNoUpdateThread() 
{
  assertNull(L"no version expected at start", handler->currentVersion());

  // Callback validates the replicated index
  replicator->publish(createRevision(1));
  client->updateNow();

  replicator->publish(createRevision(2));
  client->updateNow();

  // Publish two revisions without update, handler should be upgraded to latest
  replicator->publish(createRevision(3));
  replicator->publish(createRevision(4));
  client->updateNow();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testUpdateThread() throws Exception
void IndexReplicationClientTest::testUpdateThread() 
{
  client->startUpdateThread(10, L"index");

  replicator->publish(createRevision(1));
  assertHandlerRevision(1, handlerDir);

  replicator->publish(createRevision(2));
  assertHandlerRevision(2, handlerDir);

  // Publish two revisions without update, handler should be upgraded to latest
  replicator->publish(createRevision(3));
  replicator->publish(createRevision(4));
  assertHandlerRevision(4, handlerDir);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRestart() throws Exception
void IndexReplicationClientTest::testRestart() 
{
  replicator->publish(createRevision(1));
  client->updateNow();

  replicator->publish(createRevision(2));
  client->updateNow();

  client->stopUpdateThread();
  delete client;
  client =
      make_shared<ReplicationClient>(replicator, handler, sourceDirFactory);

  // Publish two revisions without update, handler should be upgraded to latest
  replicator->publish(createRevision(3));
  replicator->publish(createRevision(4));
  client->updateNow();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testConsistencyOnExceptions() throws
// Exception
void IndexReplicationClientTest::testConsistencyOnExceptions() throw(
    runtime_error)
{
  // so the handler's index isn't empty
  replicator->publish(createRevision(1));
  client->updateNow();
  delete client;
  delete callback;

  // wrap sourceDirFactory to return a MockDirWrapper so we can simulate errors
  shared_ptr<SourceDirectoryFactory> *const in_ = sourceDirFactory;
  shared_ptr<AtomicInteger> *const failures =
      make_shared<AtomicInteger>(atLeast(10));
  sourceDirFactory = make_shared<SourceDirectoryFactoryAnonymousInnerClass>(
      shared_from_this(), in_, failures);

  handler = make_shared<IndexReplicationHandler>(
      handlerDir,
      make_shared<CallableAnonymousInnerClass>(shared_from_this(), failures));

  // wrap handleUpdateException so we can act on the thrown exception
  client = make_shared<ReplicationClientAnonymousInnerClass>(
      shared_from_this(), replicator, handler, sourceDirFactory, failures);

  client->startUpdateThread(10, L"index");

  shared_ptr<Directory> *const baseHandlerDir = handlerDir->getDelegate();
  int numRevisions = atLeast(20);
  for (int i = 2; i < numRevisions; i++) {
    replicator->publish(createRevision(i));
    assertHandlerRevision(i, baseHandlerDir);
  }

  // disable errors -- maybe randomness didn't exhaust all allowed failures,
  // and we don't want e.g. CheckIndex to hit false errors.
  handlerDir->setMaxSizeInBytes(0);
  handlerDir->setRandomIOExceptionRate(0.0);
  handlerDir->setRandomIOExceptionRateOnOpen(0.0);
}

IndexReplicationClientTest::SourceDirectoryFactoryAnonymousInnerClass::
    SourceDirectoryFactoryAnonymousInnerClass(
        shared_ptr<IndexReplicationClientTest> outerInstance,
        shared_ptr<SourceDirectoryFactory> in_,
        shared_ptr<AtomicInteger> failures)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
  this->failures = failures;
  clientMaxSize = 100, handlerMaxSize = 100;
  clientExRate = 1.0, handlerExRate = 1.0;
}

void IndexReplicationClientTest::SourceDirectoryFactoryAnonymousInnerClass::
    cleanupSession(const wstring &sessionID) 
{
  in_->cleanupSession(sessionID);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override public
// org.apache.lucene.store.Directory getDirectory(std::wstring sessionID, std::wstring
// source) throws java.io.IOException
shared_ptr<Directory>
IndexReplicationClientTest::SourceDirectoryFactoryAnonymousInnerClass::
    getDirectory(const wstring &sessionID,
                 const wstring &source) 
{
  shared_ptr<Directory> dir = in_->getDirectory(sessionID, source);
  if (random()->nextBoolean() &&
      failures->get() > 0) { // client should fail, return wrapped dir
    shared_ptr<MockDirectoryWrapper> mdw =
        make_shared<MockDirectoryWrapper>(random(), dir);
    mdw->setRandomIOExceptionRateOnOpen(clientExRate);
    mdw->setMaxSizeInBytes(clientMaxSize);
    mdw->setRandomIOExceptionRate(clientExRate);
    mdw->setCheckIndexOnClose(false);
    clientMaxSize *= 2;
    clientExRate /= 2;
    return mdw;
  }

  if (failures->get() > 0 && random()->nextBoolean()) { // handler should fail
    outerInstance->handlerDir->setMaxSizeInBytes(handlerMaxSize);
    outerInstance->handlerDir->setRandomIOExceptionRateOnOpen(handlerExRate);
    outerInstance->handlerDir->setRandomIOExceptionRate(handlerExRate);
    handlerMaxSize *= 2;
    handlerExRate /= 2;
  } else {
    // disable errors
    outerInstance->handlerDir->setMaxSizeInBytes(0);
    outerInstance->handlerDir->setRandomIOExceptionRate(0.0);
    outerInstance->handlerDir->setRandomIOExceptionRateOnOpen(0.0);
  }
  return dir;
}

IndexReplicationClientTest::CallableAnonymousInnerClass::
    CallableAnonymousInnerClass(
        shared_ptr<IndexReplicationClientTest> outerInstance,
        shared_ptr<AtomicInteger> failures)
{
  this->outerInstance = outerInstance;
  this->failures = failures;
}

optional<bool>
IndexReplicationClientTest::CallableAnonymousInnerClass::call() throw(
    runtime_error)
{
  if (random()->nextDouble() < 0.2 && failures->get() > 0) {
    throw runtime_error(L"random exception from callback");
  }
  return nullopt;
}

IndexReplicationClientTest::ReplicationClientAnonymousInnerClass::
    ReplicationClientAnonymousInnerClass(
        shared_ptr<IndexReplicationClientTest> outerInstance,
        shared_ptr<org::apache::lucene::replicator::Replicator> replicator,
        shared_ptr<ReplicationHandler> handler,
        shared_ptr<SourceDirectoryFactory> sourceDirFactory,
        shared_ptr<AtomicInteger> failures)
    : ReplicationClient(replicator, handler, sourceDirFactory)
{
  this->outerInstance = outerInstance;
  this->failures = failures;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override protected void
// handleUpdateException(Throwable t)
void IndexReplicationClientTest::ReplicationClientAnonymousInnerClass::
    handleUpdateException(runtime_error t)
{
  if (std::dynamic_pointer_cast<IOException>(t) != nullptr) {
    if (VERBOSE) {
      wcout << L"hit exception during update: " << t << endl;
      t.printStackTrace(System::out);
    }
    try {
      // test that the index can be read and also some basic statistics
      shared_ptr<DirectoryReader> reader =
          DirectoryReader::open(outerInstance->handlerDir->getDelegate());
      try {
        int numDocs = reader->numDocs();
        // C++ TODO: Only single-argument parse and valueOf methods are
        // converted: ORIGINAL LINE: int version =
        // Integer.parseInt(reader.getIndexCommit().getUserData().get(VERSION_ID),
        // 16);
        int version = Integer::valueOf(
            reader->getIndexCommit()->getUserData()[VERSION_ID], 16);
        TestUtil::assertEquals(numDocs, version);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        reader->close();
      }
      // verify index consistency
      TestUtil::checkIndex(outerInstance->handlerDir->getDelegate());
    } catch (const IOException &e) {
      // exceptions here are bad, don't ignore them
      throw runtime_error(e);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      // count-down number of failures
      failures->decrementAndGet();
      assert((failures->get() >= 0,
              L"handler failed too many times: " + failures->get()));
      if (VERBOSE) {
        if (failures->get() == 0) {
          wcout << L"no more failures expected" << endl;
        } else {
          wcout << L"num failures left: " << failures->get() << endl;
        }
      }
    }
  } else {
    if (dynamic_cast<runtime_error>(t) != nullptr) {
      throw static_cast<runtime_error>(t);
    }
    throw runtime_error(t);
  }
}
} // namespace org::apache::lucene::replicator