using namespace std;

#include "SnapshotDeletionPolicy.h"

namespace org::apache::lucene::index
{
using Directory = org::apache::lucene::store::Directory;

SnapshotDeletionPolicy::SnapshotDeletionPolicy(
    shared_ptr<IndexDeletionPolicy> primary)
    : primary(primary)
{
}

template <typename T1>
// C++ WARNING: The following method was originally marked 'synchronized':
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public synchronized void
// onCommit(java.util.List<? extends IndexCommit> commits) throws
// java.io.IOException
void SnapshotDeletionPolicy::onCommit(deque<T1> commits) 
{
  primary->onCommit(wrapCommits(commits));
  lastCommit = commits[commits.size() - 1];
}

template <typename T1>
// C++ WARNING: The following method was originally marked 'synchronized':
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: @Override public synchronized void
// onInit(java.util.List<? extends IndexCommit> commits) throws
// java.io.IOException
void SnapshotDeletionPolicy::onInit(deque<T1> commits) 
{
  initCalled = true;
  primary->onInit(wrapCommits(commits));
  for (auto commit : commits) {
    if (refCounts.find(commit->getGeneration()) != refCounts.end()) {
      indexCommits.emplace(commit->getGeneration(), commit);
    }
  }
  if (!commits.empty()) {
    lastCommit = commits[commits.size() - 1];
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SnapshotDeletionPolicy::release(shared_ptr<IndexCommit> commit) throw(
    IOException)
{
  int64_t gen = commit->getGeneration();
  releaseGen(gen);
}

void SnapshotDeletionPolicy::releaseGen(int64_t gen) 
{
  if (!initCalled) {
    throw make_shared<IllegalStateException>(
        L"this instance is not being used by IndexWriter; be sure to use the "
        L"instance returned from writer.getConfig().getIndexDeletionPolicy()");
  }
  optional<int> refCount = refCounts[gen];
  if (!refCount) {
    throw invalid_argument(L"commit gen=" + to_wstring(gen) +
                           L" is not currently snapshotted");
  }
  int refCountInt = refCount.value();
  assert(refCountInt > 0);
  refCountInt--;
  if (refCountInt == 0) {
    refCounts.erase(gen);
    indexCommits.erase(gen);
  } else {
    refCounts.emplace(gen, refCountInt);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SnapshotDeletionPolicy::incRef(shared_ptr<IndexCommit> ic)
{
  int64_t gen = ic->getGeneration();
  optional<int> refCount = refCounts[gen];
  int refCountInt;
  if (!refCount) {
    indexCommits.emplace(gen, lastCommit);
    refCountInt = 0;
  } else {
    refCountInt = refCount.value();
  }
  refCounts.emplace(gen, refCountInt + 1);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<IndexCommit> SnapshotDeletionPolicy::snapshot() 
{
  if (!initCalled) {
    throw make_shared<IllegalStateException>(
        L"this instance is not being used by IndexWriter; be sure to use the "
        L"instance returned from writer.getConfig().getIndexDeletionPolicy()");
  }
  if (lastCommit == nullptr) {
    // No commit yet, eg this is a new IndexWriter:
    throw make_shared<IllegalStateException>(L"No index commit to snapshot");
  }

  incRef(lastCommit);

  return lastCommit;
}

// C++ WARNING: The following method was originally marked 'synchronized':
deque<std::shared_ptr<IndexCommit>> SnapshotDeletionPolicy::getSnapshots()
{
  return deque<>(indexCommits.values());
}

// C++ WARNING: The following method was originally marked 'synchronized':
int SnapshotDeletionPolicy::getSnapshotCount()
{
  int total = 0;
  for (shared_ptr<> : : optional<int> refCount : refCounts.values()) {
    total += refCount::intValue();
  }

  return total;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<IndexCommit> SnapshotDeletionPolicy::getIndexCommit(int64_t gen)
{
  return indexCommits[gen];
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: private java.util.List<IndexCommit>
// wrapCommits(java.util.List<? extends IndexCommit> commits)
deque<std::shared_ptr<IndexCommit>>
SnapshotDeletionPolicy::wrapCommits(deque<T1> commits)
{
  deque<std::shared_ptr<IndexCommit>> wrappedCommits =
      deque<std::shared_ptr<IndexCommit>>(commits.size());
  for (auto ic : commits) {
    wrappedCommits.push_back(
        make_shared<SnapshotCommitPoint>(shared_from_this(), ic));
  }
  return wrappedCommits;
}

SnapshotDeletionPolicy::SnapshotCommitPoint::SnapshotCommitPoint(
    shared_ptr<SnapshotDeletionPolicy> outerInstance,
    shared_ptr<IndexCommit> cp)
    : outerInstance(outerInstance)
{
  this->cp = cp;
}

wstring SnapshotDeletionPolicy::SnapshotCommitPoint::toString()
{
  return L"SnapshotDeletionPolicy.SnapshotCommitPoint(" + cp + L")";
}

void SnapshotDeletionPolicy::SnapshotCommitPoint::delete_()
{
  {
    lock_guard<mutex> lock(outerInstance);
    // Suppress the delete request if this commit point is
    // currently snapshotted.
    if (outerInstance->refCounts.find(cp->getGeneration()) ==
        outerInstance->refCounts.end()) {
      cp->delete ();
    }
  }
}

shared_ptr<Directory>
SnapshotDeletionPolicy::SnapshotCommitPoint::getDirectory()
{
  return cp->getDirectory();
}

shared_ptr<deque<wstring>>
SnapshotDeletionPolicy::SnapshotCommitPoint::getFileNames() 
{
  return cp->getFileNames();
}

int64_t SnapshotDeletionPolicy::SnapshotCommitPoint::getGeneration()
{
  return cp->getGeneration();
}

wstring SnapshotDeletionPolicy::SnapshotCommitPoint::getSegmentsFileName()
{
  return cp->getSegmentsFileName();
}

unordered_map<wstring, wstring>
SnapshotDeletionPolicy::SnapshotCommitPoint::getUserData() 
{
  return cp->getUserData();
}

bool SnapshotDeletionPolicy::SnapshotCommitPoint::isDeleted()
{
  return cp->isDeleted();
}

int SnapshotDeletionPolicy::SnapshotCommitPoint::getSegmentCount()
{
  return cp->getSegmentCount();
}
} // namespace org::apache::lucene::index