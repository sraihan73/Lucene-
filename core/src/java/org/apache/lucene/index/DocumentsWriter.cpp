using namespace std;

#include "DocumentsWriter.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using FlushedSegment =
    org::apache::lucene::index::DocumentsWriterPerThread::FlushedSegment;
using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using Query = org::apache::lucene::search::Query;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;

DocumentsWriter::DocumentsWriter(
    shared_ptr<FlushNotifications> flushNotifications,
    int indexCreatedVersionMajor, shared_ptr<AtomicLong> pendingNumDocs,
    bool enableTestPoints, function<wstring *()> &segmentNameSupplier,
    shared_ptr<LiveIndexWriterConfig> config,
    shared_ptr<Directory> directoryOrig, shared_ptr<Directory> directory,
    shared_ptr<FieldInfos::FieldNumbers> globalFieldNumberMap)
    : directoryOrig(directoryOrig), directory(directory),
      globalFieldNumberMap(globalFieldNumberMap),
      indexCreatedVersionMajor(indexCreatedVersionMajor),
      pendingNumDocs(pendingNumDocs), enableTestPoints(enableTestPoints),
      segmentNameSupplier(segmentNameSupplier),
      flushNotifications(flushNotifications),
      infoStream(config->getInfoStream()), config(config),
      perThreadPool(config->getIndexerThreadPool()),
      flushPolicy(config->getFlushPolicy()),
      flushControl(
          make_shared<DocumentsWriterFlushControl>(shared_from_this(), config))
{
  this->deleteQueue = make_shared<DocumentsWriterDeleteQueue>(infoStream);
}

int64_t
DocumentsWriter::deleteQueries(deque<Query> &queries) 
{
  return applyDeleteOrUpdate([&](any q) { q::addDelete(queries); });
}

void DocumentsWriter::setLastSeqNo(int64_t seqNo) { lastSeqNo = seqNo; }

int64_t DocumentsWriter::deleteTerms(deque<Term> &terms) 
{
  return applyDeleteOrUpdate([&](any q) { q::addDelete(terms); });
}

int64_t DocumentsWriter::updateDocValues(
    deque<DocValuesUpdate> &updates) 
{
  return applyDeleteOrUpdate([&](any q) { q::addDocValuesUpdates(updates); });
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t DocumentsWriter::applyDeleteOrUpdate(
    function<int64_t(DocumentsWriterDeleteQueue *)>
        &function) 
{
  // TODO why is this synchronized?
  shared_ptr<DocumentsWriterDeleteQueue> *const deleteQueue = this->deleteQueue;
  int64_t seqNo = function(deleteQueue);
  flushControl->doOnDelete();
  lastSeqNo = max(lastSeqNo, seqNo);
  if (applyAllDeletes(deleteQueue)) {
    seqNo = -seqNo;
  }
  return seqNo;
}

bool DocumentsWriter::applyAllDeletes(
    shared_ptr<DocumentsWriterDeleteQueue> deleteQueue) 
{
  if (flushControl->isFullFlush() == false &&
      flushControl->getAndResetApplyAllDeletes()) {
    if (deleteQueue != nullptr) {
      assert(assertTicketQueueModification(deleteQueue));
      ticketQueue->addDeletes(deleteQueue);
    }
    flushNotifications
        ->onDeletesApplied(); // apply deletes event forces a purge
    return true;
  }
  return false;
}

void DocumentsWriter::purgeFlushTickets(
    bool forced,
    IOUtils::IOConsumer<std::shared_ptr<DocumentsWriterFlushQueue::FlushTicket>>
        consumer) 
{
  if (forced) {
    ticketQueue->forcePurge(consumer);
  } else {
    ticketQueue->tryPurge(consumer);
  }
}

int DocumentsWriter::getNumDocs() { return numDocsInRAM->get(); }

void DocumentsWriter::ensureOpen() 
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(
        L"this DocumentsWriter is closed");
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriter::abort() 
{
  bool success = false;
  try {
    deleteQueue->clear();
    if (infoStream->isEnabled(L"DW")) {
      infoStream->message(L"DW", L"abort");
    }
    constexpr int limit = perThreadPool->getActiveThreadStateCount();
    for (int i = 0; i < limit; i++) {
      shared_ptr<ThreadState> *const perThread =
          perThreadPool->getThreadState(i);
      perThread->lock();
      try {
        abortThreadState(perThread);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        perThread->unlock();
      }
    }
    flushControl->abortPendingFlushes();
    flushControl->waitForFlush();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (infoStream->isEnabled(L"DW")) {
      infoStream->message(L"DW", L"done abort success=" +
                                     StringHelper::toString(success));
    }
  }
}

bool DocumentsWriter::flushOneDWPT() 
{
  if (infoStream->isEnabled(L"DW")) {
    infoStream->message(L"DW", L"startFlushOneDWPT");
  }
  // first check if there is one pending
  shared_ptr<DocumentsWriterPerThread> documentsWriterPerThread =
      flushControl->nextPendingFlush();
  if (documentsWriterPerThread == nullptr) {
    documentsWriterPerThread = flushControl->checkoutLargestNonPendingWriter();
  }
  if (documentsWriterPerThread != nullptr) {
    return doFlush(documentsWriterPerThread);
  }
  return false; // we didn't flush anything here
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Closeable> DocumentsWriter::lockAndAbortAll() 
{
  if (infoStream->isEnabled(L"DW")) {
    infoStream->message(L"DW", L"lockAndAbortAll");
  }
  // Make sure we move all pending tickets into the flush queue:
  ticketQueue->forcePurge([&](any ticket) {
    if (ticket::getFlushedSegment() != nullptr) {
      pendingNumDocs->addAndGet(
          -ticket::getFlushedSegment().segmentInfo::info::maxDoc());
    }
  });
  deque<std::shared_ptr<ThreadState>> threadStates =
      deque<std::shared_ptr<ThreadState>>();
  shared_ptr<AtomicBoolean> released = make_shared<AtomicBoolean>(false);
  shared_ptr<Closeable> *const release = [&]() {
    if (released->compareAndSet(false, true)) { // only once
      if (infoStream->isEnabled(L"DW")) {
        infoStream->message(L"DW", L"unlockAllAbortedThread");
      }
      perThreadPool->clearAbort();
      for (auto state : threadStates) {
        state->unlock();
      }
    }
  };
  try {
    deleteQueue->clear();
    constexpr int limit = perThreadPool->getMaxThreadStates();
    perThreadPool->setAbort();
    for (int i = 0; i < limit; i++) {
      shared_ptr<ThreadState> *const perThread =
          perThreadPool->getThreadState(i);
      perThread->lock();
      threadStates.push_back(perThread);
      abortThreadState(perThread);
    }
    deleteQueue->clear();

    // jump over any possible in flight ops:
    deleteQueue->skipSequenceNumbers(
        perThreadPool->getActiveThreadStateCount() + 1);

    flushControl->abortPendingFlushes();
    flushControl->waitForFlush();
    if (infoStream->isEnabled(L"DW")) {
      infoStream->message(L"DW", L"finished lockAndAbortAll success=true");
    }
    return release;
  } catch (const runtime_error &t) {
    if (infoStream->isEnabled(L"DW")) {
      infoStream->message(L"DW", L"finished lockAndAbortAll success=false");
    }
    try {
      // if something happens here we unlock all states again
      delete release;
    } catch (const runtime_error &t1) {
      t.addSuppressed(t1);
    }
    throw t;
  }
}

int DocumentsWriter::abortThreadState(shared_ptr<ThreadState> perThread) throw(
    IOException)
{
  assert(perThread->isHeldByCurrentThread());
  if (perThread->isInitialized()) {
    try {
      int abortedDocCount = perThread->dwpt->getNumDocsInRAM();
      subtractFlushedNumDocs(abortedDocCount);
      perThread->dwpt->abort();
      return abortedDocCount;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      flushControl->doOnAbort(perThread);
    }
  } else {
    flushControl->doOnAbort(perThread);
    // This DWPT was never initialized so it has no indexed documents:
    return 0;
  }
}

int64_t DocumentsWriter::getMaxCompletedSequenceNumber()
{
  int64_t value = lastSeqNo;
  int limit = perThreadPool->getMaxThreadStates();
  for (int i = 0; i < limit; i++) {
    shared_ptr<ThreadState> perThread = perThreadPool->getThreadState(i);
    value = max(value, perThread->lastSeqNo);
  }
  return value;
}

bool DocumentsWriter::anyChanges()
{
  /*
   * changes are either in a DWPT or in the deleteQueue.
   * yet if we currently flush deletes and / or dwpt there
   * could be a window where all changes are in the ticket queue
   * before they are published to the IW. ie we need to check if the
   * ticket queue has any tickets.
   */
  bool anyChanges = numDocsInRAM->get() != 0 || anyDeletions() ||
                    ticketQueue->hasTickets() ||
                    pendingChangesInCurrentFullFlush;
  if (infoStream->isEnabled(L"DW") && anyChanges) {
    infoStream->message(
        L"DW", L"anyChanges? numDocsInRam=" + numDocsInRAM->get() +
                   L" deletes=" + StringHelper::toString(anyDeletions()) +
                   L" hasTickets:" +
                   StringHelper::toString(ticketQueue->hasTickets()) +
                   L" pendingChangesInFullFlush: " +
                   StringHelper::toString(pendingChangesInCurrentFullFlush));
  }
  return anyChanges;
}

int DocumentsWriter::getBufferedDeleteTermsSize()
{
  return deleteQueue->getBufferedUpdatesTermsSize();
}

int DocumentsWriter::getNumBufferedDeleteTerms()
{
  return deleteQueue->numGlobalTermDeletes();
}

bool DocumentsWriter::anyDeletions() { return deleteQueue->anyChanges(); }

DocumentsWriter::~DocumentsWriter()
{
  closed = true;
  flushControl->setClosed();
}

bool DocumentsWriter::preUpdate() 
{
  ensureOpen();
  bool hasEvents = false;

  if (flushControl->anyStalledThreads() ||
      (flushControl->numQueuedFlushes() > 0 &&
       config->checkPendingFlushOnUpdate)) {
    // Help out flushing any queued DWPTs so we can un-stall:
    do {
      // Try pick up pending threads here if possible
      shared_ptr<DocumentsWriterPerThread> flushingDWPT;
      while ((flushingDWPT = flushControl->nextPendingFlush()) != nullptr) {
        // Don't push the delete here since the update could fail!
        hasEvents |= doFlush(flushingDWPT);
      }

      flushControl->waitIfStalled(); // block if stalled
    } while (flushControl->numQueuedFlushes() !=
             0); // still queued DWPTs try help flushing
  }
  return hasEvents;
}

bool DocumentsWriter::postUpdate(
    shared_ptr<DocumentsWriterPerThread> flushingDWPT,
    bool hasEvents) 
{
  hasEvents |= applyAllDeletes(deleteQueue);
  if (flushingDWPT != nullptr) {
    hasEvents |= doFlush(flushingDWPT);
  } else if (config->checkPendingFlushOnUpdate) {
    shared_ptr<DocumentsWriterPerThread> *const nextPendingFlush =
        flushControl->nextPendingFlush();
    if (nextPendingFlush != nullptr) {
      hasEvents |= doFlush(nextPendingFlush);
    }
  }

  return hasEvents;
}

void DocumentsWriter::ensureInitialized(shared_ptr<ThreadState> state) throw(
    IOException)
{
  if (state->dwpt == nullptr) {
    shared_ptr<FieldInfos::Builder> *const infos =
        make_shared<FieldInfos::Builder>(globalFieldNumberMap);
    state->dwpt = make_shared<DocumentsWriterPerThread>(
        indexCreatedVersionMajor, segmentNameSupplier->get(), directoryOrig,
        directory, config, infoStream, deleteQueue, infos, pendingNumDocs,
        enableTestPoints);
  }
}

template <typename T1, typename T2>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: long updateDocuments(final Iterable<? extends
// Iterable<? extends IndexableField>> docs, final
// org.apache.lucene.analysis.Analyzer analyzer, final
// DocumentsWriterDeleteQueue.Node<?> delNode) throws java.io.IOException
int64_t DocumentsWriter::updateDocuments(
    deque<T1> const docs, shared_ptr<Analyzer> analyzer,
    shared_ptr<DocumentsWriterDeleteQueue::Node<T2>> delNode) 
{
  bool hasEvents = preUpdate();

  shared_ptr<ThreadState> *const perThread = flushControl->obtainAndLock();
  shared_ptr<DocumentsWriterPerThread> *const flushingDWPT;
  int64_t seqNo;

  try {
    // This must happen after we've pulled the ThreadState because IW.close
    // waits for all ThreadStates to be released:
    ensureOpen();
    ensureInitialized(perThread);
    assert(perThread->isInitialized());
    shared_ptr<DocumentsWriterPerThread> *const dwpt = perThread->dwpt;
    constexpr int dwptNumDocs = dwpt->getNumDocsInRAM();
    try {
      seqNo =
          dwpt->updateDocuments(docs, analyzer, delNode, flushNotifications);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (dwpt->isAborted()) {
        flushControl->doOnAbort(perThread);
      }
      // We don't know how many documents were actually
      // counted as indexed, so we must subtract here to
      // accumulate our separate counter:
      numDocsInRAM->addAndGet(dwpt->getNumDocsInRAM() - dwptNumDocs);
    }
    constexpr bool isUpdate = delNode != nullptr && delNode->isDelete();
    flushingDWPT = flushControl->doAfterDocument(perThread, isUpdate);

    assert((seqNo > perThread->lastSeqNo,
            L"seqNo=" + to_wstring(seqNo) + L" lastSeqNo=" +
                to_wstring(perThread->lastSeqNo)));
    perThread->lastSeqNo = seqNo;

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    perThreadPool->release(perThread);
  }

  if (postUpdate(flushingDWPT, hasEvents)) {
    seqNo = -seqNo;
  }
  return seqNo;
}

template <typename T1, typename T2>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: long updateDocument(final Iterable<? extends
// IndexableField> doc, final org.apache.lucene.analysis.Analyzer analyzer, final
// DocumentsWriterDeleteQueue.Node<?> delNode) throws java.io.IOException
int64_t DocumentsWriter::updateDocument(
    deque<T1> const doc, shared_ptr<Analyzer> analyzer,
    shared_ptr<DocumentsWriterDeleteQueue::Node<T2>> delNode) 
{

  bool hasEvents = preUpdate();

  shared_ptr<ThreadState> *const perThread = flushControl->obtainAndLock();

  shared_ptr<DocumentsWriterPerThread> *const flushingDWPT;
  int64_t seqNo;
  try {
    // This must happen after we've pulled the ThreadState because IW.close
    // waits for all ThreadStates to be released:
    ensureOpen();
    ensureInitialized(perThread);
    assert(perThread->isInitialized());
    shared_ptr<DocumentsWriterPerThread> *const dwpt = perThread->dwpt;
    constexpr int dwptNumDocs = dwpt->getNumDocsInRAM();
    try {
      seqNo = dwpt->updateDocument(doc, analyzer, delNode, flushNotifications);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (dwpt->isAborted()) {
        flushControl->doOnAbort(perThread);
      }
      // We don't know whether the document actually
      // counted as being indexed, so we must subtract here to
      // accumulate our separate counter:
      numDocsInRAM->addAndGet(dwpt->getNumDocsInRAM() - dwptNumDocs);
    }
    constexpr bool isUpdate = delNode != nullptr && delNode->isDelete();
    flushingDWPT = flushControl->doAfterDocument(perThread, isUpdate);

    assert((seqNo > perThread->lastSeqNo,
            L"seqNo=" + to_wstring(seqNo) + L" lastSeqNo=" +
                to_wstring(perThread->lastSeqNo)));
    perThread->lastSeqNo = seqNo;

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    perThreadPool->release(perThread);
  }

  if (postUpdate(flushingDWPT, hasEvents)) {
    seqNo = -seqNo;
  }

  return seqNo;
}

bool DocumentsWriter::doFlush(
    shared_ptr<DocumentsWriterPerThread> flushingDWPT) 
{
  bool hasEvents = false;
  while (flushingDWPT != nullptr) {
    hasEvents = true;
    bool success = false;
    shared_ptr<DocumentsWriterFlushQueue::FlushTicket> ticket = nullptr;
    try {
      assert((currentFullFlushDelQueue == nullptr ||
                  flushingDWPT->deleteQueue == currentFullFlushDelQueue,
              L"expected: "));
      +currentFullFlushDelQueue + L"but was: " + flushingDWPT->deleteQueue +
          L" " + StringHelper::toString(flushControl->isFullFlush());
      /*
       * Since with DWPT the flush process is concurrent and several DWPT
       * could flush at the same time we must maintain the order of the
       * flushes before we can apply the flushed segment and the frozen global
       * deletes it is buffering. The reason for this is that the global
       * deletes mark a certain point in time where we took a DWPT out of
       * rotation and freeze the global deletes.
       *
       * Example: A flush 'A' starts and freezes the global deletes, then
       * flush 'B' starts and freezes all deletes occurred since 'A' has
       * started. if 'B' finishes before 'A' we need to wait until 'A' is done
       * otherwise the deletes frozen by 'B' are not applied to 'A' and we
       * might miss to deletes documents in 'A'.
       */
      try {
        assert(assertTicketQueueModification(flushingDWPT->deleteQueue));
        // Each flush is assigned a ticket in the order they acquire the
        // ticketQueue lock
        ticket = ticketQueue->addFlushTicket(flushingDWPT);
        constexpr int flushingDocsInRam = flushingDWPT->getNumDocsInRAM();
        bool dwptSuccess = false;
        try {
          // flush concurrently without locking
          shared_ptr<FlushedSegment> *const newSegment =
              flushingDWPT->flush(flushNotifications);
          ticketQueue->addSegment(ticket, newSegment);
          dwptSuccess = true;
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          subtractFlushedNumDocs(flushingDocsInRam);
          if (flushingDWPT->pendingFilesToDelete()->isEmpty() == false) {
            shared_ptr<Set<wstring>> files =
                flushingDWPT->pendingFilesToDelete();
            flushNotifications->deleteUnusedFiles(files);
            hasEvents = true;
          }
          if (dwptSuccess == false) {
            flushNotifications->flushFailed(flushingDWPT->getSegmentInfo());
            hasEvents = true;
          }
        }
        // flush was successful once we reached this point - new seg. has been
        // assigned to the ticket!
        success = true;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        if (!success && ticket != nullptr) {
          // In the case of a failure make sure we are making progress and
          // apply all the deletes since the segment flush failed since the
          // flush ticket could hold global deletes see FlushTicket#canPublish()
          ticketQueue->markTicketFailed(ticket);
        }
      }
      /*
       * Now we are done and try to flush the ticket queue if the head of the
       * queue has already finished the flush.
       */
      if (ticketQueue->getTicketCount() >=
          perThreadPool->getActiveThreadStateCount()) {
        // This means there is a backlog: the one
        // thread in innerPurge can't keep up with all
        // other threads flushing segments.  In this case
        // we forcefully stall the producers.
        flushNotifications->onTicketBacklog();
        break;
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      flushControl->doAfterFlush(flushingDWPT);
    }

    flushingDWPT = flushControl->nextPendingFlush();
  }

  if (hasEvents) {
    flushNotifications->afterSegmentsFlushed();
  }

  // If deletes alone are consuming > 1/2 our RAM
  // buffer, force them all to apply now. This is to
  // prevent too-frequent flushing of a long tail of
  // tiny segments:
  constexpr double ramBufferSizeMB = config->getRAMBufferSizeMB();
  if (ramBufferSizeMB != IndexWriterConfig::DISABLE_AUTO_FLUSH &&
      flushControl->getDeleteBytesUsed() >
          (1024 * 1024 * ramBufferSizeMB / 2)) {
    hasEvents = true;
    if (applyAllDeletes(deleteQueue) == false) {
      if (infoStream->isEnabled(L"DW")) {
        infoStream->message(
            L"DW", wstring::format(Locale::ROOT,
                                   L"force apply deletes after flush "
                                   L"bytesUsed=%.1f MB vs ramBuffer=%.1f MB",
                                   flushControl->getDeleteBytesUsed() /
                                       (1024.0 * 1024.0),
                                   ramBufferSizeMB));
      }
      flushNotifications->onDeletesApplied();
    }
  }

  return hasEvents;
}

void DocumentsWriter::subtractFlushedNumDocs(int numFlushed)
{
  int oldValue = numDocsInRAM->get();
  while (numDocsInRAM->compareAndSet(oldValue, oldValue - numFlushed) ==
         false) {
    oldValue = numDocsInRAM->get();
  }
  assert(numDocsInRAM->get() >= 0);
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool DocumentsWriter::setFlushingDeleteQueue(
    shared_ptr<DocumentsWriterDeleteQueue> session)
{
  currentFullFlushDelQueue = session;
  return true;
}

bool DocumentsWriter::assertTicketQueueModification(
    shared_ptr<DocumentsWriterDeleteQueue> deleteQueue)
{
  // assign it then we don't need to sync on DW
  shared_ptr<DocumentsWriterDeleteQueue> currentFullFlushDelQueue =
      this->currentFullFlushDelQueue;
  assert((currentFullFlushDelQueue == nullptr ||
              currentFullFlushDelQueue == deleteQueue,
          L"only modifications from the current flushing queue are permitted "
          L"while doing a full flush"));
  return true;
}

int64_t DocumentsWriter::flushAllThreads() 
{
  shared_ptr<DocumentsWriterDeleteQueue> *const flushingDeleteQueue;
  if (infoStream->isEnabled(L"DW")) {
    infoStream->message(L"DW", L"startFullFlush");
  }

  int64_t seqNo;

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    pendingChangesInCurrentFullFlush = anyChanges();
    flushingDeleteQueue = deleteQueue;
    /* Cutover to a new delete queue.  This must be synced on the flush control
     * otherwise a new DWPT could sneak into the loop with an already flushing
     * delete queue */
    seqNo = flushControl->markForFullFlush(); // swaps this.deleteQueue synced
                                              // on FlushControl
    assert(setFlushingDeleteQueue(flushingDeleteQueue));
  }
  assert(currentFullFlushDelQueue != nullptr);
  assert(currentFullFlushDelQueue != deleteQueue);

  bool anythingFlushed = false;
  try {
    shared_ptr<DocumentsWriterPerThread> flushingDWPT;
    // Help out with flushing:
    while ((flushingDWPT = flushControl->nextPendingFlush()) != nullptr) {
      anythingFlushed |= doFlush(flushingDWPT);
    }
    // If a concurrent flush is still in flight wait for it
    flushControl->waitForFlush();
    if (anythingFlushed == false &&
        flushingDeleteQueue
            ->anyChanges()) { // apply deletes if we did not flush any document
      if (infoStream->isEnabled(L"DW")) {
        infoStream->message(L"DW", Thread::currentThread().getName() +
                                       L": flush naked frozen global deletes");
      }
      assertTicketQueueModification(flushingDeleteQueue);
      ticketQueue->addDeletes(flushingDeleteQueue);
    }
    // we can't assert that we don't have any tickets in teh queue since we
    // might add a DocumentsWriterDeleteQueue concurrently if we have very small
    // ram buffers this happens quite frequently
    assert(!flushingDeleteQueue->anyChanges());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    assert(flushingDeleteQueue == currentFullFlushDelQueue);
  }
  if (anythingFlushed) {
    return -seqNo;
  } else {
    return seqNo;
  }
}

void DocumentsWriter::finishFullFlush(bool success) 
{
  try {
    if (infoStream->isEnabled(L"DW")) {
      infoStream->message(L"DW", Thread::currentThread().getName() +
                                     L" finishFullFlush success=" +
                                     StringHelper::toString(success));
    }
    assert(setFlushingDeleteQueue(nullptr));
    if (success) {
      // Release the flush lock
      flushControl->finishFullFlush();
    } else {
      flushControl->abortFullFlushes();
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    pendingChangesInCurrentFullFlush = false;
    applyAllDeletes(deleteQueue); // make sure we do execute this since we block
                                  // applying deletes during full flush
  }
}

int64_t DocumentsWriter::ramBytesUsed()
{
  return flushControl->ramBytesUsed();
}
} // namespace org::apache::lucene::index