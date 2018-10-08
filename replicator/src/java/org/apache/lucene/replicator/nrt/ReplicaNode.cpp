using namespace std;

#include "ReplicaNode.h"

namespace org::apache::lucene::replicator::nrt
{
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using TermQuery = org::apache::lucene::search::TermQuery;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using BufferedChecksumIndexInput =
    org::apache::lucene::store::BufferedChecksumIndexInput;
using ByteArrayIndexInput = org::apache::lucene::store::ByteArrayIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Lock = org::apache::lucene::store::Lock;
using IOUtils = org::apache::lucene::util::IOUtils;
using Version = org::apache::lucene::util::Version;

ReplicaNode::ReplicaNode(int id, shared_ptr<Directory> dir,
                         shared_ptr<SearcherFactory> searcherFactory,
                         shared_ptr<PrintStream> printStream) 
    : Node(id, dir, searcherFactory, printStream)
{

  if (dir->getPendingDeletions()->isEmpty() == false) {
    throw invalid_argument(
        L"Directory " + dir +
        L" still has pending deleted files; cannot initialize IndexWriter");
  }

  bool success = false;

  try {
    message(L"top: init replica dir=" + dir);

    // Obtain a write lock on this index since we "act like" an IndexWriter, to
    // prevent any other IndexWriter or ReplicaNode from using it:
    writeFileLock = dir->obtainLock(IndexWriter::WRITE_LOCK_NAME);

    state = L"init";
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    deleter = make_shared<ReplicaFileDeleter>(shared_from_this(), dir);
    success = true;
  } catch (const runtime_error &t) {
    message(L"exc on init:");
    t.printStackTrace(printStream);
    throw t;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      IOUtils::closeWhileHandlingException({shared_from_this()});
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaNode::start(int64_t curPrimaryGen) 
{

  if (state == L"init" == false) {
    throw make_shared<IllegalStateException>(L"already started");
  }

  message(L"top: now start");
  try {

    // Figure out what state our local index is in now:
    wstring segmentsFileName = SegmentInfos::getLastCommitSegmentsFileName(dir);

    // Also look for any pending_segments_N, in case we crashed mid-commit.  We
    // must "inflate" our infos gen to at least this, since otherwise we may
    // wind up re-using the pending_segments_N file name on commit, and then our
    // deleter can get angry because it still wants to delete this file:
    int64_t maxPendingGen = -1;
    for (auto fileName : dir->listAll()) {
      if (fileName.startsWith(IndexFileNames::PENDING_SEGMENTS)) {
        // C++ TODO: Only single-argument parse and valueOf methods are
        // converted: ORIGINAL LINE: long gen =
        // Long.parseLong(fileName.substring(org.apache.lucene.index.IndexFileNames.PENDING_SEGMENTS.length()+1),
        // Character.MAX_RADIX);
        int64_t gen = int64_t ::valueOf(
            fileName.substr(IndexFileNames::PENDING_SEGMENTS.length() + 1),
            Character::MAX_RADIX);
        if (gen > maxPendingGen) {
          maxPendingGen = gen;
        }
      }
    }

    shared_ptr<SegmentInfos> infos;
    if (segmentsFileName == L"") {
      // No index here yet:
      infos = make_shared<SegmentInfos>(Version::LATEST->major);
      message(L"top: init: no segments in index");
    } else {
      message(L"top: init: read existing segments commit " + segmentsFileName);
      infos = SegmentInfos::readCommit(dir, segmentsFileName);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      message(L"top: init: segments: " + infos->toString() + L" version=" +
              to_wstring(infos->getVersion()));
      shared_ptr<deque<wstring>> indexFiles = infos->files(false);

      lastCommitFiles->add(segmentsFileName);
      lastCommitFiles->addAll(indexFiles);

      // Always protect the last commit:
      deleter->incRef(lastCommitFiles);

      lastNRTFiles->addAll(indexFiles);
      deleter->incRef(lastNRTFiles);
      message(L"top: commitFiles=" + lastCommitFiles);
      message(L"top: nrtFiles=" + lastNRTFiles);
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    message(L"top: delete unknown files on init: all files=" +
            Arrays->toString(dir->listAll()));
    deleter->deleteUnknownFiles(segmentsFileName);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    message(L"top: done delete unknown files on init: all files=" +
            Arrays->toString(dir->listAll()));

    wstring s = infos->getUserData()[PRIMARY_GEN_KEY];
    int64_t myPrimaryGen;
    if (s == L"") {
      assert(infos->empty());
      myPrimaryGen = -1;
    } else {
      myPrimaryGen = StringHelper::fromString<int64_t>(s);
    }
    message(L"top: myPrimaryGen=" + to_wstring(myPrimaryGen));

    bool doCommit;

    if (infos->size() > 0 && myPrimaryGen != -1 &&
        myPrimaryGen != curPrimaryGen) {

      assert(myPrimaryGen < curPrimaryGen);

      // Primary changed while we were down.  In this case, we must sync from
      // primary before opening a reader, because it's possible current files we
      // have will need to be overwritten with different ones (if index rolled
      // back and "forked"), and we can't overwrite open files on Windows:

      constexpr int64_t initSyncStartNS = System::nanoTime();

      message(L"top: init: primary changed while we were down myPrimaryGen=" +
              to_wstring(myPrimaryGen) + L" vs curPrimaryGen=" +
              to_wstring(curPrimaryGen) + L"; sync now before mgr init");

      // Try until we succeed in copying over the latest NRT point:
      shared_ptr<CopyJob> job = nullptr;

      // We may need to overwrite files referenced by our latest commit, either
      // right now on initial sync, or on a later sync.  To make sure the index
      // is never even in an "apparently" corrupt state (where an old segments_N
      // references invalid files) we forcefully remove the commit now, and
      // refuse to start the replica if this delete fails:
      message(L"top: now delete starting commit point " + segmentsFileName);

      // If this throws exc (e.g. due to virus checker), we cannot start this
      // replica:
      assert(deleter->getRefCount(segmentsFileName) == 1);
      deleter->decRef(Collections::singleton(segmentsFileName));

      if (dir->getPendingDeletions()->isEmpty() == false) {
        // If e.g. virus checker blocks us from deleting, we absolutely cannot
        // start this node else there is a definite window during which if we
        // carsh, we cause corruption:
        throw runtime_error(
            L"replica cannot start: existing segments file=" +
            segmentsFileName +
            L" must be removed in order to start, but the file delete failed");
      }

      // So we don't later try to decRef it (illegally) again:
      bool didRemove = lastCommitFiles->remove(segmentsFileName);
      assert(didRemove);

      while (true) {
        job = newCopyJob(L"sync on startup replica=" + name() + L" myVersion=" +
                             to_wstring(infos->getVersion()),
                         nullptr, nullptr, true, nullptr);
        job->start();

        message(L"top: init: sync sis.version=" +
                to_wstring(job->getCopyState()->version));

        // Force this copy job to finish while we wait, now.  Note that this can
        // be very time consuming! NOTE: newNRTPoint detects we are still in
        // init (mgr is null) and does not cancel our copy if a flush happens
        try {
          job->runBlocking();
          job->finish();

          // Success!
          break;
        } catch (const IOException &ioe) {
          job->cancel(L"startup failed", ioe);
          if (ioe->getMessage()->contains(
                  L"checksum mismatch after file copy")) {
            // OK-ish
            message(L"top: failed to copy: " + ioe + L"; retrying");
          } else {
            throw ioe;
          }
        }
      }

      lastPrimaryGen = job->getCopyState()->primaryGen;
      std::deque<char> infosBytes = job->getCopyState()->infosBytes;

      shared_ptr<SegmentInfos> syncInfos = SegmentInfos::readCommit(
          dir,
          make_shared<BufferedChecksumIndexInput>(
              make_shared<ByteArrayIndexInput>(
                  L"SegmentInfos", job->getCopyState()->infosBytes)),
          job->getCopyState()->gen);

      // Must always commit to a larger generation than what's currently in the
      // index:
      syncInfos->updateGeneration(infos);
      infos = syncInfos;

      assert(infos->getVersion() == job->getCopyState()->version);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      message(L"  version=" + to_wstring(infos->getVersion()) + L" segments=" +
              infos->toString());
      message(L"top: init: incRef nrtFiles=" + job->getFileNames());
      deleter->incRef(job->getFileNames());
      message(L"top: init: decRef lastNRTFiles=" + lastNRTFiles);
      deleter->decRef(lastNRTFiles);

      lastNRTFiles->clear();
      lastNRTFiles->addAll(job->getFileNames());

      message(L"top: init: set lastNRTFiles=" + lastNRTFiles);
      lastFileMetaData = job->getCopyState()->files;
      message(wstring::format(
          Locale::ROOT,
          L"top: %d: start: done sync: took %.3fs for %s, opened NRT reader "
          L"version=%d",
          id, (System::nanoTime() - initSyncStartNS) / 1000000000.0,
          bytesToString(job->getTotalBytesCopied()),
          job->getCopyState()->version));

      doCommit = true;
    } else {
      doCommit = false;
      lastPrimaryGen = curPrimaryGen;
      message(L"top: same primary as before");
    }

    if (infos->getGeneration() < maxPendingGen) {
      message(L"top: move infos generation from " +
              to_wstring(infos->getGeneration()) + L" to " +
              to_wstring(maxPendingGen));
      infos->setNextWriteGeneration(maxPendingGen);
    }

    // Notify primary we started, to give it a chance to send any warming merges
    // our way to reduce NRT latency of first sync:
    sendNewReplica();

    // Finally, we are open for business, since our index now "agrees" with the
    // primary:
    mgr = make_shared<SegmentInfosSearcherManager>(dir, shared_from_this(),
                                                   infos, searcherFactory);

    // Must commit after init mgr:
    if (doCommit) {
      // Very important to commit what we just sync'd over, because we removed
      // the pre-existing commit point above if we had to overwrite any files it
      // referenced:
      commit();
    }

    message(L"top: done start");
    state = L"idle";
  } catch (const runtime_error &t) {
    if (t.what()->startsWith(L"replica cannot start") == false) {
      message(L"exc on start:");
      t.printStackTrace(printStream);
    } else {
      delete dir;
    }
    throw IOUtils::rethrowAlways(t);
  }
}

void ReplicaNode::commit() 
{

  {
    lock_guard<mutex> lock(commitLock);

    shared_ptr<SegmentInfos> infos;
    shared_ptr<deque<wstring>> indexFiles;

    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      infos = (std::static_pointer_cast<SegmentInfosSearcherManager>(mgr))
                  ->getCurrentInfos();
      indexFiles = infos->files(false);
      deleter->incRef(indexFiles);
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    message(L"top: commit primaryGen=" + to_wstring(lastPrimaryGen) +
            L" infos=" + infos->toString() + L" files=" + indexFiles);

    // fsync all index files we are now referencing
    dir->sync(indexFiles);

    unordered_map<wstring, wstring> commitData =
        unordered_map<wstring, wstring>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    commitData.emplace(PRIMARY_GEN_KEY, Long::toString(lastPrimaryGen));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    commitData.emplace(VERSION_KEY,
                       Long::toString(getCurrentSearchingVersion()));
    infos->setUserData(commitData, false);

    // write and fsync a new segments_N
    infos->commit(dir);

    // Notify current infos (which may have changed while we were doing dir.sync
    // above) what generation we are up to; this way future commits are
    // guaranteed to go to the next (unwritten) generations:
    if (mgr != nullptr) {
      (std::static_pointer_cast<SegmentInfosSearcherManager>(mgr))
          ->getCurrentInfos()
          ->updateGeneration(infos);
    }
    wstring segmentsFileName = infos->getSegmentsFileName();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    message(L"top: commit wrote segments file " + segmentsFileName +
            L" version=" + to_wstring(infos->getVersion()) + L" sis=" +
            infos->toString() + L" commitData=" + commitData);
    deleter->incRef(Collections::singletonList(segmentsFileName));
    message(L"top: commit decRef lastCommitFiles=" + lastCommitFiles);
    deleter->decRef(lastCommitFiles);
    lastCommitFiles->clear();
    lastCommitFiles->addAll(indexFiles);
    lastCommitFiles->add(segmentsFileName);
    message(L"top: commit version=" + to_wstring(infos->getVersion()) +
            L" files now " + lastCommitFiles);
  }
}

void ReplicaNode::finishNRTCopy(shared_ptr<CopyJob> job,
                                int64_t startNS) 
{
  shared_ptr<CopyState> copyState = job->getCopyState();
  message(L"top: finishNRTCopy: version=" + to_wstring(copyState->version) +
          (job->getFailed() ? L" FAILED" : L"") + L" job=" + job);

  // NOTE: if primary crashed while we were still copying then the job will hit
  // an exc trying to read bytes for the files from the primary node, and the
  // job will be marked as failed here:

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {

    if (L"syncing" == state) {
      state = L"idle";
    }

    if (curNRTCopy == job) {
      message(L"top: now clear curNRTCopy; job=" + job);
      curNRTCopy.reset();
    } else {
      assert(job->getFailed());
      message(L"top: skip clear curNRTCopy: we were cancelled; job=" + job);
    }

    if (job->getFailed()) {
      return;
    }

    // Does final file renames:
    job->finish();

    // Turn byte[] back to SegmentInfos:
    std::deque<char> infosBytes = copyState->infosBytes;
    shared_ptr<SegmentInfos> infos = SegmentInfos::readCommit(
        dir,
        make_shared<BufferedChecksumIndexInput>(
            make_shared<ByteArrayIndexInput>(L"SegmentInfos",
                                             copyState->infosBytes)),
        copyState->gen);
    assert(infos->getVersion() == copyState->version);

    // C++ TODO: There is no native C++ equivalent to 'toString':
    message(L"  version=" + to_wstring(infos->getVersion()) + L" segments=" +
            infos->toString());

    // Cutover to new searcher:
    if (mgr != nullptr) {
      (std::static_pointer_cast<SegmentInfosSearcherManager>(mgr))
          ->setCurrentInfos(infos);
    }

    // Must first incRef new NRT files, then decRef old ones, to make sure we
    // don't remove an NRT file that's in common to both:
    shared_ptr<deque<wstring>> newFiles = copyState->files.keySet();
    message(L"top: incRef newNRTFiles=" + newFiles);
    deleter->incRef(newFiles);

    // If any of our new files were previously copied merges, we clear them now,
    // so we don't try to later delete a non-existent file:
    pendingMergeFiles->removeAll(newFiles);
    message(L"top: after remove from pending merges pendingMergeFiles=" +
            pendingMergeFiles);

    message(L"top: decRef lastNRTFiles=" + lastNRTFiles);
    deleter->decRef(lastNRTFiles);
    lastNRTFiles->clear();
    lastNRTFiles->addAll(newFiles);
    message(L"top: set lastNRTFiles=" + lastNRTFiles);

    // At this point we can remove any completed merge segment files that we
    // still do not reference.  This can happen when a merge finishes, copies
    // its files out to us, but is then merged away (or dropped due to 100%
    // deletions) before we ever cutover to it in an NRT point:
    if (copyState->completedMergeFiles->isEmpty() == false) {
      message(L"now remove-if-not-ref'd completed merge files: " +
              copyState->completedMergeFiles);
      for (auto fileName : copyState->completedMergeFiles) {
        if (pendingMergeFiles->contains(fileName)) {
          pendingMergeFiles->remove(fileName);
          deleter->deleteIfNoRef(fileName);
        }
      }
    }

    lastFileMetaData = copyState->files;
  }

  int markerCount;
  shared_ptr<IndexSearcher> s = mgr->acquire();
  try {
    markerCount = s->count(
        make_shared<TermQuery>(make_shared<Term>(L"marker", L"marker")));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    mgr->release(s);
  }

  message(wstring::format(Locale::ROOT,
                          L"top: done sync: took %.3fs for %s, opened NRT "
                          L"reader version=%d markerCount=%d",
                          (System::nanoTime() - startNS) / 1000000000.0,
                          bytesToString(job->getTotalBytesCopied()),
                          copyState->version, markerCount));
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<CopyJob>
ReplicaNode::newNRTPoint(int64_t newPrimaryGen,
                         int64_t version) 
{

  if (isClosed()) {
    throw make_shared<AlreadyClosedException>(
        L"this replica is closed: state=" + state);
  }

  // Cutover (possibly) to new primary first, so we discard any pre-copied
  // merged segments up front, before checking for which files need copying.
  // While it's possible the pre-copied merged segments could still be useful to
  // us, in the case that the new primary is either the same primary (just e.g.
  // rebooted), or a promoted replica that had a newer NRT point than we did
  // that included the pre-copied merged segments, it's still a bit risky to
  // rely solely on checksum/file length to catch the difference, so we
  // defensively discard here and re-copy in that case:
  maybeNewPrimary(newPrimaryGen);

  // Caller should not "publish" us until we have finished .start():
  assert(mgr != nullptr);

  if (L"idle" == state) {
    state = L"syncing";
  }

  int64_t curVersion = getCurrentSearchingVersion();

  message(L"top: start sync sis.version=" + to_wstring(version));

  if (version == curVersion) {
    // Caller releases the CopyState:
    message(L"top: new NRT point has same version as current; skipping");
    return nullptr;
  }

  if (version < curVersion) {
    // This can happen, if two syncs happen close together, and due to thread
    // scheduling, the incoming older version runs after the newer version
    message(L"top: new NRT point (version=" + to_wstring(version) +
            L") is older than current (version=" + to_wstring(curVersion) +
            L"); skipping");
    return nullptr;
  }

  constexpr int64_t startNS = System::nanoTime();

  message(L"top: newNRTPoint");
  shared_ptr<CopyJob> job = nullptr;
  try {
    job = newCopyJob(L"NRT point sync version=" + to_wstring(version), nullptr,
                     lastFileMetaData, true,
                     make_shared<OnceDoneAnonymousInnerClass>(
                         shared_from_this(), startNS, job));
  } catch (const NodeCommunicationException &nce) {
    // E.g. primary could crash/close when we are asking it for the copy state:
    message(L"top: ignoring communication exception creating CopyJob: " + nce);
    // nce.printStackTrace(printStream);
    if (state == L"syncing") {
      state = L"idle";
    }
    return nullptr;
  }

  assert(newPrimaryGen == job->getCopyState()->primaryGen);

  shared_ptr<deque<wstring>> newNRTFiles = job->getFileNames();

  message(L"top: newNRTPoint: job files=" + newNRTFiles);

  if (curNRTCopy != nullptr) {
    job->transferAndCancel(curNRTCopy);
    assert(curNRTCopy->getFailed());
  }

  curNRTCopy = job;

  for (auto fileName : curNRTCopy->getFileNamesToCopy()) {
    assert((lastCommitFiles->contains(fileName) == false,
            L"fileName=" + fileName +
                L" is in lastCommitFiles and is being copied?"));
    {
      lock_guard<mutex> lock(mergeCopyJobs);
      for (auto mergeJob : mergeCopyJobs) {
        if (mergeJob->getFileNames()->contains(fileName)) {
          // TODO: we could maybe transferAndCancel here?  except CopyJob can't
          // transferAndCancel more than one currently
          message(L"top: now cancel merge copy job=" + mergeJob + L": file " +
                  fileName + L" is now being copied via NRT point");
          mergeJob->cancel(L"newNRTPoint is copying over the same file",
                           nullptr);
        }
      }
    }
  }

  try {
    job->start();
  } catch (const NodeCommunicationException &nce) {
    // E.g. primary could crash/close when we are asking it for the copy state:
    message(L"top: ignoring exception starting CopyJob: " + nce);
    nce->printStackTrace(printStream);
    if (state == L"syncing") {
      state = L"idle";
    }
    return nullptr;
  }

  // Runs in the background jobs thread, maybe slowly/throttled, and calls
  // finishSync once it's done:
  launch(curNRTCopy);
  return curNRTCopy;
}

ReplicaNode::OnceDoneAnonymousInnerClass::OnceDoneAnonymousInnerClass(
    shared_ptr<ReplicaNode> outerInstance, int64_t startNS,
    shared_ptr<org::apache::lucene::replicator::nrt::CopyJob> job)
{
  this->outerInstance = outerInstance;
  this->startNS = startNS;
  this->job = job;
}

void ReplicaNode::OnceDoneAnonymousInnerClass::run(shared_ptr<CopyJob> job)
{
  try {
    outerInstance->finishNRTCopy(job, startNS);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ReplicaNode::isCopying() { return curNRTCopy != nullptr; }

bool ReplicaNode::isClosed()
{
  return L"closed" == state || L"closing" == state || L"crashing" == state ||
         L"crashed" == state;
}

ReplicaNode::~ReplicaNode()
{
  message(L"top: now close");

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    state = L"closing";
    if (curNRTCopy != nullptr) {
      curNRTCopy->cancel(L"closing", nullptr);
    }
  }

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    message(L"top: close mgr");
    delete mgr;

    message(L"top: decRef lastNRTFiles=" + lastNRTFiles);
    deleter->decRef(lastNRTFiles);
    lastNRTFiles->clear();

    // NOTE: do not decRef these!
    lastCommitFiles->clear();

    message(L"top: delete if no ref pendingMergeFiles=" + pendingMergeFiles);
    for (auto fileName : pendingMergeFiles) {
      deleter->deleteIfNoRef(fileName);
    }
    pendingMergeFiles->clear();

    message(L"top: close dir");
    IOUtils::close({writeFileLock, dir});
  }
  message(L"top: done close");
  state = L"closed";
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReplicaNode::maybeNewPrimary(int64_t newPrimaryGen) 
{
  if (newPrimaryGen != lastPrimaryGen) {
    message(L"top: now change lastPrimaryGen from " +
            to_wstring(lastPrimaryGen) + L" to " + to_wstring(newPrimaryGen) +
            L" pendingMergeFiles=" + pendingMergeFiles);

    message(L"top: delete if no ref pendingMergeFiles=" + pendingMergeFiles);
    for (auto fileName : pendingMergeFiles) {
      deleter->deleteIfNoRef(fileName);
    }

    assert((newPrimaryGen > lastPrimaryGen,
            L"newPrimaryGen=" + to_wstring(newPrimaryGen) +
                L" vs lastPrimaryGen=" + to_wstring(lastPrimaryGen)));
    lastPrimaryGen = newPrimaryGen;
    pendingMergeFiles->clear();
  } else {
    message(L"top: keep current lastPrimaryGen=" + to_wstring(lastPrimaryGen));
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<CopyJob> ReplicaNode::launchPreCopyMerge(
    shared_ptr<AtomicBoolean> finished, int64_t newPrimaryGen,
    unordered_map<wstring, std::shared_ptr<FileMetaData>>
        &files) 
{

  shared_ptr<CopyJob> job;

  maybeNewPrimary(newPrimaryGen);
  constexpr int64_t primaryGenStart = lastPrimaryGen;
  shared_ptr<Set<wstring>> fileNames = files.keySet();
  message(L"now pre-copy warm merge files=" + fileNames + L" primaryGen=" +
          to_wstring(newPrimaryGen));

  for (auto fileName : fileNames) {
    assert((pendingMergeFiles->contains(fileName) == false,
            L"file \"" + fileName + L"\" is already being warmed!"));
    assert((lastNRTFiles->contains(fileName) == false,
            L"file \"" + fileName + L"\" is already NRT visible!"));
  }

  job = newCopyJob(
      L"warm merge on " + name() + L" filesNames=" + fileNames, files, nullptr,
      false,
      make_shared<OnceDoneAnonymousInnerClass2>(
          shared_from_this(), finished, job, primaryGenStart, fileNames));

  job->start();

  // When warming a merge we better not already have any of these files copied!
  assert(job->getFileNamesToCopy()->size() == files.size());

  mergeCopyJobs->add(job);
  launch(job);

  return job;
}

ReplicaNode::OnceDoneAnonymousInnerClass2::OnceDoneAnonymousInnerClass2(
    shared_ptr<ReplicaNode> outerInstance, shared_ptr<AtomicBoolean> finished,
    shared_ptr<org::apache::lucene::replicator::nrt::CopyJob> job,
    int64_t primaryGenStart, shared_ptr<Set<wstring>> fileNames)
{
  this->outerInstance = outerInstance;
  this->finished = finished;
  this->job = job;
  this->primaryGenStart = primaryGenStart;
  this->fileNames = fileNames;
}

void ReplicaNode::OnceDoneAnonymousInnerClass2::run(
    shared_ptr<CopyJob> job) 
{
  // Signals that this replica has finished
  outerInstance->mergeCopyJobs->remove(job);
  SegmentInfos::message(L"done warming merge " + fileNames + L" failed?=" +
                        StringHelper::toString(job->getFailed()));
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    if (job->getFailed() == false) {
      if (outerInstance->lastPrimaryGen != primaryGenStart) {
        SegmentInfos::message(L"merge pre copy finished but primary has "
                              L"changed; cancelling job files=" +
                              fileNames);
        job->cancel(L"primary changed during merge copy", nullptr);
      } else {
        bool abort = false;
        for (auto fileName : fileNames) {
          if (outerInstance->lastNRTFiles->contains(fileName)) {
            SegmentInfos::message(L"abort merge finish: file " + fileName +
                                  L" is referenced by last NRT point");
            abort = true;
          }
          if (outerInstance->lastCommitFiles->contains(fileName)) {
            SegmentInfos::message(L"abort merge finish: file " + fileName +
                                  L" is referenced by last commit point");
            abort = true;
          }
        }
        if (abort) {
          // Even though in newNRTPoint we have similar logic, which cancels any
          // merge copy jobs if an NRT point shows up referencing the files we
          // are warming (because primary got impatient and gave up on us), we
          // also need it here in case replica is way far behind and fails to
          // even receive the merge pre-copy request until after the newNRTPoint
          // referenced those files:
          job->cancel(L"merged segment was separately copied via NRT point",
                      nullptr);
        } else {
          job->finish();
          SegmentInfos::message(L"merge pre copy finished files=" + fileNames);
          for (auto fileName : fileNames) {
            assert((
                outerInstance->pendingMergeFiles->contains(fileName) == false,
                L"file \"" + fileName + L"\" is already in pendingMergeFiles"));
            SegmentInfos::message(L"add file " + fileName +
                                  L" to pendingMergeFiles");
            outerInstance->pendingMergeFiles->add(fileName);
          }
        }
      }
    } else {
      SegmentInfos::message(L"merge copy finished with failure");
    }
  }
  finished->set(true);
}

shared_ptr<IndexOutput> ReplicaNode::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> ioContext) 
{
  return dir->createTempOutput(prefix, suffix, IOContext::DEFAULT);
}

deque<unordered_map::Entry<wstring, std::shared_ptr<FileMetaData>>>
ReplicaNode::getFilesToCopy(
    unordered_map<wstring, std::shared_ptr<FileMetaData>>
        &files) 
{

  deque<unordered_map::Entry<wstring, std::shared_ptr<FileMetaData>>> toCopy =
      deque<unordered_map::Entry<wstring, std::shared_ptr<FileMetaData>>>();
  for (auto ent : files) {
    wstring fileName = ent.first;
    shared_ptr<FileMetaData> fileMetaData = ent.second;
    if (fileIsIdentical(fileName, fileMetaData) == false) {
      toCopy.push_back(ent);
    }
  }

  return toCopy;
}

bool ReplicaNode::fileIsIdentical(
    const wstring &fileName,
    shared_ptr<FileMetaData> srcMetaData) 
{

  shared_ptr<FileMetaData> destMetaData = readLocalFileMetaData(fileName);
  if (destMetaData == nullptr) {
    // Something went wrong in reading the file (it's corrupt, truncated, does
    // not exist, etc.):
    return false;
  }

  if (Arrays::equals(destMetaData->header, srcMetaData->header) == false ||
      Arrays::equals(destMetaData->footer, srcMetaData->footer) == false) {
    // Segment name was reused!  This is rare but possible and otherwise
    // devastating:
    if (Node::VERBOSE_FILES) {
      message(L"file " + fileName +
              L": will copy [header/footer is different]");
    }
    return false;
  } else {
    return true;
  }
}

void ReplicaNode::startCopyFile(const wstring &name)
{
  if (copying->putIfAbsent(name, Boolean::TRUE) != nullptr) {
    throw make_shared<IllegalStateException>(
        L"file " + name + L" is being copied in two places!");
  }
}

void ReplicaNode::finishCopyFile(const wstring &name)
{
  if (copying->remove(name) == nullptr) {
    throw make_shared<IllegalStateException>(
        L"file " + name + L" was not actually being copied?");
  }
}
} // namespace org::apache::lucene::replicator::nrt