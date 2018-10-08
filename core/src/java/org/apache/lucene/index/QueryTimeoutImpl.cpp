using namespace std;

#include "QueryTimeoutImpl.h"

namespace org::apache::lucene::index
{
//    import static System.nanoTime;

QueryTimeoutImpl::QueryTimeoutImpl(int64_t timeAllowed)
{
  if (timeAllowed < 0LL) {
    timeAllowed = numeric_limits<int64_t>::max();
  }
  timeoutAt = nanoTime() + TimeUnit::NANOSECONDS::convert(
                               timeAllowed, TimeUnit::MILLISECONDS);
}

optional<int64_t> QueryTimeoutImpl::getTimeoutAt() { return timeoutAt; }

bool QueryTimeoutImpl::shouldExit()
{
  return timeoutAt && nanoTime() - timeoutAt > 0;
}

void QueryTimeoutImpl::reset() { timeoutAt = nullopt; }

wstring QueryTimeoutImpl::toString()
{
  return L"timeoutAt: " + timeoutAt + L" (System.nanoTime(): " + nanoTime() +
         L")";
}
} // namespace org::apache::lucene::index