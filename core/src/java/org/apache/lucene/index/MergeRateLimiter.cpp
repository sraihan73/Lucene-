using namespace std;

#include "MergeRateLimiter.h"

namespace org::apache::lucene::index
{
using RateLimiter = org::apache::lucene::store::RateLimiter;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using OneMergeProgress =
    org::apache::lucene::index::MergePolicy::OneMergeProgress;
using PauseReason =
    org::apache::lucene::index::MergePolicy::OneMergeProgress::PauseReason;

MergeRateLimiter::MergeRateLimiter(shared_ptr<OneMergeProgress> mergeProgress)
    : mergeProgress(mergeProgress)
{
  // Initially no IO limit; use setter here so minPauseCheckBytes is set:
  setMBPerSec(numeric_limits<double>::infinity());
}

void MergeRateLimiter::setMBPerSec(double mbPerSec)
{
  // Synchronized to make updates to mbPerSec and minPauseCheckBytes atomic.
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    // 0.0 is allowed: it means the merge is paused
    if (mbPerSec < 0.0) {
      throw invalid_argument(L"mbPerSec must be positive; got: " +
                             to_wstring(mbPerSec));
    }
    this->mbPerSec = mbPerSec;

    // NOTE: Double.POSITIVE_INFINITY casts to Long.MAX_VALUE
    this->minPauseCheckBytes = min(
        1024 * 1024, static_cast<int64_t>((MIN_PAUSE_CHECK_MSEC / 1000.0) *
                                            mbPerSec * 1024 * 1024));
    assert(minPauseCheckBytes >= 0);
  }

  mergeProgress->wakeup();
}

double MergeRateLimiter::getMBPerSec() { return mbPerSec; }

int64_t MergeRateLimiter::getTotalBytesWritten()
{
  return totalBytesWritten->get();
}

int64_t MergeRateLimiter::pause(int64_t bytes) throw(
    MergePolicy::MergeAbortedException)
{
  totalBytesWritten->addAndGet(bytes);

  // While loop because we may wake up and check again when our rate limit
  // is changed while we were pausing:
  int64_t paused = 0;
  int64_t delta;
  while ((delta = maybePause(bytes, System::nanoTime())) >= 0) {
    // Keep waiting.
    paused += delta;
  }

  return paused;
}

int64_t MergeRateLimiter::getTotalStoppedNS()
{
  return mergeProgress->getPauseTimes()[OneMergeProgress::PauseReason::STOPPED];
}

int64_t MergeRateLimiter::getTotalPausedNS()
{
  return mergeProgress->getPauseTimes()[OneMergeProgress::PauseReason::PAUSED];
}

int64_t MergeRateLimiter::maybePause(int64_t bytes, int64_t curNS) throw(
    MergePolicy::MergeAbortedException)
{
  // Now is a good time to abort the merge:
  if (mergeProgress->isAborted()) {
    throw make_shared<MergePolicy::MergeAbortedException>(L"Merge aborted.");
  }

  double rate = mbPerSec; // read from volatile rate once.
  double secondsToPause = (bytes / 1024.0 / 1024.0) / rate;

  // Time we should sleep until; this is purely instantaneous
  // rate (just adds seconds onto the last time we had paused to);
  // maybe we should also offer decayed recent history one?
  int64_t targetNS =
      lastNS + static_cast<int64_t>(1000000000 * secondsToPause);

  int64_t curPauseNS = targetNS - curNS;

  // We don't bother with thread pausing if the pause is smaller than 2 msec.
  if (curPauseNS <= MIN_PAUSE_NS) {
    // Set to curNS, not targetNS, to enforce the instant rate, not
    // the "averaged over all history" rate:
    lastNS = curNS;
    return -1;
  }

  // Defensive: don't sleep for too long; the loop above will call us again if
  // we should keep sleeping and the rate may be adjusted in between.
  if (curPauseNS > MAX_PAUSE_NS) {
    curPauseNS = MAX_PAUSE_NS;
  }

  int64_t start = System::nanoTime();
  try {
    mergeProgress->pauseNanos(curPauseNS,
                              rate == 0.0
                                  ? OneMergeProgress::PauseReason::STOPPED
                                  : OneMergeProgress::PauseReason::PAUSED,
                              [&]() { return rate == mbPerSec; });
  } catch (const InterruptedException &ie) {
    throw make_shared<ThreadInterruptedException>(ie);
  }
  return System::nanoTime() - start;
}

int64_t MergeRateLimiter::getMinPauseCheckBytes()
{
  return minPauseCheckBytes;
}
} // namespace org::apache::lucene::index