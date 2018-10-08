using namespace std;

#include "IndexRevision.h"

namespace org::apache::lucene::replicator
{
using IndexCommit = org::apache::lucene::index::IndexCommit;
using IndexDeletionPolicy = org::apache::lucene::index::IndexDeletionPolicy;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
const wstring IndexRevision::SOURCE = L"index";

shared_ptr<RevisionFile>
IndexRevision::newRevisionFile(const wstring &file,
                               shared_ptr<Directory> dir) 
{
  shared_ptr<RevisionFile> revFile = make_shared<RevisionFile>(file);
  revFile->size = dir->fileLength(file);
  return revFile;
}

unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
IndexRevision::revisionFiles(shared_ptr<IndexCommit> commit) 
{
  shared_ptr<deque<wstring>> commitFiles = commit->getFileNames();
  deque<std::shared_ptr<RevisionFile>> revisionFiles =
      deque<std::shared_ptr<RevisionFile>>(commitFiles->size());
  wstring segmentsFile = commit->getSegmentsFileName();
  shared_ptr<Directory> dir = commit->getDirectory();

  for (auto file : commitFiles) {
    if (!file.equals(segmentsFile)) {
      revisionFiles.push_back(newRevisionFile(file, dir));
    }
  }
  revisionFiles.push_back(
      newRevisionFile(segmentsFile, dir)); // segments_N must be last
  return Collections::singletonMap(SOURCE, revisionFiles);
}

wstring IndexRevision::revisionVersion(shared_ptr<IndexCommit> commit)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return Long::toString(commit->getGeneration(), RADIX);
}

IndexRevision::IndexRevision(shared_ptr<IndexWriter> writer) 
    : writer(writer), commit(sdp->snapshot()),
      sdp(std::static_pointer_cast<SnapshotDeletionPolicy>(delPolicy)),
      version(revisionVersion(commit)), sourceFiles(revisionFiles(commit))
{
  shared_ptr<IndexDeletionPolicy> delPolicy =
      writer->getConfig()->getIndexDeletionPolicy();
  if (!(std::dynamic_pointer_cast<SnapshotDeletionPolicy>(delPolicy) !=
        nullptr)) {
    throw invalid_argument(
        L"IndexWriter must be created with SnapshotDeletionPolicy");
  }
}

int IndexRevision::compareTo(const wstring &version)
{
  // C++ TODO: Only single-argument parse and valueOf methods are converted:
  // ORIGINAL LINE: long gen = Long.parseLong(version, RADIX);
  int64_t gen = int64_t ::valueOf(version, RADIX);
  int64_t commitGen = commit->getGeneration();
  return commitGen < gen ? -1 : (commitGen > gen ? 1 : 0);
}

int IndexRevision::compareTo(shared_ptr<Revision> o)
{
  shared_ptr<IndexRevision> other = std::static_pointer_cast<IndexRevision>(o);
  return commit->compareTo(other->commit);
}

wstring IndexRevision::getVersion() { return version; }

unordered_map<wstring, deque<std::shared_ptr<RevisionFile>>>
IndexRevision::getSourceFiles()
{
  return sourceFiles;
}

shared_ptr<InputStream>
IndexRevision::open(const wstring &source,
                    const wstring &fileName) 
{
  assert((source == SOURCE,
          L"invalid source; expected=" + SOURCE + L" got=" + source));
  return make_shared<IndexInputInputStream>(
      commit->getDirectory()->openInput(fileName, IOContext::READONCE));
}

void IndexRevision::release() 
{
  sdp->release(commit);
  writer->deleteUnusedFiles();
}

wstring IndexRevision::toString()
{
  return L"IndexRevision version=" + version + L" files=" + sourceFiles;
}
} // namespace org::apache::lucene::replicator