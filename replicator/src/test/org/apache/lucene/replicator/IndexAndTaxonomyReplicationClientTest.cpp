using namespace std;

#include "IndexAndTaxonomyReplicationClientTest.h"

namespace org::apache::lucene::replicator
{
using Document = org::apache::lucene::document::Document;
using DrillDownQuery = org::apache::lucene::facet::DrillDownQuery;
using FacetField = org::apache::lucene::facet::FacetField;
using Facets = org::apache::lucene::facet::Facets;
using FacetsCollector = org::apache::lucene::facet::FacetsCollector;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using FastTaxonomyFacetCounts =
    org::apache::lucene::facet::taxonomy::FastTaxonomyFacetCounts;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using CheckIndex = org::apache::lucene::index::CheckIndex;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using SnapshotDirectoryTaxonomyWriter = org::apache::lucene::replicator::
    IndexAndTaxonomyRevision::SnapshotDirectoryTaxonomyWriter;
using ReplicationHandler =
    org::apache::lucene::replicator::ReplicationClient::ReplicationHandler;
using SourceDirectoryFactory =
    org::apache::lucene::replicator::ReplicationClient::SourceDirectoryFactory;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using IOUtils = org::apache::lucene::util::IOUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;

IndexAndTaxonomyReplicationClientTest::IndexAndTaxonomyReadyCallback::
    IndexAndTaxonomyReadyCallback(
        shared_ptr<Directory> indexDir,
        shared_ptr<Directory> taxoDir) 
    : indexDir(indexDir), taxoDir(taxoDir)
{
  config = make_shared<FacetsConfig>();
  config->setHierarchical(L"A", true);
  if (DirectoryReader::indexExists(indexDir)) {
    indexReader = DirectoryReader::open(indexDir);
    lastIndexGeneration = indexReader->getIndexCommit()->getGeneration();
    taxoReader = make_shared<DirectoryTaxonomyReader>(taxoDir);
  }
}

optional<bool> IndexAndTaxonomyReplicationClientTest::
    IndexAndTaxonomyReadyCallback::call() 
{
  if (indexReader == nullptr) {
    indexReader = DirectoryReader::open(indexDir);
    lastIndexGeneration = indexReader->getIndexCommit()->getGeneration();
    taxoReader = make_shared<DirectoryTaxonomyReader>(taxoDir);
  } else {
    // verify search index
    shared_ptr<DirectoryReader> newReader =
        DirectoryReader::openIfChanged(indexReader);
    assertNotNull(
        L"should not have reached here if no changes were made to the index",
        newReader);
    int64_t newGeneration = newReader->getIndexCommit()->getGeneration();
    assertTrue(L"expected newer generation; current=" +
                   to_wstring(lastIndexGeneration) + L" new=" +
                   to_wstring(newGeneration),
               newGeneration > lastIndexGeneration);
    indexReader->close();
    indexReader = newReader;
    lastIndexGeneration = newGeneration;
    TestUtil::checkIndex(indexDir);

    // verify taxonomy index
    shared_ptr<DirectoryTaxonomyReader> newTaxoReader =
        TaxonomyReader::openIfChanged(taxoReader);
    if (newTaxoReader != nullptr) {
      delete taxoReader;
      taxoReader = newTaxoReader;
    }
    TestUtil::checkIndex(taxoDir);

    // verify faceted search
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: int id =
    // Integer.parseInt(indexReader.getIndexCommit().getUserData().get(VERSION_ID),
    // 16);
    int id = Integer::valueOf(
        indexReader->getIndexCommit()->getUserData()[VERSION_ID], 16);
    shared_ptr<IndexSearcher> searcher =
        make_shared<IndexSearcher>(indexReader);
    shared_ptr<FacetsCollector> fc = make_shared<FacetsCollector>();
    searcher->search(make_shared<MatchAllDocsQuery>(), fc);
    shared_ptr<Facets> facets =
        make_shared<FastTaxonomyFacetCounts>(taxoReader, config, fc);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(
        1, facets->getSpecificValue(L"A", {Integer::toString(id, 16)})
               ->intValue());

    shared_ptr<DrillDownQuery> drillDown = make_shared<DrillDownQuery>(config);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    drillDown->add(L"A", Integer::toString(id, 16));
    shared_ptr<TopDocs> docs = searcher->search(drillDown, 10);
    TestUtil::assertEquals(1, docs->totalHits);
  }
  return nullopt;
}

IndexAndTaxonomyReplicationClientTest::IndexAndTaxonomyReadyCallback::
    ~IndexAndTaxonomyReadyCallback()
{
  IOUtils::close({indexReader, taxoReader});
}

const wstring IndexAndTaxonomyReplicationClientTest::VERSION_ID = L"version";

void IndexAndTaxonomyReplicationClientTest::assertHandlerRevision(
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

shared_ptr<Revision> IndexAndTaxonomyReplicationClientTest::createRevision(
    int const id) 
{
  publishIndexWriter->addDocument(newDocument(publishTaxoWriter, id));
  publishIndexWriter->setLiveCommitData(
      (make_shared<HashMapAnonymousInnerClass>(shared_from_this(), id))
          ->entrySet());
  publishIndexWriter->commit();
  publishTaxoWriter->commit();
  return make_shared<IndexAndTaxonomyRevision>(publishIndexWriter,
                                               publishTaxoWriter);
}

IndexAndTaxonomyReplicationClientTest::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<IndexAndTaxonomyReplicationClientTest> outerInstance, int id)
{
  this->outerInstance = outerInstance;
  this->id = id;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  this->put(VERSION_ID, Integer::toString(id, 16));
}

shared_ptr<Document> IndexAndTaxonomyReplicationClientTest::newDocument(
    shared_ptr<TaxonomyWriter> taxoWriter, int id) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  doc->push_back(make_shared<FacetField>(L"A", Integer::toString(id, 16)));
  return config->build(taxoWriter, doc);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void IndexAndTaxonomyReplicationClientTest::setUp() 
{
  ReplicatorTestCase::setUp();
  publishIndexDir = newDirectory();
  publishTaxoDir = newDirectory();
  handlerIndexDir = newMockDirectory();
  handlerTaxoDir = newMockDirectory();
  clientWorkDir = createTempDir(L"replicationClientTest");
  sourceDirFactory = make_shared<PerSessionDirectoryFactory>(clientWorkDir);
  replicator = make_shared<LocalReplicator>();
  callback = make_shared<IndexAndTaxonomyReadyCallback>(handlerIndexDir,
                                                        handlerTaxoDir);
  handler = make_shared<IndexAndTaxonomyReplicationHandler>(
      handlerIndexDir, handlerTaxoDir, callback);
  client =
      make_shared<ReplicationClient>(replicator, handler, sourceDirFactory);

  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(nullptr);
  conf->setIndexDeletionPolicy(
      make_shared<SnapshotDeletionPolicy>(conf->getIndexDeletionPolicy()));
  publishIndexWriter = make_shared<IndexWriter>(publishIndexDir, conf);
  publishTaxoWriter =
      make_shared<SnapshotDirectoryTaxonomyWriter>(publishTaxoDir);
  config = make_shared<FacetsConfig>();
  config->setHierarchical(L"A", true);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After @Override public void tearDown() throws Exception
void IndexAndTaxonomyReplicationClientTest::tearDown() 
{
  delete publishIndexWriter;
  IOUtils::close({client, callback, publishTaxoWriter, replicator,
                  publishIndexDir, publishTaxoDir, handlerIndexDir,
                  handlerTaxoDir});
  ReplicatorTestCase::tearDown();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNoUpdateThread() throws Exception
void IndexAndTaxonomyReplicationClientTest::testNoUpdateThread() throw(
    runtime_error)
{
  assertNull(L"no version expected at start", handler->currentVersion());

  // Callback validates the replicated index
  replicator->publish(createRevision(1));
  client->updateNow();

  // make sure updating twice, when in fact there's nothing to update, works
  client->updateNow();

  replicator->publish(createRevision(2));
  client->updateNow();

  // Publish two revisions without update, handler should be upgraded to latest
  replicator->publish(createRevision(3));
  replicator->publish(createRevision(4));
  client->updateNow();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRestart() throws Exception
void IndexAndTaxonomyReplicationClientTest::testRestart() 
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
// ORIGINAL LINE: @Test public void testUpdateThread() throws Exception
void IndexAndTaxonomyReplicationClientTest::testUpdateThread() throw(
    runtime_error)
{
  client->startUpdateThread(10, L"indexTaxo");

  replicator->publish(createRevision(1));
  assertHandlerRevision(1, handlerIndexDir);

  replicator->publish(createRevision(2));
  assertHandlerRevision(2, handlerIndexDir);

  // Publish two revisions without update, handler should be upgraded to latest
  replicator->publish(createRevision(3));
  replicator->publish(createRevision(4));
  assertHandlerRevision(4, handlerIndexDir);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRecreateTaxonomy() throws Exception
void IndexAndTaxonomyReplicationClientTest::testRecreateTaxonomy() throw(
    runtime_error)
{
  replicator->publish(createRevision(1));
  client->updateNow();

  // recreate index and taxonomy
  shared_ptr<Directory> newTaxo = newDirectory();
  delete (make_shared<DirectoryTaxonomyWriter>(newTaxo));
  publishTaxoWriter->replaceTaxonomy(newTaxo);
  publishIndexWriter->deleteAll();
  replicator->publish(createRevision(2));

  client->updateNow();
  delete newTaxo;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testConsistencyOnExceptions() throws
// Exception
void IndexAndTaxonomyReplicationClientTest::testConsistencyOnExceptions() throw(
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

  handler = make_shared<IndexAndTaxonomyReplicationHandler>(
      handlerIndexDir, handlerTaxoDir,
      make_shared<CallableAnonymousInnerClass>(shared_from_this(), failures));

  shared_ptr<AtomicBoolean> *const failed = make_shared<AtomicBoolean>();

  // wrap handleUpdateException so we can act on the thrown exception
  client = make_shared<ReplicationClientAnonymousInnerClass>(
      shared_from_this(), replicator, handler, sourceDirFactory, failures,
      failed);

  client->startUpdateThread(10, L"indexAndTaxo");

  shared_ptr<Directory> *const baseHandlerIndexDir =
      handlerIndexDir->getDelegate();
  int numRevisions = atLeast(20) + 2;
  for (int i = 2; i < numRevisions && failed->get() == false; i++) {
    replicator->publish(createRevision(i));
    assertHandlerRevision(i, baseHandlerIndexDir);
  }

  // disable errors -- maybe randomness didn't exhaust all allowed failures,
  // and we don't want e.g. CheckIndex to hit false errors.
  handlerIndexDir->setMaxSizeInBytes(0);
  handlerIndexDir->setRandomIOExceptionRate(0.0);
  handlerIndexDir->setRandomIOExceptionRateOnOpen(0.0);
  handlerTaxoDir->setMaxSizeInBytes(0);
  handlerTaxoDir->setRandomIOExceptionRate(0.0);
  handlerTaxoDir->setRandomIOExceptionRateOnOpen(0.0);
}

IndexAndTaxonomyReplicationClientTest::
    SourceDirectoryFactoryAnonymousInnerClass::
        SourceDirectoryFactoryAnonymousInnerClass(
            shared_ptr<IndexAndTaxonomyReplicationClientTest> outerInstance,
            shared_ptr<SourceDirectoryFactory> in_,
            shared_ptr<AtomicInteger> failures)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
  this->failures = failures;
  clientMaxSize = 100, handlerIndexMaxSize = 100, handlerTaxoMaxSize = 100;
  clientExRate = 1.0, handlerIndexExRate = 1.0, handlerTaxoExRate = 1.0;
}

void IndexAndTaxonomyReplicationClientTest::
    SourceDirectoryFactoryAnonymousInnerClass::cleanupSession(
        const wstring &sessionID) 
{
  in_->cleanupSession(sessionID);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override public
// org.apache.lucene.store.Directory getDirectory(std::wstring sessionID, std::wstring
// source) throws java.io.IOException
shared_ptr<Directory> IndexAndTaxonomyReplicationClientTest::
    SourceDirectoryFactoryAnonymousInnerClass::getDirectory(
        const wstring &sessionID, const wstring &source) 
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
    if (random()->nextBoolean()) {                      // index dir fail
      outerInstance->handlerIndexDir->setMaxSizeInBytes(handlerIndexMaxSize);
      outerInstance->handlerIndexDir->setRandomIOExceptionRate(
          handlerIndexExRate);
      outerInstance->handlerIndexDir->setRandomIOExceptionRateOnOpen(
          handlerIndexExRate);
      handlerIndexMaxSize *= 2;
      handlerIndexExRate /= 2;
    } else { // taxo dir fail
      outerInstance->handlerTaxoDir->setMaxSizeInBytes(handlerTaxoMaxSize);
      outerInstance->handlerTaxoDir->setRandomIOExceptionRate(
          handlerTaxoExRate);
      outerInstance->handlerTaxoDir->setRandomIOExceptionRateOnOpen(
          handlerTaxoExRate);
      outerInstance->handlerTaxoDir->setCheckIndexOnClose(false);
      handlerTaxoMaxSize *= 2;
      handlerTaxoExRate /= 2;
    }
  } else {
    // disable all errors
    outerInstance->handlerIndexDir->setMaxSizeInBytes(0);
    outerInstance->handlerIndexDir->setRandomIOExceptionRate(0.0);
    outerInstance->handlerIndexDir->setRandomIOExceptionRateOnOpen(0.0);
    outerInstance->handlerTaxoDir->setMaxSizeInBytes(0);
    outerInstance->handlerTaxoDir->setRandomIOExceptionRate(0.0);
    outerInstance->handlerTaxoDir->setRandomIOExceptionRateOnOpen(0.0);
  }

  return dir;
}

IndexAndTaxonomyReplicationClientTest::CallableAnonymousInnerClass::
    CallableAnonymousInnerClass(
        shared_ptr<IndexAndTaxonomyReplicationClientTest> outerInstance,
        shared_ptr<AtomicInteger> failures)
{
  this->outerInstance = outerInstance;
  this->failures = failures;
}

optional<bool> IndexAndTaxonomyReplicationClientTest::
    CallableAnonymousInnerClass::call() 
{
  if (random()->nextDouble() < 0.2 && failures->get() > 0) {
    throw runtime_error(L"random exception from callback");
  }
  return nullopt;
}

IndexAndTaxonomyReplicationClientTest::ReplicationClientAnonymousInnerClass::
    ReplicationClientAnonymousInnerClass(
        shared_ptr<IndexAndTaxonomyReplicationClientTest> outerInstance,
        shared_ptr<org::apache::lucene::replicator::Replicator> replicator,
        shared_ptr<ReplicationHandler> handler,
        shared_ptr<SourceDirectoryFactory> sourceDirFactory,
        shared_ptr<AtomicInteger> failures, shared_ptr<AtomicBoolean> failed)
    : ReplicationClient(replicator, handler, sourceDirFactory)
{
  this->outerInstance = outerInstance;
  this->failures = failures;
  this->failed = failed;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override protected void
// handleUpdateException(Throwable t)
void IndexAndTaxonomyReplicationClientTest::
    ReplicationClientAnonymousInnerClass::handleUpdateException(runtime_error t)
{
  if (std::dynamic_pointer_cast<IOException>(t) != nullptr) {
    try {
      if (VERBOSE) {
        wcout << L"hit exception during update: " << t << endl;
        t.printStackTrace(System::out);
      }

      // test that the index can be read and also some basic statistics
      shared_ptr<DirectoryReader> reader =
          DirectoryReader::open(outerInstance->handlerIndexDir->getDelegate());
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
      // verify index is fully consistent
      TestUtil::checkIndex(outerInstance->handlerIndexDir->getDelegate());

      // verify taxonomy index is fully consistent (since we only add one
      // category to all documents, there's nothing much more to validate.
      shared_ptr<ByteArrayOutputStream> bos =
          make_shared<ByteArrayOutputStream>(1024);
      shared_ptr<CheckIndex::Status> indexStatus = nullptr;

      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.index.CheckIndex
      // checker = new
      // org.apache.lucene.index.CheckIndex(handlerTaxoDir.getDelegate()))
      {
        org::apache::lucene::index::CheckIndex checker =
            org::apache::lucene::index::CheckIndex(
                outerInstance->handlerTaxoDir->getDelegate());
        checker->setFailFast(true);
        checker->setInfoStream(
            make_shared<PrintStream>(bos, false, IOUtils::UTF_8), false);
        try {
          indexStatus = checker->checkIndex(nullptr);
        }
        // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
        catch (IOException | runtime_error ioe) {
          // ok: we fallback below
        }
      }

    } catch (const IOException &e) {
      failed->set(true);
      throw runtime_error(e);
    } catch (const runtime_error &e) {
      failed->set(true);
      throw e;
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
    failed->set(true);
    if (dynamic_cast<runtime_error>(t) != nullptr) {
      throw static_cast<runtime_error>(t);
    }
    throw runtime_error(t);
  }
}
} // namespace org::apache::lucene::replicator