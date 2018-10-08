using namespace std;

#include "SimplePrimaryNode.h"

namespace org::apache::lucene::replicator::nrt
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LogMergePolicy = org::apache::lucene::index::LogMergePolicy;
using MergePolicy = org::apache::lucene::index::MergePolicy;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using Term = org::apache::lucene::index::Term;
using TieredMergePolicy = org::apache::lucene::index::TieredMergePolicy;
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
using IndexInput = org::apache::lucene::store::IndexInput;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

SimplePrimaryNode::MergePreCopy::MergePreCopy(
    unordered_map<wstring, std::shared_ptr<FileMetaData>> &files)
    : files(files)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool SimplePrimaryNode::MergePreCopy::tryAddConnection(shared_ptr<Connection> c)
{
  if (finished_ == false) {
    connections.push_back(c);
    return true;
  } else {
    return false;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool SimplePrimaryNode::MergePreCopy::finished()
{
  if (connections.empty()) {
    finished_ = true;
    return true;
  } else {
    return false;
  }
}

SimplePrimaryNode::SimplePrimaryNode(
    shared_ptr<Random> random, shared_ptr<Path> indexPath, int id, int tcpPort,
    int64_t primaryGen, int64_t forcePrimaryVersion,
    shared_ptr<SearcherFactory> searcherFactory, bool doFlipBitsDuringCopy,
    bool doCheckIndexOnClose) 
    : PrimaryNode(initWriter(id, random, indexPath, doCheckIndexOnClose), id,
                  primaryGen, forcePrimaryVersion, searcherFactory,
                  System::out),
      tcpPort(tcpPort), random(make_shared<Random>(random->nextLong())),
      doFlipBitsDuringCopy(doFlipBitsDuringCopy)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SimplePrimaryNode::setReplicas(std::deque<int> &replicaIDs,
                                    std::deque<int> &replicaTCPPorts)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  message(L"top: set replicasIDs=" + Arrays->toString(replicaIDs) +
          L" tcpPorts=" + Arrays->toString(replicaTCPPorts));
  this->replicaIDs = replicaIDs;
  this->replicaTCPPorts = replicaTCPPorts;
}

shared_ptr<IndexWriter>
SimplePrimaryNode::initWriter(int id, shared_ptr<Random> random,
                              shared_ptr<Path> indexPath,
                              bool doCheckIndexOnClose) 
{
  shared_ptr<Directory> dir = SimpleReplicaNode::getDirectory(
      random, id, indexPath, doCheckIndexOnClose);

  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random);
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random, 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriterConfig> iwc =
      LuceneTestCase::newIndexWriterConfig(random, analyzer);

  shared_ptr<MergePolicy> mp = iwc->getMergePolicy();
  // iwc.setInfoStream(new PrintStreamInfoStream(System.out));

  // Force more frequent merging so we stress merge warming:
  if (std::dynamic_pointer_cast<TieredMergePolicy>(mp) != nullptr) {
    shared_ptr<TieredMergePolicy> tmp =
        std::static_pointer_cast<TieredMergePolicy>(mp);
    tmp->setSegmentsPerTier(3);
    tmp->setMaxMergeAtOnce(3);
  } else if (std::dynamic_pointer_cast<LogMergePolicy>(mp) != nullptr) {
    shared_ptr<LogMergePolicy> lmp =
        std::static_pointer_cast<LogMergePolicy>(mp);
    lmp->setMergeFactor(3);
  }

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, iwc);

  TestUtil::reduceOpenFiles(writer);
  return writer;
}

void SimplePrimaryNode::preCopyMergedSegmentFiles(
    shared_ptr<SegmentCommitInfo> info,
    unordered_map<wstring, std::shared_ptr<FileMetaData>>
        &files) 
{
  std::deque<int> replicaTCPPorts = this->replicaTCPPorts;
  if (replicaTCPPorts.empty()) {
    message(L"no replicas; skip warming " + info);
    return;
  }

  message(L"top: warm merge " + info + L" to " + replicaTCPPorts.size() +
          L" replicas; tcpPort=" + to_wstring(tcpPort) + L": files=" +
          files.keySet());

  shared_ptr<MergePreCopy> preCopy = make_shared<MergePreCopy>(files);
  warmingSegments.push_back(preCopy);

  try {

    shared_ptr<Set<wstring>> fileNames = files.keySet();

    // Ask all currently known replicas to pre-copy this newly merged segment's
    // files:
    for (auto replicaTCPPort : replicaTCPPorts) {
      try {
        shared_ptr<Connection> c = make_shared<Connection>(replicaTCPPort);
        c->out->writeByte(SimpleReplicaNode::CMD_PRE_COPY_MERGE);
        c->out->writeVLong(primaryGen);
        c->out->writeVInt(tcpPort);
        SimpleServer::writeFilesMetaData(c->out, files);
        c->flush();
        c->s->shutdownOutput();
        message(L"warm connection " + c->s);
        preCopy->connections.push_back(c);
      } catch (const runtime_error &t) {
        message(L"top: ignore exception trying to warm to replica port " +
                to_wstring(replicaTCPPort) + L": " + t);
        // t.printStackTrace(System.out);
      }
    }

    int64_t startNS = System::nanoTime();
    int64_t lastWarnNS = startNS;

    // TODO: maybe ... place some sort of time limit on how long we are willing
    // to wait for slow replica(s) to finish copying?
    while (preCopy->finished() == false) {
      try {
        delay(10);
      } catch (const InterruptedException &ie) {
        throw make_shared<ThreadInterruptedException>(ie);
      }

      if (isClosed()) {
        message(L"top: primary is closing: now cancel segment warming");
        {
          lock_guard<mutex> lock(preCopy->connections);
          IOUtils::closeWhileHandlingException(preCopy->connections);
        }
        return;
      }

      int64_t ns = System::nanoTime();
      if (ns - lastWarnNS > 1000000000LL) {
        message(wstring::format(Locale::ROOT,
                                L"top: warning: still warming merge " + info +
                                    L" to " + preCopy->connections.size() +
                                    L" replicas for %.1f sec...",
                                (ns - startNS) / 1000000000.0));
        lastWarnNS = ns;
      }

      {
        // Process keep-alives:
        lock_guard<mutex> lock(preCopy->connections);
        deque<std::shared_ptr<Connection>>::const_iterator it =
            preCopy->connections.begin();
        while (it != preCopy->connections.end()) {
          shared_ptr<Connection> c = *it;
          try {
            int64_t nowNS = System::nanoTime();
            bool done = false;
            while (c->sockIn->available() > 0) {
              char b = c->in_->readByte();
              if (b == 0) {
                // keep-alive
                c->lastKeepAliveNS = nowNS;
                message(L"keep-alive for socket=" + c->s + L" merge files=" +
                        files.keySet());
              } else {
                // merge is done pre-copying to this node
                if (b != 1) {
                  // C++ TODO: This exception's constructor requires an
                  // argument: ORIGINAL LINE: throw new
                  // IllegalArgumentException();
                  throw invalid_argument();
                }
                message(L"connection socket=" + c->s +
                        L" is done warming its merge " + info + L" files=" +
                        files.keySet());
                IOUtils::closeWhileHandlingException({c});
                (*it)->remove();
                done = true;
                break;
              }
            }

            // If > 2 sec since we saw a keep-alive, assume this replica is
            // dead:
            if (done == false && nowNS - c->lastKeepAliveNS > 2000000000LL) {
              message(L"top: warning: replica socket=" + c->s +
                      L" for segment=" + info +
                      L" seems to be dead; closing files=" + files.keySet());
              IOUtils::closeWhileHandlingException({c});
              (*it)->remove();
              done = true;
            }

            if (done == false && random->nextInt(1000) == 17) {
              message(L"top: warning: now randomly dropping replica from merge "
                      L"warming; files=" +
                      files.keySet());
              IOUtils::closeWhileHandlingException({c});
              (*it)->remove();
              done = true;
            }

          } catch (const runtime_error &t) {
            message(L"top: ignore exception trying to read byte during warm "
                    L"for segment=" +
                    info + L" to replica socket=" + c->s + L": " + t +
                    L" files=" + files.keySet());
            IOUtils::closeWhileHandlingException({c});
            (*it)->remove();
          }
          it++;
        }
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ TODO: The Java deque 'remove(Object)' method is not converted:
    warmingSegments.remove(preCopy);
  }
}

void SimplePrimaryNode::handleFlush(
    shared_ptr<DataInput> topIn, shared_ptr<DataOutput> topOut,
    shared_ptr<BufferedOutputStream> bos) 
{
  Thread::currentThread().setName(L"flush");

  int atLeastMarkerCount = topIn->readVInt();

  std::deque<int> replicaTCPPorts;
  std::deque<int> replicaIDs;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    replicaTCPPorts = this->replicaTCPPorts;
    replicaIDs = this->replicaIDs;
  }

  message(L"now flush; " + replicaIDs.size() + L" replicas");

  if (flushAndRefresh()) {
    // Something did get flushed (there were indexing ops since the last flush):

    verifyAtLeastMarkerCount(atLeastMarkerCount, nullptr);

    // Tell caller the version before pushing to replicas, so that even if we
    // crash after this, caller will know what version we (possibly) pushed to
    // some replicas.  Alternatively we could make this 2 separate ops?
    int64_t version = getCopyStateVersion();
    message(L"send flushed version=" + to_wstring(version));
    topOut->writeLong(version);
    bos->flush();

    // Notify current replicas:
    for (int i = 0; i < replicaIDs.size(); i++) {
      int replicaID = replicaIDs[i];
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (Connection c = new
      // Connection(replicaTCPPorts[i]))
      {
        Connection c = Connection(replicaTCPPorts[i]);
        try {
          message(L"send NEW_NRT_POINT to R" + to_wstring(replicaID) +
                  L" at tcpPort=" + to_wstring(replicaTCPPorts[i]));
          c->out->writeByte(SimpleReplicaNode::CMD_NEW_NRT_POINT);
          c->out->writeVLong(version);
          c->out->writeVLong(primaryGen);
          c->out->writeInt(tcpPort);
          c->flush();
          // TODO: we should use multicast to broadcast files out to replicas
          // TODO: ... replicas could copy from one another instead of just
          // primary
          // TODO: we could also prioritize one replica at a time?
        } catch (const runtime_error &t) {
          message(L"top: failed to connect R" + to_wstring(replicaID) +
                  L" for newNRTPoint; skipping: " + t.what());
        }
      }
    }
  } else {
    // No changes flushed:
    topOut->writeLong(-getCopyStateVersion());
  }
}

void SimplePrimaryNode::writeCopyState(
    shared_ptr<CopyState> state, shared_ptr<DataOutput> out) 
{
  // TODO (opto): we could encode to byte[] once when we created the copyState,
  // and then just send same byts to all replicas...
  out->writeVInt(state->infosBytes.size());
  out->writeBytes(state->infosBytes, 0, state->infosBytes.size());
  out->writeVLong(state->gen);
  out->writeVLong(state->version);
  SimpleServer::writeFilesMetaData(out, state->files);

  out->writeVInt(state->completedMergeFiles->size());
  for (auto fileName : state->completedMergeFiles) {
    out->writeString(fileName);
  }
  out->writeVLong(state->primaryGen);
}

bool SimplePrimaryNode::handleFetchFiles(
    shared_ptr<Random> random, shared_ptr<Socket> socket,
    shared_ptr<DataInput> destIn, shared_ptr<DataOutput> destOut,
    shared_ptr<BufferedOutputStream> bos) 
{
  Thread::currentThread().setName(L"send");

  int replicaID = destIn->readVInt();
  message(L"top: start fetch for R" + to_wstring(replicaID) + L" socket=" +
          socket);
  char b = destIn->readByte();
  shared_ptr<CopyState> copyState;
  if (b == 0) {
    // Caller already has CopyState
    copyState.reset();
  } else if (b == 1) {
    // Caller does not have CopyState; we pull the latest one:
    copyState = getCopyState();
    Thread::currentThread().setName(L"send-R" + to_wstring(replicaID) + L"-" +
                                    to_wstring(copyState->version));
  } else {
    // Protocol error:
    throw invalid_argument(L"invalid CopyState byte=" + to_wstring(b));
  }

  try {
    if (copyState != nullptr) {
      // Serialize CopyState on the wire to the client:
      writeCopyState(copyState, destOut);
      bos->flush();
    }

    std::deque<char> buffer(16384);
    int fileCount = 0;
    int64_t totBytesSent = 0;
    while (true) {
      char done = destIn->readByte();
      if (done == 1) {
        break;
      } else if (done != 0) {
        throw invalid_argument(L"expected 0 or 1 byte but got " +
                               to_wstring(done));
      }

      // Name of the file the replica wants us to send:
      wstring fileName = destIn->readString();

      // Starting offset in the file we should start sending bytes from:
      int64_t fpStart = destIn->readVLong();

      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput
      // in = dir.openInput(fileName, org.apache.lucene.store.IOContext.DEFAULT))
      {
        org::apache::lucene::store::IndexInput in_ = dir->openInput(
            fileName, org::apache::lucene::store::IOContext::DEFAULT);
        int64_t len = in_->length();
        // message("fetch " + fileName + ": send len=" + len);
        destOut->writeVLong(len);
        in_->seek(fpStart);
        int64_t upto = fpStart;
        while (upto < len) {
          int chunk = static_cast<int>(min(buffer.size(), (len - upto)));
          in_->readBytes(buffer, 0, chunk);
          if (doFlipBitsDuringCopy) {
            if (random->nextInt(3000) == 17 &&
                bitFlipped->contains(fileName) == false) {
              bitFlipped->add(fileName);
              message(L"file " + fileName + L" to R" + to_wstring(replicaID) +
                      L": now randomly flipping a bit at byte=" +
                      to_wstring(upto));
              int x = random->nextInt(chunk);
              int bit = random->nextInt(8);
              buffer[x] ^= 1 << bit;
            }
          }
          destOut->writeBytes(buffer, 0, chunk);
          upto += chunk;
          totBytesSent += chunk;
        }
      }

      fileCount++;
    }

    message(L"top: done fetch files for R" + to_wstring(replicaID) +
            L": sent " + to_wstring(fileCount) + L" files; sent " +
            to_wstring(totBytesSent) + L" bytes");
  } catch (const runtime_error &t) {
    message(L"top: exception during fetch: " + t.what() +
            L"; now close socket");
    socket->close();
    return false;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (copyState != nullptr) {
      message(L"top: fetch: now release CopyState");
      releaseCopyState(copyState);
    }
  }

  return true;
}

const shared_ptr<org::apache::lucene::document::FieldType>
    SimplePrimaryNode::tokenizedWithTermVectors;

SimplePrimaryNode::StaticConstructor::StaticConstructor()
{
  tokenizedWithTermVectors = make_shared<FieldType>(TextField::TYPE_STORED);
  tokenizedWithTermVectors->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  tokenizedWithTermVectors->setStoreTermVectors(true);
  tokenizedWithTermVectors->setStoreTermVectorOffsets(true);
  tokenizedWithTermVectors->setStoreTermVectorPositions(true);
}

SimplePrimaryNode::StaticConstructor SimplePrimaryNode::staticConstructor;

void SimplePrimaryNode::handleIndexing(
    shared_ptr<Socket> socket, shared_ptr<AtomicBoolean> stop,
    shared_ptr<InputStream> is, shared_ptr<DataInput> in_,
    shared_ptr<DataOutput> out,
    shared_ptr<BufferedOutputStream> bos) throw(IOException,
                                                InterruptedException)
{
  Thread::currentThread().setName(L"indexing");
  message(L"start handling indexing socket=" + socket);
  while (true) {
    while (true) {
      if (is->available() > 0) {
        break;
      }
      if (stop->get()) {
        return;
      }
      delay(10);
    }
    char cmd;
    try {
      cmd = in_->readByte();
    } catch (const EOFException &eofe) {
      // done
      return;
    }
    // message("INDEXING OP " + cmd);
    if (cmd == CMD_ADD_DOC) {
      handleAddDocument(in_, out);
      out->writeByte(static_cast<char>(1));
      bos->flush();
    } else if (cmd == CMD_UPDATE_DOC) {
      handleUpdateDocument(in_, out);
      out->writeByte(static_cast<char>(1));
      bos->flush();
    } else if (cmd == CMD_DELETE_DOC) {
      handleDeleteDocument(in_, out);
      out->writeByte(static_cast<char>(1));
      bos->flush();
    } else if (cmd == CMD_DELETE_ALL_DOCS) {
      writer->deleteAll();
      out->writeByte(static_cast<char>(1));
      bos->flush();
    } else if (cmd == CMD_FORCE_MERGE) {
      writer->forceMerge(1);
      out->writeByte(static_cast<char>(1));
      bos->flush();
    } else if (cmd == CMD_INDEXING_DONE) {
      out->writeByte(static_cast<char>(1));
      bos->flush();
      break;
    } else {
      throw invalid_argument(L"cmd must be add, update or delete; got " +
                             to_wstring(cmd));
    }
  }
}

void SimplePrimaryNode::handleAddDocument(
    shared_ptr<DataInput> in_, shared_ptr<DataOutput> out) 
{
  int fieldCount = in_->readVInt();
  shared_ptr<Document> doc = make_shared<Document>();
  for (int i = 0; i < fieldCount; i++) {
    wstring name = in_->readString();
    wstring value = in_->readString();
    // NOTE: clearly NOT general!
    if (name == L"docid" || name == L"marker") {
      doc->push_back(make_shared<StringField>(name, value, Field::Store::YES));
    } else if (name == L"title") {
      doc->push_back(
          make_shared<StringField>(L"title", value, Field::Store::YES));
      doc->push_back(make_shared<Field>(L"titleTokenized", value,
                                        tokenizedWithTermVectors));
    } else if (name == L"body") {
      doc->push_back(
          make_shared<Field>(L"body", value, tokenizedWithTermVectors));
    } else {
      throw invalid_argument(L"unhandled field name " + name);
    }
  }
  writer->addDocument(doc);
}

void SimplePrimaryNode::handleUpdateDocument(
    shared_ptr<DataInput> in_, shared_ptr<DataOutput> out) 
{
  int fieldCount = in_->readVInt();
  shared_ptr<Document> doc = make_shared<Document>();
  wstring docid = L"";
  for (int i = 0; i < fieldCount; i++) {
    wstring name = in_->readString();
    wstring value = in_->readString();
    // NOTE: clearly NOT general!
    if (name == L"docid") {
      docid = value;
      doc->push_back(
          make_shared<StringField>(L"docid", value, Field::Store::YES));
    } else if (name == L"marker") {
      doc->push_back(
          make_shared<StringField>(L"marker", value, Field::Store::YES));
    } else if (name == L"title") {
      doc->push_back(
          make_shared<StringField>(L"title", value, Field::Store::YES));
      doc->push_back(make_shared<Field>(L"titleTokenized", value,
                                        tokenizedWithTermVectors));
    } else if (name == L"body") {
      doc->push_back(
          make_shared<Field>(L"body", value, tokenizedWithTermVectors));
    } else {
      throw invalid_argument(L"unhandled field name " + name);
    }
  }

  writer->updateDocument(make_shared<Term>(L"docid", docid), doc);
}

void SimplePrimaryNode::handleDeleteDocument(
    shared_ptr<DataInput> in_, shared_ptr<DataOutput> out) 
{
  wstring docid = in_->readString();
  writer->deleteDocuments({make_shared<Term>(L"docid", docid)});
}

void SimplePrimaryNode::handleOneConnection(
    shared_ptr<Random> random, shared_ptr<ServerSocket> ss,
    shared_ptr<AtomicBoolean> stop, shared_ptr<InputStream> is,
    shared_ptr<Socket> socket, shared_ptr<DataInput> in_,
    shared_ptr<DataOutput> out,
    shared_ptr<BufferedOutputStream> bos) throw(IOException,
                                                InterruptedException)
{

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

    case CMD_FLUSH:
      handleFlush(in_, out, bos);
      break;

    case CMD_FETCH_FILES:
      // Replica (other node) is asking us (primary node) for files to copy
      handleFetchFiles(random, socket, in_, out, bos);
      break;

    case CMD_INDEXING:
      handleIndexing(socket, stop, is, in_, out, bos);
      break;

    case CMD_GET_SEARCHING_VERSION:
      out->writeVLong(getCurrentSearchingVersion());
      break;

    case CMD_SEARCH: {
      Thread::currentThread().setName(L"search");
      shared_ptr<IndexSearcher> searcher = mgr->acquire();
      try {
        int64_t version = (std::static_pointer_cast<DirectoryReader>(
                                 searcher->getIndexReader()))
                                ->getVersion();
        int hitCount = searcher->count(
            make_shared<TermQuery>(make_shared<Term>(L"body", L"the")));
        // message("version=" + version + " searcher=" + searcher);
        out->writeVLong(version);
        out->writeVInt(hitCount);
        bos->flush();
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        mgr->release(searcher);
      }
      bos->flush();
    }
      goto outerContinue;

    case CMD_SEARCH_ALL: {
      Thread::currentThread().setName(L"search all");
      shared_ptr<IndexSearcher> searcher = mgr->acquire();
      try {
        int64_t version = (std::static_pointer_cast<DirectoryReader>(
                                 searcher->getIndexReader()))
                                ->getVersion();
        int hitCount = searcher->count(make_shared<MatchAllDocsQuery>());
        // message("version=" + version + " searcher=" + searcher);
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

    case CMD_MARKER_SEARCH: {
      Thread::currentThread().setName(L"msearch");
      int expectedAtLeastCount = in_->readVInt();
      verifyAtLeastMarkerCount(expectedAtLeastCount, out);
      bos->flush();
    }
      goto outerContinue;

    case CMD_COMMIT:
      Thread::currentThread().setName(L"commit");
      commit();
      out->writeByte(static_cast<char>(1));
      break;

    case CMD_CLOSE:
      Thread::currentThread().setName(L"close");
      message(L"top close: now close server socket");
      ss->close();
      out->writeByte(static_cast<char>(1));
      message(L"top close: done close server socket");
      break;

    case CMD_SET_REPLICAS: {
      Thread::currentThread().setName(L"set repls");
      int count = in_->readVInt();
      std::deque<int> replicaIDs(count);
      std::deque<int> replicaTCPPorts(count);
      for (int i = 0; i < count; i++) {
        replicaIDs[i] = in_->readVInt();
        replicaTCPPorts[i] = in_->readVInt();
      }
      out->writeByte(static_cast<char>(1));
      setReplicas(replicaIDs, replicaTCPPorts);
      break;
    }
    case CMD_NEW_REPLICA: {
      Thread::currentThread().setName(L"new repl");
      int replicaTCPPort = in_->readVInt();
      message(L"new replica: " + warmingSegments.size() +
              L" current warming merges");
      // Step through all currently warming segments and try to add this replica
      // if it isn't there already:
      {
        lock_guard<mutex> lock(warmingSegments);
        for (auto preCopy : warmingSegments) {
          message(L"warming segment " + preCopy->files.keySet());
          bool found = false;
          {
            lock_guard<mutex> lock(preCopy->connections);
            for (auto c : preCopy->connections) {
              if (c->destTCPPort == replicaTCPPort) {
                found = true;
                break;
              }
            }
          }

          if (found) {
            message(L"this replica is already warming this segment; skipping");
            // It's possible (maybe) that the replica started up, then a merge
            // kicked off, and it warmed to this new replica, all before the
            // replica sent us this command:
            continue;
          }

          // OK, this new replica is not already warming this segment, so
          // attempt (could fail) to start warming now:

          shared_ptr<Connection> c = make_shared<Connection>(replicaTCPPort);
          if (preCopy->tryAddConnection(c) == false) {
            // This can happen, if all other replicas just now finished warming
            // this segment, and so we were just a bit too late.  In this case
            // the segment will be copied over in the next nrt point sent to
            // this replica
            message(L"failed to add connection to segment warmer (too late); "
                    L"closing");
            delete c;
          }
          c->out->writeByte(SimpleReplicaNode::CMD_PRE_COPY_MERGE);
          c->out->writeVLong(primaryGen);
          c->out->writeVInt(tcpPort);
          SimpleServer::writeFilesMetaData(c->out, preCopy->files);
          c->flush();
          c->s->shutdownOutput();
          message(L"successfully started warming");
        }
      }
      break;
    }
    default:
      throw invalid_argument(L"unrecognized cmd=" + to_wstring(cmd) +
                             L" via socket=" + socket);
    }
    bos->flush();
    break;
  outerContinue:;
  }
outerBreak:;
}

void SimplePrimaryNode::verifyAtLeastMarkerCount(
    int expectedAtLeastCount, shared_ptr<DataOutput> out) 
{
  shared_ptr<IndexSearcher> searcher = mgr->acquire();
  try {
    int64_t version =
        (std::static_pointer_cast<DirectoryReader>(searcher->getIndexReader()))
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
      throw make_shared<IllegalStateException>(
          L"at flush: marker count " + to_wstring(hitCount) +
          L" but expected at least " + to_wstring(expectedAtLeastCount) +
          L" version=" + to_wstring(version));
    }

    if (out != nullptr) {
      out->writeVLong(version);
      out->writeVInt(hitCount);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    mgr->release(searcher);
  }
}
} // namespace org::apache::lucene::replicator::nrt