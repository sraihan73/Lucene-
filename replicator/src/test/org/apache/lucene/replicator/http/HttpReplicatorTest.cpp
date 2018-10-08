using namespace std;

#include "HttpReplicatorTest.h"

namespace org::apache::lucene::replicator::http
{
using org::apache::http::impl::conn::BasicHttpClientConnectionManager;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using IndexReplicationHandler =
    org::apache::lucene::replicator::IndexReplicationHandler;
using IndexRevision = org::apache::lucene::replicator::IndexRevision;
using LocalReplicator = org::apache::lucene::replicator::LocalReplicator;
using PerSessionDirectoryFactory =
    org::apache::lucene::replicator::PerSessionDirectoryFactory;
using ReplicationClient = org::apache::lucene::replicator::ReplicationClient;
using Replicator = org::apache::lucene::replicator::Replicator;
using ReplicatorTestCase = org::apache::lucene::replicator::ReplicatorTestCase;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::eclipse::jetty::server::Server;
using org::eclipse::jetty::servlet::ServletHandler;
using org::eclipse::jetty::servlet::ServletHolder;
using org::junit::Before;
using org::junit::Test;

void HttpReplicatorTest::startServer() 
{
  shared_ptr<ServletHandler> replicationHandler = make_shared<ServletHandler>();
  shared_ptr<ReplicationService> service = make_shared<ReplicationService>(
      Collections::singletonMap(L"s1", serverReplicator));
  replicationServlet = make_shared<ReplicationServlet>(service);
  shared_ptr<ServletHolder> servlet =
      make_shared<ServletHolder>(replicationServlet);
  replicationHandler->addServletWithMapping(
      servlet, ReplicationService::REPLICATION_CONTEXT + L"/*");
  server = newHttpServer(replicationHandler);
  port = serverPort(server);
  host = serverHost(server);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before @Override public void setUp() throws Exception
void HttpReplicatorTest::setUp() 
{
  ReplicatorTestCase::setUp();
  if (VERBOSE) {
    System::setProperty(L"org.eclipse.jetty.LEVEL",
                        L"DEBUG"); // sets stderr logging to DEBUG level
  }
  clientWorkDir = createTempDir(L"httpReplicatorTest");
  handlerIndexDir = newDirectory();
  serverIndexDir = newDirectory();
  serverReplicator = make_shared<LocalReplicator>();
  startServer();

  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  writer = make_shared<IndexWriter>(serverIndexDir, conf);
  reader = DirectoryReader::open(writer);
}

void HttpReplicatorTest::tearDown() 
{
  stopHttpServer(server);
  writer->rollback();
  IOUtils::close({reader, handlerIndexDir, serverIndexDir});
  ReplicatorTestCase::tearDown();
}

void HttpReplicatorTest::publishRevision(int id) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  writer->setLiveCommitData(
      Collections::singletonMap(L"ID", Integer::toString(id, 16)).entrySet());
  writer->commit();
  serverReplicator->publish(make_shared<IndexRevision>(writer));
}

void HttpReplicatorTest::reopenReader() 
{
  shared_ptr<DirectoryReader> newReader =
      DirectoryReader::openIfChanged(reader);
  assertNotNull(newReader);
  reader->close();
  reader = newReader;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasic() throws Exception
void HttpReplicatorTest::testBasic() 
{
  shared_ptr<Replicator> replicator = make_shared<HttpReplicator>(
      host, port, ReplicationService::REPLICATION_CONTEXT + L"/s1",
      getClientConnectionManager());
  shared_ptr<ReplicationClient> client = make_shared<ReplicationClient>(
      replicator,
      make_shared<IndexReplicationHandler>(handlerIndexDir, nullptr),
      make_shared<PerSessionDirectoryFactory>(clientWorkDir));

  publishRevision(1);
  client->updateNow();
  reopenReader();
  // C++ TODO: Only single-argument parse and valueOf methods are converted:
  // ORIGINAL LINE: assertEquals(1,
  // Integer.parseInt(reader.getIndexCommit().getUserData().get("ID"), 16));
  assertEquals(
      1, Integer::valueOf(reader->getIndexCommit()->getUserData()[L"ID"], 16));

  publishRevision(2);
  client->updateNow();
  reopenReader();
  // C++ TODO: Only single-argument parse and valueOf methods are converted:
  // ORIGINAL LINE: assertEquals(2,
  // Integer.parseInt(reader.getIndexCommit().getUserData().get("ID"), 16));
  assertEquals(
      2, Integer::valueOf(reader->getIndexCommit()->getUserData()[L"ID"], 16));

  delete client;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testServerErrors() throws Exception
void HttpReplicatorTest::testServerErrors() 
{
  // tests the behaviour of the client when the server sends an error
  // must use BasicClientConnectionManager to test whether the client is closed
  // correctly
  shared_ptr<BasicHttpClientConnectionManager> conMgr =
      make_shared<BasicHttpClientConnectionManager>();
  shared_ptr<Replicator> replicator = make_shared<HttpReplicator>(
      host, port, ReplicationService::REPLICATION_CONTEXT + L"/s1", conMgr);
  shared_ptr<ReplicationClient> client = make_shared<ReplicationClient>(
      replicator,
      make_shared<IndexReplicationHandler>(handlerIndexDir, nullptr),
      make_shared<PerSessionDirectoryFactory>(clientWorkDir));

  try {
    publishRevision(5);

    try {
      replicationServlet->setRespondWithError(true);
      client->updateNow();
      fail(L"expected exception");
    } catch (const runtime_error &t) {
      // expected
    }

    replicationServlet->setRespondWithError(false);
    client->updateNow(); // now it should work
    reopenReader();
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: assertEquals(5,
    // Integer.parseInt(reader.getIndexCommit().getUserData().get("ID"), 16));
    assertEquals(5, Integer::valueOf(
                        reader->getIndexCommit()->getUserData()[L"ID"], 16));

    delete client;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    replicationServlet->setRespondWithError(false);
  }
}
} // namespace org::apache::lucene::replicator::http