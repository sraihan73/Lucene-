using namespace std;

#include "ConcurrentMergeScheduler.h"

namespace org::apache::lucene::index
{
using OneMerge = org::apache::lucene::index::MergePolicy::OneMerge;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RateLimitedIndexOutput =
    org::apache::lucene::store::RateLimitedIndexOutput;
using RateLimiter = org::apache::lucene::store::RateLimiter;
using CollectionUtil = org::apache::lucene::util::CollectionUtil;
using IOUtils = org::apache::lucene::util::IOUtils;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
const wstring ConcurrentMergeScheduler::DEFAULT_CPU_CORE_COUNT_PROPERTY =
    L"lucene.cms.override_core_count";
const wstring ConcurrentMergeScheduler::DEFAULT_SPINS_PROPERTY =
    L"lucene.cms.override_spins";

ConcurrentMergeScheduler::ConcurrentMergeScheduler() {}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::setMaxMergesAndThreads(int maxMergeCount,
                                                      int maxThreadCount)
{
  if (maxMergeCount == AUTO_DETECT_MERGES_AND_THREADS &&
      maxThreadCount == AUTO_DETECT_MERGES_AND_THREADS) {
    // OK
    this->maxMergeCount = AUTO_DETECT_MERGES_AND_THREADS;
    this->maxThreadCount = AUTO_DETECT_MERGES_AND_THREADS;
  } else if (maxMergeCount == AUTO_DETECT_MERGES_AND_THREADS) {
    throw invalid_argument(L"both maxMergeCount and maxThreadCount must be "
                           L"AUTO_DETECT_MERGES_AND_THREADS");
  } else if (maxThreadCount == AUTO_DETECT_MERGES_AND_THREADS) {
    throw invalid_argument(L"both maxMergeCount and maxThreadCount must be "
                           L"AUTO_DETECT_MERGES_AND_THREADS");
  } else {
    if (maxThreadCount < 1) {
      throw invalid_argument(L"maxThreadCount should be at least 1");
    }
    if (maxMergeCount < 1) {
      throw invalid_argument(L"maxMergeCount should be at least 1");
    }
    if (maxThreadCount > maxMergeCount) {
      throw invalid_argument(L"maxThreadCount should be <= maxMergeCount (= " +
                             to_wstring(maxMergeCount) + L")");
    }
    this->maxThreadCount = maxThreadCount;
    this->maxMergeCount = maxMergeCount;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::setDefaultMaxMergesAndThreads(bool spins)
{
  if (spins) {
    maxThreadCount = 1;
    maxMergeCount = 6;
  } else {
    int coreCount = Runtime::getRuntime().availableProcessors();

    // Let tests override this to help reproducing a failure on a machine that
    // has a different core count than the one where the test originally failed:
    try {
      wstring value = System::getProperty(DEFAULT_CPU_CORE_COUNT_PROPERTY);
      if (value != L"") {
        coreCount = stoi(value);
      }
    } catch (const runtime_error &ignored) {
    }

    maxThreadCount = max(1, min(4, coreCount / 2));
    maxMergeCount = maxThreadCount + 5;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::setForceMergeMBPerSec(double v)
{
  forceMergeMBPerSec = v;
  updateMergeThreads();
}

// C++ WARNING: The following method was originally marked 'synchronized':
double ConcurrentMergeScheduler::getForceMergeMBPerSec()
{
  return forceMergeMBPerSec;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::enableAutoIOThrottle()
{
  doAutoIOThrottle = true;
  targetMBPerSec = START_MB_PER_SEC;
  updateMergeThreads();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::disableAutoIOThrottle()
{
  doAutoIOThrottle = false;
  updateMergeThreads();
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ConcurrentMergeScheduler::getAutoIOThrottle() { return doAutoIOThrottle; }

// C++ WARNING: The following method was originally marked 'synchronized':
double ConcurrentMergeScheduler::getIORateLimitMBPerSec()
{
  if (doAutoIOThrottle) {
    return targetMBPerSec;
  } else {
    return numeric_limits<double>::infinity();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int ConcurrentMergeScheduler::getMaxThreadCount() { return maxThreadCount; }

// C++ WARNING: The following method was originally marked 'synchronized':
int ConcurrentMergeScheduler::getMaxMergeCount() { return maxMergeCount; }

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::removeMergeThread()
{
  shared_ptr<Thread> currentThread = Thread::currentThread();
  // Paranoia: don't trust Thread.equals:
  for (int i = 0; i < mergeThreads.size(); i++) {
    if (mergeThreads[i] == currentThread) {
      mergeThreads.erase(mergeThreads.begin() + i);
      return;
    }
  }

  assert((false, L"merge thread " + currentThread + L" was not found"));
}

shared_ptr<Directory>
ConcurrentMergeScheduler::wrapForMerge(shared_ptr<OneMerge> merge,
                                       shared_ptr<Directory> in_)
{
  shared_ptr<Thread> mergeThread = Thread::currentThread();
  if (!MergeThread::typeid->isInstance(mergeThread)) {
    throw make_shared<AssertionError>(
        L"wrapForMerge should be called from MergeThread. Current thread: " +
        mergeThread);
  }

  // Return a wrapped Directory which has rate-limited output.
  shared_ptr<RateLimiter> rateLimiter =
      (std::static_pointer_cast<MergeThread>(mergeThread))->rateLimiter;
  return make_shared<FilterDirectoryAnonymousInnerClass>(
      shared_from_this(), in_, mergeThread, rateLimiter);
}

ConcurrentMergeScheduler::FilterDirectoryAnonymousInnerClass::
    FilterDirectoryAnonymousInnerClass(
        shared_ptr<ConcurrentMergeScheduler> outerInstance,
        shared_ptr<Directory> in_, shared_ptr<Thread> mergeThread,
        shared_ptr<RateLimiter> rateLimiter)
    : org::apache::lucene::store::FilterDirectory(in_)
{
  this->outerInstance = outerInstance;
  this->in_ = in_;
  this->mergeThread = mergeThread;
  this->rateLimiter = rateLimiter;
}

shared_ptr<IndexOutput>
ConcurrentMergeScheduler::FilterDirectoryAnonymousInnerClass::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  ensureOpen();

  // This Directory is only supposed to be used during merging,
  // so all writes should have MERGE context, else there is a bug
  // somewhere that is failing to pass down the right IOContext:
  assert((context->context == IOContext::Context::MERGE,
          L"got context=" + context->context));

  // Because rateLimiter is bound to a particular merge thread, this method
  // should always be called from that context. Verify this.
  assert((mergeThread == Thread::currentThread(),
          L"Not the same merge thread, current="));
  +Thread::currentThread() + L", expected=" + mergeThread;

  return make_shared<RateLimitedIndexOutput>(rateLimiter,
                                             in_->createOutput(name, context));
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::updateMergeThreads()
{

  // Only look at threads that are alive & not in the
  // process of stopping (ie have an active merge):
  const deque<std::shared_ptr<MergeThread>> activeMerges =
      deque<std::shared_ptr<MergeThread>>();

  int threadIdx = 0;
  while (threadIdx < mergeThreads.size()) {
    shared_ptr<MergeThread> *const mergeThread = mergeThreads[threadIdx];
    if (!mergeThread->isAlive()) {
      // Prune any dead threads
      mergeThreads.erase(mergeThreads.begin() + threadIdx);
      continue;
    }
    activeMerges.push_back(mergeThread);
    threadIdx++;
  }

  // Sort the merge threads, largest first:
  CollectionUtil::timSort(activeMerges);

  constexpr int activeMergeCount = activeMerges.size();

  int bigMergeCount = 0;

  for (threadIdx = activeMergeCount - 1; threadIdx >= 0; threadIdx--) {
    shared_ptr<MergeThread> mergeThread = activeMerges[threadIdx];
    if (mergeThread->merge->estimatedMergeBytes >
        MIN_BIG_MERGE_MB * 1024 * 1024) {
      bigMergeCount = 1 + threadIdx;
      break;
    }
  }

  int64_t now = System::nanoTime();

  shared_ptr<StringBuilder> message;
  if (verbose()) {
    message = make_shared<StringBuilder>();
    message->append(wstring::format(
        Locale::ROOT,
        L"updateMergeThreads ioThrottle=%s targetMBPerSec=%.1f MB/sec",
        doAutoIOThrottle, targetMBPerSec));
  } else {
    message.reset();
  }

  for (threadIdx = 0; threadIdx < activeMergeCount; threadIdx++) {
    shared_ptr<MergeThread> mergeThread = activeMerges[threadIdx];

    shared_ptr<OneMerge> merge = mergeThread->merge;

    // pause the thread if maxThreadCount is smaller than the number of merge
    // threads.
    constexpr bool doPause = threadIdx < bigMergeCount - maxThreadCount;

    double newMBPerSec;
    if (doPause) {
      newMBPerSec = 0.0;
    } else if (merge->maxNumSegments != -1) {
      newMBPerSec = forceMergeMBPerSec;
    } else if (doAutoIOThrottle == false) {
      newMBPerSec = numeric_limits<double>::infinity();
    } else if (merge->estimatedMergeBytes < MIN_BIG_MERGE_MB * 1024 * 1024) {
      // Don't rate limit small merges:
      newMBPerSec = numeric_limits<double>::infinity();
    } else {
      newMBPerSec = targetMBPerSec;
    }

    shared_ptr<MergeRateLimiter> rateLimiter = mergeThread->rateLimiter;
    double curMBPerSec = rateLimiter->getMBPerSec();

    if (verbose()) {
      int64_t mergeStartNS = merge->mergeStartNS;
      if (mergeStartNS == -1) {
        // IndexWriter didn't start the merge yet:
        mergeStartNS = now;
      }
      message->append(L'\n');
      message->append(wstring::format(
          Locale::ROOT,
          L"merge thread %s estSize=%.1f MB (written=%.1f MB) runTime=%.1fs "
          L"(stopped=%.1fs, paused=%.1fs) rate=%s\n",
          mergeThread->getName(), bytesToMB(merge->estimatedMergeBytes),
          bytesToMB(rateLimiter->getTotalBytesWritten()),
          nsToSec(now - mergeStartNS),
          nsToSec(rateLimiter->getTotalStoppedNS()),
          nsToSec(rateLimiter->getTotalPausedNS()),
          rateToString(rateLimiter->getMBPerSec())));

      if (newMBPerSec != curMBPerSec) {
        if (newMBPerSec == 0.0) {
          message->append(L"  now stop");
        } else if (curMBPerSec == 0.0) {
          if (newMBPerSec == numeric_limits<double>::infinity()) {
            message->append(L"  now resume");
          } else {
            message->append(wstring::format(
                Locale::ROOT, L"  now resume to %.1f MB/sec", newMBPerSec));
          }
        } else {
          message->append(wstring::format(
              Locale::ROOT, L"  now change from %.1f MB/sec to %.1f MB/sec",
              curMBPerSec, newMBPerSec));
        }
      } else if (curMBPerSec == 0.0) {
        message->append(L"  leave stopped");
      } else {
        message->append(wstring::format(
            Locale::ROOT, L"  leave running at %.1f MB/sec", curMBPerSec));
      }
    }

    rateLimiter->setMBPerSec(newMBPerSec);
  }
  if (verbose()) {
    this->message(message->toString());
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::initDynamicDefaults(
    shared_ptr<IndexWriter> writer) 
{
  if (maxThreadCount == AUTO_DETECT_MERGES_AND_THREADS) {
    bool spins = IOUtils::spins(writer->getDirectory());

    // Let tests override this to help reproducing a failure on a machine that
    // has a different core count than the one where the test originally failed:
    try {
      wstring value = System::getProperty(DEFAULT_SPINS_PROPERTY);
      if (value != L"") {
        spins = StringHelper::fromString<bool>(value);
      }
    } catch (const runtime_error &ignored) {
      // that's fine we might hit a SecurityException etc. here just continue
    }
    setDefaultMaxMergesAndThreads(spins);
    if (verbose()) {
      message(L"initDynamicDefaults spins=" + StringHelper::toString(spins) +
              L" maxThreadCount=" + to_wstring(maxThreadCount) +
              L" maxMergeCount=" + to_wstring(maxMergeCount));
    }
  }
}

wstring ConcurrentMergeScheduler::rateToString(double mbPerSec)
{
  if (mbPerSec == 0.0) {
    return L"stopped";
  } else if (mbPerSec == numeric_limits<double>::infinity()) {
    return L"unlimited";
  } else {
    return wstring::format(Locale::ROOT, L"%.1f MB/sec", mbPerSec);
  }
}

ConcurrentMergeScheduler::~ConcurrentMergeScheduler() { sync(); }

void ConcurrentMergeScheduler::sync()
{
  bool interrupted = false;
  try {
    while (true) {
      shared_ptr<MergeThread> toSync = nullptr;
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {
        for (auto t : mergeThreads) {
          // In case a merge thread is calling us, don't try to sync on
          // itself, since that will never finish!
          if (t->isAlive() && t != Thread::currentThread()) {
            toSync = t;
            break;
          }
        }
      }
      if (toSync != nullptr) {
        try {
          toSync->join();
        } catch (const InterruptedException &ie) {
          // ignore this Exception, we will retry until all threads are dead
          interrupted = true;
        }
      } else {
        break;
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // finally, restore interrupt status:
    if (interrupted) {
      Thread::currentThread().interrupt();
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int ConcurrentMergeScheduler::mergeThreadCount()
{
  shared_ptr<Thread> currentThread = Thread::currentThread();
  int count = 0;
  for (auto mergeThread : mergeThreads) {
    if (currentThread != mergeThread && mergeThread->isAlive() &&
        mergeThread->merge->isAborted() == false) {
      count++;
    }
  }
  return count;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::merge(shared_ptr<IndexWriter> writer,
                                     MergeTrigger trigger,
                                     bool newMergesFound) 
{

  assert(!Thread::holdsLock(writer));

  initDynamicDefaults(writer);

  if (trigger == MergeTrigger::CLOSING) {
    // Disable throttling on close:
    targetMBPerSec = MAX_MERGE_MB_PER_SEC;
    updateMergeThreads();
  }

  // First, quickly run through the newly proposed merges
  // and add any orthogonal merges (ie a merge not
  // involving segments already pending to be merged) to
  // the queue.  If we are way behind on merging, many of
  // these newly proposed merges will likely already be
  // registered.

  if (verbose()) {
    message(L"now merge");
    message(L"  index: " + writer->segString());
  }

  // Iterate, pulling from the IndexWriter's queue of
  // pending merges, until it's empty:
  while (true) {

    if (maybeStall(writer) == false) {
      break;
    }

    shared_ptr<OneMerge> merge = writer->getNextMerge();
    if (merge == nullptr) {
      if (verbose()) {
        message(L"  no more merges pending; now return");
      }
      return;
    }

    bool success = false;
    try {
      if (verbose()) {
        message(L"  consider merge " + writer->segString(merge->segments));
      }

      // OK to spawn a new merge thread to handle this
      // merge:
      shared_ptr<MergeThread> *const newMergeThread =
          getMergeThread(writer, merge);
      mergeThreads.push_back(newMergeThread);

      updateIOThrottle(newMergeThread->merge, newMergeThread->rateLimiter);

      if (verbose()) {
        message(L"    launch new thread [" + newMergeThread->getName() + L"]");
      }

      newMergeThread->start();
      updateMergeThreads();

      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        writer->mergeFinish(merge);
      }
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool ConcurrentMergeScheduler::maybeStall(shared_ptr<IndexWriter> writer)
{
  int64_t startStallTime = 0;
  while (writer->hasPendingMerges() && mergeThreadCount() >= maxMergeCount) {

    // This means merging has fallen too far behind: we
    // have already created maxMergeCount threads, and
    // now there's at least one more merge pending.
    // Note that only maxThreadCount of
    // those created merge threads will actually be
    // running; the rest will be paused (see
    // updateMergeThreads).  We stall this producer
    // thread to prevent creation of new segments,
    // until merging has caught up:

    if (find(mergeThreads.begin(), mergeThreads.end(),
             Thread::currentThread()) != mergeThreads.end()))
      {
        // Never stall a merge thread since this blocks the thread from
        // finishing and calling updateMergeThreads, and blocking it
        // accomplishes nothing anyway (it's not really a segment producer):
        return false;
      }

    if (verbose() && startStallTime == 0) {
      message(L"    too many merges; stalling...");
    }
    startStallTime = System::currentTimeMillis();
    doStall();
  }

  if (verbose() && startStallTime != 0) {
    message(L"  stalled for " + (System::currentTimeMillis() - startStallTime) +
            L" msec");
  }

  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::doStall()
{
  try {
    // Defensively wait for only .25 seconds in case we are missing a
    // .notify/All somewhere:
    wait(250);
  } catch (const InterruptedException &ie) {
    throw make_shared<ThreadInterruptedException>(ie);
  }
}

void ConcurrentMergeScheduler::doMerge(
    shared_ptr<IndexWriter> writer,
    shared_ptr<OneMerge> merge) 
{
  writer->merge(merge);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<MergeThread> ConcurrentMergeScheduler::getMergeThread(
    shared_ptr<IndexWriter> writer,
    shared_ptr<OneMerge> merge) 
{
  shared_ptr<MergeThread> *const thread =
      make_shared<MergeThread>(shared_from_this(), writer, merge);
  thread->setDaemon(true);
  thread->setName(L"Lucene Merge Thread #" + to_wstring(mergeThreadCount_++));
  return thread;
}

ConcurrentMergeScheduler::MergeThread::MergeThread(
    shared_ptr<ConcurrentMergeScheduler> outerInstance,
    shared_ptr<IndexWriter> writer, shared_ptr<OneMerge> merge)
    : writer(writer), merge(merge),
      rateLimiter(make_shared<MergeRateLimiter>(merge->getMergeProgress())),
      outerInstance(outerInstance)
{
}

int ConcurrentMergeScheduler::MergeThread::compareTo(
    shared_ptr<MergeThread> other)
{
  // Larger merges sort first:
  return Long::compare(other->merge->estimatedMergeBytes,
                       merge->estimatedMergeBytes);
}

void ConcurrentMergeScheduler::MergeThread::run()
{
  try {
    if (outerInstance->verbose()) {
      outerInstance->message(L"  merge thread: start");
    }

    outerInstance->doMerge(writer, merge);

    if (outerInstance->verbose()) {
      outerInstance->message(L"  merge thread: done");
    }

    // Let CMS run new merges if necessary:
    try {
      outerInstance->merge(writer, MergeTrigger::MERGE_FINISHED, true);
    } catch (const AlreadyClosedException &ace) {
      // OK
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }

  } catch (const runtime_error &exc) {

    if (std::dynamic_pointer_cast<MergePolicy::MergeAbortedException>(exc) !=
        nullptr) {
      // OK to ignore
    } else if (outerInstance->suppressExceptions == false) {
      // suppressExceptions is normally only set during
      // testing.
      outerInstance->handleMergeException(writer->getDirectory(), exc);
    }

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    {
      lock_guard<mutex> lock(outerInstance);
      outerInstance->removeMergeThread();

      outerInstance->updateMergeThreads();

      // In case we had stalled indexing, we can now wake up
      // and possibly unstall:
      outerInstance->notifyAll();
    }
  }
}

void ConcurrentMergeScheduler::handleMergeException(shared_ptr<Directory> dir,
                                                    runtime_error exc)
{
  throw make_shared<MergePolicy::MergeException>(exc, dir);
}

void ConcurrentMergeScheduler::setSuppressExceptions()
{
  if (verbose()) {
    message(L"will suppress merge exceptions");
  }
  suppressExceptions = true;
}

void ConcurrentMergeScheduler::clearSuppressExceptions()
{
  if (verbose()) {
    message(L"will not suppress merge exceptions");
  }
  suppressExceptions = false;
}

wstring ConcurrentMergeScheduler::toString()
{
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(getClass().getSimpleName() + L": ");
  sb->append(L"maxThreadCount=")->append(maxThreadCount)->append(L", ");
  sb->append(L"maxMergeCount=")->append(maxMergeCount)->append(L", ");
  sb->append(L"ioThrottle=")->append(doAutoIOThrottle);
  return sb->toString();
}

bool ConcurrentMergeScheduler::isBacklog(int64_t now,
                                         shared_ptr<OneMerge> merge)
{
  double mergeMB = bytesToMB(merge->estimatedMergeBytes);
  for (auto mergeThread : mergeThreads) {
    int64_t mergeStartNS = mergeThread->merge->mergeStartNS;
    if (mergeThread->isAlive() && mergeThread->merge != merge &&
        mergeStartNS != -1 &&
        mergeThread->merge->estimatedMergeBytes >=
            MIN_BIG_MERGE_MB * 1024 * 1024 &&
        nsToSec(now - mergeStartNS) > 3.0) {
      double otherMergeMB = bytesToMB(mergeThread->merge->estimatedMergeBytes);
      double ratio = otherMergeMB / mergeMB;
      if (ratio > 0.3 && ratio < 3.0) {
        return true;
      }
    }
  }

  return false;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ConcurrentMergeScheduler::updateIOThrottle(
    shared_ptr<OneMerge> newMerge,
    shared_ptr<MergeRateLimiter> rateLimiter) 
{
  if (doAutoIOThrottle == false) {
    return;
  }

  double mergeMB = bytesToMB(newMerge->estimatedMergeBytes);
  if (mergeMB < MIN_BIG_MERGE_MB) {
    // Only watch non-trivial merges for throttling; this is safe because the MP
    // must eventually have to do larger merges:
    return;
  }

  int64_t now = System::nanoTime();

  // Simplistic closed-loop feedback control: if we find any other similarly
  // sized merges running, then we are falling behind, so we bump up the
  // IO throttle, else we lower it:
  bool newBacklog = isBacklog(now, newMerge);

  bool curBacklog = false;

  if (newBacklog == false) {
    if (mergeThreads.size() > maxThreadCount) {
      // If there are already more than the maximum merge threads allowed, count
      // that as backlog:
      curBacklog = true;
    } else {
      // Now see if any still-running merges are backlog'd:
      for (auto mergeThread : mergeThreads) {
        if (isBacklog(now, mergeThread->merge)) {
          curBacklog = true;
          break;
        }
      }
    }
  }

  double curMBPerSec = targetMBPerSec;

  if (newBacklog) {
    // This new merge adds to the backlog: increase IO throttle by 20%
    targetMBPerSec *= 1.20;
    if (targetMBPerSec > MAX_MERGE_MB_PER_SEC) {
      targetMBPerSec = MAX_MERGE_MB_PER_SEC;
    }
    if (verbose()) {
      if (curMBPerSec == targetMBPerSec) {
        message(wstring::format(Locale::ROOT,
                                L"io throttle: new merge backlog; leave IO "
                                L"rate at ceiling %.1f MB/sec",
                                targetMBPerSec));
      } else {
        message(wstring::format(
            Locale::ROOT,
            L"io throttle: new merge backlog; increase IO rate to %.1f MB/sec",
            targetMBPerSec));
      }
    }
  } else if (curBacklog) {
    // We still have an existing backlog; leave the rate as is:
    if (verbose()) {
      message(wstring::format(
          Locale::ROOT,
          L"io throttle: current merge backlog; leave IO rate at %.1f MB/sec",
          targetMBPerSec));
    }
  } else {
    // We are not falling behind: decrease IO throttle by 10%
    targetMBPerSec /= 1.10;
    if (targetMBPerSec < MIN_MERGE_MB_PER_SEC) {
      targetMBPerSec = MIN_MERGE_MB_PER_SEC;
    }
    if (verbose()) {
      if (curMBPerSec == targetMBPerSec) {
        message(wstring::format(Locale::ROOT,
                                L"io throttle: no merge backlog; leave IO rate "
                                L"at floor %.1f MB/sec",
                                targetMBPerSec));
      } else {
        message(wstring::format(
            Locale::ROOT,
            L"io throttle: no merge backlog; decrease IO rate to %.1f MB/sec",
            targetMBPerSec));
      }
    }
  }

  double rate;

  if (newMerge->maxNumSegments != -1) {
    rate = forceMergeMBPerSec;
  } else {
    rate = targetMBPerSec;
  }
  rateLimiter->setMBPerSec(rate);
  targetMBPerSecChanged();
}

void ConcurrentMergeScheduler::targetMBPerSecChanged() {}

double ConcurrentMergeScheduler::nsToSec(int64_t ns)
{
  return ns / 1000000000.0;
}

double ConcurrentMergeScheduler::bytesToMB(int64_t bytes)
{
  return bytes / 1024.0 / 1024.0;
}
} // namespace org::apache::lucene::index