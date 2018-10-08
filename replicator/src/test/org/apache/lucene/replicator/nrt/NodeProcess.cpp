using namespace std;

#include "NodeProcess.h"

namespace org::apache::lucene::replicator::nrt
{
using Document = org::apache::lucene::document::Document;

NodeProcess::NodeProcess(shared_ptr<Process> p, int id, int tcpPort,
                         shared_ptr<Thread> pumper, bool isPrimary,
                         int64_t initCommitVersion,
                         int64_t initInfosVersion,
                         shared_ptr<AtomicBoolean> nodeIsClosing)
    : p(p), tcpPort(tcpPort), id(id), pumper(pumper),
      lock(make_shared<ReentrantLock>()), isPrimary(isPrimary),
      initCommitVersion(initCommitVersion), initInfosVersion(initInfosVersion),
      nodeIsClosing(nodeIsClosing)
{
  assert((initInfosVersion >= initCommitVersion,
          L"initInfosVersion=" + to_wstring(initInfosVersion) +
              L" initCommitVersion=" + to_wstring(initCommitVersion)));
}

wstring NodeProcess::toString()
{
  if (isPrimary) {
    return L"P" + to_wstring(id) + L" tcpPort=" + to_wstring(tcpPort);
  } else {
    return L"R" + to_wstring(id) + L" tcpPort=" + to_wstring(tcpPort);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void NodeProcess::crash()
{
  if (isOpen) {
    isOpen = false;
    p->destroy();
    try {
      p->waitFor();
      pumper->join();
    } catch (const InterruptedException &ie) {
      Thread::currentThread().interrupt();
      throw runtime_error(ie);
    }
  }
}

bool NodeProcess::commit() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new Connection(tcpPort))
  {
    Connection c = Connection(tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_COMMIT);
    c->flush();
    c->s->shutdownOutput();
    if (c->in_->readByte() != 1) {
      throw runtime_error(L"commit failed");
    }
    return true;
  }
}

void NodeProcess::commitAsync() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new Connection(tcpPort))
  {
    Connection c = Connection(tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_COMMIT);
    c->flush();
  }
}

int64_t NodeProcess::getSearchingVersion() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new Connection(tcpPort))
  {
    Connection c = Connection(tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_GET_SEARCHING_VERSION);
    c->flush();
    c->s->shutdownOutput();
    return c->in_->readVLong();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t NodeProcess::flush(int atLeastMarkerCount) 
{
  assert(isPrimary);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new Connection(tcpPort))
  {
    Connection c = Connection(tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_FLUSH);
    c->out->writeVInt(atLeastMarkerCount);
    c->flush();
    c->s->shutdownOutput();
    return c->in_->readLong();
  }
}

NodeProcess::~NodeProcess() { shutdown(); }

// C++ WARNING: The following method was originally marked 'synchronized':
bool NodeProcess::shutdown()
{
  lock->lock();
  try {
    // System.out.println("PARENT: now shutdown node=" + id + " isOpen=" +
    // isOpen);
    if (isOpen) {
      // Ask the child process to shutdown gracefully:
      isOpen = false;
      // System.out.println("PARENT: send CMD_CLOSE to node=" + id);
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (Connection c = new
      // Connection(tcpPort))
      {
        Connection c = Connection(tcpPort);
        try {
          c->out->writeByte(SimplePrimaryNode::CMD_CLOSE);
          c->flush();
          if (c->in_->readByte() != 1) {
            throw runtime_error(L"shutdown failed");
          }
        } catch (const runtime_error &t) {
          wcout << L"top: shutdown failed; ignoring" << endl;
          t.printStackTrace(System::out);
        }
      }
      try {
        p->waitFor();
        pumper->join();
      } catch (const InterruptedException &ie) {
        Thread::currentThread().interrupt();
        throw runtime_error(ie);
      }
    }

    return true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    lock->unlock();
  }
}

void NodeProcess::newNRTPoint(int64_t version, int64_t primaryGen,
                              int primaryTCPPort) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new Connection(tcpPort))
  {
    Connection c = Connection(tcpPort);
    c->out->writeByte(SimpleReplicaNode::CMD_NEW_NRT_POINT);
    c->out->writeVLong(version);
    c->out->writeVLong(primaryGen);
    c->out->writeInt(primaryTCPPort);
    c->flush();
  }
}

void NodeProcess::addOrUpdateDocument(shared_ptr<Connection> c,
                                      shared_ptr<Document> doc,
                                      bool isUpdate) 
{
  if (isPrimary == false) {
    throw make_shared<IllegalStateException>(L"only primary can index");
  }
  int fieldCount = 0;

  wstring title = doc[L"title"];
  if (title != L"") {
    fieldCount++;
  }

  wstring docid = doc[L"docid"];
  assert(docid != L"");
  fieldCount++;

  wstring body = doc[L"body"];
  if (body != L"") {
    fieldCount++;
  }

  wstring marker = doc[L"marker"];
  if (marker != L"") {
    fieldCount++;
  }

  c->out->writeByte(isUpdate ? SimplePrimaryNode::CMD_UPDATE_DOC
                             : SimplePrimaryNode::CMD_ADD_DOC);
  c->out->writeVInt(fieldCount);
  c->out->writeString(L"docid");
  c->out->writeString(docid);
  if (title != L"") {
    c->out->writeString(L"title");
    c->out->writeString(title);
  }
  if (body != L"") {
    c->out->writeString(L"body");
    c->out->writeString(body);
  }
  if (marker != L"") {
    c->out->writeString(L"marker");
    c->out->writeString(marker);
  }
  c->flush();
  c->in_->readByte();
}

void NodeProcess::deleteDocument(shared_ptr<Connection> c,
                                 const wstring &docid) 
{
  if (isPrimary == false) {
    throw make_shared<IllegalStateException>(
        L"only primary can delete documents");
  }
  c->out->writeByte(SimplePrimaryNode::CMD_DELETE_DOC);
  c->out->writeString(docid);
  c->flush();
  c->in_->readByte();
}

void NodeProcess::deleteAllDocuments(shared_ptr<Connection> c) throw(
    IOException)
{
  if (isPrimary == false) {
    throw make_shared<IllegalStateException>(
        L"only primary can delete documents");
  }
  c->out->writeByte(SimplePrimaryNode::CMD_DELETE_ALL_DOCS);
  c->flush();
  c->in_->readByte();
}

void NodeProcess::forceMerge(shared_ptr<Connection> c) 
{
  if (isPrimary == false) {
    throw make_shared<IllegalStateException>(L"only primary can force merge");
  }
  c->out->writeByte(SimplePrimaryNode::CMD_FORCE_MERGE);
  c->flush();
  c->in_->readByte();
}
} // namespace org::apache::lucene::replicator::nrt