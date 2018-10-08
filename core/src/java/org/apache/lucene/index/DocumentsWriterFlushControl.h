#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <deque>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DocumentsWriterPerThread;
}

namespace org::apache::lucene::index
{
class BlockedFlush;
}
namespace org::apache::lucene::index
{
class DocumentsWriterStallControl;
}
namespace org::apache::lucene::index
{
class DocumentsWriterPerThreadPool;
}
namespace org::apache::lucene::index
{
class FlushPolicy;
}
namespace org::apache::lucene::index
{
class DocumentsWriter;
}
namespace org::apache::lucene::index
{
class LiveIndexWriterConfig;
}
namespace org::apache::lucene::util
{
class InfoStream;
}
namespace org::apache::lucene::index
{
class ThreadState;
}
namespace org::apache::lucene::index
{
class DocumentsWriterDeleteQueue;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::index
{

using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using Accountable = org::apache::lucene::util::Accountable;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * This class controls {@link DocumentsWriterPerThread} flushing during
 * indexing. It tracks the memory consumption per
 * {@link DocumentsWriterPerThread} and uses a configured {@link FlushPolicy} to
 * decide if a {@link DocumentsWriterPerThread} must flush.
 * <p>
 * In addition to the {@link FlushPolicy} the flush control might set certain
 * {@link DocumentsWriterPerThread} as flush pending iff a
 * {@link DocumentsWriterPerThread} exceeds the
 * {@link IndexWriterConfig#getRAMPerThreadHardLimitMB()} to prevent address
 * space exhaustion.
 */
class DocumentsWriterFlushControl final
    : public std::enable_shared_from_this<DocumentsWriterFlushControl>,
      public Accountable
{
  GET_CLASS_NAME(DocumentsWriterFlushControl)

private:
  const int64_t hardMaxBytesPerDWPT;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t activeBytes_ = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t flushBytes_ = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile int numPending = 0;
  int numPending = 0;
  int numDocsSinceStalled = 0; // only with assert
public:
  const std::shared_ptr<AtomicBoolean> flushDeletes =
      std::make_shared<AtomicBoolean>(false);

private:
  bool fullFlush = false;
  const std::deque<std::shared_ptr<DocumentsWriterPerThread>> flushQueue =
      std::deque<std::shared_ptr<DocumentsWriterPerThread>>();
  // only for safety reasons if a DWPT is close to the RAM limit
  const std::deque<std::shared_ptr<BlockedFlush>> blockedFlushes =
      std::deque<std::shared_ptr<BlockedFlush>>();
  const std::shared_ptr<
      IdentityHashMap<std::shared_ptr<DocumentsWriterPerThread>, int64_t>>
      flushingWriters = std::make_shared<IdentityHashMap<
          std::shared_ptr<DocumentsWriterPerThread>, int64_t>>();

public:
  double maxConfiguredRamBuffer = 0;
  int64_t peakActiveBytes = 0;      // only with assert
  int64_t peakFlushBytes = 0;       // only with assert
  int64_t peakNetBytes = 0;         // only with assert
  int64_t peakDelta = 0;            // only with assert
  bool flushByRAMWasDisabled = false; // only with assert
  const std::shared_ptr<DocumentsWriterStallControl> stallControl;

private:
  const std::shared_ptr<DocumentsWriterPerThreadPool> perThreadPool;
  const std::shared_ptr<FlushPolicy> flushPolicy;
  bool closed = false;
  const std::shared_ptr<DocumentsWriter> documentsWriter;
  const std::shared_ptr<LiveIndexWriterConfig> config;
  const std::shared_ptr<InfoStream> infoStream;

public:
  DocumentsWriterFlushControl(std::shared_ptr<DocumentsWriter> documentsWriter,
                              std::shared_ptr<LiveIndexWriterConfig> config);

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t activeBytes();

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t flushBytes();

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t netBytes();

private:
  int64_t stallLimitBytes();

  bool assertMemory();

  void commitPerThreadBytes(std::shared_ptr<ThreadState> perThread);

  // only for asserts
  bool updatePeaks(int64_t delta);

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DocumentsWriterPerThread>
  doAfterDocument(std::shared_ptr<ThreadState> perThread, bool isUpdate);

private:
  std::shared_ptr<DocumentsWriterPerThread>
  checkout(std::shared_ptr<ThreadState> perThread, bool markPending);

  bool assertNumDocsSinceStalled(bool stalled);

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void doAfterFlush(std::shared_ptr<DocumentsWriterPerThread> dwpt);

private:
  int64_t stallStartNS = 0;

  bool updateStallState();

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void waitForFlush();

  /**
   * Sets flush pending state on the given {@link ThreadState}. The
   * {@link ThreadState} must have indexed at least on Document and must not be
   * already pending.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void setFlushPending(std::shared_ptr<ThreadState> perThread);

  // C++ WARNING: The following method was originally marked 'synchronized':
  void doOnAbort(std::shared_ptr<ThreadState> state);

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DocumentsWriterPerThread>
  tryCheckoutForFlush(std::shared_ptr<ThreadState> perThread);

private:
  void checkoutAndBlock(std::shared_ptr<ThreadState> perThread);

  std::shared_ptr<DocumentsWriterPerThread>
  internalTryCheckOutForFlush(std::shared_ptr<ThreadState> perThread);

public:
  virtual std::wstring toString();

  std::shared_ptr<DocumentsWriterPerThread> nextPendingFlush();

  // C++ WARNING: The following method was originally marked 'synchronized':
  void setClosed();

  /**
   * Returns an iterator that provides access to all currently active {@link
   * ThreadState}s
   */
  std::shared_ptr<Iterator<std::shared_ptr<ThreadState>>>
  allActiveThreadStates();

private:
  std::shared_ptr<Iterator<std::shared_ptr<ThreadState>>>
  getPerThreadsIterator(int const upto);

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<ThreadState>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<DocumentsWriterFlushControl> outerInstance;

    int upto = 0;

  public:
    IteratorAnonymousInnerClass(
        std::shared_ptr<DocumentsWriterFlushControl> outerInstance, int upto);

    int i = 0;

    bool hasNext();

    std::shared_ptr<ThreadState> next();

    void remove();
  };

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void doOnDelete();

  /** Returns heap bytes currently consumed by buffered deletes/updates that
   * would be freed if we pushed all deletes.  This does not include bytes
   * consumed by already pushed delete/update packets. */
  int64_t getDeleteBytesUsed();

  int64_t ramBytesUsed() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int numFlushingDWPT();

  bool getAndResetApplyAllDeletes();

  void setApplyAllDeletes();

  std::shared_ptr<ThreadState> obtainAndLock();

  int64_t markForFullFlush();

private:
  bool
  assertActiveDeleteQueue(std::shared_ptr<DocumentsWriterDeleteQueue> queue);

  const std::deque<std::shared_ptr<DocumentsWriterPerThread>> fullFlushBuffer =
      std::deque<std::shared_ptr<DocumentsWriterPerThread>>();

public:
  void addFlushableState(std::shared_ptr<ThreadState> perThread);

  /**
   * Prunes the blockedQueue by removing all DWPT that are associated with the
   * given flush queue.
   */
private:
  void
  pruneBlockedQueue(std::shared_ptr<DocumentsWriterDeleteQueue> flushingQueue);

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void finishFullFlush();

  bool assertBlockedFlushes(
      std::shared_ptr<DocumentsWriterDeleteQueue> flushingQueue);

  // C++ WARNING: The following method was originally marked 'synchronized':
  void abortFullFlushes();

  // C++ WARNING: The following method was originally marked 'synchronized':
  void abortPendingFlushes();

  /**
   * Returns <code>true</code> if a full flush is currently running
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool isFullFlush();

  /**
   * Returns the number of flushes that are already checked out but not yet
   * actively flushing
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  int numQueuedFlushes();

  /**
   * Returns the number of flushes that are checked out but not yet available
   * for flushing. This only applies during a full flush if a DWPT needs
   * flushing but must not be flushed until the full flush has finished.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  int numBlockedFlushes();

private:
  class BlockedFlush : public std::enable_shared_from_this<BlockedFlush>
  {
    GET_CLASS_NAME(BlockedFlush)
  public:
    const std::shared_ptr<DocumentsWriterPerThread> dwpt;
    const int64_t bytes;
    // C++ TODO: No base class can be determined:
    BlockedFlush(std::shared_ptr<DocumentsWriterPerThread> dwpt,
                 int64_t bytes); // super();
  };

  /**
   * This method will block if too many DWPT are currently flushing and no
   * checked out DWPT are available
   */
public:
  void waitIfStalled();

  /**
   * Returns <code>true</code> iff stalled
   */
  bool anyStalledThreads();

  /**
   * Returns the {@link IndexWriter} {@link InfoStream}
   */
  std::shared_ptr<InfoStream> getInfoStream();

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<ThreadState> findLargestNonPendingWriter();

  /**
   * Returns the largest non-pending flushable DWPT or <code>null</code> if
   * there is none.
   */
  std::shared_ptr<DocumentsWriterPerThread> checkoutLargestNonPendingWriter();
};

} // namespace org::apache::lucene::index
