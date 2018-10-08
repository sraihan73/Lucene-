using namespace std;

#include "TestStressNRTReplication.h"

namespace org::apache::lucene::replicator::nrt
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using IOUtils = org::apache::lucene::util::IOUtils;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::SuppressForbidden;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using com::carrotsearch::randomizedtesting::SeedUtils;
const optional<int> TestStressNRTReplication::NUM_NODES = nullptr;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void test() throws Exception
void TestStressNRTReplication::test() 
{

  Node::globalStartNS = System::nanoTime();

  message(L"change thread name from " + Thread::currentThread().getName());
  Thread::currentThread().setName(L"main");

  childTempDir = createTempDir(L"child");

  // We are parent process:

  // Silly bootstrapping:
  versionToTransLogLocation.emplace(0LL, 0LL);

  versionToMarker.emplace(0LL, 0);

  int numNodes;

  if (!NUM_NODES) {
    numNodes = TestUtil::nextInt(random(), 2, 10);
  } else {
    numNodes = NUM_NODES.value();
  }

  wcout << L"TEST: using " << numNodes << L" nodes" << endl;

  transLogPath = createTempDir(L"NRTReplication")->resolve(L"translog");
  transLog = make_shared<SimpleTransLog>(transLogPath);

  // state.rateLimiters = new RateLimiter[numNodes];
  indexPaths = std::deque<std::shared_ptr<Path>>(numNodes);
  nodes = std::deque<std::shared_ptr<NodeProcess>>(numNodes);
  nodeTimeStamps = std::deque<int64_t>(numNodes);
  Arrays::fill(nodeTimeStamps, Node::globalStartNS);
  starting = std::deque<bool>(numNodes);

  for (int i = 0; i < numNodes; i++) {
    indexPaths[i] = createTempDir(L"index" + to_wstring(i));
  }

  std::deque<std::shared_ptr<Thread>> indexers(
      TestUtil::nextInt(random(), 1, 3));
  wcout << L"TEST: launch " << indexers.size() << L" indexer threads" << endl;
  for (int i = 0; i < indexers.size(); i++) {
    indexers[i] = make_shared<IndexThread>(shared_from_this());
    indexers[i]->setName(L"indexer" + to_wstring(i));
    indexers[i]->setDaemon(true);
    indexers[i]->start();
  }

  std::deque<std::shared_ptr<Thread>> searchers(
      TestUtil::nextInt(random(), 1, 3));
  wcout << L"TEST: launch " << searchers.size() << L" searcher threads" << endl;
  for (int i = 0; i < searchers.size(); i++) {
    searchers[i] = make_shared<SearchThread>(shared_from_this());
    searchers[i]->setName(L"searcher" + to_wstring(i));
    searchers[i]->setDaemon(true);
    searchers[i]->start();
  }

  shared_ptr<Thread> restarter = make_shared<RestartThread>(shared_from_this());
  restarter->setName(L"restarter");
  restarter->setDaemon(true);
  restarter->start();

  int runTimeSec;
  if (TEST_NIGHTLY) {
    runTimeSec = RANDOM_MULTIPLIER * TestUtil::nextInt(random(), 120, 240);
  } else {
    runTimeSec = RANDOM_MULTIPLIER * TestUtil::nextInt(random(), 45, 120);
  }

  wcout << L"TEST: will run for " << runTimeSec << L" sec" << endl;

  int64_t endTime = System::nanoTime() + runTimeSec * 1000000000LL;

  sendReplicasToPrimary();

  while (failed->get() == false && System::nanoTime() < endTime) {

    // Wait a bit:
    delay(
        TestUtil::nextInt(random(), min(runTimeSec * 4, 200), runTimeSec * 4));
    if (primary != nullptr && random()->nextBoolean()) {
      shared_ptr<NodeProcess> curPrimary = primary;
      if (curPrimary != nullptr) {

        // Save these before we start flush:
        int64_t nextTransLogLoc = transLog->getNextLocation();
        int markerUptoSav = markerUpto->get();
        message(L"top: now flush primary; at least marker count=" +
                to_wstring(markerUptoSav));

        int64_t result;
        try {
          result = primary->flush(markerUptoSav);
        } catch (const runtime_error &t) {
          message(L"top: flush failed; skipping: " + t.what());
          result = -1;
        }
        if (result > 0) {
          // There were changes
          message(L"top: flush finished with changed; new primary version=" +
                  to_wstring(result));
          lastPrimaryVersion = result;
          addTransLogLoc(lastPrimaryVersion, nextTransLogLoc);
          addVersionMarker(lastPrimaryVersion, markerUptoSav);
        }
      }
    }

    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    int liveCount = 0;
    for (int i = 0; i < nodes.size(); i++) {
      shared_ptr<NodeProcess> node = nodes[i];
      if (node != nullptr) {
        if (sb->length() != 0) {
          sb->append(L" ");
        }
        liveCount++;
        if (node->isPrimary) {
          sb->append(L'P');
        } else {
          sb->append(L'R');
        }
        sb->append(i);
      }
    }

    message(L"PG=" + to_wstring(primary == nullptr ? L"X" : primaryGen) + L" " +
            to_wstring(liveCount) + L" (of " + nodes.size() +
            L") nodes running: " + sb);

    // Commit a random node, primary or replica

    if (random()->nextInt(10) == 1) {
      shared_ptr<NodeProcess> node = nodes[random()->nextInt(nodes.size())];
      if (node != nullptr && node->nodeIsClosing->get() == false) {
        // TODO: if this node is primary, it means we committed an unpublished
        // version (not exposed as an NRT point)... not sure it matters. maybe
        // we somehow allow IW to commit a specific sis (the one we just
        // flushed)?
        message(L"top: now commit node=" + node);
        try {
          node->commitAsync();
        } catch (const runtime_error &t) {
          message(L"top: hit exception during commit with R" +
                  to_wstring(node->id) + L"; skipping");
          t.printStackTrace(System::out);
        }
      }
    }
  }

  message(L"TEST: top: test done, now close");
  stop->set(true);
  for (auto thread : indexers) {
    thread->join();
  }
  for (auto thread : searchers) {
    thread->join();
  }
  restarter->join();

  // Close replicas before primary so we cancel any in-progres replications:
  wcout << L"TEST: top: now close replicas" << endl;
  deque<std::shared_ptr<Closeable>> toClose =
      deque<std::shared_ptr<Closeable>>();
  for (auto node : nodes) {
    if (node != primary && node != nullptr) {
      toClose.push_back(node);
    }
  }
  IOUtils::close(toClose);
  IOUtils::close({primary});
  IOUtils::close({transLog});

  if (failed->get() == false) {
    message(L"TEST: top: now checkIndex");
    for (auto path : indexPaths) {
      message(L"TEST: check " + path);
      shared_ptr<MockDirectoryWrapper> dir = newMockFSDirectory(path);
      // Just too slow otherwise
      dir->setCrossCheckTermVectorsOnClose(false);
      delete dir;
    }
  } else {
    message(L"TEST: failed; skip checkIndex");
  }
}

bool TestStressNRTReplication::anyNodesStarting()
{
  for (int id = 0; id < nodes.size(); id++) {
    if (starting[id]) {
      return true;
    }
  }

  return false;
}

void TestStressNRTReplication::promoteReplica() 
{
  message(L"top: primary crashed; now pick replica to promote");
  int64_t maxSearchingVersion = -1;
  shared_ptr<NodeProcess> replicaToPromote = nullptr;

  // We must promote the most current replica, because otherwise file name reuse
  // can cause a replication to fail when it needs to copy over a file currently
  // held open for searching.  This also minimizes recovery work since the most
  // current replica means less xlog replay to catch up:
  for (auto node : nodes) {
    if (node != nullptr) {
      message(L"ask " + node + L" for its current searching version");
      int64_t searchingVersion;
      try {
        searchingVersion = node->getSearchingVersion();
      } catch (const runtime_error &t) {
        message(L"top: hit SocketException during getSearchingVersion with R" +
                to_wstring(node->id) + L"; skipping");
        t.printStackTrace(System::out);
        continue;
      }
      message(node + L" has searchingVersion=" + to_wstring(searchingVersion));
      if (searchingVersion > maxSearchingVersion) {
        maxSearchingVersion = searchingVersion;
        replicaToPromote = node;
      }
    }
  }

  if (replicaToPromote == nullptr) {
    message(L"top: no replicas running; skipping primary promotion");
    return;
  }

  message(L"top: promote " + replicaToPromote + L" version=" +
          to_wstring(maxSearchingVersion) + L"; now commit");
  try {
    replicaToPromote->commit();
  } catch (const runtime_error &t) {
    // Something wrong with this replica; skip it:
    message(L"top: hit exception during commit with R" +
            to_wstring(replicaToPromote->id) + L"; skipping");
    t.printStackTrace(System::out);
    return;
  }

  message(L"top: now shutdown " + replicaToPromote);
  if (replicaToPromote->shutdown() == false) {
    message(L"top: shutdown failed for R" + to_wstring(replicaToPromote->id) +
            L"; skipping primary promotion");
    return;
  }

  int id = replicaToPromote->id;
  message(L"top: now startPrimary " + replicaToPromote);
  startPrimary(replicaToPromote->id);
}

void TestStressNRTReplication::startPrimary(int id) 
{
  message(to_wstring(id) + L": top: startPrimary lastPrimaryVersion=" +
          to_wstring(lastPrimaryVersion));
  assert(nodes[id] == nullptr);

  // Force version of new primary to advance beyond where old primary was, so we
  // never re-use versions.  It may have already advanced beyond newVersion,
  // e.g. if it flushed new segments while during xlog replay:

  // First start node as primary (it opens an IndexWriter) but do not publish it
  // for searching until we replay xlog:
  shared_ptr<NodeProcess> newPrimary =
      startNode(id, indexPaths[id], true, lastPrimaryVersion + 1);
  if (newPrimary == nullptr) {
    message(L"top: newPrimary failed to start; abort");
    return;
  }

  // Get xlog location that this node was guaranteed to already have indexed
  // through; this may replay some ops already indexed but it's OK because the
  // ops are idempotent: we updateDocument (by docid) on replay even for
  // original addDocument:
  optional<int64_t> startTransLogLoc;
  optional<int> markerCount;
  if (newPrimary->initCommitVersion == 0) {
    startTransLogLoc = 0LL;
    markerCount = 0;
  } else {
    startTransLogLoc = versionToTransLogLocation[newPrimary->initCommitVersion];
    markerCount = versionToMarker[newPrimary->initCommitVersion];
  }
  assert((startTransLogLoc,
          L"newPrimary.initCommitVersion=" +
              to_wstring(newPrimary->initCommitVersion) +
              L" is missing from versionToTransLogLocation: keys=" +
              versionToTransLogLocation.keySet()));
  assert((markerCount, L"newPrimary.initCommitVersion=" +
                           to_wstring(newPrimary->initCommitVersion) +
                           L" is missing from versionToMarker: keys=" +
                           versionToMarker.keySet()));

  // When the primary starts, the userData in its latest commit point tells us
  // which version it had indexed up to, so we know where to replay from in the
  // xlog.  However, we forcefuly advance the version, and then IW on init (or
  // maybe getReader) also adds 1 to it. Since we publish the primary in this
  // state (before xlog replay is done), a replica can start up at this point
  // and pull this version, and possibly later be chosen as a primary, causing
  // problems if the version is known recorded in the translog map_obj.  So we
  // record it here:

  addTransLogLoc(newPrimary->initInfosVersion, startTransLogLoc);
  addVersionMarker(newPrimary->initInfosVersion, markerCount);

  assert(newPrimary->initInfosVersion >= lastPrimaryVersion);
  message(L"top: now change lastPrimaryVersion from " +
          to_wstring(lastPrimaryVersion) + L" to " +
          to_wstring(newPrimary->initInfosVersion) +
          L"; startup marker count " + markerCount);
  lastPrimaryVersion = newPrimary->initInfosVersion;

  int64_t nextTransLogLoc = transLog->getNextLocation();
  int64_t t0 = System::nanoTime();
  message(L"top: start translog replay " + startTransLogLoc + L" (version=" +
          to_wstring(newPrimary->initCommitVersion) + L") to " +
          to_wstring(nextTransLogLoc) + L" (translog end)");
  try {
    transLog->replay(newPrimary, startTransLogLoc, nextTransLogLoc);
  } catch (const IOException &ioe) {
    message(L"top: replay xlog failed; shutdown new primary");
    ioe->printStackTrace(System::out);
    newPrimary->shutdown();
    return;
  }

  int64_t t1 = System::nanoTime();
  message(L"top: done translog replay; took " +
          to_wstring((t1 - t0) / 1000000.0) + L" msec; now publish primary");

  // Publish new primary only after translog has succeeded in replaying; this is
  // important, for this test anyway, so we keep a "linear" history so enforcing
  // marker counts is correct.  E.g., if we publish first and replay translog
  // concurrently with incoming ops, then a primary commit that happens while
  // translog is still replaying will incorrectly record the translog loc into
  // the commit user data when in fact that commit did NOT reflect all prior
  // ops.  So if we crash and start up again from that commit point, we are
  // missing ops.
  nodes[id] = newPrimary;
  primary = newPrimary;

  sendReplicasToPrimary();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "ProcessBuilder requires
// java.io.File for CWD") NodeProcess startNode(final int id, java.nio.file.Path
// indexPath, bool isPrimary, long forcePrimaryVersion) throws
// java.io.IOException
shared_ptr<NodeProcess> TestStressNRTReplication::startNode(
    int const id, shared_ptr<Path> indexPath, bool isPrimary,
    int64_t forcePrimaryVersion) 
{
  nodeTimeStamps[id] = System::nanoTime();
  deque<wstring> cmd = deque<wstring>();

  shared_ptr<NodeProcess> curPrimary = primary;

  cmd.push_back(System::getProperty(L"java.home") +
                System::getProperty(L"file.separator") + L"bin" +
                System::getProperty(L"file.separator") + L"java");
  cmd.push_back(L"-Xmx512m");

  if (curPrimary != nullptr) {
    cmd.push_back(L"-Dtests.nrtreplication.primaryTCPPort=" +
                  to_wstring(curPrimary->tcpPort));
  } else if (isPrimary == false) {
    // We cannot start a replica when there is no primary:
    return nullptr;
  }

  // This is very costly (takes more time to check than it did to index); we do
  // this ourselves in the end instead of each time a replica is restarted:
  // cmd.add("-Dtests.nrtreplication.checkonclose=true");

  cmd.push_back(L"-Dtests.nrtreplication.node=true");
  cmd.push_back(L"-Dtests.nrtreplication.nodeid=" + to_wstring(id));
  cmd.push_back(L"-Dtests.nrtreplication.startNS=" +
                to_wstring(Node::globalStartNS));
  cmd.push_back(L"-Dtests.nrtreplication.indexpath=" + indexPath);
  if (isPrimary) {
    cmd.push_back(L"-Dtests.nrtreplication.isPrimary=true");
    cmd.push_back(L"-Dtests.nrtreplication.forcePrimaryVersion=" +
                  to_wstring(forcePrimaryVersion));
    if (DO_CRASH_PRIMARY) {
      cmd.push_back(L"-Dtests.nrtreplication.doRandomCrash=true");
    }
    if (DO_CLOSE_PRIMARY) {
      cmd.push_back(L"-Dtests.nrtreplication.doRandomClose=true");
    }
  } else {
    if (DO_CRASH_REPLICA) {
      cmd.push_back(L"-Dtests.nrtreplication.doRandomCrash=true");
    }
    if (DO_CLOSE_REPLICA) {
      cmd.push_back(L"-Dtests.nrtreplication.doRandomClose=true");
    }
  }

  if (DO_BIT_FLIPS_DURING_COPY) {
    cmd.push_back(L"-Dtests.nrtreplication.doFlipBitsDuringCopy=true");
  }

  int64_t myPrimaryGen = primaryGen;
  cmd.push_back(L"-Dtests.nrtreplication.primaryGen=" +
                to_wstring(myPrimaryGen));

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

  shared_ptr<Writer> childLog;

  if (SEPARATE_CHILD_OUTPUT) {
    shared_ptr<Path> childOut = childTempDir->resolve(to_wstring(id) + L".log");
    message(L"logging to " + childOut);
    childLog = Files::newBufferedWriter(childOut, StandardCharsets::UTF_8,
                                        StandardOpenOption::APPEND,
                                        StandardOpenOption::CREATE);
    childLog->write(L"\n\nSTART NEW CHILD:\n");
  } else {
    childLog.reset();
  }

  // message("child process command: " + cmd);
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
  bool willCrash = false;

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
      if (p->exitValue() == 0) {
        message(L"zero exit status; assuming failed to remove segments_N; "
                L"skipping");
        return nullptr;
      }

      // Hackity hack, in case primary crashed/closed and we haven't noticed
      // (reaped the process) yet:
      if (isPrimary == false) {
        for (int i = 0; i < 100; i++) {
          shared_ptr<NodeProcess> primary2 = primary;
          if (primaryGen != myPrimaryGen || primary2 == nullptr ||
              primary2->nodeIsClosing->get()) {
            // OK: primary crashed while we were trying to start, so it's
            // expected/allowed that we could not start the replica:
            message(L"primary crashed/closed while replica R" + to_wstring(id) +
                    L" tried to start; skipping");
            return nullptr;
          } else {
            try {
              delay(10);
            } catch (const InterruptedException &ie) {
              throw make_shared<ThreadInterruptedException>(ie);
            }
          }
        }
      }

      // Should fail the test:
      message(L"top: now fail test replica R" + to_wstring(id) +
              L" failed to start");
      failed->set(true);
      throw runtime_error(L"replica R" + to_wstring(id) + L" failed to start");
    }

    if (childLog != nullptr) {
      childLog->write(l);
      childLog->write(L"\n");
      childLog->flush();
    } else if (logTimeStart->matcher(l).matches()) {
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
    }
  }

  constexpr bool finalWillCrash = willCrash;
  shared_ptr<AtomicBoolean> *const nodeIsClosing = make_shared<AtomicBoolean>();

  // Baby sits the child process, pulling its stdout and printing to our stdout,
  // calling nodeClosed once it exits:
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
        if (childLog != nullptr) {
          try {
            childLog->write(L"process done; exitValue=" +
                            to_wstring(exitValue) + L"\n");
            childLog->close();
          } catch (const IOException &ioe) {
            throw runtime_error(ioe);
          }
        }
        if (exitValue != 0 && finalWillCrash == false &&
            crashingNodes->remove(id) == false) {
          // should fail test
          failed->set(true);
          if (childLog != nullptr) {
            throw runtime_error(
                L"node " + to_wstring(id) +
                L" process had unexpected non-zero exit status=" +
                to_wstring(exitValue) + L"; see " + childLog + L" for details");
          } else {
            throw runtime_error(
                L"node " + to_wstring(id) +
                L" process had unexpected non-zero exit status=" +
                to_wstring(exitValue));
          }
        }
        nodeClosed(id);
      },
      r, System::out, childLog, nodeIsClosing);
  pumper->setName(L"pump" + to_wstring(id));

  message(L"top: node=" + to_wstring(id) + L" started at tcpPort=" +
          to_wstring(tcpPort) + L" initCommitVersion=" +
          to_wstring(initCommitVersion) + L" initInfosVersion=" +
          to_wstring(initInfosVersion));
  return make_shared<NodeProcess>(p, id, tcpPort, pumper, isPrimary,
                                  initCommitVersion, initInfosVersion,
                                  nodeIsClosing);
}

void TestStressNRTReplication::nodeClosed(int id)
{
  shared_ptr<NodeProcess> oldNode = nodes[id];
  if (primary != nullptr && oldNode == primary) {
    message(L"top: " + primary + L": primary process finished");
    primary.reset();
    primaryGen++;
  } else {
    message(L"top: " + oldNode + L": replica process finished");
  }
  if (oldNode != nullptr) {
    oldNode->isOpen = false;
  }
  nodes[id].reset();
  nodeTimeStamps[id] = System::nanoTime();

  sendReplicasToPrimary();
}

void TestStressNRTReplication::sendReplicasToPrimary()
{
  shared_ptr<NodeProcess> curPrimary = primary;
  if (curPrimary != nullptr) {
    deque<std::shared_ptr<NodeProcess>> replicas =
        deque<std::shared_ptr<NodeProcess>>();
    for (auto node : nodes) {
      if (node != nullptr && node->isPrimary == false) {
        replicas.push_back(node);
      }
    }

    message(L"top: send " + replicas.size() + L" replicas to primary");

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (Connection c = new
    // Connection(curPrimary.tcpPort))
    {
      Connection c = Connection(curPrimary->tcpPort);
      try {
        c->out->writeByte(SimplePrimaryNode::CMD_SET_REPLICAS);
        c->out->writeVInt(replicas.size());
        for (auto replica : replicas) {
          c->out->writeVInt(replica->id);
          c->out->writeVInt(replica->tcpPort);
        }
        c->flush();
        c->in_->readByte();
      } catch (const runtime_error &t) {
        message(L"top: ignore exc sending replicas to primary P" +
                to_wstring(curPrimary->id) + L" at tcpPort=" +
                to_wstring(curPrimary->tcpPort));
        t.printStackTrace(System::out);
      }
    }
  }
}

void TestStressNRTReplication::addVersionMarker(int64_t version, int count)
{
  // System.out.println("ADD VERSION MARKER version=" + version + " count=" +
  // count);
  if (versionToMarker.find(version) != versionToMarker.end()) {
    int curCount = versionToMarker[version];
    if (curCount != count) {
      message(L"top: wrong marker count version=" + to_wstring(version) +
              L" count=" + to_wstring(count) + L" curCount=" +
              to_wstring(curCount));
      throw make_shared<IllegalStateException>(
          L"version=" + to_wstring(version) + L" count=" + to_wstring(count) +
          L" curCount=" + to_wstring(curCount));
    }
  } else {
    message(L"top: record marker count: version=" + to_wstring(version) +
            L" count=" + to_wstring(count));
    versionToMarker.emplace(version, count);
  }
}

void TestStressNRTReplication::addTransLogLoc(int64_t version, int64_t loc)
{
  message(L"top: record transLogLoc: version=" + to_wstring(version) +
          L" loc=" + to_wstring(loc));
  versionToTransLogLocation.emplace(version, loc);
}

TestStressNRTReplication::RestartThread::RestartThread(
    shared_ptr<TestStressNRTReplication> outerInstance)
    : outerInstance(outerInstance)
{
}

void TestStressNRTReplication::RestartThread::run()
{

  deque<std::shared_ptr<Thread>> startupThreads =
      Collections::synchronizedList(deque<std::shared_ptr<Thread>>());

  try {
    while (outerInstance->stop->get() == false) {
      delay(TestUtil::nextInt(LuceneTestCase::random(), 50, 500));
      // message("top: restarter cycle");

      // Randomly crash full cluster:
      if (DO_FULL_CLUSTER_CRASH &&
          LuceneTestCase::random()->nextInt(500) == 17) {
        message(L"top: full cluster crash");
        for (int i = 0; i < outerInstance->nodes.size(); i++) {
          if (outerInstance->starting[i]) {
            message(L"N" + to_wstring(i) +
                    L": top: wait for startup so we can crash...");
            while (outerInstance->starting[i]) {
              delay(10);
            }
            message(L"N" + to_wstring(i) + L": top: done wait for startup");
          }
          shared_ptr<NodeProcess> node = outerInstance->nodes[i];
          if (node != nullptr) {
            outerInstance->crashingNodes->add(i);
            message(L"top: N" + to_wstring(node->id) +
                    L": top: now crash node");
            node->crash();
            message(L"top: N" + to_wstring(node->id) +
                    L": top: done crash node");
          }
        }
      }

      deque<int> downNodes = deque<int>();
      shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
      int64_t nowNS = System::nanoTime();
      for (int i = 0; i < outerInstance->nodes.size(); i++) {
        b->append(L' ');
        double sec = (nowNS - outerInstance->nodeTimeStamps[i]) / 1000000000.0;
        wstring prefix;
        if (outerInstance->nodes[i] == nullptr) {
          downNodes.push_back(i);
          if (outerInstance->starting[i]) {
            prefix = L"s";
          } else {
            prefix = L"x";
          }
        } else {
          prefix = L"";
        }
        if (outerInstance->primary != nullptr &&
            outerInstance->nodes[i] == outerInstance->primary) {
          prefix += L"p";
        }
        b->append(
            wstring::format(Locale::ROOT, L"%s%d(%.1fs)", prefix, i, sec));
      }
      message(L"node status" + b->toString());
      message(L"downNodes=" + downNodes);

      // If primary is down, promote a replica:
      if (outerInstance->primary == nullptr) {
        if (outerInstance->anyNodesStarting()) {
          message(L"top: skip promote replica: nodes are still starting");
          continue;
        }
        outerInstance->promoteReplica();
      }

      // Randomly start up a down a replica:

      // Stop or start a replica
      if (downNodes.empty() == false) {
        int idx =
            downNodes[LuceneTestCase::random()->nextInt(downNodes.size())];
        if (outerInstance->starting[idx] == false) {
          if (outerInstance->primary == nullptr) {
            if (downNodes.size() == outerInstance->nodes.size()) {
              // Cold start: entire cluster is down, start this node up as the
              // new primary
              message(L"N" + to_wstring(idx) + L": top: cold start as primary");
              outerInstance->startPrimary(idx);
            }
          } else if (LuceneTestCase::random()->nextDouble() <
                     (static_cast<double>(downNodes.size())) /
                         outerInstance->nodes.size()) {
            // Start up replica:
            outerInstance->starting[idx] = true;
            message(L"N" + to_wstring(idx) + L": top: start up: launch thread");
            shared_ptr<Thread> t = make_shared<ThreadAnonymousInnerClass>(
                shared_from_this(), startupThreads, idx);
            t->setName(L"start R" + to_wstring(idx));
            t->start();
            startupThreads.push_back(t);
          }
        } else {
          message(L"node " + to_wstring(idx) + L" still starting");
        }
      }
    }

    wcout << L"Restarter: now stop: join " << startupThreads.size()
          << L" startup threads" << endl;

    while (startupThreads.size() > 0) {
      delay(10);
    }

  } catch (const runtime_error &t) {
    outerInstance->failed->set(true);
    outerInstance->stop->set(true);
    throw runtime_error(t);
  }
}

TestStressNRTReplication::RestartThread::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(shared_ptr<RestartThread> outerInstance,
                              deque<std::shared_ptr<Thread>> &startupThreads,
                              int idx)
{
  this->outerInstance = outerInstance;
  this->startupThreads = startupThreads;
  this->idx = idx;
}

void TestStressNRTReplication::RestartThread::ThreadAnonymousInnerClass::run()
{
  try {
    message(L"N" + to_wstring(idx) + L": top: start up thread");
    outerInstance->outerInstance->nodes[idx] =
        outerInstance->outerInstance->startNode(
            idx, outerInstance->outerInstance->indexPaths[idx], false, -1);
    outerInstance->outerInstance->sendReplicasToPrimary();
  } catch (const runtime_error &t) {
    outerInstance->outerInstance->failed.set(true);
    outerInstance->outerInstance->stop.set(true);
    throw runtime_error(t);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    outerInstance->outerInstance->starting[idx] = false;
    // C++ TODO: The Java deque 'remove(Object)' method is not converted:
    startupThreads.remove(Thread::currentThread());
  }
}

TestStressNRTReplication::SearchThread::SearchThread(
    shared_ptr<TestStressNRTReplication> outerInstance)
    : outerInstance(outerInstance)
{
}

void TestStressNRTReplication::SearchThread::run()
{
  // Maps version to number of hits for silly 'the' TermQuery:
  shared_ptr<Query> theQuery =
      make_shared<TermQuery>(make_shared<Term>(L"body", L"the"));

  // Persists connections
  unordered_map<int, std::shared_ptr<Connection>> connections =
      unordered_map<int, std::shared_ptr<Connection>>();

  while (outerInstance->stop->get() == false) {
    shared_ptr<NodeProcess> node =
        outerInstance->nodes[LuceneTestCase::random()->nextInt(
            outerInstance->nodes.size())];
    if (node == nullptr || node->isOpen == false) {
      continue;
    }

    if (node->lock->tryLock() == false) {
      // Node is in the process of closing or crashing or something
      continue;
    }

    bool nodeIsPrimary = node == outerInstance->primary;

    try {

      Thread::currentThread().setName(L"Searcher node=" + node);

      // System.out.println("S: cycle; conns=" + connections);

      shared_ptr<Connection> c = connections[node->id];

      int64_t version;
      try {
        if (c == nullptr) {
          // System.out.println("S: new connection " + node.id + " " +
          // Thread.currentThread().getName());
          c = make_shared<Connection>(node->tcpPort);
          connections.emplace(node->id, c);
        } else {
          // System.out.println("S: reuse connection " + node.id + " " +
          // Thread.currentThread().getName());
        }

        c->out->writeByte(SimplePrimaryNode::CMD_SEARCH);
        c->flush();

        while (c->sockIn->available() == 0) {
          if (outerInstance->stop->get()) {
            break;
          }
          if (node->isOpen == false) {
            // C++ TODO: The following line could not be converted:
            throw java.io.IOException(L"node closed");
          }
          delay(1);
        }
        version = c->in_->readVLong();

        while (c->sockIn->available() == 0) {
          if (outerInstance->stop->get()) {
            break;
          }
          if (node->isOpen == false) {
            // C++ TODO: The following line could not be converted:
            throw java.io.IOException(L"node closed");
          }
          delay(1);
        }
        int hitCount = c->in_->readVInt();

        optional<int> oldHitCount = outerInstance->hitCounts[version];

        // TODO: we never prune this map_obj...
        if (!oldHitCount) {
          outerInstance->hitCounts.emplace(version, hitCount);
          message(L"top: searcher: record search hitCount version=" +
                  to_wstring(version) + L" hitCount=" + to_wstring(hitCount) +
                  L" node=" + node);
          if (nodeIsPrimary && version > outerInstance->lastPrimaryVersion) {
            // It's possible a search request sees a new primary version because
            // it's in the process of flushing, but then the primary crashes. In
            // this case we need to ensure new primary forces its version beyond
            // this:
            message(L"top: searcher: set lastPrimaryVersion=" +
                    to_wstring(outerInstance->lastPrimaryVersion) + L" vs " +
                    to_wstring(version));
            outerInstance->lastPrimaryVersion = version;
          }
        } else {
          // Just ensure that all nodes show the same hit count for
          // the same version, i.e. they really are replicas of one another:
          if (oldHitCount.value() != hitCount) {
            outerInstance->failed->set(true);
            outerInstance->stop->set(true);
            message(L"top: searcher: wrong version hitCount: version=" +
                    to_wstring(version) + L" oldHitCount=" +
                    oldHitCount.value() + L" hitCount=" + to_wstring(hitCount));
            fail(L"version=" + to_wstring(version) + L" oldHitCount=" +
                 oldHitCount.value() + L" hitCount=" + to_wstring(hitCount));
          }
        }
      } catch (const IOException &ioe) {
        // message("top: searcher: ignore exc talking to node " + node + ": " +
        // ioe); ioe.printStackTrace(System.out);
        IOUtils::closeWhileHandlingException({c});
        connections.erase(node->id);
        continue;
      }

      // This can be null if primary is flushing, has already refreshed its
      // searcher, but is e.g. still notifying replicas and hasn't yet returned
      // the version to us, in which case this searcher thread can see the
      // version before the main thread has added it to versionToMarker:
      optional<int> expectedAtLeastHitCount =
          outerInstance->versionToMarker[version];

      if (expectedAtLeastHitCount && expectedAtLeastHitCount > 0 &&
          LuceneTestCase::random()->nextInt(10) == 7) {
        try {
          c->out->writeByte(SimplePrimaryNode::CMD_MARKER_SEARCH);
          c->out->writeVInt(expectedAtLeastHitCount);
          c->flush();
          while (c->sockIn->available() == 0) {
            if (outerInstance->stop->get()) {
              break;
            }
            if (node->isOpen == false) {
              // C++ TODO: The following line could not be converted:
              throw java.io.IOException(L"node died");
            }
            delay(1);
          }

          version = c->in_->readVLong();

          while (c->sockIn->available() == 0) {
            if (outerInstance->stop->get()) {
              break;
            }
            if (node->isOpen == false) {
              // C++ TODO: The following line could not be converted:
              throw java.io.IOException(L"node died");
            }
            delay(1);
          }

          int hitCount = c->in_->readVInt();

          // Look for data loss: make sure all marker docs are visible:

          if (hitCount < expectedAtLeastHitCount) {

            wstring failMessage =
                L"node=" + node + L": documents were lost version=" +
                to_wstring(version) + L" hitCount=" + to_wstring(hitCount) +
                L" vs expectedAtLeastHitCount=" + expectedAtLeastHitCount;
            message(failMessage);
            outerInstance->failed->set(true);
            outerInstance->stop->set(true);
            fail(failMessage);
          }
        } catch (const IOException &ioe) {
          // message("top: searcher: ignore exc talking to node " + node + ": "
          // + ioe); throw new RuntimeException(ioe);
          // ioe.printStackTrace(System.out);
          IOUtils::closeWhileHandlingException({c});
          connections.erase(node->id);
          continue;
        }
      }

      delay(10);

    } catch (const runtime_error &t) {
      outerInstance->failed->set(true);
      outerInstance->stop->set(true);
      throw runtime_error(t);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      node->lock->unlock();
    }
  }
  wcout << L"Searcher: now stop" << endl;
  IOUtils::closeWhileHandlingException({connections.values()});
}

TestStressNRTReplication::IndexThread::IndexThread(
    shared_ptr<TestStressNRTReplication> outerInstance)
    : outerInstance(outerInstance)
{
}

void TestStressNRTReplication::IndexThread::run()
{

  try {
    shared_ptr<LineFileDocs> docs =
        make_shared<LineFileDocs>(LuceneTestCase::random());
    int docCount = 0;

    // How often we do an update/delete vs add:
    double updatePct = LuceneTestCase::random()->nextDouble();

    // Varies how many docs/sec we index:
    int sleepChance = TestUtil::nextInt(LuceneTestCase::random(), 4, 100);

    message(L"top: indexer: updatePct=" + to_wstring(updatePct) +
            L" sleepChance=" + to_wstring(sleepChance));

    int64_t lastTransLogLoc = outerInstance->transLog->getNextLocation();

    shared_ptr<NodeProcess> curPrimary = nullptr;
    shared_ptr<Connection> c = nullptr;

    while (outerInstance->stop->get() == false) {

      try {
        while (outerInstance->stop->get() == false && curPrimary == nullptr) {
          delay(10);
          curPrimary = outerInstance->primary;
          if (curPrimary != nullptr) {
            c = make_shared<Connection>(curPrimary->tcpPort);
            c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
            break;
          }
        }

        if (outerInstance->stop->get()) {
          break;
        }

        Thread::currentThread().setName(L"indexer p" +
                                        to_wstring(curPrimary->id));

        if (LuceneTestCase::random()->nextInt(10) == 7) {
          // We use the marker docs to check for data loss in search thread:
          shared_ptr<Document> doc = make_shared<Document>();
          int id = outerInstance->markerID->getAndIncrement();
          wstring idString = L"m" + to_wstring(id);
          doc->push_back(LuceneTestCase::newStringField(L"docid", idString,
                                                        Field::Store::YES));
          doc->push_back(LuceneTestCase::newStringField(L"marker", L"marker",
                                                        Field::Store::YES));
          curPrimary->addOrUpdateDocument(c, doc, false);
          outerInstance->transLog->addDocument(idString, doc);
          // Only increment after primary replies:
          outerInstance->markerUpto->getAndIncrement();
          // message("index marker=" + idString + "; translog is " +
          // Node.bytesToString(Files.size(transLogPath)));
        }

        if (docCount > 0 &&
            LuceneTestCase::random()->nextDouble() < updatePct) {
          int randomID = LuceneTestCase::random()->nextInt(docCount);
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wstring randomIDString = Integer::toString(randomID);
          if (LuceneTestCase::random()->nextBoolean()) {
            // Replace previous doc
            shared_ptr<Document> doc = docs->nextDoc();
            (std::static_pointer_cast<Field>(doc->getField(L"docid")))
                ->setStringValue(randomIDString);
            curPrimary->addOrUpdateDocument(c, doc, true);
            outerInstance->transLog->updateDocument(randomIDString, doc);
          } else {
            // Delete previous doc
            curPrimary->deleteDocument(c, randomIDString);
            outerInstance->transLog->deleteDocuments(randomIDString);
          }
        } else {
          // Add new doc:
          shared_ptr<Document> doc = docs->nextDoc();
          // C++ TODO: There is no native C++ equivalent to 'toString':
          wstring idString = Integer::toString(docCount++);
          (std::static_pointer_cast<Field>(doc->getField(L"docid")))
              ->setStringValue(idString);
          curPrimary->addOrUpdateDocument(c, doc, false);
          outerInstance->transLog->addDocument(idString, doc);
        }
      } catch (const IOException &se) {
        // Assume primary crashed
        if (c != nullptr) {
          message(L"top: indexer lost connection to primary");
        }
        try {
          delete c;
        } catch (const runtime_error &t) {
        }
        curPrimary.reset();
        c.reset();
      }

      if (LuceneTestCase::random()->nextInt(sleepChance) == 0) {
        delay(10);
      }

      if (LuceneTestCase::random()->nextInt(100) == 17) {
        int pauseMS = TestUtil::nextInt(LuceneTestCase::random(), 500, 2000);
        wcout << L"Indexer: now pause for " << pauseMS << L" msec..." << endl;
        delay(pauseMS);
        wcout << L"Indexer: done pause for a bit..." << endl;
      }
    }
    if (curPrimary != nullptr) {
      try {
        c->out->writeByte(SimplePrimaryNode::CMD_INDEXING_DONE);
        c->flush();
        c->in_->readByte();
      } catch (const IOException &se) {
        // Assume primary crashed
        message(L"top: indexer lost connection to primary");
        try {
          delete c;
        } catch (const runtime_error &t) {
        }
        curPrimary.reset();
        c.reset();
      }
    }
    wcout << L"Indexer: now stop" << endl;
  } catch (const runtime_error &t) {
    outerInstance->failed->set(true);
    outerInstance->stop->set(true);
    throw runtime_error(t);
  }
}

void TestStressNRTReplication::message(const wstring &message)
{
  int64_t now = System::nanoTime();
  wcout << wstring::format(Locale::ROOT, L"%5.3fs       :     parent [%11s] %s",
                           (now - Node::globalStartNS) / 1000000000.0,
                           Thread::currentThread().getName(), message)
        << endl;
}

void TestStressNRTReplication::message(const wstring &message,
                                       int64_t localStartNS)
{
  int64_t now = System::nanoTime();
  wcout << wstring::format(Locale::ROOT, L"%5.3fs %5.1fs:     parent [%11s] %s",
                           (now - Node::globalStartNS) / 1000000000.0,
                           (now - localStartNS) / 1000000000.0,
                           Thread::currentThread().getName(), message)
        << endl;
}
} // namespace org::apache::lucene::replicator::nrt