using namespace std;

#include "TestNRTReplication.h"

namespace org::apache::lucene::replicator::nrt
{
using Document = org::apache::lucene::document::Document;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using LockObtainFailedException =
    org::apache::lucene::store::LockObtainFailedException;
using IOUtils = org::apache::lucene::util::IOUtils;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::SuppressForbidden;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::SeedUtils;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "ProcessBuilder requires
// java.io.File for CWD") private NodeProcess startNode(int primaryTCPPort, final
// int id, java.nio.file.Path indexPath, long forcePrimaryVersion, bool
// willCrash) throws java.io.IOException
shared_ptr<NodeProcess> TestNRTReplication::startNode(
    int primaryTCPPort, int const id, shared_ptr<Path> indexPath,
    int64_t forcePrimaryVersion, bool willCrash) 
{
  deque<wstring> cmd = deque<wstring>();

  cmd.push_back(System::getProperty(L"java.home") +
                System::getProperty(L"file.separator") + L"bin" +
                System::getProperty(L"file.separator") + L"java");
  cmd.push_back(L"-Xmx512m");

  int64_t myPrimaryGen;
  if (primaryTCPPort != -1) {
    // I am a replica
    cmd.push_back(L"-Dtests.nrtreplication.primaryTCPPort=" +
                  to_wstring(primaryTCPPort));
    myPrimaryGen = lastPrimaryGen;
  } else {
    myPrimaryGen = nextPrimaryGen++;
    lastPrimaryGen = myPrimaryGen;
  }
  cmd.push_back(L"-Dtests.nrtreplication.primaryGen=" +
                to_wstring(myPrimaryGen));
  cmd.push_back(L"-Dtests.nrtreplication.closeorcrash=false");

  cmd.push_back(L"-Dtests.nrtreplication.node=true");
  cmd.push_back(L"-Dtests.nrtreplication.nodeid=" + to_wstring(id));
  cmd.push_back(L"-Dtests.nrtreplication.startNS=" +
                to_wstring(Node::globalStartNS));
  cmd.push_back(L"-Dtests.nrtreplication.indexpath=" + indexPath);
  cmd.push_back(L"-Dtests.nrtreplication.checkonclose=true");

  if (primaryTCPPort == -1) {
    // We are the primary node
    cmd.push_back(L"-Dtests.nrtreplication.isPrimary=true");
    cmd.push_back(L"-Dtests.nrtreplication.forcePrimaryVersion=" +
                  to_wstring(forcePrimaryVersion));
  }

  // Mixin our own counter because this is called from a fresh thread which
  // means the seed otherwise isn't changing each time we spawn a new node:
  int64_t seed = random()->nextLong() * nodeStartCounter->incrementAndGet();

  cmd.push_back(L"-Dtests.seed=" + SeedUtils::formatSeed(seed));
  cmd.push_back(L"-ea");
  cmd.push_back(L"-cp");
  cmd.push_back(System::getProperty(L"java.class.path"));
  cmd.push_back(L"org.junit.runner.JUnitCore");
  cmd.push_back(
      getClassName()->replace(getClass().getSimpleName(), L"SimpleServer"));

  message(L"child process command: " + cmd);
  shared_ptr<ProcessBuilder> pb = make_shared<ProcessBuilder>(cmd);
  pb->redirectErrorStream(true);

  // Important, so that the scary looking hs_err_<pid>.log appear under our test
  // temp dir:
  pb->directory(childTempDir->toFile());

  shared_ptr<Process> p = pb->start();

  shared_ptr<BufferedReader> r;
  try {
    r = make_shared<BufferedReader>(
        make_shared<InputStreamReader>(p->getInputStream(), IOUtils::UTF_8));
  } catch (const UnsupportedEncodingException &uee) {
    throw runtime_error(uee);
  }

  int tcpPort = -1;
  int64_t initCommitVersion = -1;
  int64_t initInfosVersion = -1;
  shared_ptr<Pattern> logTimeStart = Pattern::compile(L"^[0-9\\.]+s .*");
  bool sawExistingSegmentsFile = false;

  while (true) {
    wstring l = r->readLine();
    if (l == L"") {
      message(L"top: node=" + to_wstring(id) + L" failed to start");
      try {
        p->waitFor();
      } catch (const InterruptedException &ie) {
        throw runtime_error(ie);
      }
      message(L"exit value=" + p->exitValue());
      message(L"top: now fail test replica R" + to_wstring(id) +
              L" failed to start");
      throw runtime_error(L"replica R" + to_wstring(id) + L" failed to start");
    }

    if (logTimeStart->matcher(l).matches()) {
      // Already a well-formed log output:
      wcout << l << endl;
    } else {
      message(l);
    }

    if (StringHelper::startsWith(l, L"PORT: ")) {
      tcpPort = stoi(l.substr(6)->trim());
    } else if (StringHelper::startsWith(l, L"COMMIT VERSION: ")) {
      initCommitVersion = stoi(l.substr(16)->trim());
    } else if (StringHelper::startsWith(l, L"INFOS VERSION: ")) {
      initInfosVersion = stoi(l.substr(15)->trim());
    } else if (l.find(L"will crash after") != wstring::npos) {
      willCrash = true;
    } else if (StringHelper::startsWith(l, L"NODE STARTED")) {
      break;
    } else if (l.find(L"replica cannot start: existing segments file=") !=
               wstring::npos) {
      sawExistingSegmentsFile = true;
    }
  }

  constexpr bool finalWillCrash = willCrash;

  // Baby sits the child process, pulling its stdout and printing to our stdout:
  shared_ptr<AtomicBoolean> nodeClosing = make_shared<AtomicBoolean>();
  shared_ptr<Thread> pumper = ThreadPumper::start(
      [&]() {
        message(L"now wait for process " + p);
        try {
          p->waitFor();
        } catch (const runtime_error &t) {
          throw runtime_error(t);
        }

        message(L"done wait for process " + p);
        int exitValue = p->exitValue();
        message(L"exit value=" + to_wstring(exitValue) + L" willCrash=" +
                StringHelper::toString(finalWillCrash));
        if (exitValue != 0 && finalWillCrash == false) {
          // should fail test
          throw runtime_error(L"node " + to_wstring(id) +
                              L" process had unexpected non-zero exit status=" +
                              to_wstring(exitValue));
        }
      },
      r, System::out, nullptr, nodeClosing);
  pumper->setName(L"pump" + to_wstring(id));

  message(L"top: node=" + to_wstring(id) + L" started at tcpPort=" +
          to_wstring(tcpPort) + L" initCommitVersion=" +
          to_wstring(initCommitVersion) + L" initInfosVersion=" +
          to_wstring(initInfosVersion));
  return make_shared<NodeProcess>(p, id, tcpPort, pumper, primaryTCPPort == -1,
                                  initCommitVersion, initInfosVersion,
                                  nodeClosing);
}

void TestNRTReplication::setUp() 
{
  LuceneTestCase::setUp();
  Node::globalStartNS = System::nanoTime();
  childTempDir = createTempDir(L"child");
  docs = make_shared<LineFileDocs>(random());
}

void TestNRTReplication::tearDown() 
{
  LuceneTestCase::tearDown();
  delete docs;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testReplicateDeleteAllDocuments() throws
// Exception
void TestNRTReplication::testReplicateDeleteAllDocuments() 
{

  shared_ptr<Path> primaryPath = createTempDir(L"primary");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, primaryPath, -1, false);

  shared_ptr<Path> replicaPath = createTempDir(L"replica");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, replicaPath, -1, false);

  // Tell primary current replicas:
  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  shared_ptr<Connection> primaryC = make_shared<Connection>(primary->tcpPort);
  primaryC->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = docs->nextDoc();
    primary->addOrUpdateDocument(primaryC, doc, false);
  }

  // Nothing in replica index yet
  assertVersionAndHits(replica, 0, 0);

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Wait for replica to show the change
  waitForVersionAndHits(replica, primaryVersion1, 10);

  // Delete all docs from primary
  if (random()->nextBoolean()) {
    // Inefficiently:
    for (int id = 0; id < 10; id++) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      primary->deleteDocument(primaryC, Integer::toString(id));
    }
  } else {
    // Efficiently:
    primary->deleteAllDocuments(primaryC);
  }

  // Replica still shows 10 docs:
  assertVersionAndHits(replica, primaryVersion1, 10);

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion2 = primary->flush(0);
  assertTrue(primaryVersion2 > primaryVersion1);

  // Wait for replica to show the change
  waitForVersionAndHits(replica, primaryVersion2, 0);

  // Index 10 docs again:
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = docs->nextDoc();
    primary->addOrUpdateDocument(primaryC, doc, false);
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion3 = primary->flush(0);
  assertTrue(primaryVersion3 > primaryVersion2);

  // Wait for replica to show the change
  waitForVersionAndHits(replica, primaryVersion3, 10);

  delete primaryC;

  delete replica;
  delete primary;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testReplicateForceMerge() throws
// Exception
void TestNRTReplication::testReplicateForceMerge() 
{

  shared_ptr<Path> primaryPath = createTempDir(L"primary");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, primaryPath, -1, false);

  shared_ptr<Path> replicaPath = createTempDir(L"replica");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, replicaPath, -1, false);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  shared_ptr<Connection> primaryC = make_shared<Connection>(primary->tcpPort);
  primaryC->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = docs->nextDoc();
    primary->addOrUpdateDocument(primaryC, doc, false);
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Index 10 more docs into primary:
  for (int i = 0; i < 10; i++) {
    shared_ptr<Document> doc = docs->nextDoc();
    primary->addOrUpdateDocument(primaryC, doc, false);
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion2 = primary->flush(0);
  assertTrue(primaryVersion2 > primaryVersion1);

  primary->forceMerge(primaryC);

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion3 = primary->flush(0);
  assertTrue(primaryVersion3 > primaryVersion2);

  // Wait for replica to show the change
  waitForVersionAndHits(replica, primaryVersion3, 20);

  delete primaryC;

  delete replica;
  delete primary;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testReplicaCrashNoCommit() throws
// Exception
void TestNRTReplication::testReplicaCrashNoCommit() 
{

  shared_ptr<Path> primaryPath = createTempDir(L"primary");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, primaryPath, -1, false);

  shared_ptr<Path> replicaPath = createTempDir(L"replica");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, replicaPath, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Wait for replica to sync up:
  waitForVersionAndHits(replica, primaryVersion1, 10);

  // Crash replica:
  replica->crash();

  // Restart replica:
  replica = startNode(primary->tcpPort, 1, replicaPath, -1, false);

  // On startup the replica searches the last commit (empty here):
  assertVersionAndHits(replica, 0, 0);

  // Ask replica to sync:
  replica->newNRTPoint(primaryVersion1, 0, primary->tcpPort);
  waitForVersionAndHits(replica, primaryVersion1, 10);

  delete replica;
  delete primary;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testReplicaCrashWithCommit() throws
// Exception
void TestNRTReplication::testReplicaCrashWithCommit() 
{

  shared_ptr<Path> primaryPath = createTempDir(L"primary");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, primaryPath, -1, false);

  shared_ptr<Path> replicaPath = createTempDir(L"replica");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, replicaPath, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Wait for replica to sync up:
  waitForVersionAndHits(replica, primaryVersion1, 10);

  // Commit and crash replica:
  replica->commit();
  replica->crash();

  // Restart replica:
  replica = startNode(primary->tcpPort, 1, replicaPath, -1, false);

  // On startup the replica searches the last commit:
  assertVersionAndHits(replica, primaryVersion1, 10);

  delete replica;
  delete primary;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testIndexingWhileReplicaIsDown() throws
// Exception
void TestNRTReplication::testIndexingWhileReplicaIsDown() 
{

  shared_ptr<Path> primaryPath = createTempDir(L"primary");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, primaryPath, -1, false);

  shared_ptr<Path> replicaPath = createTempDir(L"replica");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, replicaPath, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Wait for replica to sync up:
  waitForVersionAndHits(replica, primaryVersion1, 10);

  // Commit and crash replica:
  replica->commit();
  replica->crash();

  sendReplicasToPrimary(primary);

  // Index 10 more docs, while replica is down
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // And flush:
  int64_t primaryVersion2 = primary->flush(0);
  assertTrue(primaryVersion2 > primaryVersion1);

  // Now restart replica:
  replica = startNode(primary->tcpPort, 1, replicaPath, -1, false);

  sendReplicasToPrimary(primary, {replica});

  // On startup the replica still searches its last commit:
  assertVersionAndHits(replica, primaryVersion1, 10);

  // Now ask replica to sync:
  replica->newNRTPoint(primaryVersion2, 0, primary->tcpPort);

  waitForVersionAndHits(replica, primaryVersion2, 20);

  delete replica;
  delete primary;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testCrashPrimary1() throws Exception
void TestNRTReplication::testCrashPrimary1() 
{

  shared_ptr<Path> path1 = createTempDir(L"1");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, path1, -1, true);

  shared_ptr<Path> path2 = createTempDir(L"2");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, path2, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Wait for replica to sync up:
  waitForVersionAndHits(replica, primaryVersion1, 10);

  // Crash primary:
  primary->crash();

  // Promote replica:
  replica->commit();
  delete replica;

  primary = startNode(-1, 1, path2, -1, false);

  // Should still see 10 docs:
  assertVersionAndHits(primary, primaryVersion1, 10);

  delete primary;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testCrashPrimary2() throws Exception
void TestNRTReplication::testCrashPrimary2() 
{

  shared_ptr<Path> path1 = createTempDir(L"1");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, path1, -1, true);

  shared_ptr<Path> path2 = createTempDir(L"2");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, path2, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Wait for replica to sync up:
  waitForVersionAndHits(replica, primaryVersion1, 10);

  primary->commit();

  // Index 10 docs, but crash before replicating or committing:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Crash primary:
  primary->crash();

  // Restart it:
  primary = startNode(-1, 0, path1, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 more docs
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  int64_t primaryVersion2 = primary->flush(0);
  assertTrue(primaryVersion2 > primaryVersion1);

  // Wait for replica to sync up:
  waitForVersionAndHits(replica, primaryVersion2, 20);

  delete primary;
  delete replica;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testCrashPrimary3() throws Exception
void TestNRTReplication::testCrashPrimary3() 
{

  shared_ptr<Path> path1 = createTempDir(L"1");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, path1, -1, true);

  shared_ptr<Path> path2 = createTempDir(L"2");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, path2, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Wait for replica to sync up:
  waitForVersionAndHits(replica, primaryVersion1, 10);

  replica->commit();

  delete replica;
  primary->crash();

  // At this point replica is "in the future": it has 10 docs committed, but the
  // primary crashed before committing so it has 0 docs

  // Restart primary:
  primary = startNode(-1, 0, path1, -1, true);

  // Index 20 docs into primary:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 20; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Flush primary, but there are no replicas to sync to:
  int64_t primaryVersion2 = primary->flush(0);

  // Now restart replica, which on init should detect on a "lost branch" because
  // its 10 docs that were committed came from a different primary node:
  replica = startNode(primary->tcpPort, 1, path2, -1, true);

  assertVersionAndHits(replica, primaryVersion2, 20);

  delete primary;
  delete replica;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testCrashPrimaryWhileCopying() throws
// Exception
void TestNRTReplication::testCrashPrimaryWhileCopying() 
{

  shared_ptr<Path> path1 = createTempDir(L"1");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, path1, -1, true);

  shared_ptr<Path> path2 = createTempDir(L"2");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, path2, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Index 100 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 100; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Refresh primary, which also pushes (async) to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  delay(TestUtil::nextInt(random(), 1, 30));

  // Crash primary, likely/hopefully while replica is still copying
  primary->crash();

  // Could see either 100 docs (replica finished before crash) or 0 docs:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(replica.tcpPort))
  {
    Connection c = Connection(replica->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_SEARCH_ALL);
    c->flush();
    int64_t version = c->in_->readVLong();
    int hitCount = c->in_->readVInt();
    if (version == 0) {
      TestUtil::assertEquals(0, hitCount);
    } else {
      TestUtil::assertEquals(primaryVersion1, version);
      TestUtil::assertEquals(100, hitCount);
    }
  }

  delete primary;
  delete replica;
}

void TestNRTReplication::assertWriteLockHeld(shared_ptr<Path> path) throw(
    runtime_error)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.FSDirectory dir =
  // org.apache.lucene.store.FSDirectory.open(path))
  {
    org::apache::lucene::store::FSDirectory dir =
        org::apache::lucene::store::FSDirectory::open(path);
    expectThrows(LockObtainFailedException::typeid,
                 [&]() { dir->obtainLock(IndexWriter::WRITE_LOCK_NAME); });
  }
}

void TestNRTReplication::testCrashReplica() 
{

  shared_ptr<Path> path1 = createTempDir(L"1");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, path1, -1, true);

  shared_ptr<Path> path2 = createTempDir(L"2");
  shared_ptr<NodeProcess> replica =
      startNode(primary->tcpPort, 1, path2, -1, true);

  assertWriteLockHeld(path2);

  sendReplicasToPrimary(primary, {replica});

  // Index 10 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Refresh primary, which also pushes to replica:
  int64_t primaryVersion1 = primary->flush(0);
  assertTrue(primaryVersion1 > 0);

  // Wait for replica to sync up:
  waitForVersionAndHits(replica, primaryVersion1, 10);

  // Crash replica
  replica->crash();

  sendReplicasToPrimary(primary);

  // Lots of new flushes while replica is down:
  int64_t primaryVersion2 = 0;
  for (int iter = 0; iter < 10; iter++) {
    // Index 10 docs into primary:
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (Connection c = new
    // Connection(primary.tcpPort))
    {
      Connection c = Connection(primary->tcpPort);
      c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
      for (int i = 0; i < 10; i++) {
        shared_ptr<Document> doc = docs->nextDoc();
        primary->addOrUpdateDocument(c, doc, false);
      }
    }
    primaryVersion2 = primary->flush(0);
  }

  // Start up replica again:
  replica = startNode(primary->tcpPort, 1, path2, -1, true);

  sendReplicasToPrimary(primary, {replica});

  // Now ask replica to sync:
  replica->newNRTPoint(primaryVersion2, 0, primary->tcpPort);

  // Make sure it sees all docs that were indexed while it was down:
  assertVersionAndHits(primary, primaryVersion2, 110);

  delete replica;
  delete primary;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testFullClusterCrash() throws Exception
void TestNRTReplication::testFullClusterCrash() 
{

  shared_ptr<Path> path1 = createTempDir(L"1");
  shared_ptr<NodeProcess> primary = startNode(-1, 0, path1, -1, true);

  shared_ptr<Path> path2 = createTempDir(L"2");
  shared_ptr<NodeProcess> replica1 =
      startNode(primary->tcpPort, 1, path2, -1, true);

  shared_ptr<Path> path3 = createTempDir(L"3");
  shared_ptr<NodeProcess> replica2 =
      startNode(primary->tcpPort, 2, path3, -1, true);

  sendReplicasToPrimary(primary, {replica1, replica2});

  // Index 50 docs into primary:
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  int64_t primaryVersion1 = 0;
  for (int iter = 0; iter < 5; iter++) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (Connection c = new
    // Connection(primary.tcpPort))
    {
      Connection c = Connection(primary->tcpPort);
      c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
      for (int i = 0; i < 10; i++) {
        shared_ptr<Document> doc = docs->nextDoc();
        primary->addOrUpdateDocument(c, doc, false);
      }
    }

    // Refresh primary, which also pushes to replicas:
    primaryVersion1 = primary->flush(0);
    assertTrue(primaryVersion1 > 0);
  }

  // Wait for replicas to sync up:
  waitForVersionAndHits(replica1, primaryVersion1, 50);
  waitForVersionAndHits(replica2, primaryVersion1, 50);

  primary->commit();
  replica1->commit();
  replica2->commit();

  // Index 10 more docs, but don't sync to replicas:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    for (int i = 0; i < 10; i++) {
      shared_ptr<Document> doc = docs->nextDoc();
      primary->addOrUpdateDocument(c, doc, false);
    }
  }

  // Full cluster crash
  primary->crash();
  replica1->crash();
  replica2->crash();

  // Full cluster restart
  primary = startNode(-1, 0, path1, -1, true);
  replica1 = startNode(primary->tcpPort, 1, path2, -1, true);
  replica2 = startNode(primary->tcpPort, 2, path3, -1, true);

  // Only 50 because we didn't commit primary before the crash:

  // It's -1 because it's unpredictable how IW changes segments version on init:
  assertVersionAndHits(primary, -1, 50);
  assertVersionAndHits(replica1, primary->initInfosVersion, 50);
  assertVersionAndHits(replica2, primary->initInfosVersion, 50);

  delete primary;
  delete replica1;
  delete replica2;
}

void TestNRTReplication::sendReplicasToPrimary(
    shared_ptr<NodeProcess> primary,
    deque<NodeProcess> &replicas) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_SET_REPLICAS);
    c->out->writeVInt(replicas->length);
    for (int id = 0; id < replicas->length; id++) {
      shared_ptr<NodeProcess> replica = replicas[id];
      c->out->writeVInt(replica->id);
      c->out->writeVInt(replica->tcpPort);
    }
    c->flush();
    c->in_->readByte();
  }
}

void TestNRTReplication::assertVersionAndHits(
    shared_ptr<NodeProcess> node, int64_t expectedVersion,
    int expectedHitCount) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new Connection(node.tcpPort))
  {
    Connection c = Connection(node->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_SEARCH_ALL);
    c->flush();
    int64_t version = c->in_->readVLong();
    int hitCount = c->in_->readVInt();
    if (expectedVersion != -1) {
      assertEquals(L"wrong searcher version, with hitCount=" +
                       to_wstring(hitCount),
                   expectedVersion, version);
    }
    TestUtil::assertEquals(expectedHitCount, hitCount);
  }
}

void TestNRTReplication::waitForVersionAndHits(
    shared_ptr<NodeProcess> node, int64_t expectedVersion,
    int expectedHitCount) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new Connection(node.tcpPort))
  {
    Connection c = Connection(node->tcpPort);
    while (true) {
      c->out->writeByte(SimplePrimaryNode::CMD_SEARCH_ALL);
      c->flush();
      int64_t version = c->in_->readVLong();
      int hitCount = c->in_->readVInt();

      if (version == expectedVersion) {
        TestUtil::assertEquals(expectedHitCount, hitCount);
        break;
      }

      assertTrue(version < expectedVersion);
      delay(10);
    }
  }
}

void TestNRTReplication::message(const wstring &message)
{
  int64_t now = System::nanoTime();
  wcout << wstring::format(Locale::ROOT, L"%5.3fs       :     parent [%11s] %s",
                           (now - Node::globalStartNS) / 1000000000.0,
                           Thread::currentThread().getName(), message)
        << endl;
}
} // namespace org::apache::lucene::replicator::nrt