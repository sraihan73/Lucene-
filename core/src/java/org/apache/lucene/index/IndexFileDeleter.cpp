using namespace std;

#include "IndexFileDeleter.h"

namespace org::apache::lucene::index
{
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using CollectionUtil = org::apache::lucene::util::CollectionUtil;
using Constants = org::apache::lucene::util::Constants;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
bool IndexFileDeleter::VERBOSE_REF_COUNTS = false;

bool IndexFileDeleter::locked()
{
  return writer == nullptr || Thread::holdsLock(writer);
}

IndexFileDeleter::IndexFileDeleter(
    std::deque<wstring> &files, shared_ptr<Directory> directoryOrig,
    shared_ptr<Directory> directory, shared_ptr<IndexDeletionPolicy> policy,
    shared_ptr<SegmentInfos> segmentInfos, shared_ptr<InfoStream> infoStream,
    shared_ptr<IndexWriter> writer, bool initialIndexExists,
    bool isReaderInit) 
    : infoStream(infoStream), directoryOrig(directoryOrig),
      directory(directory), policy(policy), writer(writer)
{
  Objects::requireNonNull(writer);

  const wstring currentSegmentsFile = segmentInfos->getSegmentsFileName();

  if (infoStream->isEnabled(L"IFD")) {
    infoStream->message(L"IFD", L"init: current segments file is \"" +
                                    currentSegmentsFile +
                                    L"\"; deletionPolicy=" + policy);
  }

  // First pass: walk the files and initialize our ref
  // counts:
  shared_ptr<CommitPoint> currentCommitPoint = nullptr;

  if (currentSegmentsFile != L"") {
    shared_ptr<Matcher> m = IndexFileNames::CODEC_FILE_PATTERN->matcher(L"");
    for (auto fileName : files) {
      m->reset(fileName);
      if (!fileName.endsWith(L"write.lock") &&
          (m->matches() || fileName.startsWith(IndexFileNames::SEGMENTS) ||
           fileName.startsWith(IndexFileNames::PENDING_SEGMENTS))) {

        // Add this file to refCounts with initial count 0:
        getRefCount(fileName);

        if (fileName.startsWith(IndexFileNames::SEGMENTS) &&
            !fileName.equals(IndexFileNames::OLD_SEGMENTS_GEN)) {

          // This is a commit (segments or segments_N), and
          // it's valid (<= the max gen).  Load it, then
          // incref all files it refers to:
          if (infoStream->isEnabled(L"IFD")) {
            infoStream->message(L"IFD",
                                L"init: load commit \"" + fileName + L"\"");
          }
          shared_ptr<SegmentInfos> sis =
              SegmentInfos::readCommit(directoryOrig, fileName);

          shared_ptr<CommitPoint> *const commitPoint =
              make_shared<CommitPoint>(commitsToDelete, directoryOrig, sis);
          if (sis->getGeneration() == segmentInfos->getGeneration()) {
            currentCommitPoint = commitPoint;
          }
          commits.push_back(commitPoint);
          incRef(sis, true);

          if (lastSegmentInfos->empty() ||
              sis->getGeneration() > lastSegmentInfos->getGeneration()) {
            lastSegmentInfos = sis;
          }
        }
      }
    }
  }

  if (currentCommitPoint == nullptr && currentSegmentsFile != L"" &&
      initialIndexExists) {
    // We did not in fact see the segments_N file
    // corresponding to the segmentInfos that was passed
    // in.  Yet, it must exist, because our caller holds
    // the write lock.  This can happen when the directory
    // listing was stale (eg when index accessed via NFS
    // client with stale directory listing cache).  So we
    // try now to explicitly open this commit point:
    shared_ptr<SegmentInfos> sis;
    try {
      sis = SegmentInfos::readCommit(directoryOrig, currentSegmentsFile);
    } catch (const IOException &e) {
      throw make_shared<CorruptIndexException>(
          L"unable to read current segments_N file", currentSegmentsFile, e);
    }
    if (infoStream->isEnabled(L"IFD")) {
      infoStream->message(L"IFD", L"forced open of current segments file " +
                                      segmentInfos->getSegmentsFileName());
    }
    currentCommitPoint =
        make_shared<CommitPoint>(commitsToDelete, directoryOrig, sis);
    commits.push_back(currentCommitPoint);
    incRef(sis, true);
  }

  if (isReaderInit) {
    // Incoming SegmentInfos may have NRT changes not yet visible in the latest
    // commit, so we have to protect its files from deletion too:
    checkpoint(segmentInfos, false);
  }

  // We keep commits deque in sorted order (oldest to newest):
  CollectionUtil::timSort(commits);
  shared_ptr<deque<wstring>> relevantFiles =
      unordered_set<wstring>(refCounts.keySet());
  shared_ptr<Set<wstring>> pendingDeletions =
      directoryOrig->getPendingDeletions();
  if (pendingDeletions->isEmpty() == false) {
    relevantFiles->addAll(pendingDeletions);
  }
  // refCounts only includes "normal" filenames (does not include write.lock)
  inflateGens(segmentInfos, relevantFiles, infoStream);

  // Now delete anything with ref count at 0.  These are
  // presumably abandoned files eg due to crash of
  // IndexWriter.
  shared_ptr<Set<wstring>> toDelete = unordered_set<wstring>();
  for (auto entry : refCounts) {
    shared_ptr<RefCount> rc = entry.second;
    const wstring fileName = entry.first;
    if (0 == rc->count) {
      // A segments_N file should never have ref count 0 on init:
      if (StringHelper::startsWith(fileName, IndexFileNames::SEGMENTS)) {
        throw make_shared<IllegalStateException>(
            L"file \"" + fileName +
            L"\" has refCount=0, which should never happen on init");
      }
      if (infoStream->isEnabled(L"IFD")) {
        infoStream->message(L"IFD", L"init: removing unreferenced file \"" +
                                        fileName + L"\"");
      }
      toDelete->add(fileName);
    }
  }

  deleteFiles(toDelete);

  // Finally, give policy a chance to remove things on
  // startup:
  policy->onInit(commits);

  // Always protect the incoming segmentInfos since
  // sometime it may not be the most recent commit
  checkpoint(segmentInfos, false);

  if (currentCommitPoint == nullptr) {
    startingCommitDeleted = false;
  } else {
    startingCommitDeleted = currentCommitPoint->isDeleted();
  }

  deleteCommits();
}

void IndexFileDeleter::inflateGens(shared_ptr<SegmentInfos> infos,
                                   shared_ptr<deque<wstring>> files,
                                   shared_ptr<InfoStream> infoStream)
{

  int64_t maxSegmentGen = numeric_limits<int64_t>::min();
  int64_t maxSegmentName = numeric_limits<int64_t>::min();

  // Confusingly, this is the union of liveDocs, field infos, doc values
  // (and maybe others, in the future) gens.  This is somewhat messy,
  // since it means DV updates will suddenly write to the next gen after
  // live docs' gen, for example, but we don't have the APIs to ask the
  // codec which file is which:
  unordered_map<wstring, int64_t> maxPerSegmentGen =
      unordered_map<wstring, int64_t>();

  for (auto fileName : files) {
    if (fileName.equals(IndexFileNames::OLD_SEGMENTS_GEN) ||
        fileName.equals(IndexWriter::WRITE_LOCK_NAME)) {
      // do nothing
    } else if (fileName.startsWith(IndexFileNames::SEGMENTS)) {
      try {
        maxSegmentGen =
            max(SegmentInfos::generationFromSegmentsFileName(fileName),
                maxSegmentGen);
      } catch (const NumberFormatException &ignore) {
        // trash file: we have to handle this since we allow anything starting
        // with 'segments' here
      }
    } else if (fileName.startsWith(IndexFileNames::PENDING_SEGMENTS)) {
      try {
        maxSegmentGen = max(
            SegmentInfos::generationFromSegmentsFileName(fileName.substr(8)),
            maxSegmentGen);
      } catch (const NumberFormatException &ignore) {
        // trash file: we have to handle this since we allow anything starting
        // with 'pending_segments' here
      }
    } else {
      wstring segmentName = IndexFileNames::parseSegmentName(fileName);
      assert((StringHelper::startsWith(segmentName, L"_"),
              L"wtf? file=" + fileName));

      if (fileName.toLowerCase(Locale::ROOT)->endsWith(L".tmp")) {
        // A temp file: don't try to look at its gen
        continue;
      }

      // C++ TODO: Only single-argument parse and valueOf methods are converted:
      // ORIGINAL LINE: maxSegmentName = Math.max(maxSegmentName,
      // Long.parseLong(segmentName.substring(1), Character.MAX_RADIX));
      maxSegmentName =
          max(maxSegmentName,
              int64_t ::valueOf(segmentName.substr(1), Character::MAX_RADIX));

      optional<int64_t> curGen = maxPerSegmentGen[segmentName];
      if (!curGen) {
        curGen = 0LL;
      }

      try {
        curGen = max(curGen, IndexFileNames::parseGeneration(fileName));
      } catch (const NumberFormatException &ignore) {
        // trash file: we have to handle this since codec regex is only so good
      }
      maxPerSegmentGen.emplace(segmentName, curGen);
    }
  }

  // Generation is advanced before write:
  infos->setNextWriteGeneration(max(infos->getGeneration(), maxSegmentGen));
  if (infos->counter < 1 + maxSegmentName) {
    if (infoStream->isEnabled(L"IFD")) {
      infoStream->message(L"IFD", L"init: inflate infos.counter to " +
                                      to_wstring(1 + maxSegmentName) +
                                      L" vs current=" +
                                      to_wstring(infos->counter));
    }
    infos->counter = 1 + maxSegmentName;
  }

  for (auto info : infos) {
    optional<int64_t> gen = maxPerSegmentGen[info->info->name];
    assert(gen);
    int64_t genLong = gen;
    if (info->getNextWriteDelGen() < genLong + 1) {
      if (infoStream->isEnabled(L"IFD")) {
        infoStream->message(L"IFD", L"init: seg=" + info->info->name +
                                        L" set nextWriteDelGen=" +
                                        to_wstring(genLong + 1) +
                                        L" vs current=" +
                                        to_wstring(info->getNextWriteDelGen()));
      }
      info->setNextWriteDelGen(genLong + 1);
    }
    if (info->getNextWriteFieldInfosGen() < genLong + 1) {
      if (infoStream->isEnabled(L"IFD")) {
        infoStream->message(L"IFD",
                            L"init: seg=" + info->info->name +
                                L" set nextWriteFieldInfosGen=" +
                                to_wstring(genLong + 1) + L" vs current=" +
                                to_wstring(info->getNextWriteFieldInfosGen()));
      }
      info->setNextWriteFieldInfosGen(genLong + 1);
    }
    if (info->getNextWriteDocValuesGen() < genLong + 1) {
      if (infoStream->isEnabled(L"IFD")) {
        infoStream->message(L"IFD",
                            L"init: seg=" + info->info->name +
                                L" set nextWriteDocValuesGen=" +
                                to_wstring(genLong + 1) + L" vs current=" +
                                to_wstring(info->getNextWriteDocValuesGen()));
      }
      info->setNextWriteDocValuesGen(genLong + 1);
    }
  }
}

void IndexFileDeleter::ensureOpen() 
{
  writer->ensureOpen(false);
  // since we allow 'closing' state, we must still check this, we could be
  // closing because we hit e.g. OOM
  if (writer->tragedy->get() != nullptr) {
    throw make_shared<AlreadyClosedException>(
        L"refusing to delete any files: this IndexWriter hit an unrecoverable "
        L"exception",
        writer->tragedy->get());
  }
}

bool IndexFileDeleter::isClosed()
{
  try {
    ensureOpen();
    return false;
  } catch (const AlreadyClosedException &ace) {
    return true;
  }
}

void IndexFileDeleter::deleteCommits() 
{

  int size = commitsToDelete.size();

  if (size > 0) {

    // First decref all files that had been referred to by
    // the now-deleted commits:
    runtime_error firstThrowable = nullptr;
    for (int i = 0; i < size; i++) {
      shared_ptr<CommitPoint> commit = commitsToDelete[i];
      if (infoStream->isEnabled(L"IFD")) {
        infoStream->message(L"IFD", L"deleteCommits: now decRef commit \"" +
                                        commit->getSegmentsFileName() + L"\"");
      }
      try {
        decRef(commit->files);
      } catch (const runtime_error &t) {
        firstThrowable = IOUtils::useOrSuppress(firstThrowable, t);
      }
    }
    commitsToDelete.clear();

    if (firstThrowable != nullptr) {
      throw IOUtils::rethrowAlways(firstThrowable);
    }

    // Now compact commits to remove deleted ones (preserving the sort):
    size = commits.size();
    int readFrom = 0;
    int writeTo = 0;
    while (readFrom < size) {
      shared_ptr<CommitPoint> commit = commits[readFrom];
      if (!commit->deleted) {
        if (writeTo != readFrom) {
          commits[writeTo] = commits[readFrom];
        }
        writeTo++;
      }
      readFrom++;
    }

    while (size > writeTo) {
      commits.erase(commits.begin() + size - 1);
      size--;
    }
  }
}

void IndexFileDeleter::refresh() 
{
  assert(locked());
  shared_ptr<Set<wstring>> toDelete = unordered_set<wstring>();

  std::deque<wstring> files = directory->listAll();

  shared_ptr<Matcher> m = IndexFileNames::CODEC_FILE_PATTERN->matcher(L"");

  for (int i = 0; i < files.size(); i++) {
    wstring fileName = files[i];
    m->reset(fileName);
    if (!StringHelper::endsWith(fileName, L"write.lock") &&
        refCounts.find(fileName) == refCounts.end() &&
        (m->matches() ||
         StringHelper::startsWith(fileName, IndexFileNames::SEGMENTS) ||
         StringHelper::startsWith(fileName,
                                  IndexFileNames::PENDING_SEGMENTS))) {
      // Unreferenced file, so remove it
      if (infoStream->isEnabled(L"IFD")) {
        infoStream->message(
            L"IFD", L"refresh: removing newly created unreferenced file \"" +
                        fileName + L"\"");
      }
      toDelete->add(fileName);
    }
  }

  deleteFiles(toDelete);
}

IndexFileDeleter::~IndexFileDeleter()
{
  // DecRef old files from the last checkpoint, if any:
  assert(locked());

  if (!lastFiles.empty()) {
    try {
      decRef(lastFiles);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      lastFiles.clear();
    }
  }
}

void IndexFileDeleter::revisitPolicy() 
{
  assert(locked());
  if (infoStream->isEnabled(L"IFD")) {
    infoStream->message(L"IFD", L"now revisitPolicy");
  }

  if (commits.size() > 0) {
    policy->onCommit(commits);
    deleteCommits();
  }
}

void IndexFileDeleter::checkpoint(shared_ptr<SegmentInfos> segmentInfos,
                                  bool isCommit) 
{
  assert(locked());

  assert(Thread::holdsLock(writer));
  int64_t t0 = System::nanoTime();
  if (infoStream->isEnabled(L"IFD")) {
    infoStream->message(
        L"IFD", L"now checkpoint \"" +
                    writer->segString(writer->toLiveInfos(segmentInfos)) +
                    L"\" [" + to_wstring(segmentInfos->size()) + L" segments " +
                    L"; isCommit = " + StringHelper::toString(isCommit) + L"]");
  }

  // Incref the files:
  incRef(segmentInfos, isCommit);

  if (isCommit) {
    // Append to our commits deque:
    commits.push_back(
        make_shared<CommitPoint>(commitsToDelete, directoryOrig, segmentInfos));

    // Tell policy so it can remove commits:
    policy->onCommit(commits);

    // Decref files for commits that were deleted by the policy:
    deleteCommits();
  } else {
    // DecRef old files from the last checkpoint, if any:
    try {
      decRef(lastFiles);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      lastFiles.clear();
    }

    // Save files so we can decr on next checkpoint/commit:
    lastFiles.addAll(segmentInfos->files(false));
  }

  if (infoStream->isEnabled(L"IFD")) {
    int64_t t1 = System::nanoTime();
    infoStream->message(L"IFD", to_wstring((t1 - t0) / 1000000) +
                                    L" msec to checkpoint");
  }
}

void IndexFileDeleter::incRef(shared_ptr<SegmentInfos> segmentInfos,
                              bool isCommit) 
{
  assert(locked());
  // If this is a commit point, also incRef the
  // segments_N file:
  for (auto fileName : segmentInfos->files(isCommit)) {
    incRef(fileName);
  }
}

void IndexFileDeleter::incRef(shared_ptr<deque<wstring>> files)
{
  assert(locked());
  for (auto file : files) {
    incRef(file);
  }
}

void IndexFileDeleter::incRef(const wstring &fileName)
{
  assert(locked());
  shared_ptr<RefCount> rc = getRefCount(fileName);
  if (infoStream->isEnabled(L"IFD")) {
    if (VERBOSE_REF_COUNTS) {
      infoStream->message(L"IFD", L"  IncRef \"" + fileName +
                                      L"\": pre-incr count is " +
                                      to_wstring(rc->count));
    }
  }
  rc->IncRef();
}

void IndexFileDeleter::decRef(shared_ptr<deque<wstring>> files) throw(
    IOException)
{
  assert(locked());
  shared_ptr<Set<wstring>> toDelete = unordered_set<wstring>();
  runtime_error firstThrowable = nullptr;
  for (auto file : files) {
    try {
      if (decRef(file)) {
        toDelete->add(file);
      }
    } catch (const runtime_error &t) {
      firstThrowable = IOUtils::useOrSuppress(firstThrowable, t);
    }
  }

  try {
    deleteFiles(toDelete);
  } catch (const runtime_error &t) {
    firstThrowable = IOUtils::useOrSuppress(firstThrowable, t);
  }

  if (firstThrowable != nullptr) {
    throw IOUtils::rethrowAlways(firstThrowable);
  }
}

bool IndexFileDeleter::decRef(const wstring &fileName)
{
  assert(locked());
  shared_ptr<RefCount> rc = getRefCount(fileName);
  if (infoStream->isEnabled(L"IFD")) {
    if (VERBOSE_REF_COUNTS) {
      infoStream->message(L"IFD", L"  DecRef \"" + fileName +
                                      L"\": pre-decr count is " +
                                      to_wstring(rc->count));
    }
  }
  if (rc->DecRef() == 0) {
    // This file is no longer referenced by any past
    // commit points nor by the in-memory SegmentInfos:
    refCounts.erase(fileName);
    return true;
  } else {
    return false;
  }
}

void IndexFileDeleter::decRef(shared_ptr<SegmentInfos> segmentInfos) throw(
    IOException)
{
  assert(locked());
  decRef(segmentInfos->files(false));
}

bool IndexFileDeleter::exists(const wstring &fileName)
{
  assert(locked());
  if (refCounts.find(fileName) == refCounts.end()) {
    return false;
  } else {
    return getRefCount(fileName)->count > 0;
  }
}

shared_ptr<RefCount> IndexFileDeleter::getRefCount(const wstring &fileName)
{
  assert(locked());
  shared_ptr<RefCount> rc;
  if (refCounts.find(fileName) == refCounts.end()) {
    rc = make_shared<RefCount>(fileName);
    refCounts.emplace(fileName, rc);
  } else {
    rc = refCounts[fileName];
  }
  return rc;
}

void IndexFileDeleter::deleteNewFiles(
    shared_ptr<deque<wstring>> files) 
{
  assert(locked());
  shared_ptr<Set<wstring>> toDelete = unordered_set<wstring>();
  for (auto fileName : files) {
    // NOTE: it's very unusual yet possible for the
    // refCount to be present and 0: it can happen if you
    // open IW on a crashed index, and it removes a bunch
    // of unref'd files, and then you add new docs / do
    // merging, and it reuses that segment name.
    // TestCrash.testCrashAfterReopen can hit this:
    if (refCounts.find(fileName) == refCounts.end() ||
        refCounts[fileName]->count == 0) {
      if (infoStream->isEnabled(L"IFD")) {
        infoStream->message(L"IFD",
                            L"will delete new file \"" + fileName + L"\"");
      }
      toDelete->add(fileName);
    }
  }

  deleteFiles(toDelete);
}

void IndexFileDeleter::deleteFiles(shared_ptr<deque<wstring>> names) throw(
    IOException)
{
  assert(locked());
  ensureOpen();

  if (infoStream->isEnabled(L"IFD")) {
    if (names->size() > 0) {
      infoStream->message(L"IFD", L"delete " + names + L"");
    }
  }

  // We make two passes, first deleting any segments_N files, second deleting
  // the rest.  We do this so that if we throw exc or JVM crashes during
  // deletions, even when not on Windows, we don't leave the index in an
  // "apparently corrupt" state:
  for (auto name : names) {
    if (name.startsWith(IndexFileNames::SEGMENTS) == false) {
      continue;
    }
    deleteFile(name);
  }

  for (auto name : names) {
    if (name.startsWith(IndexFileNames::SEGMENTS) == true) {
      continue;
    }
    deleteFile(name);
  }
}

void IndexFileDeleter::deleteFile(const wstring &fileName) 
{
  try {
    directory->deleteFile(fileName);
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NoSuchFileException | FileNotFoundException e) {
    if (Constants::WINDOWS) {
      // TODO: can we remove this OS-specific hacky logic?  If windows
      // deleteFile is buggy, we should instead contain this workaround in a
      // WindowsFSDirectory ... LUCENE-6684: we suppress this assert for
      // Windows, since a file could be in a confusing "pending delete" state,
      // where we already deleted it once, yet it still shows up in directory
      // listings, and if you try to delete it again you'll hit NSFE/FNFE:
    } else {
      throw e;
    }
  }
}

IndexFileDeleter::RefCount::RefCount(const wstring &fileName)
    : fileName(fileName)
{
}

int IndexFileDeleter::RefCount::IncRef()
{
  if (!initDone) {
    initDone = true;
  } else {
    assert((count > 0, Thread::currentThread().getName() +
                           L": RefCount is 0 pre-increment for file \"" +
                           fileName + L"\""));
  }
  return ++count;
}

int IndexFileDeleter::RefCount::DecRef()
{
  assert((count > 0, Thread::currentThread().getName() +
                         L": RefCount is 0 pre-decrement for file \"" +
                         fileName + L"\""));
  return --count;
}

IndexFileDeleter::CommitPoint::CommitPoint(
    shared_ptr<deque<std::shared_ptr<CommitPoint>>> commitsToDelete,
    shared_ptr<Directory> directoryOrig,
    shared_ptr<SegmentInfos> segmentInfos) 
    : userData(segmentInfos->getUserData()), segmentCount(segmentInfos->size())
{
  this->directoryOrig = directoryOrig;
  this->commitsToDelete = commitsToDelete;
  segmentsFileName = segmentInfos->getSegmentsFileName();
  generation = segmentInfos->getGeneration();
  files = Collections::unmodifiableCollection(segmentInfos->files(true));
}

wstring IndexFileDeleter::CommitPoint::toString()
{
  return L"IndexFileDeleter.CommitPoint(" + segmentsFileName + L")";
}

int IndexFileDeleter::CommitPoint::getSegmentCount() { return segmentCount; }

wstring IndexFileDeleter::CommitPoint::getSegmentsFileName()
{
  return segmentsFileName;
}

shared_ptr<deque<wstring>> IndexFileDeleter::CommitPoint::getFileNames()
{
  return files;
}

shared_ptr<Directory> IndexFileDeleter::CommitPoint::getDirectory()
{
  return directoryOrig;
}

int64_t IndexFileDeleter::CommitPoint::getGeneration() { return generation; }

unordered_map<wstring, wstring> IndexFileDeleter::CommitPoint::getUserData()
{
  return userData;
}

void IndexFileDeleter::CommitPoint::delete_()
{
  if (!deleted) {
    deleted = true;
    commitsToDelete->add(shared_from_this());
  }
}

bool IndexFileDeleter::CommitPoint::isDeleted() { return deleted; }
} // namespace org::apache::lucene::index