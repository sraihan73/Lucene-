using namespace std;

#include "DocumentsWriterFlushControl.h"

namespace org::apache::lucene::index
{
using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using Accountable = org::apache::lucene::util::Accountable;
using InfoStream = org::apache::lucene::util::InfoStream;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;

DocumentsWriterFlushControl::DocumentsWriterFlushControl(
    shared_ptr<DocumentsWriter> documentsWriter,
    shared_ptr<LiveIndexWriterConfig> config)
    : hardMaxBytesPerDWPT(config->getRAMPerThreadHardLimitMB() * 1024 * 1024),
      stallControl(make_shared<DocumentsWriterStallControl>()),
      perThreadPool(documentsWriter->perThreadPool),
      flushPolicy(documentsWriter->flushPolicy),
      documentsWriter(documentsWriter), config(config),
      infoStream(config->getInfoStream())
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DocumentsWriterFlushControl::activeBytes() { return activeBytes_; }

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DocumentsWriterFlushControl::flushBytes() { return flushBytes_; }

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DocumentsWriterFlushControl::netBytes()
{
  return flushBytes_ + activeBytes_;
}

int64_t DocumentsWriterFlushControl::stallLimitBytes()
{
  constexpr double maxRamMB = config->getRAMBufferSizeMB();
  return maxRamMB != IndexWriterConfig::DISABLE_AUTO_FLUSH
             ? static_cast<int64_t>(2 * (maxRamMB * 1024 * 1024))
             : numeric_limits<int64_t>::max();
}

bool DocumentsWriterFlushControl::assertMemory()
{
  constexpr double maxRamMB = config->getRAMBufferSizeMB();
  // We can only assert if we have always been flushing by RAM usage; otherwise
  // the assert will false trip if e.g. the flush-by-doc-count * doc size was
  // large enough to use far more RAM than the sudden change to IWC's
  // maxRAMBufferSizeMB:
  if (maxRamMB != IndexWriterConfig::DISABLE_AUTO_FLUSH &&
      flushByRAMWasDisabled == false) {
    // for this assert we must be tolerant to ram buffer changes!
    maxConfiguredRamBuffer = max(maxRamMB, maxConfiguredRamBuffer);
    constexpr int64_t ram = flushBytes_ + activeBytes_;
    constexpr int64_t ramBufferBytes =
        static_cast<int64_t>(maxConfiguredRamBuffer * 1024 * 1024);
    // take peakDelta into account - worst case is that all flushing, pending
    // and blocked DWPT had maxMem and the last doc had the peakDelta

    // 2 * ramBufferBytes -> before we stall we need to cross the 2xRAM Buffer
    // border this is still a valid limit (numPending + numFlushingDWPT() +
    // numBlockedFlushes()) * peakDelta) -> those are the total number of DWPT
    // that are not active but not yet fully flushed all of them could
    // theoretically be taken out of the loop once they crossed the RAM buffer
    // and the last document was the peak delta (numDocsSinceStalled *
    // peakDelta) -> at any given time there could be n threads in flight that
    // crossed the stall control before we reached the limit and each of them
    // could hold a peak document
    constexpr int64_t expected =
        (2 * ramBufferBytes) +
        ((numPending + numFlushingDWPT() + numBlockedFlushes()) * peakDelta) +
        (numDocsSinceStalled * peakDelta);
    // the expected ram consumption is an upper bound at this point and not
    // really the expected consumption
    if (peakDelta < (ramBufferBytes >> 1)) {
      /*
       * if we are indexing with very low maxRamBuffer like 0.1MB memory can
       * easily overflow if we check out some DWPT based on docCount and have
       * several DWPT in flight indexing large documents (compared to the ram
       * buffer). This means that those DWPT and their threads will not hit
       * the stall control before asserting the memory which would in turn
       * fail. To prevent this we only assert if the the largest document seen
       * is smaller than the 1/2 of the maxRamBufferMB
       */
      assert((ram <= expected,
              L"actual mem: " + to_wstring(ram) + L" byte, expected mem: " +
                  to_wstring(expected) + L" byte, flush mem: " +
                  to_wstring(flushBytes_) + L", active mem: " +
                  to_wstring(activeBytes_) + L", pending DWPT: " +
                  to_wstring(numPending) + L", flushing DWPT: "));
      +to_wstring(numFlushingDWPT()) + L", blocked DWPT: " +
          to_wstring(numBlockedFlushes()) + L", peakDelta mem: " +
          to_wstring(peakDelta) + L" bytes, ramBufferBytes=" +
          to_wstring(ramBufferBytes) + L", maxConfiguredRamBuffer=" +
          to_wstring(maxConfiguredRamBuffer);
    }
  } else {
    flushByRAMWasDisabled = true;
  }
  return true;
}

void DocumentsWriterFlushControl::commitPerThreadBytes(
    shared_ptr<ThreadState> perThread)
{
  constexpr int64_t delta =
      perThread->dwpt->bytesUsed() - perThread->bytesUsed;
  perThread->bytesUsed += delta;
  /*
   * We need to differentiate here if we are pending since setFlushPending
   * moves the perThread memory to the flushBytes and we could be set to
   * pending during a delete
   */
  if (perThread->flushPending) {
    flushBytes_ += delta;
  } else {
    activeBytes_ += delta;
  }
  assert(updatePeaks(delta));
}

bool DocumentsWriterFlushControl::updatePeaks(int64_t delta)
{
  peakActiveBytes = max(peakActiveBytes, activeBytes_);
  peakFlushBytes = max(peakFlushBytes, flushBytes_);
  peakNetBytes = max(peakNetBytes, netBytes());
  peakDelta = max(peakDelta, delta);

  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DocumentsWriterPerThread>
DocumentsWriterFlushControl::doAfterDocument(shared_ptr<ThreadState> perThread,
                                             bool isUpdate)
{
  try {
    commitPerThreadBytes(perThread);
    if (!perThread->flushPending) {
      if (isUpdate) {
        flushPolicy->onUpdate(shared_from_this(), perThread);
      } else {
        flushPolicy->onInsert(shared_from_this(), perThread);
      }
      if (!perThread->flushPending &&
          perThread->bytesUsed > hardMaxBytesPerDWPT) {
        // Safety check to prevent a single DWPT exceeding its RAM limit. This
        // is super important since we can not address more than 2048 MB per
        // DWPT
        setFlushPending(perThread);
      }
    }
    return checkout(perThread, false);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    bool stalled = updateStallState();
    assert(assertNumDocsSinceStalled(stalled) && assertMemory());
  }
}

shared_ptr<DocumentsWriterPerThread>
DocumentsWriterFlushControl::checkout(shared_ptr<ThreadState> perThread,
                                      bool markPending)
{
  if (fullFlush) {
    if (perThread->flushPending) {
      checkoutAndBlock(perThread);
      return nextPendingFlush();
    } else {
      return nullptr;
    }
  } else {
    if (markPending) {
      assert(perThread->isFlushPending() == false);
      setFlushPending(perThread);
    }
    return tryCheckoutForFlush(perThread);
  }
}

bool DocumentsWriterFlushControl::assertNumDocsSinceStalled(bool stalled)
{
  /*
   *  updates the number of documents "finished" while we are in a stalled
   * state. this is important for asserting memory upper bounds since it
   * corresponds to the number of threads that are in-flight and crossed the
   * stall control check before we actually stalled. see #assertMemory()
   */
  if (stalled) {
    numDocsSinceStalled++;
  } else {
    numDocsSinceStalled = 0;
  }
  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::doAfterFlush(
    shared_ptr<DocumentsWriterPerThread> dwpt)
{
  assert(flushingWriters->containsKey(dwpt));
  try {
    optional<int64_t> bytes = flushingWriters->remove(dwpt);
    flushBytes_ -= bytes.value();
    perThreadPool->recycle(dwpt);
    assert(assertMemory());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    try {
      updateStallState();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      notifyAll();
    }
  }
}

bool DocumentsWriterFlushControl::updateStallState()
{

  assert(Thread::holdsLock(shared_from_this()));
  constexpr int64_t limit = stallLimitBytes();
  /*
   * we block indexing threads if net byte grows due to slow flushes
   * yet, for small ram buffers and large documents we can easily
   * reach the limit without any ongoing flushes. we need to ensure
   * that we don't stall/block if an ongoing or pending flush can
   * not free up enough memory to release the stall lock.
   */
  constexpr bool stall =
      (activeBytes_ + flushBytes_) > limit && activeBytes_ < limit && !closed;

  if (infoStream->isEnabled(L"DWFC")) {
    if (stall != stallControl->anyStalledThreads()) {
      if (stall) {
        infoStream->message(
            L"DW", wstring::format(Locale::ROOT,
                                   L"now stalling flushes: netBytes: %.1f MB "
                                   L"flushBytes: %.1f MB fullFlush: %b",
                                   netBytes() / 1024.0 / 1024.0,
                                   flushBytes() / 1024.0 / 1024.0, fullFlush));
        stallStartNS = System::nanoTime();
      } else {
        infoStream->message(
            L"DW",
            wstring::format(Locale::ROOT,
                            L"done stalling flushes for %.1f msec: netBytes: "
                            L"%.1f MB flushBytes: %.1f MB fullFlush: %b",
                            (System::nanoTime() - stallStartNS) / 1000000.0,
                            netBytes() / 1024.0 / 1024.0,
                            flushBytes() / 1024.0 / 1024.0, fullFlush));
      }
    }
  }

  stallControl->updateStalled(stall);
  return stall;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::waitForFlush()
{
  while (flushingWriters->size() != 0) {
    try {
      this->wait();
    } catch (const InterruptedException &e) {
      throw make_shared<ThreadInterruptedException>(e);
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::setFlushPending(
    shared_ptr<ThreadState> perThread)
{
  assert(!perThread->flushPending);
  if (perThread->dwpt->getNumDocsInRAM() > 0) {
    perThread->flushPending = true; // write access synced
    constexpr int64_t bytes = perThread->bytesUsed;
    flushBytes_ += bytes;
    activeBytes_ -= bytes;
    numPending++; // write access synced
    assert(assertMemory());
  } // don't assert on numDocs since we could hit an abort excp. while selecting
    // that dwpt for flushing
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::doOnAbort(shared_ptr<ThreadState> state)
{
  try {
    if (state->flushPending) {
      flushBytes_ -= state->bytesUsed;
    } else {
      activeBytes_ -= state->bytesUsed;
    }
    assert(assertMemory());
    // Take it out of the loop this DWPT is stale
    perThreadPool->reset(state);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    updateStallState();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DocumentsWriterPerThread>
DocumentsWriterFlushControl::tryCheckoutForFlush(
    shared_ptr<ThreadState> perThread)
{
  return perThread->flushPending ? internalTryCheckOutForFlush(perThread)
                                 : nullptr;
}

void DocumentsWriterFlushControl::checkoutAndBlock(
    shared_ptr<ThreadState> perThread)
{
  perThread->lock();
  try {
    assert((perThread->flushPending, L"can not block non-pending threadstate"));
    assert((fullFlush, L"can not block if fullFlush == false"));
    shared_ptr<DocumentsWriterPerThread> *const dwpt;
    constexpr int64_t bytes = perThread->bytesUsed;
    dwpt = perThreadPool->reset(perThread);
    numPending--;
    blockedFlushes.push_back(make_shared<BlockedFlush>(dwpt, bytes));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    perThread->unlock();
  }
}

shared_ptr<DocumentsWriterPerThread>
DocumentsWriterFlushControl::internalTryCheckOutForFlush(
    shared_ptr<ThreadState> perThread)
{
  assert(Thread::holdsLock(shared_from_this()));
  assert(perThread->flushPending);
  try {
    // We are pending so all memory is already moved to flushBytes
    if (perThread->tryLock()) {
      try {
        if (perThread->isInitialized()) {
          assert(perThread->isHeldByCurrentThread());
          shared_ptr<DocumentsWriterPerThread> *const dwpt;
          constexpr int64_t bytes = perThread->bytesUsed; // do that before
                                                            // replace!
          dwpt = perThreadPool->reset(perThread);
          assert((!flushingWriters->containsKey(dwpt),
                  L"DWPT is already flushing"));
          // Record the flushing DWPT to reduce flushBytes in doAfterFlush
          flushingWriters->put(dwpt, static_cast<int64_t>(bytes));
          numPending--; // write access synced
          return dwpt;
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        perThread->unlock();
      }
    }
    return nullptr;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    updateStallState();
  }
}

wstring DocumentsWriterFlushControl::toString()
{
  return L"DocumentsWriterFlushControl [activeBytes=" +
         to_wstring(activeBytes_) + L", flushBytes=" + to_wstring(flushBytes_) +
         L"]";
}

shared_ptr<DocumentsWriterPerThread>
DocumentsWriterFlushControl::nextPendingFlush()
{
  int numPending;
  bool fullFlush;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    shared_ptr<DocumentsWriterPerThread> *const poll;
    if ((poll = flushQueue.pop_front()) != nullptr) {
      updateStallState();
      return poll;
    }
    fullFlush = this->fullFlush;
    numPending = this->numPending;
  }
  if (numPending > 0 &&
      !fullFlush) { // don't check if we are doing a full flush
    constexpr int limit = perThreadPool->getActiveThreadStateCount();
    for (int i = 0; i < limit && numPending > 0; i++) {
      shared_ptr<ThreadState> *const next = perThreadPool->getThreadState(i);
      if (next->flushPending) {
        shared_ptr<DocumentsWriterPerThread> *const dwpt =
            tryCheckoutForFlush(next);
        if (dwpt != nullptr) {
          return dwpt;
        }
      }
    }
  }
  return nullptr;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::setClosed()
{
  // set by DW to signal that we should not release new DWPT after close
  this->closed = true;
}

shared_ptr<Iterator<std::shared_ptr<ThreadState>>>
DocumentsWriterFlushControl::allActiveThreadStates()
{
  return getPerThreadsIterator(perThreadPool->getActiveThreadStateCount());
}

shared_ptr<Iterator<std::shared_ptr<ThreadState>>>
DocumentsWriterFlushControl::getPerThreadsIterator(int const upto)
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this(), upto);
}

DocumentsWriterFlushControl::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(
        shared_ptr<DocumentsWriterFlushControl> outerInstance, int upto)
{
  this->outerInstance = outerInstance;
  this->upto = upto;
  i = 0;
}

bool DocumentsWriterFlushControl::IteratorAnonymousInnerClass::hasNext()
{
  return i < upto;
}

shared_ptr<ThreadState>
DocumentsWriterFlushControl::IteratorAnonymousInnerClass::next()
{
  return outerInstance->perThreadPool->getThreadState(i++);
}

void DocumentsWriterFlushControl::IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>(L"remove() not supported.");
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::doOnDelete()
{
  // pass null this is a global delete no update
  flushPolicy->onDelete(shared_from_this(), nullptr);
}

int64_t DocumentsWriterFlushControl::getDeleteBytesUsed()
{
  return documentsWriter->deleteQueue->ramBytesUsed();
}

int64_t DocumentsWriterFlushControl::ramBytesUsed()
{
  // TODO: improve this to return more detailed info?
  return getDeleteBytesUsed() + netBytes();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int DocumentsWriterFlushControl::numFlushingDWPT()
{
  return flushingWriters->size();
}

bool DocumentsWriterFlushControl::getAndResetApplyAllDeletes()
{
  return flushDeletes->getAndSet(false);
}

void DocumentsWriterFlushControl::setApplyAllDeletes()
{
  flushDeletes->set(true);
}

shared_ptr<ThreadState> DocumentsWriterFlushControl::obtainAndLock()
{
  shared_ptr<ThreadState> *const perThread =
      perThreadPool->getAndLock(Thread::currentThread(), documentsWriter);
  bool success = false;
  try {
    if (perThread->isInitialized() &&
        perThread->dwpt->deleteQueue != documentsWriter->deleteQueue) {
      // There is a flush-all in process and this DWPT is
      // now stale -- enroll it for flush and try for
      // another DWPT:
      addFlushableState(perThread);
    }
    success = true;
    // simply return the ThreadState even in a flush all case sine we already
    // hold the lock
    return perThread;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) { // make sure we unlock if this fails
      perThreadPool->release(perThread);
    }
  }
}

int64_t DocumentsWriterFlushControl::markForFullFlush()
{
  shared_ptr<DocumentsWriterDeleteQueue> *const flushingQueue;
  int64_t seqNo;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    assert(
        (!fullFlush,
         L"called DWFC#markForFullFlush() while full flush is still running"));
    assert((fullFlushBuffer.empty(),
            L"full flush buffer should be empty: " + fullFlushBuffer));
    fullFlush = true;
    flushingQueue = documentsWriter->deleteQueue;
    // Set a new delete queue - all subsequent DWPT will use this queue until
    // we do another full flush

    // Insert a gap in seqNo of current active thread count, in the worst case
    // each of those threads now have one operation in flight.  It's fine if we
    // have some sequence numbers that were never assigned:
    seqNo = documentsWriter->deleteQueue->getLastSequenceNumber() +
            perThreadPool->getActiveThreadStateCount() + 2;
    flushingQueue->maxSeqNo = seqNo + 1;

    shared_ptr<DocumentsWriterDeleteQueue> newQueue =
        make_shared<DocumentsWriterDeleteQueue>(
            infoStream, flushingQueue->generation + 1, seqNo + 1);

    documentsWriter->deleteQueue = newQueue;
  }
  constexpr int limit = perThreadPool->getActiveThreadStateCount();
  for (int i = 0; i < limit; i++) {
    shared_ptr<ThreadState> *const next = perThreadPool->getThreadState(i);
    next->lock();
    try {
      if (!next->isInitialized()) {
        continue;
      }
      assert((next->dwpt->deleteQueue == flushingQueue ||
                  next->dwpt->deleteQueue == documentsWriter->deleteQueue,
              L" flushingQueue: "));
      +flushingQueue + L" currentqueue: " + documentsWriter->deleteQueue +
          L" perThread queue: " + next->dwpt->deleteQueue + L" numDocsInRam: " +
          to_wstring(next->dwpt->getNumDocsInRAM());
      if (next->dwpt->deleteQueue != flushingQueue) {
        // this one is already a new DWPT
        continue;
      }
      addFlushableState(next);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      next->unlock();
    }
  }
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    /* make sure we move all DWPT that are where concurrently marked as
     * pending and moved to blocked are moved over to the flushQueue. There is
     * a chance that this happens since we marking DWPT for full flush without
     * blocking indexing.*/
    pruneBlockedQueue(flushingQueue);
    assert(assertBlockedFlushes(documentsWriter->deleteQueue));
    flushQueue.addAll(fullFlushBuffer);
    fullFlushBuffer.clear();
    updateStallState();
  }
  assert(assertActiveDeleteQueue(documentsWriter->deleteQueue));
  return seqNo;
}

bool DocumentsWriterFlushControl::assertActiveDeleteQueue(
    shared_ptr<DocumentsWriterDeleteQueue> queue)
{
  constexpr int limit = perThreadPool->getActiveThreadStateCount();
  for (int i = 0; i < limit; i++) {
    shared_ptr<ThreadState> *const next = perThreadPool->getThreadState(i);
    next->lock();
    try {
      assert(
          (!next->isInitialized() || next->dwpt->deleteQueue == queue,
           L"isInitialized: " + StringHelper::toString(next->isInitialized()) +
               L" numDocs: " +
               to_wstring(next->isInitialized() ? next->dwpt->getNumDocsInRAM()
                                                : 0)));
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      next->unlock();
    }
  }
  return true;
}

void DocumentsWriterFlushControl::addFlushableState(
    shared_ptr<ThreadState> perThread)
{
  if (infoStream->isEnabled(L"DWFC")) {
    infoStream->message(L"DWFC", L"addFlushableState " + perThread->dwpt);
  }
  shared_ptr<DocumentsWriterPerThread> *const dwpt = perThread->dwpt;
  assert(perThread->isHeldByCurrentThread());
  assert(perThread->isInitialized());
  assert(fullFlush);
  assert(dwpt->deleteQueue != documentsWriter->deleteQueue);
  if (dwpt->getNumDocsInRAM() > 0) {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      if (!perThread->flushPending) {
        setFlushPending(perThread);
      }
      shared_ptr<DocumentsWriterPerThread> *const flushingDWPT =
          internalTryCheckOutForFlush(perThread);
      assert(
          (flushingDWPT != nullptr, L"DWPT must never be null here since we "
                                    L"hold the lock and it holds documents"));
      assert((dwpt == flushingDWPT, L"flushControl returned different DWPT"));
      fullFlushBuffer.push_back(flushingDWPT);
    }
  } else {
    perThreadPool->reset(perThread); // make this state inactive
  }
}

void DocumentsWriterFlushControl::pruneBlockedQueue(
    shared_ptr<DocumentsWriterDeleteQueue> flushingQueue)
{
  deque<std::shared_ptr<BlockedFlush>>::const_iterator iterator =
      blockedFlushes.begin();
  while (iterator != blockedFlushes.end()) {
    shared_ptr<BlockedFlush> blockedFlush = *iterator;
    if (blockedFlush->dwpt->deleteQueue == flushingQueue) {
      (*iterator)->remove();
      assert((!flushingWriters->containsKey(blockedFlush->dwpt),
              L"DWPT is already flushing"));
      // Record the flushing DWPT to reduce flushBytes in doAfterFlush
      flushingWriters->put(blockedFlush->dwpt,
                           static_cast<int64_t>(blockedFlush->bytes));
      // don't decr pending here - it's already done when DWPT is blocked
      flushQueue.push_back(blockedFlush->dwpt);
    }
    iterator++;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::finishFullFlush()
{
  assert(fullFlush);
  assert(flushQueue.empty());
  assert(flushingWriters->isEmpty());
  try {
    if (!blockedFlushes.empty()) {
      assert(assertBlockedFlushes(documentsWriter->deleteQueue));
      pruneBlockedQueue(documentsWriter->deleteQueue);
      assert(blockedFlushes.empty());
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    fullFlush = false;
    updateStallState();
  }
}

bool DocumentsWriterFlushControl::assertBlockedFlushes(
    shared_ptr<DocumentsWriterDeleteQueue> flushingQueue)
{
  for (auto blockedFlush : blockedFlushes) {
    assert(blockedFlush->dwpt->deleteQueue == flushingQueue);
  }
  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::abortFullFlushes()
{
  try {
    abortPendingFlushes();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    fullFlush = false;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushControl::abortPendingFlushes()
{
  try {
    for (auto dwpt : flushQueue) {
      try {
        documentsWriter->subtractFlushedNumDocs(dwpt->getNumDocsInRAM());
        dwpt->abort();
      } catch (const runtime_error &ex) {
        // that's fine we just abort everything here this is best effort
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        doAfterFlush(dwpt);
      }
    }
    for (auto blockedFlush : blockedFlushes) {
      try {
        flushingWriters->put(blockedFlush->dwpt,
                             static_cast<int64_t>(blockedFlush->bytes));
        documentsWriter->subtractFlushedNumDocs(
            blockedFlush->dwpt->getNumDocsInRAM());
        blockedFlush->dwpt->abort();
      } catch (const runtime_error &ex) {
        // that's fine we just abort everything here this is best effort
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        doAfterFlush(blockedFlush->dwpt);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    flushQueue.clear();
    blockedFlushes.clear();
    updateStallState();
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool DocumentsWriterFlushControl::isFullFlush() { return fullFlush; }

// C++ WARNING: The following method was originally marked 'synchronized':
int DocumentsWriterFlushControl::numQueuedFlushes()
{
  return flushQueue.size();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int DocumentsWriterFlushControl::numBlockedFlushes()
{
  return blockedFlushes.size();
}

// C++ TODO: No base class can be determined:
DocumentsWriterFlushControl::BlockedFlush::BlockedFlush(
    shared_ptr<DocumentsWriterPerThread> dwpt, int64_t bytes)
    : dwpt(dwpt), bytes(bytes)
{
}

void DocumentsWriterFlushControl::waitIfStalled()
{
  stallControl->waitIfStalled();
}

bool DocumentsWriterFlushControl::anyStalledThreads()
{
  return stallControl->anyStalledThreads();
}

shared_ptr<InfoStream> DocumentsWriterFlushControl::getInfoStream()
{
  return infoStream;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<ThreadState>
DocumentsWriterFlushControl::findLargestNonPendingWriter()
{
  shared_ptr<ThreadState> maxRamUsingThreadState = nullptr;
  int64_t maxRamSoFar = 0;
  Iterator<std::shared_ptr<ThreadState>> activePerThreadsIterator =
      allActiveThreadStates();
  int count = 0;
  while (activePerThreadsIterator->hasNext()) {
    shared_ptr<ThreadState> next = activePerThreadsIterator->next();
    if (!next->flushPending) {
      constexpr int64_t nextRam = next->bytesUsed;
      if (nextRam > 0 && next->dwpt->getNumDocsInRAM() > 0) {
        if (infoStream->isEnabled(L"FP")) {
          infoStream->message(L"FP",
                              L"thread state has " + to_wstring(nextRam) +
                                  L" bytes; docInRAM=" +
                                  to_wstring(next->dwpt->getNumDocsInRAM()));
        }
        count++;
        if (nextRam > maxRamSoFar) {
          maxRamSoFar = nextRam;
          maxRamUsingThreadState = next;
        }
      }
    }
    activePerThreadsIterator++;
  }
  if (infoStream->isEnabled(L"FP")) {
    infoStream->message(L"FP", to_wstring(count) +
                                   L" in-use non-flushing threads states");
  }
  return maxRamUsingThreadState;
}

shared_ptr<DocumentsWriterPerThread>
DocumentsWriterFlushControl::checkoutLargestNonPendingWriter()
{
  shared_ptr<ThreadState> largestNonPendingWriter =
      findLargestNonPendingWriter();
  if (largestNonPendingWriter != nullptr) {
    // we only lock this very briefly to swap it's DWPT out - we don't go
    // through the DWPTPool and it's free queue
    largestNonPendingWriter->lock();
    try {
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {
        try {
          if (largestNonPendingWriter->isInitialized() == false) {
            return nextPendingFlush();
          } else {
            return checkout(largestNonPendingWriter,
                            largestNonPendingWriter->isFlushPending() == false);
          }
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          updateStallState();
        }
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      largestNonPendingWriter->unlock();
    }
  }
  return nullptr;
}
} // namespace org::apache::lucene::index