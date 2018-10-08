using namespace std;

#include "ReplicaFileDeleter.h"

namespace org::apache::lucene::replicator::nrt
{
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

ReplicaFileDeleter::ReplicaFileDeleter(
    shared_ptr<Node> node, shared_ptr<Directory> dir) 
    : dir(dir), node(node)
{
}

bool ReplicaFileDeleter::slowFileExists(
    shared_ptr<Directory> dir, const wstring &fileName) 
{
  try {
    delete dir->openInput(fileName, IOContext::DEFAULT);
    return true;
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NoSuchFileException | FileNotFoundException e) {
    return false;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaFileDeleter::incRef(
    shared_ptr<deque<wstring>> fileNames) 
{
  for (auto fileName : fileNames) {

    assert((slowFileExists(dir, fileName),
            L"file " + fileName + L" does not exist!"));

    optional<int> curCount = refCounts[fileName];
    if (!curCount) {
      refCounts.emplace(fileName, 1);
    } else {
      refCounts.emplace(fileName, curCount.value() + 1);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaFileDeleter::decRef(
    shared_ptr<deque<wstring>> fileNames) 
{
  shared_ptr<Set<wstring>> toDelete = unordered_set<wstring>();
  for (auto fileName : fileNames) {
    optional<int> curCount = refCounts[fileName];
    assert((curCount, L"fileName=" + fileName));
    assert(curCount.value() > 0);
    if (curCount.value() == 1) {
      refCounts.erase(fileName);
      toDelete->add(fileName);
    } else {
      refCounts.emplace(fileName, curCount.value() - 1);
    }
  }

  delete (toDelete);

  // TODO: this local IR could incRef files here, like we do now with IW's NRT
  // readers ... then we can assert this again:

  // we can't assert this, e.g a search can be running when we switch to a new
  // NRT point, holding a previous IndexReader still open for a bit:
  /*
  // We should never attempt deletion of a still-open file:
  Set<std::wstring> delOpen = ((MockDirectoryWrapper) dir).getOpenDeletedFiles();
  if (delOpen.isEmpty() == false) {
    node.message("fail: we tried to delete these still-open files: " + delOpen);
    throw new AssertionError("we tried to delete these still-open files: " +
  delOpen);
  }
  */
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaFileDeleter::delete_(
    shared_ptr<deque<wstring>> toDelete) 
{
  if (Node::VERBOSE_FILES) {
    node->message(L"now delete " + toDelete->size() + L" files: " + toDelete);
  }

  // First pass: delete any segments_N files.  We do these first to be certain
  // stale commit points are removed before we remove any files they reference,
  // in case we crash right now:
  for (auto fileName : toDelete) {
    assert(refCounts.find(fileName) != refCounts.end() == false);
    if (fileName.startsWith(IndexFileNames::SEGMENTS)) {
      delete (fileName);
    }
  }

  // Only delete other files if we were able to remove the segments_N files;
  // this way we never leave a corrupt commit in the index even in the presense
  // of virus checkers:
  for (auto fileName : toDelete) {
    assert(refCounts.find(fileName) != refCounts.end() == false);
    if (fileName.startsWith(IndexFileNames::SEGMENTS) == false) {
      delete (fileName);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaFileDeleter::delete_(const wstring &fileName) 
{
  if (Node::VERBOSE_FILES) {
    node->message(L"file " + fileName + L": now delete");
  }
  dir->deleteFile(fileName);
}

// C++ WARNING: The following method was originally marked 'synchronized':
optional<int> ReplicaFileDeleter::getRefCount(const wstring &fileName)
{
  return refCounts[fileName];
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaFileDeleter::deleteIfNoRef(const wstring &fileName) throw(
    IOException)
{
  if (refCounts.find(fileName) != refCounts.end() == false) {
    deleteNewFile(fileName);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaFileDeleter::deleteNewFile(const wstring &fileName) throw(
    IOException)
{
  delete (fileName);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaFileDeleter::deleteUnknownFiles(
    const wstring &segmentsFileName) 
{
  shared_ptr<Set<wstring>> toDelete = unordered_set<wstring>();
  for (auto fileName : dir->listAll()) {
    if (refCounts.find(fileName) != refCounts.end() == false &&
        fileName.equals(L"write.lock") == false &&
        fileName.equals(segmentsFileName) == false) {
      node->message(L"will delete unknown file \"" + fileName + L"\"");
      toDelete->add(fileName);
    }
  }

  delete (toDelete);
}
} // namespace org::apache::lucene::replicator::nrt