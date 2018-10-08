using namespace std;

#include "IndexAndTaxonomyReplicationHandler.h"

namespace org::apache::lucene::replicator
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexCommit = org::apache::lucene::index::IndexCommit;
using ReplicationHandler =
    org::apache::lucene::replicator::ReplicationClient::ReplicationHandler;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using InfoStream = org::apache::lucene::util::InfoStream;
const wstring IndexAndTaxonomyReplicationHandler::INFO_STREAM_COMPONENT =
    L"IndexAndTaxonomyReplicationHandler";

IndexAndTaxonomyReplicationHandler::IndexAndTaxonomyReplicationHandler(
    shared_ptr<Directory> indexDir, shared_ptr<Directory> taxoDir,
    shared_ptr<Callable<bool>> callback) 
    : indexDir(indexDir), taxoDir(taxoDir), callback(callback)
{
  currentRevisionFiles_.clear();
  currentVersion_ = L"";
  constexpr bool indexExists = DirectoryReader::indexExists(indexDir);
  constexpr bool taxoExists = DirectoryReader::indexExists(taxoDir);
  if (indexExists != taxoExists) {
    throw make_shared<IllegalStateException>(
        L"search and taxonomy indexes must either both exist or not: index=" +
        StringHelper::toString(indexExists) + L" taxo=" +
        StringHelper::toString(taxoExists));
  }
  if (indexExists) { // both indexes exist
    shared_ptr<IndexCommit> *const indexCommit =
        IndexReplicationHandler::getLastCommit(indexDir);
    shared_ptr<IndexCommit> *const taxoCommit =
        IndexReplicationHandler::getLastCommit(taxoDir);
    currentRevisionFiles_ =
        IndexAndTaxonomyRevision::revisionFiles(indexCommit, taxoCommit);
    currentVersion_ =
        IndexAndTaxonomyRevision::revisionVersion(indexCommit, taxoCommit);
    shared_ptr<InfoStream> *const infoStream = InfoStream::getDefault();
    if (infoStream->isEnabled(INFO_STREAM_COMPONENT)) {
      infoStream->message(INFO_STREAM_COMPONENT,
                          L"constructor(): currentVersion=" + currentVersion_ +
                              L" currentRevisionFiles=" +
                              currentRevisionFiles_);
      infoStream->message(INFO_STREAM_COMPONENT,
                          L"constructor(): indexCommit=" + indexCommit +
                              L" taxoCommit=" + taxoCommit);
    }
  }
}

wstring IndexAndTaxonomyReplicationHandler::currentVersion()
{
  return currentVersion_;
}

unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
IndexAndTaxonomyReplicationHandler::currentRevisionFiles()
{
  return currentRevisionFiles_;
}

void IndexAndTaxonomyReplicationHandler::revisionReady(
    const wstring &version,
    unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
        &revisionFiles,
    unordered_map<wstring, deque<wstring>> &copiedFiles,
    unordered_map<wstring, std::shared_ptr<Directory>>
        &sourceDirectory) 
{
  shared_ptr<Directory> taxoClientDir =
      sourceDirectory[IndexAndTaxonomyRevision::TAXONOMY_SOURCE];
  shared_ptr<Directory> indexClientDir =
      sourceDirectory[IndexAndTaxonomyRevision::INDEX_SOURCE];
  deque<wstring> taxoFiles =
      copiedFiles[IndexAndTaxonomyRevision::TAXONOMY_SOURCE];
  deque<wstring> indexFiles =
      copiedFiles[IndexAndTaxonomyRevision::INDEX_SOURCE];
  wstring taxoSegmentsFile =
      IndexReplicationHandler::getSegmentsFile(taxoFiles, true);
  wstring indexSegmentsFile =
      IndexReplicationHandler::getSegmentsFile(indexFiles, false);
  wstring taxoPendingFile =
      taxoSegmentsFile == L"" ? L"" : L"pending_" + taxoSegmentsFile;
  wstring indexPendingFile = L"pending_" + indexSegmentsFile;

  bool success = false;
  try {
    // copy taxonomy files before index files
    IndexReplicationHandler::copyFiles(taxoClientDir, taxoDir, taxoFiles);
    IndexReplicationHandler::copyFiles(indexClientDir, indexDir, indexFiles);

    // fsync all copied files (except segmentsFile)
    if (!taxoFiles.empty()) {
      taxoDir->sync(taxoFiles);
    }
    indexDir->sync(indexFiles);

    // now copy, fsync, and rename segmentsFile, taxonomy first because it is ok
    // if a reader sees a more advanced taxonomy than the index.

    if (taxoSegmentsFile != L"") {
      taxoDir->copyFrom(taxoClientDir, taxoSegmentsFile, taxoPendingFile,
                        IOContext::READONCE);
    }
    indexDir->copyFrom(indexClientDir, indexSegmentsFile, indexPendingFile,
                       IOContext::READONCE);

    if (taxoSegmentsFile != L"") {
      taxoDir->sync(Collections::singletonList(taxoPendingFile));
    }
    indexDir->sync(Collections::singletonList(indexPendingFile));

    if (taxoSegmentsFile != L"") {
      taxoDir->rename(taxoPendingFile, taxoSegmentsFile);
      taxoDir->syncMetaData();
    }

    indexDir->rename(indexPendingFile, indexSegmentsFile);
    indexDir->syncMetaData();

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      if (taxoSegmentsFile != L"") {
        taxoFiles.push_back(
            taxoSegmentsFile); // add it back so it gets deleted too
        taxoFiles.push_back(taxoPendingFile);
      }
      IndexReplicationHandler::cleanupFilesOnFailure(taxoDir, taxoFiles);
      indexFiles.push_back(
          indexSegmentsFile); // add it back so it gets deleted too
      indexFiles.push_back(indexPendingFile);
      IndexReplicationHandler::cleanupFilesOnFailure(indexDir, indexFiles);
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
  IndexReplicationHandler::cleanupOldIndexFiles(indexDir, indexSegmentsFile,
                                                infoStream);
  IndexReplicationHandler::cleanupOldIndexFiles(taxoDir, taxoSegmentsFile,
                                                infoStream);

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

void IndexAndTaxonomyReplicationHandler::setInfoStream(
    shared_ptr<InfoStream> infoStream)
{
  if (infoStream == nullptr) {
    infoStream = InfoStream::NO_OUTPUT;
  }
  this->infoStream = infoStream;
}
} // namespace org::apache::lucene::replicator