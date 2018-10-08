using namespace std;

#include "PrimaryNode.h"

namespace org::apache::lucene::replicator::nrt
{
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using StandardDirectoryReader =
    org::apache::lucene::index::StandardDirectoryReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using SearcherManager = org::apache::lucene::search::SearcherManager;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using RAMFile = org::apache::lucene::store::RAMFile;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

PrimaryNode::PrimaryNode(shared_ptr<IndexWriter> writer, int id,
                         int64_t primaryGen, int64_t forcePrimaryVersion,
                         shared_ptr<SearcherFactory> searcherFactory,
                         shared_ptr<PrintStream> printStream) 
    : Node(id, writer->getDirectory(), searcherFactory, printStream),
      writer(writer), primaryGen(primaryGen)
{
  message(L"top: now init primary");

  try {
    // So that when primary node's IndexWriter finishes a merge, but before it
    // cuts over to the merged segment, it copies it out to the replicas.  This
    // ensures the whole system's NRT latency remains low even when a large
    // merge completes:
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    writer->getConfig()->setMergedSegmentWarmer(
        make_shared<PreCopyMergedSegmentWarmer>(shared_from_this()));

    message(L"IWC:\n" + writer->getConfig());
    message(L"dir:\n" + writer->getDirectory());
    message(L"commitData: " + writer->getLiveCommitData());

    // Record our primaryGen in the userData, and set initial version to 0:
    unordered_map<wstring, wstring> commitData =
        unordered_map<wstring, wstring>();
    deque<unordered_map::Entry<wstring, wstring>> iter =
        writer->getLiveCommitData();
    if (iter.size() > 0) {
      for (auto ent : iter) {
        commitData.emplace(ent.first, ent.second);
      }
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    commitData.emplace(PRIMARY_GEN_KEY, Long::toString(primaryGen));
    if (commitData[VERSION_KEY] == L"") {
      commitData.emplace(VERSION_KEY, L"0");
      message(L"add initial commitData version=0");
    } else {
      message(L"keep current commitData version=" + commitData[VERSION_KEY]);
    }
    writer->setLiveCommitData(commitData.entrySet(), false);

    // We forcefully advance the SIS version to an unused future version.  This
    // is necessary if the previous primary crashed and we are starting up on an
    // "older" index, else versions can be illegally reused but show different
    // results:
    if (forcePrimaryVersion != -1) {
      message(L"now forcePrimaryVersion to version=" +
              to_wstring(forcePrimaryVersion));
      writer->advanceSegmentInfosVersion(forcePrimaryVersion);
    }

    mgr = make_shared<SearcherManager>(writer, true, true, searcherFactory);
    setCurrentInfos(Collections::emptySet<wstring>());
    message(L"init: infos version=" + to_wstring(curInfos->getVersion()));

  } catch (const runtime_error &t) {
    message(L"init: exception");
    t.printStackTrace(printStream);
    throw runtime_error(t);
  }
}

int64_t PrimaryNode::getPrimaryGen() { return primaryGen; }

bool PrimaryNode::flushAndRefresh() 
{
  message(L"top: now flushAndRefresh");
  shared_ptr<Set<wstring>> completedMergeFiles;
  {
    lock_guard<mutex> lock(finishedMergedFiles);
    completedMergeFiles =
        Collections::unmodifiableSet(unordered_set<>(finishedMergedFiles));
  }
  mgr->maybeRefreshBlocking();
  bool result = setCurrentInfos(completedMergeFiles);
  if (result) {
    message(L"top: opened NRT reader version=" +
            to_wstring(curInfos->getVersion()));
    finishedMergedFiles->removeAll(completedMergeFiles);
    message(L"flushAndRefresh: version=" + to_wstring(curInfos->getVersion()) +
            L" completedMergeFiles=" + completedMergeFiles +
            L" finishedMergedFiles=" + finishedMergedFiles);
  } else {
    message(L"top: no changes in flushAndRefresh; still version=" +
            to_wstring(curInfos->getVersion()));
  }
  return result;
}

int64_t PrimaryNode::getCopyStateVersion() { return copyState->version; }

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t PrimaryNode::getLastCommitVersion()
{
  deque<unordered_map::Entry<wstring, wstring>> iter =
      writer->getLiveCommitData();
  assert(iter.size() > 0);
  for (auto ent : iter) {
    if (ent.first::equals(VERSION_KEY)) {
      return static_cast<int64_t>(ent.second);
    }
  }

  // In ctor we always install an initial version:
  throw make_shared<AssertionError>(L"missing VERSION_KEY");
}

void PrimaryNode::commit() 
{
  unordered_map<wstring, wstring> commitData =
      unordered_map<wstring, wstring>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  commitData.emplace(PRIMARY_GEN_KEY, Long::toString(primaryGen));
  // TODO (opto): it's a bit wasteful that we put "last refresh" version here,
  // not the actual version we are committing, because it means on xlog replay
  // we are replaying more ops than necessary.
  // C++ TODO: There is no native C++ equivalent to 'toString':
  commitData.emplace(VERSION_KEY, Long::toString(copyState->version));
  message(L"top: commit commitData=" + commitData);
  writer->setLiveCommitData(commitData.entrySet(), false);
  writer->commit();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<CopyState> PrimaryNode::getCopyState() 
{
  ensureOpen(false);
  // message("top: getCopyState replicaID=" + replicaID + " replicaNodeID=" +
  // replicaNodeID + " version=" + curInfos.getVersion() + " infos=" +
  // curInfos.toString());
  assert(curInfos == copyState->infos);
  writer->incRefDeleter(copyState->infos);
  int count = copyingCount->incrementAndGet();
  assert(count > 0);
  return copyState;
}

void PrimaryNode::releaseCopyState(shared_ptr<CopyState> copyState) throw(
    IOException)
{
  // message("top: releaseCopyState version=" + copyState.version);
  assert(copyState->infos->size() > 0);
  writer->decRefDeleter(copyState->infos);
  int count = copyingCount->decrementAndGet();
  assert(count >= 0);
}

bool PrimaryNode::isClosed() { return isClosed(false); }

bool PrimaryNode::isClosed(bool allowClosing)
{
  return L"closed" == state || (allowClosing == false && L"closing" == state);
}

void PrimaryNode::ensureOpen(bool allowClosing)
{
  if (isClosed(allowClosing)) {
    throw make_shared<AlreadyClosedException>(state);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool PrimaryNode::setCurrentInfos(
    shared_ptr<Set<wstring>> completedMergeFiles) 
{

  shared_ptr<IndexSearcher> searcher = nullptr;
  shared_ptr<SegmentInfos> infos;
  try {
    searcher = mgr->acquire();
    infos = (std::static_pointer_cast<StandardDirectoryReader>(
                 searcher->getIndexReader()))
                ->getSegmentInfos();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (searcher != nullptr) {
      mgr->release(searcher);
    }
  }
  if (curInfos->size() > 0 && infos->getVersion() == curInfos->getVersion()) {
    // no change
    // C++ TODO: There is no native C++ equivalent to 'toString':
    message(L"top: skip switch to infos: version=" +
            to_wstring(infos->getVersion()) + L" is unchanged: " +
            infos->toString());
    return false;
  }

  shared_ptr<SegmentInfos> oldInfos = curInfos;
  writer->incRefDeleter(infos);
  curInfos = infos;
  if (oldInfos->size() > 0) {
    writer->decRefDeleter(oldInfos);
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  message(L"top: switch to infos=" + infos->toString() + L" version=" +
          to_wstring(infos->getVersion()));

  // Serialize the SegmentInfos:
  shared_ptr<RAMOutputStream> out =
      make_shared<RAMOutputStream>(make_shared<RAMFile>(), true);
  infos->write(dir, out);
  std::deque<char> infosBytes(static_cast<int>(out->getFilePointer()));
  out->writeTo(infosBytes, 0);

  unordered_map<wstring, std::shared_ptr<FileMetaData>> filesMetaData =
      unordered_map<wstring, std::shared_ptr<FileMetaData>>();
  for (auto info : infos) {
    for (auto fileName : info->files()) {
      shared_ptr<FileMetaData> metaData = readLocalFileMetaData(fileName);
      // NOTE: we hold a refCount on this infos, so this file better exist:
      assert(metaData != nullptr);
      assert(filesMetaData.find(fileName) != filesMetaData.end() == false);
      filesMetaData.emplace(fileName, metaData);
    }
  }

  lastFileMetaData = filesMetaData;

  message(L"top: set copyState primaryGen=" + to_wstring(primaryGen) +
          L" version=" + to_wstring(infos->getVersion()) + L" files=" +
          filesMetaData.keySet());
  copyState = make_shared<CopyState>(lastFileMetaData, infos->getVersion(),
                                     infos->getGeneration(), infosBytes,
                                     completedMergeFiles, primaryGen, curInfos);
  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void PrimaryNode::waitForAllRemotesToClose() 
{

  // Wait for replicas to finish or crash:
  while (true) {
    int count = copyingCount->get();
    if (count == 0) {
      return;
    }
    message(L"pendingCopies: " + to_wstring(count));

    try {
      wait(10);
    } catch (const InterruptedException &ie) {
      throw make_shared<ThreadInterruptedException>(ie);
    }
  }
}

PrimaryNode::~PrimaryNode()
{
  state = L"closing";
  message(L"top: close primary");

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    waitForAllRemotesToClose();
    if (curInfos->size() > 0) {
      writer->decRefDeleter(curInfos);
      curInfos.reset();
    }
  }

  delete mgr;

  writer->rollback();
  delete dir;

  state = L"closed";
}
} // namespace org::apache::lucene::replicator::nrt