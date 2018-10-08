using namespace std;

#include "DocumentsWriterPerThreadPool.h"

namespace org::apache::lucene::index
{
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

DocumentsWriterPerThreadPool::ThreadState::ThreadState(
    shared_ptr<DocumentsWriterPerThread> dpwt)
{
  this->dwpt = dpwt;
}

void DocumentsWriterPerThreadPool::ThreadState::reset()
{
  assert(this->isHeldByCurrentThread());
  this->dwpt.reset();
  this->bytesUsed = 0;
  this->flushPending = false;
}

bool DocumentsWriterPerThreadPool::ThreadState::isInitialized()
{
  assert(this->isHeldByCurrentThread());
  return dwpt != nullptr;
}

int64_t DocumentsWriterPerThreadPool::ThreadState::getBytesUsedPerThread()
{
  assert(this->isHeldByCurrentThread());
  // public for FlushPolicy
  return bytesUsed;
}

shared_ptr<DocumentsWriterPerThread>
DocumentsWriterPerThreadPool::ThreadState::getDocumentsWriterPerThread()
{
  assert(this->isHeldByCurrentThread());
  // public for FlushPolicy
  return dwpt;
}

bool DocumentsWriterPerThreadPool::ThreadState::isFlushPending()
{
  return flushPending;
}

// C++ WARNING: The following method was originally marked 'synchronized':
int DocumentsWriterPerThreadPool::getActiveThreadStateCount()
{
  return threadStates.size();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterPerThreadPool::setAbort() { aborted = true; }

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterPerThreadPool::clearAbort()
{
  aborted = false;
  notifyAll();
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<ThreadState> DocumentsWriterPerThreadPool::newThreadState()
{
  while (aborted) {
    try {
      wait();
    } catch (const InterruptedException &ie) {
      throw make_shared<ThreadInterruptedException>(ie);
    }
  }
  shared_ptr<ThreadState> threadState = make_shared<ThreadState>(nullptr);
  threadState->lock(); // lock so nobody else will get this ThreadState
  threadStates.push_back(threadState);
  return threadState;
}

shared_ptr<DocumentsWriterPerThread>
DocumentsWriterPerThreadPool::reset(shared_ptr<ThreadState> threadState)
{
  assert(threadState->isHeldByCurrentThread());
  shared_ptr<DocumentsWriterPerThread> *const dwpt = threadState->dwpt;
  threadState->reset();
  return dwpt;
}

void DocumentsWriterPerThreadPool::recycle(
    shared_ptr<DocumentsWriterPerThread> dwpt)
{
  // don't recycle DWPT by default
}

shared_ptr<ThreadState> DocumentsWriterPerThreadPool::getAndLock(
    shared_ptr<Thread> requestingThread,
    shared_ptr<DocumentsWriter> documentsWriter)
{
  shared_ptr<ThreadState> threadState = nullptr;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    if (freeList.empty()) {
      // ThreadState is already locked before return by this method:
      return newThreadState();
    } else {
      // Important that we are LIFO here! This way if number of concurrent
      // indexing threads was once high, but has now reduced, we only use a
      // limited number of thread states:
      threadState = freeList.pop_back();

      if (threadState->dwpt == nullptr) {
        // This thread-state is not initialized, e.g. it
        // was just flushed. See if we can instead find
        // another free thread state that already has docs
        // indexed. This way if incoming thread concurrency
        // has decreased, we don't leave docs
        // indefinitely buffered, tying up RAM.  This
        // will instead get those thread states flushed,
        // freeing up RAM for larger segment flushes:
        for (int i = 0; i < freeList.size(); i++) {
          shared_ptr<ThreadState> ts = freeList[i];
          if (ts->dwpt != nullptr) {
            // Use this one instead, and swap it with
            // the un-initialized one:
            freeList[i] = threadState;
            threadState = ts;
            break;
          }
        }
      }
    }
  }

  // This could take time, e.g. if the threadState is [briefly] checked for
  // flushing:
  threadState->lock();

  return threadState;
}

void DocumentsWriterPerThreadPool::release(shared_ptr<ThreadState> state)
{
  state->unlock();
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this()) { freeList.push_back(state); }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<ThreadState> DocumentsWriterPerThreadPool::getThreadState(int ord)
{
  return threadStates[ord];
}

// C++ WARNING: The following method was originally marked 'synchronized':
int DocumentsWriterPerThreadPool::getMaxThreadStates()
{
  return threadStates.size();
}
} // namespace org::apache::lucene::index