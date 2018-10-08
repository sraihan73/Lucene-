using namespace std;

#include "SameThreadExecutorService.h"

namespace org::apache::lucene::util
{

void SameThreadExecutorService::execute(Runnable command)
{
  checkShutdown();
  command();
}

deque<Runnable> SameThreadExecutorService::shutdownNow()
{
  shutdown();
  return Collections::emptyList();
}

void SameThreadExecutorService::shutdown() { this->shutdown_ = true; }

bool SameThreadExecutorService::isTerminated()
{
  // Simplified: we don't check for any threads hanging in execute (we could
  // introduce an atomic counter, but there seems to be no point).
  return shutdown_ == true;
}

bool SameThreadExecutorService::isShutdown() { return shutdown_ == true; }

bool SameThreadExecutorService::awaitTermination(
    int64_t timeout, shared_ptr<TimeUnit> unit) 
{
  // See comment in isTerminated();
  return true;
}

void SameThreadExecutorService::checkShutdown()
{
  if (shutdown_) {
    // C++ TODO: The following line could not be converted:
    throw java.util.concurrent.RejectedExecutionException(
        L"Executor is shut down.");
  }
}
} // namespace org::apache::lucene::util