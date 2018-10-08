#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThread.h"

#include  "core/src/java/org/apache/lucene/index/DocumentsWriter.h"

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

/**
 * {@link DocumentsWriterPerThreadPool} controls {@link ThreadState} instances
 * and their thread assignments during indexing. Each {@link ThreadState} holds
 * a reference to a {@link DocumentsWriterPerThread} that is once a
 * {@link ThreadState} is obtained from the pool exclusively used for indexing a
 * single document by the obtaining thread. Each indexing thread must obtain
 * such a {@link ThreadState} to make progress. Depending on the
 * {@link DocumentsWriterPerThreadPool} implementation {@link ThreadState}
 * assignments might differ from document to document.
 * <p>
 * Once a {@link DocumentsWriterPerThread} is selected for flush the thread pool
 * is reusing the flushing {@link DocumentsWriterPerThread}s ThreadState with a
 * new {@link DocumentsWriterPerThread} instance.
 * </p>
 */
class DocumentsWriterPerThreadPool final
    : public std::enable_shared_from_this<DocumentsWriterPerThreadPool>
{
  GET_CLASS_NAME(DocumentsWriterPerThreadPool)

  /**
   * {@link ThreadState} references and guards a
   * {@link DocumentsWriterPerThread} instance that is used during indexing to
   * build a in-memory index segment. {@link ThreadState} also holds all flush
   * related per-thread data controlled by {@link DocumentsWriterFlushControl}.
   * <p>
   * A {@link ThreadState}, its methods and members should only accessed by one
   * thread a time. Users must acquire the lock via {@link ThreadState#lock()}
   * and release the lock in a finally block via {@link ThreadState#unlock()}
   * before accessing the state.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("serial") final static class ThreadState
  // extends java.util.concurrent.locks.ReentrantLock
  class ThreadState final : public ReentrantLock
  {
  public:
    std::shared_ptr<DocumentsWriterPerThread> dwpt;
    // TODO this should really be part of DocumentsWriterFlushControl
    // write access guarded by DocumentsWriterFlushControl
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool flushPending = false;
    bool flushPending = false;
    // TODO this should really be part of DocumentsWriterFlushControl
    // write access guarded by DocumentsWriterFlushControl
    int64_t bytesUsed = 0;

    // set by DocumentsWriter after each indexing op finishes
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile long lastSeqNo;
    int64_t lastSeqNo = 0;

    ThreadState(std::shared_ptr<DocumentsWriterPerThread> dpwt);

  private:
    void reset();

  public:
    bool isInitialized();

    /**
     * Returns the number of currently active bytes in this ThreadState's
     * {@link DocumentsWriterPerThread}
     */
    int64_t getBytesUsedPerThread();

    /**
     * Returns this {@link ThreadState}s {@link DocumentsWriterPerThread}
     */
    std::shared_ptr<DocumentsWriterPerThread> getDocumentsWriterPerThread();

    /**
     * Returns <code>true</code> iff this {@link ThreadState} is marked as flush
     * pending otherwise <code>false</code>
     */
    bool isFlushPending();

  protected:
    std::shared_ptr<ThreadState> shared_from_this()
    {
      return std::static_pointer_cast<ThreadState>(
          java.util.concurrent.locks.ReentrantLock::shared_from_this());
    }
  };

private:
  const std::deque<std::shared_ptr<ThreadState>> threadStates =
      std::deque<std::shared_ptr<ThreadState>>();

  const std::deque<std::shared_ptr<ThreadState>> freeList =
      std::deque<std::shared_ptr<ThreadState>>();

  bool aborted = false;

  /**
   * Returns the active number of {@link ThreadState} instances.
   */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int getActiveThreadStateCount();

  // C++ WARNING: The following method was originally marked 'synchronized':
  void setAbort();

  // C++ WARNING: The following method was originally marked 'synchronized':
  void clearAbort();

  /**
   * Returns a new {@link ThreadState} iff any new state is available otherwise
   * <code>null</code>.
   * <p>
   * NOTE: the returned {@link ThreadState} is already locked iff non-
   * <code>null</code>.
   *
   * @return a new {@link ThreadState} iff any new state is available otherwise
   *         <code>null</code>
   */
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<ThreadState> newThreadState();

public:
  std::shared_ptr<DocumentsWriterPerThread>
  reset(std::shared_ptr<ThreadState> threadState);

  void recycle(std::shared_ptr<DocumentsWriterPerThread> dwpt);

  // TODO: maybe we should try to do load leveling here: we want roughly even
  // numbers of items (docs, deletes, DV updates) to most take advantage of
  // concurrency while flushing

  /** This method is used by DocumentsWriter/FlushControl to obtain a
   * ThreadState to do an indexing operation (add/updateDocument). */
  std::shared_ptr<ThreadState>
  getAndLock(std::shared_ptr<Thread> requestingThread,
             std::shared_ptr<DocumentsWriter> documentsWriter);

  void release(std::shared_ptr<ThreadState> state);

  /**
   * Returns the <i>i</i>th active {@link ThreadState} where <i>i</i> is the
   * given ord.
   *
   * @param ord
   *          the ordinal of the {@link ThreadState}
   * @return the <i>i</i>th active {@link ThreadState} where <i>i</i> is the
   *         given ord.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<ThreadState> getThreadState(int ord);

  // TODO: merge this with getActiveThreadStateCount: they are the same!
  // C++ WARNING: The following method was originally marked 'synchronized':
  int getMaxThreadStates();
};

} // #include  "core/src/java/org/apache/lucene/index/
