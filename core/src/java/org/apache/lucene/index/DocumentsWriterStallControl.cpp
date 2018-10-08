using namespace std;

#include "DocumentsWriterStallControl.h"

namespace org::apache::lucene::index
{
using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterStallControl::updateStalled(bool stalled)
{
  if (this->stalled != stalled) {
    this->stalled = stalled;
    if (stalled) {
      wasStalled_ = true;
    }
    notifyAll();
  }
}

void DocumentsWriterStallControl::waitIfStalled()
{
  if (stalled) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      if (stalled) { // react on the first wakeup call!
        // don't loop here, higher level logic will re-stall!
        try {
          incWaiters();
          // Defensive, in case we have a concurrency bug that fails to
          // .notify/All our thread: just wait for up to 1 second here, and let
          // caller re-stall if it's still needed:
          wait(1000);
          decrWaiters();
        } catch (const InterruptedException &e) {
          throw make_shared<ThreadInterruptedException>(e);
        }
      }
    }
  }
}

bool DocumentsWriterStallControl::anyStalledThreads() { return stalled; }

void DocumentsWriterStallControl::incWaiters()
{
  numWaiting++;
  assert((waiting.emplace(Thread::currentThread(), Boolean::TRUE) == nullptr));
  assert(numWaiting > 0);
}

void DocumentsWriterStallControl::decrWaiters()
{
  numWaiting--;
  assert(waiting.erase(Thread::currentThread()) != nullptr);
  assert(numWaiting >= 0);
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool DocumentsWriterStallControl::hasBlocked()
{ // for tests
  return numWaiting > 0;
}

bool DocumentsWriterStallControl::isHealthy()
{                  // for tests
  return !stalled; // volatile read!
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool DocumentsWriterStallControl::isThreadQueued(shared_ptr<Thread> t)
{ // for tests
  return waiting.find(t) != waiting.end();
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool DocumentsWriterStallControl::wasStalled()
{ // for tests
  return wasStalled_;
}
} // namespace org::apache::lucene::index