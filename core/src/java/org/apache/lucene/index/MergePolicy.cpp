using namespace std;

#include "MergePolicy.h"

namespace org::apache::lucene::index
{
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using MergeInfo = org::apache::lucene::store::MergeInfo;
using Bits = org::apache::lucene::util::Bits;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using InfoStream = org::apache::lucene::util::InfoStream;

MergePolicy::OneMergeProgress::OneMergeProgress()
    : pauseTimesNS(
          make_shared<EnumMap<PauseReason, std::shared_ptr<AtomicLong>>>(
              PauseReason::typeid))
{
  // Place all the pause reasons in there immediately so that we can simply
  // update values.
  for (PauseReason p : PauseReason::values()) {
    pauseTimesNS->put(p, make_shared<AtomicLong>());
  }
}

void MergePolicy::OneMergeProgress::abort()
{
  aborted = true;
  wakeup(); // wakeup any paused merge thread.
}

bool MergePolicy::OneMergeProgress::isAborted() { return aborted; }

void MergePolicy::OneMergeProgress::pauseNanos(
    int64_t pauseNanos, PauseReason reason,
    function<bool()> &condition) 
{
  if (Thread::currentThread() != owner) {
    throw runtime_error(
        L"Only the merge owner thread can call pauseNanos(). This thread: " +
        Thread::currentThread().getName() + L", owner thread: " + owner);
  }

  int64_t start = System::nanoTime();
  shared_ptr<AtomicLong> timeUpdate = pauseTimesNS->get(reason);
  pauseLock->lock();
  try {
    while (pauseNanos > 0 && !aborted && condition()) {
      pauseNanos = pausing->awaitNanos(pauseNanos);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    pauseLock->unlock();
    timeUpdate->addAndGet(System::nanoTime() - start);
  }
}

void MergePolicy::OneMergeProgress::wakeup()
{
  pauseLock->lock();
  try {
    pausing->signalAll();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    pauseLock->unlock();
  }
}

unordered_map<PauseReason, int64_t>
MergePolicy::OneMergeProgress::getPauseTimes()
{
  shared_ptr<
      Set<unordered_map::Entry<PauseReason, std::shared_ptr<AtomicLong>>>>
      entries = pauseTimesNS->entrySet();
  return entries->stream().collect(Collectors::toMap(
      [&](e) { e::getKey(); }, [&](e) { e::getValue()->get(); }));
}

void MergePolicy::OneMergeProgress::setMergeThread(shared_ptr<Thread> owner)
{
  assert(this->owner == nullptr);
  this->owner = owner;
}

MergePolicy::OneMerge::OneMerge(
    deque<std::shared_ptr<SegmentCommitInfo>> &segments)
    : segments(deque<>(segments)),
      mergeProgress(make_shared<OneMergeProgress>()), totalMaxDoc(count)
{
  if (0 == segments.size()) {
    throw runtime_error(L"segments must include at least one segment");
  }
  // clone the deque, as the in deque may be based off original SegmentInfos and
  // may be modified
  int count = 0;
  for (auto info : segments) {
    count += info->info->maxDoc();
  }
}

void MergePolicy::OneMerge::mergeInit() 
{
  mergeProgress->setMergeThread(Thread::currentThread());
}

void MergePolicy::OneMerge::mergeFinished()  {}

shared_ptr<CodecReader> MergePolicy::OneMerge::wrapForMerge(
    shared_ptr<CodecReader> reader) 
{
  return reader;
}

void MergePolicy::OneMerge::setMergeInfo(shared_ptr<SegmentCommitInfo> info)
{
  this->info = info;
}

shared_ptr<SegmentCommitInfo> MergePolicy::OneMerge::getMergeInfo()
{
  return info;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void MergePolicy::OneMerge::setException(runtime_error error)
{
  this->error = error;
}

// C++ WARNING: The following method was originally marked 'synchronized':
runtime_error MergePolicy::OneMerge::getException() { return error; }

wstring MergePolicy::OneMerge::segString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  constexpr int numSegments = segments.size();
  for (int i = 0; i < numSegments; i++) {
    if (i > 0) {
      b->append(L' ');
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    b->append(segments[i]->toString());
  }
  if (info != nullptr) {
    b->append(L" into ")->append(info->info->name);
  }
  if (maxNumSegments != -1) {
    b->append(L" [maxNumSegments=" + to_wstring(maxNumSegments) + L"]");
  }
  if (isAborted()) {
    b->append(L" [ABORTED]");
  }
  return b->toString();
}

int64_t MergePolicy::OneMerge::totalBytesSize() { return totalMergeBytes; }

int MergePolicy::OneMerge::totalNumDocs()
{
  int total = 0;
  for (auto info : segments) {
    total += info->info->maxDoc();
  }
  return total;
}

shared_ptr<MergeInfo> MergePolicy::OneMerge::getStoreMergeInfo()
{
  return make_shared<MergeInfo>(totalMaxDoc, estimatedMergeBytes, isExternal,
                                maxNumSegments);
}

bool MergePolicy::OneMerge::isAborted() { return mergeProgress->isAborted(); }

void MergePolicy::OneMerge::setAborted() { this->mergeProgress->abort(); }

void MergePolicy::OneMerge::checkAborted() 
{
  if (isAborted()) {
    throw make_shared<MergePolicy::MergeAbortedException>(
        L"merge is aborted: " + segString());
  }
}

shared_ptr<OneMergeProgress> MergePolicy::OneMerge::getMergeProgress()
{
  return mergeProgress;
}

MergePolicy::MergeSpecification::MergeSpecification() {}

void MergePolicy::MergeSpecification::add(shared_ptr<OneMerge> merge)
{
  merges.push_back(merge);
}

wstring MergePolicy::MergeSpecification::segString(shared_ptr<Directory> dir)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  b->append(L"MergeSpec:\n");
  constexpr int count = merges.size();
  for (int i = 0; i < count; i++) {
    b->append(L"  ")->append(1 + i)->append(L": ")->append(
        merges[i]->segString());
  }
  return b->toString();
}

MergePolicy::MergeException::MergeException(const wstring &message,
                                            shared_ptr<Directory> dir)
    : RuntimeException(message)
{
  this->dir = dir;
}

MergePolicy::MergeException::MergeException(runtime_error exc,
                                            shared_ptr<Directory> dir)
    : RuntimeException(exc)
{
  this->dir = dir;
}

shared_ptr<Directory> MergePolicy::MergeException::getDirectory()
{
  return dir;
}

MergePolicy::MergeAbortedException::MergeAbortedException()
    : java::io::IOException(L"merge is aborted")
{
}

MergePolicy::MergeAbortedException::MergeAbortedException(
    const wstring &message)
    : java::io::IOException(message)
{
}

MergePolicy::MergePolicy()
    : MergePolicy(DEFAULT_NO_CFS_RATIO, DEFAULT_MAX_CFS_SEGMENT_SIZE)
{
}

MergePolicy::MergePolicy(double defaultNoCFSRatio,
                         int64_t defaultMaxCFSSegmentSize)
{
  this->noCFSRatio = defaultNoCFSRatio;
  this->maxCFSSegmentSize = defaultMaxCFSSegmentSize;
}

bool MergePolicy::useCompoundFile(
    shared_ptr<SegmentInfos> infos, shared_ptr<SegmentCommitInfo> mergedInfo,
    shared_ptr<MergeContext> mergeContext) 
{
  if (getNoCFSRatio() == 0.0) {
    return false;
  }
  int64_t mergedInfoSize = size(mergedInfo, mergeContext);
  if (mergedInfoSize > maxCFSSegmentSize) {
    return false;
  }
  if (getNoCFSRatio() >= 1.0) {
    return true;
  }
  int64_t totalSize = 0;
  for (auto info : infos) {
    totalSize += size(info, mergeContext);
  }
  return mergedInfoSize <= getNoCFSRatio() * totalSize;
}

int64_t
MergePolicy::size(shared_ptr<SegmentCommitInfo> info,
                  shared_ptr<MergeContext> mergeContext) 
{
  int64_t byteSize = info->sizeInBytes();
  int delCount = mergeContext->numDeletesToMerge(info);
  assert((assertDelCount(delCount, info)));
  double delRatio = info->info->maxDoc() <= 0
                        ? 0.0f
                        : static_cast<float>(delCount) /
                              static_cast<float>(info->info->maxDoc());
  assert(delRatio <= 1.0);
  return (info->info->maxDoc() <= 0
              ? byteSize
              : static_cast<int64_t>(byteSize * (1.0 - delRatio)));
}

bool MergePolicy::assertDelCount(int delCount,
                                 shared_ptr<SegmentCommitInfo> info)
{
  assert(
      (delCount >= 0, L"delCount must be positive: " + to_wstring(delCount)));
  assert((delCount <= info->info->maxDoc(),
          L"delCount: " + to_wstring(delCount) + L" must be leq than maxDoc: " +
              to_wstring(info->info->maxDoc())));
  return true;
}

bool MergePolicy::isMerged(
    shared_ptr<SegmentInfos> infos, shared_ptr<SegmentCommitInfo> info,
    shared_ptr<MergeContext> mergeContext) 
{
  assert(mergeContext != nullptr);
  int delCount = mergeContext->numDeletesToMerge(info);
  assert((assertDelCount(delCount, info)));
  return delCount == 0 && useCompoundFile(infos, info, mergeContext) ==
                              info->info->getUseCompoundFile();
}

double MergePolicy::getNoCFSRatio() { return noCFSRatio; }

void MergePolicy::setNoCFSRatio(double noCFSRatio)
{
  if (noCFSRatio < 0.0 || noCFSRatio > 1.0) {
    throw invalid_argument(L"noCFSRatio must be 0.0 to 1.0 inclusive; got " +
                           to_wstring(noCFSRatio));
  }
  this->noCFSRatio = noCFSRatio;
}

double MergePolicy::getMaxCFSSegmentSizeMB()
{
  return maxCFSSegmentSize / 1024 / 1024.0;
}

void MergePolicy::setMaxCFSSegmentSizeMB(double v)
{
  if (v < 0.0) {
    throw invalid_argument(L"maxCFSSegmentSizeMB must be >=0 (got " +
                           to_wstring(v) + L")");
  }
  v *= 1024 * 1024;
  this->maxCFSSegmentSize = v > numeric_limits<int64_t>::max()
                                ? numeric_limits<int64_t>::max()
                                : static_cast<int64_t>(v);
}

bool MergePolicy::keepFullyDeletedSegment(
    IOSupplier<std::shared_ptr<CodecReader>>
        readerIOSupplier) 
{
  return false;
}

int MergePolicy::numDeletesToMerge(
    shared_ptr<SegmentCommitInfo> info, int delCount,
    IOSupplier<std::shared_ptr<CodecReader>> readerSupplier) 
{
  return delCount;
}

wstring
MergePolicy::segString(shared_ptr<MergeContext> mergeContext,
                       deque<std::shared_ptr<SegmentCommitInfo>> &infos)
{
  return StreamSupport::stream(infos.spliterator(), false)
      .map_obj([&](any info) {
        info->toString(mergeContext->numDeletedDocs(info) -
                       info::getDelCount());
      })
      .collect(Collectors::joining(L" "));
}

void MergePolicy::message(const wstring &message,
                          shared_ptr<MergeContext> mergeContext)
{
  if (verbose(mergeContext)) {
    mergeContext->getInfoStream()->message(L"MP", message);
  }
}

bool MergePolicy::verbose(shared_ptr<MergeContext> mergeContext)
{
  return mergeContext->getInfoStream()->isEnabled(L"MP");
}
} // namespace org::apache::lucene::index