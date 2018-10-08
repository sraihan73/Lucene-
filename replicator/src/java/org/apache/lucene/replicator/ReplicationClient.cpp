using namespace std;

#include "ReplicationClient.h"

namespace org::apache::lucene::replicator
{
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

ReplicationClient::ReplicationThread::ReplicationThread(
    shared_ptr<ReplicationClient> outerInstance, int64_t interval)
    : interval(interval), outerInstance(outerInstance)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override public void
// run()
void ReplicationClient::ReplicationThread::run()
{
  while (true) {
    int64_t time = System::currentTimeMillis();
    outerInstance->updateLock->lock();
    try {
      outerInstance->doUpdate();
    } catch (const runtime_error &t) {
      outerInstance->handleUpdateException(t);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      outerInstance->updateLock->unlock();
    }
    time = System::currentTimeMillis() - time;

    // adjust timeout to compensate the time spent doing the replication.
    constexpr int64_t timeout = interval - time;
    if (timeout > 0) {
      try {
        // this will return immediately if we were ordered to stop (count=0)
        // or the timeout has elapsed. if it returns true, it means count=0,
        // so terminate.
        if (stop->await(timeout, TimeUnit::MILLISECONDS)) {
          return;
        }
      } catch (const InterruptedException &e) {
        // if we were interruted, somebody wants to terminate us, so just
        // throw the exception further.
        Thread::currentThread().interrupt();
        throw make_shared<ThreadInterruptedException>(e);
      }
    }
  }
}

const wstring ReplicationClient::INFO_STREAM_COMPONENT = L"ReplicationThread";

ReplicationClient::ReplicationClient(shared_ptr<Replicator> replicator,
                                     shared_ptr<ReplicationHandler> handler,
                                     shared_ptr<SourceDirectoryFactory> factory)
    : replicator(replicator), handler(handler), factory(factory)
{
}

void ReplicationClient::copyBytes(
    shared_ptr<IndexOutput> out, shared_ptr<InputStream> in_) 
{
  int numBytes;
  while ((numBytes = in_->read(copyBuffer)) > 0) {
    out->writeBytes(copyBuffer, 0, numBytes);
  }
}

void ReplicationClient::doUpdate() 
{
  shared_ptr<SessionToken> session = nullptr;
  const unordered_map<wstring, std::shared_ptr<Directory>> sourceDirectory =
      unordered_map<wstring, std::shared_ptr<Directory>>();
  const unordered_map<wstring, deque<wstring>> copiedFiles =
      unordered_map<wstring, deque<wstring>>();
  bool notify = false;
  try {
    const wstring version = handler->currentVersion();
    session = replicator->checkForUpdate(version);
    if (infoStream->isEnabled(INFO_STREAM_COMPONENT)) {
      infoStream->message(INFO_STREAM_COMPONENT,
                          L"doUpdate(): handlerVersion=" + version +
                              L" session=" + session);
    }
    if (session == nullptr) {
      // already up to date
      return;
    }
    unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
        requiredFiles = this->requiredFiles(session->sourceFiles);
    if (infoStream->isEnabled(INFO_STREAM_COMPONENT)) {
      infoStream->message(INFO_STREAM_COMPONENT,
                          L"doUpdate(): requiredFiles=" + requiredFiles);
    }
    for (auto e : requiredFiles) {
      wstring source = e.first;
      shared_ptr<Directory> dir = factory->getDirectory(session->id, source);
      sourceDirectory.emplace(source, dir);
      deque<wstring> cpFiles = deque<wstring>();
      copiedFiles.emplace(source, cpFiles);
      for (shared_ptr<RevisionFile> file : e.second) {
        if (closed) {
          // if we're closed, abort file copy
          if (infoStream->isEnabled(INFO_STREAM_COMPONENT)) {
            infoStream->message(
                INFO_STREAM_COMPONENT,
                L"doUpdate(): detected client was closed); abort file copy");
          }
          return;
        }
        shared_ptr<InputStream> in_ = nullptr;
        shared_ptr<IndexOutput> out = nullptr;
        try {
          in_ = replicator->obtainFile(session->id, source, file->fileName);
          out = dir->createOutput(file->fileName, IOContext::DEFAULT);
          copyBytes(out, in_);
          cpFiles.push_back(file->fileName);
          // TODO add some validation, on size / checksum
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          IOUtils::close({in_, out});
        }
      }
    }
    // only notify if all required files were successfully obtained.
    notify = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (session != nullptr) {
      try {
        replicator->release(session->id);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (!notify) { // cleanup after ourselves
          IOUtils::close({sourceDirectory.values()});
          factory->cleanupSession(session->id);
        }
      }
    }
  }

  // notify outside the try-finally above, so the session is released sooner.
  // the handler may take time to finish acting on the copied files, but the
  // session itself is no longer needed.
  try {
    if (notify && !closed) { // no use to notify if we are closed already
      handler->revisionReady(session->version, session->sourceFiles,
                             copiedFiles, sourceDirectory);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({sourceDirectory.values()});
    if (session != nullptr) {
      factory->cleanupSession(session->id);
    }
  }
}

void ReplicationClient::ensureOpen()
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(
        L"this update client has already been closed");
  }
}

void ReplicationClient::handleUpdateException(runtime_error t)
{
  shared_ptr<StringWriter> *const sw = make_shared<StringWriter>();
  t.printStackTrace(make_shared<PrintWriter>(sw));
  if (infoStream->isEnabled(INFO_STREAM_COMPONENT)) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    infoStream->message(INFO_STREAM_COMPONENT,
                        L"an error occurred during revision update: " +
                            sw->toString());
  }
}

unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
ReplicationClient::requiredFiles(
    unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
        &newRevisionFiles)
{
  unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
      handlerRevisionFiles = handler->currentRevisionFiles();
  if (handlerRevisionFiles.empty()) {
    return newRevisionFiles;
  }

  unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>> requiredFiles =
      unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>();
  for (auto e : handlerRevisionFiles) {
    // put the handler files in a Set, for faster contains() checks later
    shared_ptr<Set<wstring>> handlerFiles = unordered_set<wstring>();
    for (shared_ptr<RevisionFile> file : e.second) {
      handlerFiles->add(file->fileName);
    }

    // make sure to preserve revisionFiles order
    deque<std::shared_ptr<RevisionFile>> res =
        deque<std::shared_ptr<RevisionFile>>();
    wstring source = e.first;
    assert((newRevisionFiles.find(source) != newRevisionFiles.end(),
            L"source not found in newRevisionFiles: " + newRevisionFiles));
    for (auto file : newRevisionFiles[source]) {
      if (!handlerFiles->contains(file->fileName)) {
        res.push_back(file);
      }
    }
    requiredFiles.emplace(source, res);
  }

  return requiredFiles;
}

// C++ WARNING: The following method was originally marked 'synchronized':
ReplicationClient::~ReplicationClient()
{
  if (!closed) {
    stopUpdateThread();
    closed = true;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicationClient::startUpdateThread(int64_t intervalMillis,
                                          const wstring &threadName)
{
  ensureOpen();
  if (updateThread != nullptr && updateThread->isAlive()) {
    throw make_shared<IllegalStateException>(
        L"cannot start an update thread when one is running, must first call "
        L"'stopUpdateThread()'");
  }
  threadName = threadName == L"" ? INFO_STREAM_COMPONENT
                                 : L"ReplicationThread-" + threadName;
  updateThread =
      make_shared<ReplicationThread>(shared_from_this(), intervalMillis);
  updateThread->setName(threadName);
  updateThread->start();
  // we rely on isAlive to return true in isUpdateThreadAlive, assert to be on
  // the safe side
  assert((updateThread->isAlive(), L"updateThread started but not alive?"));
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicationClient::stopUpdateThread()
{
  if (updateThread != nullptr) {
    // this will trigger the thread to terminate if it awaits the lock.
    // otherwise, if it's in the middle of replication, we wait for it to
    // stop.
    updateThread->stop->countDown();
    try {
      updateThread->join();
    } catch (const InterruptedException &e) {
      Thread::currentThread().interrupt();
      throw make_shared<ThreadInterruptedException>(e);
    }
    updateThread.reset();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReplicationClient::isUpdateThreadAlive()
{
  return updateThread != nullptr && updateThread->isAlive();
}

wstring ReplicationClient::toString()
{
  wstring res = L"ReplicationClient";
  if (updateThread != nullptr) {
    res += L" (" + updateThread->getName() + L")";
  }
  return res;
}

void ReplicationClient::updateNow() 
{
  ensureOpen();
  updateLock->lock();
  try {
    doUpdate();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    updateLock->unlock();
  }
}

void ReplicationClient::setInfoStream(shared_ptr<InfoStream> infoStream)
{
  if (infoStream == nullptr) {
    infoStream = InfoStream::NO_OUTPUT;
  }
  this->infoStream = infoStream;
}
} // namespace org::apache::lucene::replicator