using namespace std;

#include "QuickPatchThreadsFilter.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::ThreadFilter;

QuickPatchThreadsFilter::StaticConstructor::StaticConstructor()
{
  isJ9 = StringHelper::startsWith(Constants::JAVA_VENDOR, L"IBM");
}

QuickPatchThreadsFilter::StaticConstructor
    QuickPatchThreadsFilter::staticConstructor;

bool QuickPatchThreadsFilter::reject(shared_ptr<Thread> t)
{
  if (isJ9) {
    // LUCENE-6518
    if (L"ClassCache Reaper" == t->getName()) {
      return true;
    }

    // LUCENE-4736
    std::deque<std::shared_ptr<StackTraceElement>> stack = t->getStackTrace();
    if (stack.size() > 0 && stack[stack.size() - 1]->getClassName().equals(
                                L"java.util.Timer$TimerImpl")) {
      return true;
    }
  }
  return false;
}
} // namespace org::apache::lucene::util