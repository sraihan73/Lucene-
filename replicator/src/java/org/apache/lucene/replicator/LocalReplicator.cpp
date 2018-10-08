using namespace std;

#include "LocalReplicator.h"

namespace org::apache::lucene::replicator
{
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;

LocalReplicator::RefCountedRevision::RefCountedRevision(
    shared_ptr<Revision> revision)
    : revision(revision)
{
}

void LocalReplicator::RefCountedRevision::decRef() 
{
  if (refCount->get() <= 0) {
    throw make_shared<IllegalStateException>(
        L"this revision is already released");
  }

  constexpr int rc = refCount->decrementAndGet();
  if (rc == 0) {
    bool success = false;
    try {
      revision->release();
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        // Put reference back on failure
        refCount->incrementAndGet();
      }
    }
  } else if (rc < 0) {
    throw make_shared<IllegalStateException>(
        L"too many decRef calls: refCount is " + to_wstring(rc) +
        L" after decrement");
  }
}

void LocalReplicator::RefCountedRevision::incRef()
{
  refCount->incrementAndGet();
}

LocalReplicator::ReplicationSession::ReplicationSession(
    shared_ptr<SessionToken> session, shared_ptr<RefCountedRevision> revision)
    : session(session), revision(revision)
{
  lastAccessTime = TimeUnit::MILLISECONDS::convert(System::nanoTime(),
                                                   TimeUnit::NANOSECONDS);
}

bool LocalReplicator::ReplicationSession::isExpired(
    int64_t expirationThreshold)
{
  return lastAccessTime < (TimeUnit::MILLISECONDS::convert(
                               System::nanoTime(), TimeUnit::NANOSECONDS) -
                           expirationThreshold);
}

void LocalReplicator::ReplicationSession::markAccessed()
{
  lastAccessTime = TimeUnit::MILLISECONDS::convert(System::nanoTime(),
                                                   TimeUnit::NANOSECONDS);
}

void LocalReplicator::checkExpiredSessions() 
{
  // make a "to-delete" deque so we don't risk deleting from the map_obj while
  // iterating it
  const deque<std::shared_ptr<ReplicationSession>> toExpire =
      deque<std::shared_ptr<ReplicationSession>>();
  for (auto token : sessions) {
    if (token->second.isExpired(expirationThresholdMilllis)) {
      toExpire.push_back(token->second);
    }
  }
  for (auto token : toExpire) {
    releaseSession(token->session->id);
  }
}

void LocalReplicator::releaseSession(const wstring &sessionID) throw(
    IOException)
{
  shared_ptr<ReplicationSession> session = sessions.erase(sessionID);
  // if we're called concurrently by close() and release(), could be that one
  // thread beats the other to release the session.
  if (session != nullptr) {
    session->revision->decRef();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LocalReplicator::ensureOpen()
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(
        L"This replicator has already been closed");
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<SessionToken>
LocalReplicator::checkForUpdate(const wstring &currentVersion)
{
  ensureOpen();
  if (currentRevision == nullptr) { // no published revisions yet
    return nullptr;
  }

  if (currentVersion != L"" &&
      currentRevision->revision->compareTo(currentVersion) <= 0) {
    // currentVersion is newer or equal to latest published revision
    return nullptr;
  }

  // currentVersion is either null or older than latest published revision
  currentRevision->incRef();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  const wstring sessionID = Integer::toString(sessionToken->incrementAndGet());
  shared_ptr<SessionToken> *const sessionToken =
      make_shared<SessionToken>(sessionID, currentRevision->revision);
  shared_ptr<ReplicationSession> *const timedSessionToken =
      make_shared<ReplicationSession>(sessionToken, currentRevision);
  sessions.emplace(sessionID, timedSessionToken);
  return sessionToken;
}

// C++ WARNING: The following method was originally marked 'synchronized':
LocalReplicator::~LocalReplicator()
{
  if (!closed) {
    // release all managed revisions
    for (auto session : sessions) {
      session->second.revision.decRef();
    }
    sessions.clear();
    closed = true;
  }
}

int64_t LocalReplicator::getExpirationThreshold()
{
  return expirationThresholdMilllis;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<InputStream>
LocalReplicator::obtainFile(const wstring &sessionID, const wstring &source,
                            const wstring &fileName) 
{
  ensureOpen();
  shared_ptr<ReplicationSession> session = sessions[sessionID];
  if (session != nullptr && session->isExpired(expirationThresholdMilllis)) {
    releaseSession(sessionID);
    session.reset();
  }
  // session either previously expired, or we just expired it
  if (session == nullptr) {
    throw make_shared<SessionExpiredException>(
        L"session (" + sessionID + L") expired while obtaining file: source=" +
        source + L" file=" + fileName);
  }
  sessions[sessionID]->markAccessed();
  return session->revision->revision->open(source, fileName);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LocalReplicator::publish(shared_ptr<Revision> revision) 
{
  ensureOpen();
  if (currentRevision != nullptr) {
    int compare = revision->compareTo(currentRevision->revision);
    if (compare == 0) {
      // same revision published again, ignore but release it
      revision->release();
      return;
    }

    if (compare < 0) {
      revision->release();
      throw invalid_argument(L"Cannot publish an older revision: rev=" +
                             revision + L" current=" + currentRevision);
    }
  }

  // swap revisions
  shared_ptr<RefCountedRevision> *const oldRevision = currentRevision;
  currentRevision = make_shared<RefCountedRevision>(revision);
  if (oldRevision != nullptr) {
    oldRevision->decRef();
  }

  // check for expired sessions
  checkExpiredSessions();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LocalReplicator::release(const wstring &sessionID) 
{
  ensureOpen();
  releaseSession(sessionID);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LocalReplicator::setExpirationThreshold(
    int64_t expirationThreshold) 
{
  ensureOpen();
  this->expirationThresholdMilllis = expirationThreshold;
  checkExpiredSessions();
}
} // namespace org::apache::lucene::replicator