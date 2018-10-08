using namespace std;

#include "RateLimiter.h"

namespace org::apache::lucene::store
{
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

RateLimiter::SimpleRateLimiter::SimpleRateLimiter(double mbPerSec)
{
  setMBPerSec(mbPerSec);
  lastNS = System::nanoTime();
}

void RateLimiter::SimpleRateLimiter::setMBPerSec(double mbPerSec)
{
  this->mbPerSec = mbPerSec;
  minPauseCheckBytes = static_cast<int64_t>((MIN_PAUSE_CHECK_MSEC / 1000.0) *
                                              mbPerSec * 1024 * 1024);
}

int64_t RateLimiter::SimpleRateLimiter::getMinPauseCheckBytes()
{
  return minPauseCheckBytes;
}

double RateLimiter::SimpleRateLimiter::getMBPerSec() { return this->mbPerSec; }

int64_t RateLimiter::SimpleRateLimiter::pause(int64_t bytes)
{

  int64_t startNS = System::nanoTime();

  double secondsToPause = (bytes / 1024.0 / 1024.0) / mbPerSec;

  int64_t targetNS;

  // Sync'd to read + write lastNS:
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {

    // Time we should sleep until; this is purely instantaneous
    // rate (just adds seconds onto the last time we had paused to);
    // maybe we should also offer decayed recent history one?
    targetNS = lastNS + static_cast<int64_t>(1000000000 * secondsToPause);

    if (startNS >= targetNS) {
      // OK, current time is already beyond the target sleep time,
      // no pausing to do.

      // Set to startNS, not targetNS, to enforce the instant rate, not
      // the "averaaged over all history" rate:
      lastNS = startNS;
      return 0;
    }

    lastNS = targetNS;
  }

  int64_t curNS = startNS;

  // While loop because Thread.sleep doesn't always sleep
  // enough:
  while (true) {
    constexpr int64_t pauseNS = targetNS - curNS;
    if (pauseNS > 0) {
      try {
        // NOTE: except maybe on real-time JVMs, minimum realistic sleep time
        // is 1 msec; if you pass just 1 nsec the default impl rounds
        // this up to 1 msec:
        int sleepNS;
        int sleepMS;
        if (pauseNS > 100000LL * numeric_limits<int>::max()) {
          // Not really practical (sleeping for 25 days) but we shouldn't
          // overflow int:
          sleepMS = numeric_limits<int>::max();
          sleepNS = 0;
        } else {
          sleepMS = static_cast<int>(pauseNS / 1000000);
          sleepNS = static_cast<int>(pauseNS % 1000000);
        }
        Thread::sleep(sleepMS, sleepNS);
      } catch (const InterruptedException &ie) {
        throw make_shared<ThreadInterruptedException>(ie);
      }
      curNS = System::nanoTime();
      continue;
    }
    break;
  }

  return curNS - startNS;
}
} // namespace org::apache::lucene::store