using namespace std;

#include "IndexReplicationHandler.h"

namespace org::apache::lucene::replicator
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexCommit = org::apache::lucene::index::IndexCommit;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexNotFoundException =
    org::apache::lucene::index::IndexNotFoundException;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using ReplicationHandler =
    org::apache::lucene::replicator::ReplicationClient::ReplicationHandler;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
const wstring IndexReplicationHandler::INFO_STREAM_COMPONENT =
    L"IndexReplicationHandler";

shared_ptr<IndexCommit> IndexReplicationHandler::getLastCommit(
    shared_ptr<Directory> dir) 
{
  try {
    if (DirectoryReader::indexExists(dir)) {
      deque<std::shared_ptr<IndexCommit>> commits =
          DirectoryReader::listCommits(dir);
      // listCommits guarantees that we get at least one commit back, or
      // IndexNotFoundException which we handle below
      return commits[commits.size() - 1];
    }
  } catch (const IndexNotFoundException &e) {
    // ignore the exception and return null
  }
  return nullptr;
}

wstring IndexReplicationHandler::getSegmentsFile(deque<wstring> &files,
                                                 bool allowEmpty)
{
  if (files.empty()) {
    if (allowEmpty) {
      return L"";
    } else {
      throw make_shared<IllegalStateException>(
          L"empty deque of files not allowed");
    }
  }

  wstring segmentsFile = files.pop_back();
  if (!StringHelper::startsWith(segmentsFile, IndexFileNames::SEGMENTS) ||
      segmentsFile == IndexFileNames::OLD_SEGMENTS_GEN) {
    throw make_shared<IllegalStateException>(
        L"last file to copy+sync must be segments_N but got " + segmentsFile +
        L"; check your Revision implementation!");
  }
  return segmentsFile;
}

void IndexReplicationHandler::cleanupFilesOnFailure(shared_ptr<Directory> dir,
                                                    deque<wstring> &files)
{
  for (auto file : files) {
    // suppress any exception because if we're here, it means copy
    // failed, and we must cleanup after ourselves.
    IOUtils::deleteFilesIgnoringExceptions(dir, {file});
  }
}

void IndexReplicationHandler::cleanupOldIndexFiles(
    shared_ptr<Directory> dir, const wstring &segmentsFile,
    shared_ptr<InfoStream> infoStream)
{
  try {
    shared_ptr<IndexCommit> commit = getLastCommit(dir);
    // commit == null means weird IO errors occurred, ignore them
    // if there were any IO errors reading the expected commit point (i.e.
    // segments files mismatch), then ignore that commit either.
    if (commit != nullptr && commit->getSegmentsFileName() == segmentsFile) {
      shared_ptr<Set<wstring>> commitFiles = unordered_set<wstring>();
      commitFiles->addAll(commit->getFileNames());
      shared_ptr<Matcher> matcher =
          IndexFileNames::CODEC_FILE_PATTERN->matcher(L"");
      for (auto file : dir->listAll()) {
        if (!commitFiles->contains(file) &&
            (matcher->reset(file).matches() ||
             file.startsWith(IndexFileNames::SEGMENTS))) {
          // suppress exceptions, it's just a best effort
          IOUtils::deleteFilesIgnoringExceptions(dir, {file});
        }
      }
    }
  } catch (const runtime_error &t) {
    // ignore any errors that happen during this state and only log it. this
    // cleanup will have a chance to succeed the next time we get a new
    // revision.
    if (infoStream->isEnabled(INFO_STREAM_COMPONENT)) {
      infoStream->message(INFO_STREAM_COMPONENT,
                          L"cleanupOldIndexFiles(): failed on error " +
                              t.what());
    }
  }
}

void IndexReplicationHandler::copyFiles(
    shared_ptr<Directory> source, shared_ptr<Directory> target,
    deque<wstring> &files) 
{
  if (!source->equals(target)) {
    for (auto file : files) {
      target->copyFrom(source, file, file, IOContext::READONCE);
    }
  }
}

IndexReplicationHandler::IndexReplicationHandler(
    shared_ptr<Directory> indexDir,
    shared_ptr<Callable<bool>> callback) 
    : indexDir(indexDir), callback(callback)
{
  currentRevisionFiles_.clear();
  currentVersion_ = L"";
  if (DirectoryReader::indexExists(indexDir)) {
    const deque<std::shared_ptr<IndexCommit>> commits =
        DirectoryReader::listCommits(indexDir);
    shared_ptr<IndexCommit> *const commit = commits[commits.size() - 1];
    currentRevisionFiles_ = IndexRevision::revisionFiles(commit);
    currentVersion_ = IndexRevision::revisionVersion(commit);
    shared_ptr<InfoStream> *const infoStream = InfoStream::getDefault();
    if (infoStream->isEnabled(INFO_STREAM_COMPONENT)) {
      infoStream->message(INFO_STREAM_COMPONENT,
                          L"constructor(): currentVersion=" + currentVersion_ +
                              L" currentRevisionFiles=" +
                              currentRevisionFiles_);
      infoStream->message(INFO_STREAM_COMPONENT,
                          L"constructor(): commit=" + commit);
    }
  }
}

wstring IndexReplicationHandler::currentVersion() { return currentVersion_; }

unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
IndexReplicationHandler::currentRevisionFiles()
{
  return currentRevisionFiles_;
}

void IndexReplicationHandler::revisionReady(
    const wstring &version,
    unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
        &revisionFiles,
    unordered_map<wstring, deque<wstring>> &copiedFiles,
    unordered_map<wstring, std::shared_ptr<Directory>>
        &sourceDirectory) 
{
  if (revisionFiles.size() > 1) {
    throw invalid_argument(L"this handler handles only a single source; got " +
                           revisionFiles.keySet());
  }

  shared_ptr<Directory> clientDir = sourceDirectory.values().begin()->next();
  deque<wstring> files = copiedFiles.values().begin()->next();
  wstring segmentsFile = getSegmentsFile(files, false);
  wstring pendingSegmentsFile = L"pending_" + segmentsFile;

  bool success = false;
  try {
    // copy files from the client to index directory
    copyFiles(clientDir, indexDir, files);

    // fsync all copied files (except segmentsFile)
    indexDir->sync(files);

    // now copy and fsync segmentsFile as pending, then rename (simulating
    // lucene commit)
    indexDir->copyFrom(clientDir, segmentsFile, pendingSegmentsFile,
                       IOContext::READONCE);
    indexDir->sync(Collections::singletonList(pendingSegmentsFile));
    indexDir->rename(pendingSegmentsFile, segmentsFile);
    indexDir->syncMetaData();

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      files.push_back(segmentsFile); // add it back so it gets deleted too
      files.push_back(pendingSegmentsFile);
      cleanupFilesOnFailure(indexDir, files);
    }
  }

  // all files have been successfully copied + sync'd. update the handler's
  // state
  currentRevisionFiles_ = revisionFiles;
  currentVersion_ = version;

  if (infoStream->isEnabled(INFO_STREAM_COMPONENT)) {
    infoStream->message(INFO_STREAM_COMPONENT,
                        L"revisionReady(): currentVersion=" + currentVersion_ +
                            L" currentRevisionFiles=" + currentRevisionFiles_);
  }

  // Cleanup the index directory from old and unused index files.
  // NOTE: we don't use IndexWriter.deleteUnusedFiles here since it may have
  // side-effects, e.g. if it hits sudden IO errors while opening the index
  // (and can end up deleting the entire index). It is not our job to protect
  // against those errors, app will probably hit them elsewhere.
  cleanupOldIndexFiles(indexDir, segmentsFile, infoStream);

  // successfully updated the index, notify the callback that the index is
  // ready.
  if (callback != nullptr) {
    try {
      callback->call();
    } catch (const runtime_error &e) {
      throw make_shared<IOException>(e);
    }
  }
}

void IndexReplicationHandler::setInfoStream(shared_ptr<InfoStream> infoStream)
{
  if (infoStream == nullptr) {
    infoStream = InfoStream::NO_OUTPUT;
  }
  this->infoStream = infoStream;
}
} // namespace org::apache::lucene::replicator