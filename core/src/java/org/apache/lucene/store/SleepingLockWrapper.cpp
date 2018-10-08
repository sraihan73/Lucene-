using namespace std;

#include "SleepingLockWrapper.h"

namespace org::apache::lucene::store
{
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
int64_t SleepingLockWrapper::DEFAULT_POLL_INTERVAL = 1000;

SleepingLockWrapper::SleepingLockWrapper(shared_ptr<Directory> delegate_,
                                         int64_t lockWaitTimeout)
    : SleepingLockWrapper(delegate_, lockWaitTimeout, DEFAULT_POLL_INTERVAL)
{
}

SleepingLockWrapper::SleepingLockWrapper(shared_ptr<Directory> delegate_,
                                         int64_t lockWaitTimeout,
                                         int64_t pollInterval)
    : FilterDirectory(delegate_), lockWaitTimeout(lockWaitTimeout),
      pollInterval(pollInterval)
{
  if (lockWaitTimeout < 0 && lockWaitTimeout != LOCK_OBTAIN_WAIT_FOREVER) {
    throw invalid_argument(
        L"lockWaitTimeout should be LOCK_OBTAIN_WAIT_FOREVER or a non-negative "
        L"number (got " +
        to_wstring(lockWaitTimeout) + L")");
  }
  if (pollInterval < 0) {
    throw invalid_argument(L"pollInterval must be a non-negative number (got " +
                           to_wstring(pollInterval) + L")");
  }
}

shared_ptr<Lock>
SleepingLockWrapper::obtainLock(const wstring &lockName) 
{
  shared_ptr<LockObtainFailedException> failureReason = nullptr;
  int64_t maxSleepCount = lockWaitTimeout / pollInterval;
  int64_t sleepCount = 0;

  do {
    try {
      return in_->obtainLock(lockName);
    } catch (const LockObtainFailedException &failed) {
      if (failureReason == nullptr) {
        failureReason = failed;
      }
    }
    try {
      delay(pollInterval);
    } catch (const InterruptedException &ie) {
      throw make_shared<ThreadInterruptedException>(ie);
    }
  } while (sleepCount++ < maxSleepCount ||
           lockWaitTimeout == LOCK_OBTAIN_WAIT_FOREVER);

  // we failed to obtain the lock in the required time
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring reason = L"Lock obtain timed out: " + this->toString();
  if (failureReason != nullptr) {
    reason += L": " + failureReason;
  }
  throw make_shared<LockObtainFailedException>(reason, failureReason);
}

wstring SleepingLockWrapper::toString()
{
  return L"SleepingLockWrapper(" + in_ + L")";
}
} // namespace org::apache::lucene::store