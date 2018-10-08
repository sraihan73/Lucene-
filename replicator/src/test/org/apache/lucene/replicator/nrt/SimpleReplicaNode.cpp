using namespace std;

#include "SimpleReplicaNode.h"

namespace org::apache::lucene::replicator::nrt
{
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using RateLimitedIndexOutput =
    org::apache::lucene::store::RateLimitedIndexOutput;
using RateLimiter = org::apache::lucene::store::RateLimiter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

SimpleReplicaNode::SimpleReplicaNode(
    shared_ptr<Random> random, int id, int tcpPort, shared_ptr<Path> indexPath,
    int64_t curPrimaryGen, int primaryTCPPort,
    shared_ptr<SearcherFactory> searcherFactory,
    bool doCheckIndexOnClose) 
    : ReplicaNode(id, getDirectory(random, id, indexPath, doCheckIndexOnClose),
                  searcherFactory, System::out),
      tcpPort(tcpPort), jobs(make_shared<Jobs>(shared_from_this())),
      fetchRateLimiter(make_shared<RateLimiter::SimpleRateLimiter>(mbPerSec)),
      random(make_shared<Random>(random->nextLong()))
{

  // Random IO throttling on file copies: 5 - 20 MB/sec:
  double mbPerSec = 5 * (1.0 + 3 * random->nextDouble());
  message(wstring::format(Locale::ROOT,
                          L"top: will rate limit file fetch to %.2f MB/sec",
                          mbPerSec));
  this->curPrimaryTCPPort = primaryTCPPort;

  start(curPrimaryGen);

  // Handles fetching files from primary:
  jobs->setName(L"R" + to_wstring(id) + L".copyJobs");
  jobs->setDaemon(true);
  jobs->start();
}

void SimpleReplicaNode::launch(shared_ptr<CopyJob> job) { jobs->launch(job); }

SimpleReplicaNode::~SimpleReplicaNode()
{
  // Can't be sync'd when calling jobs since it can lead to deadlock:
  delete jobs;
  message(L"top: jobs closed");
  {
    lock_guard<mutex> lock(mergeCopyJobs);
    for (auto job : mergeCopyJobs) {
      message(L"top: cancel merge copy job " + job);
      job->cancel(L"jobs closing", nullptr);
    }
  }
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
}

shared_ptr<CopyJob> SimpleReplicaNode::newCopyJob(
    const wstring &reason,
    unordered_map<wstring, std::shared_ptr<FileMetaData>> &files,
    unordered_map<wstring, std::shared_ptr<FileMetaData>> &prevFiles,
    bool highPriority,
    shared_ptr<CopyJob::OnceDone> onceDone) 
{
  shared_ptr<Connection> c;
  shared_ptr<CopyState> copyState;

  // Exceptions in here mean something went wrong talking over the socket, which
  // are fine (e.g. primary node crashed):
  try {
    c = make_shared<Connection>(curPrimaryTCPPort);
    c->out->writeByte(SimplePrimaryNode::CMD_FETCH_FILES);
    c->out->writeVInt(id);
    if (files.empty()) {
      // No incoming CopyState: ask primary for latest one now
      c->out->writeByte(static_cast<char>(1));
      c->flush();
      copyState = SimpleServer::readCopyState(c->in_);
      files = copyState->files;
    } else {
      c->out->writeByte(static_cast<char>(0));
      copyState.reset();
    }
  } catch (const runtime_error &t) {
    throw make_shared<NodeCommunicationException>(
        L"exc while reading files to copy", t);
  }

  return make_shared<SimpleCopyJob>(reason, c, copyState, shared_from_this(),
                                    files, highPriority, onceDone);
}

shared_ptr<Directory>
SimpleReplicaNode::getDirectory(shared_ptr<Random> random, int id,
                                shared_ptr<Path> path,
                                bool doCheckIndexOnClose) 
{
  shared_ptr<MockDirectoryWrapper> dir =
      LuceneTestCase::newMockFSDirectory(path);

  dir->setAssertNoUnrefencedFilesOnClose(true);
  dir->setCheckIndexOnClose(doCheckIndexOnClose);

  // Corrupt any index files not referenced by current commit point; this is
  // important (increases test evilness) because we may have done a hard crash
  // of the previous JVM writing to this directory and so MDW's
  // corrupt-unknown-files-on-close never ran:
  Node::nodeMessage(System::out, id, L"top: corrupt unknown files");
  dir->corruptUnknownFiles();

  return dir;
}

void SimpleReplicaNode::handleOneConnection(
    shared_ptr<ServerSocket> ss, shared_ptr<AtomicBoolean> stop,
    shared_ptr<InputStream> is, shared_ptr<Socket> socket,
    shared_ptr<DataInput> in_, shared_ptr<DataOutput> out,
    shared_ptr<BufferedOutputStream> bos) throw(IOException,
                                                InterruptedException)
{
  // message("one connection: " + socket);
  while (true) {
    char cmd;
    while (true) {
      if (is->available() > 0) {
        break;
      }
      if (stop->get()) {
        return;
      }
      delay(10);
    }

    try {
      cmd = in_->readByte();
    } catch (const EOFException &eofe) {
      break;
    }

    switch (cmd) {
    case CMD_NEW_NRT_POINT: {
      int64_t version = in_->readVLong();
      int64_t newPrimaryGen = in_->readVLong();
      Thread::currentThread().setName(L"recv-" + to_wstring(version));
      curPrimaryTCPPort = in_->readInt();
      message(L"newNRTPoint primaryTCPPort=" + to_wstring(curPrimaryTCPPort) +
              L" version=" + to_wstring(version) + L" newPrimaryGen=" +
              to_wstring(newPrimaryGen));
      newNRTPoint(newPrimaryGen, version);
    } break;

    case SimplePrimaryNode::CMD_GET_SEARCHING_VERSION:
      // This is called when primary has crashed and we need to elect a new
      // primary from all the still running replicas:

      // Tricky: if a sync is just finishing up, i.e. managed to finish copying
      // all files just before we crashed primary, and is now in the process of
      // opening a new reader, we need to wait for it, to be sure we really pick
      // the most current replica:
      if (isCopying()) {
        message(L"top: getSearchingVersion: now wait for finish sync");
        // TODO: use immediate concurrency instead of polling:
        while (isCopying() && stop->get() == false) {
          delay(10);
          message(L"top: curNRTCopy=" + curNRTCopy);
        }
        message(L"top: getSearchingVersion: done wait for finish sync");
      }
      if (stop->get() == false) {
        out->writeVLong(getCurrentSearchingVersion());
      } else {
        message(L"top: getSearchingVersion: stop waiting for finish sync: stop "
                L"is set");
      }
      break;

    case SimplePrimaryNode::CMD_SEARCH: {
      Thread::currentThread().setName(L"search");
      shared_ptr<IndexSearcher> searcher = mgr->acquire();
      try {
        int64_t version = (std::static_pointer_cast<DirectoryReader>(
                                 searcher->getIndexReader()))
                                ->getVersion();
        int hitCount = searcher->count(
            make_shared<TermQuery>(make_shared<Term>(L"body", L"the")));
        // node.message("version=" + version + " searcher=" + searcher);
        out->writeVLong(version);
        out->writeVInt(hitCount);
        bos->flush();
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(searcher);
      }
    }
      goto outerContinue;

    case SimplePrimaryNode::CMD_SEARCH_ALL: {
      Thread::currentThread().setName(L"search all");
      shared_ptr<IndexSearcher> searcher = mgr->acquire();
      try {
        int64_t version = (std::static_pointer_cast<DirectoryReader>(
                                 searcher->getIndexReader()))
                                ->getVersion();
        int hitCount = searcher->count(make_shared<MatchAllDocsQuery>());
        // node.message("version=" + version + " searcher=" + searcher);
        out->writeVLong(version);
        out->writeVInt(hitCount);
        bos->flush();
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(searcher);
      }
    }
      goto outerContinue;

    case SimplePrimaryNode::CMD_MARKER_SEARCH: {
      Thread::currentThread().setName(L"msearch");
      int expectedAtLeastCount = in_->readVInt();
      shared_ptr<IndexSearcher> searcher = mgr->acquire();
      try {
        int64_t version = (std::static_pointer_cast<DirectoryReader>(
                                 searcher->getIndexReader()))
                                ->getVersion();
        int hitCount = searcher->count(
            make_shared<TermQuery>(make_shared<Term>(L"marker", L"marker")));
        if (hitCount < expectedAtLeastCount) {
          message(L"marker search: expectedAtLeastCount=" +
                  to_wstring(expectedAtLeastCount) + L" but hitCount=" +
                  to_wstring(hitCount));
          shared_ptr<TopDocs> hits = searcher->search(
              make_shared<TermQuery>(make_shared<Term>(L"marker", L"marker")),
              expectedAtLeastCount);
          deque<int> seen = deque<int>();
          for (auto hit : hits->scoreDocs) {
            shared_ptr<Document> doc = searcher->doc(hit->doc);
            seen.push_back(stoi(doc[L"docid"]->substr(1)));
          }
          sort(seen.begin(), seen.end());
          message(L"saw markers:");
          for (auto marker : seen) {
            message(L"saw m" + to_wstring(marker));
          }
        }

        out->writeVLong(version);
        out->writeVInt(hitCount);
        bos->flush();
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(searcher);
      }
    }
      goto outerContinue;

    case SimplePrimaryNode::CMD_COMMIT:
      Thread::currentThread().setName(L"commit");
      commit();
      out->writeByte(static_cast<char>(1));
      break;

    case SimplePrimaryNode::CMD_CLOSE:
      Thread::currentThread().setName(L"close");
      ss->close();
      out->writeByte(static_cast<char>(1));
      goto outerBreak;

    case CMD_PRE_COPY_MERGE: {
      Thread::currentThread().setName(L"merge copy");

      int64_t newPrimaryGen = in_->readVLong();
      curPrimaryTCPPort = in_->readVInt();
      unordered_map<wstring, std::shared_ptr<FileMetaData>> files =
          SimpleServer::readFilesMetaData(in_);
      message(L"done reading files to copy files=" + files.keySet());
      shared_ptr<AtomicBoolean> finished = make_shared<AtomicBoolean>();
      shared_ptr<CopyJob> job =
          launchPreCopyMerge(finished, newPrimaryGen, files);
      message(L"done launching copy job files=" + files.keySet());

      // Silly keep alive mechanism, else if e.g. we (replica node) crash, the
      // primary won't notice for a very long time:
      bool success = false;
      try {
        int count = 0;
        while (true) {
          if (finished->get() || stop->get()) {
            break;
          }
          delay(10);
          count++;
          if (count == 100) {
            // Once per second or so, we send a keep alive
            message(L"send merge pre copy keep alive... files=" +
                    files.keySet());

            // To be evil, we sometimes fail to keep-alive, e.g. simulating a
            // long GC pausing us:
            if (random->nextBoolean()) {
              out->writeByte(static_cast<char>(0));
              count = 0;
            }
          }
        }

        out->writeByte(static_cast<char>(1));
        bos->flush();
        success = true;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        message(L"done merge copy files=" + files.keySet() + L" success=" +
                StringHelper::toString(success));
      }
      break;
    }
    default:
      throw invalid_argument(L"unrecognized cmd=" + to_wstring(cmd));
    }
    bos->flush();

    break;
  outerContinue:;
  }
outerBreak:;
}

void SimpleReplicaNode::sendNewReplica() 
{
  message(L"send new_replica to primary tcpPort=" +
          to_wstring(curPrimaryTCPPort));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(curPrimaryTCPPort))
  {
    Connection c = Connection(curPrimaryTCPPort);
    try {
      c->out->writeByte(SimplePrimaryNode::CMD_NEW_REPLICA);
      c->out->writeVInt(tcpPort);
      c->flush();
      c->s->shutdownOutput();
    } catch (const runtime_error &t) {
      message(L"ignoring exc " + t +
              L" sending new_replica to primary tcpPort=" +
              to_wstring(curPrimaryTCPPort));
    }
  }
}

shared_ptr<IndexOutput> SimpleReplicaNode::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> ioContext) 
{
  return make_shared<RateLimitedIndexOutput>(
      fetchRateLimiter,
      ReplicaNode::createTempOutput(prefix, suffix, ioContext));
}
} // namespace org::apache::lucene::replicator::nrt