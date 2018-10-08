#pragma once
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class FieldNumbers;
}
namespace org::apache::lucene::index
{
class FlushNotifications;
}
namespace org::apache::lucene::util
{
class InfoStream;
}
namespace org::apache::lucene::index
{
class LiveIndexWriterConfig;
}
namespace org::apache::lucene::index
{
class DocumentsWriterDeleteQueue;
}
namespace org::apache::lucene::index
{
class DocumentsWriterFlushQueue;
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
class DocumentsWriterFlushControl;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class DocValuesUpdate;
}
namespace org::apache::lucene::index
{
class FlushTicket;
}
namespace org::apache::lucene::util
{
class IOUtils;
}
namespace org::apache::lucene::util
{
template <typename T>
class IOConsumer;
}
namespace org::apache::lucene::store
{
class AlreadyClosedException;
}
namespace org::apache::lucene::index
{
class ThreadState;
}
namespace org::apache::lucene::index
{
class DocumentsWriterPerThread;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::index
{
template <typename T>
class Node;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using Query = org::apache::lucene::search::Query;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * This class accepts multiple added documents and directly
 * writes segment files.
 *
 * Each added document is passed to the indexing chain,
 * which in turn processes the document into the different
 * codec formats.  Some formats write bytes to files
 * immediately, e.g. stored fields and term vectors, while
 * others are buffered by the indexing chain and written
 * only on flush.
 *
 * Once we have used our allowed RAM buffer, or the number
 * of added docs is large enough (in the case we are
 * flushing by doc count instead of RAM usage), we create a
 * real segment and flush it to the Directory.
 *
 * Threads:
 *
 * Multiple threads are allowed into addDocument at once.
 * There is an initial synchronized call to getThreadState
 * which allocates a ThreadState for this thread.  The same
 * thread will get the same ThreadState over time (thread
 * affinity) so that if there are consistent patterns (for
 * example each thread is indexing a different content
 * source) then we make better use of RAM.  Then
 * processDocument is called on that ThreadState without
 * synchronization (most of the "heavy lifting" is in this
 * call).  Finally the synchronized "finishDocument" is
 * called to flush changes to the directory.
 *
 * When flush is called by IndexWriter we forcefully idle
 * all threads and flush only once they are all idle.  This
 * means you can call flush with a given thread even while
 * other threads are actively adding/deleting documents.
 *
 *
 * Exceptions:
 *
 * Because this class directly updates in-memory posting
 * lists, and flushes stored fields and term vectors
 * directly to files in the directory, there are certain
 * limited times when an exception can corrupt this state.
 * For example, a disk full while flushing stored fields
 * leaves this file in a corrupt state.  Or, an OOM
 * exception while appending to the in-memory posting lists
 * can corrupt that posting deque.  We call such exceptions
 * "aborting exceptions".  In these cases we must call
 * abort() to discard all docs added since the last flush.
 *
 * All other exceptions ("non-aborting exceptions") can
 * still partially update the index structures.  These
 * updates are consistent, but, they represent only a part
 * of the document seen up until the exception was hit.
 * When this happens, we immediately mark the document as
 * deleted so that the document is always atomically ("all
 * or none") added to the index.
 */

class DocumentsWriter final
    : public std::enable_shared_from_this<DocumentsWriter>,
      public Accountable
{
  GET_CLASS_NAME(DocumentsWriter)
private:
  const std::shared_ptr<Directory> directoryOrig; // no wrapping, for infos
  const std::shared_ptr<Directory> directory;
  const std::shared_ptr<FieldInfos::FieldNumbers> globalFieldNumberMap;
  const int indexCreatedVersionMajor;
  const std::shared_ptr<AtomicLong> pendingNumDocs;
  const bool enableTestPoints;
  const std::function<std::wstring *()> segmentNameSupplier;
  const std::shared_ptr<FlushNotifications> flushNotifications;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed;
  bool closed = false;

  const std::shared_ptr<InfoStream> infoStream;

  const std::shared_ptr<LiveIndexWriterConfig> config;

  const std::shared_ptr<AtomicInteger> numDocsInRAM =
      std::make_shared<AtomicInteger>(0);

  // TODO: cut over to BytesRefHash in BufferedDeletes
public:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile DocumentsWriterDeleteQueue deleteQueue;
  std::shared_ptr<DocumentsWriterDeleteQueue> deleteQueue;

private:
  const std::shared_ptr<DocumentsWriterFlushQueue> ticketQueue =
      std::make_shared<DocumentsWriterFlushQueue>();
  /*
   * we preserve changes during a full flush since IW might not checkout before
   * we release all changes. NRT Readers otherwise suddenly return true from
   * isCurrent while there are actually changes currently committed. See also
   * #anyChanges() & #flushAllThreads
   */
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool pendingChangesInCurrentFullFlush;
  bool pendingChangesInCurrentFullFlush = false;

public:
  const std::shared_ptr<DocumentsWriterPerThreadPool> perThreadPool;
  const std::shared_ptr<FlushPolicy> flushPolicy;
  const std::shared_ptr<DocumentsWriterFlushControl> flushControl;

private:
  int64_t lastSeqNo = 0;

public:
  DocumentsWriter(
      std::shared_ptr<FlushNotifications> flushNotifications,
      int indexCreatedVersionMajor, std::shared_ptr<AtomicLong> pendingNumDocs,
      bool enableTestPoints,
      std::function<std::wstring *()> &segmentNameSupplier,
      std::shared_ptr<LiveIndexWriterConfig> config,
      std::shared_ptr<Directory> directoryOrig,
      std::shared_ptr<Directory> directory,
      std::shared_ptr<FieldInfos::FieldNumbers> globalFieldNumberMap);

  int64_t deleteQueries(std::deque<Query> &queries) ;

  void setLastSeqNo(int64_t seqNo);

  int64_t deleteTerms(std::deque<Term> &terms) ;

  int64_t
  updateDocValues(std::deque<DocValuesUpdate> &updates) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t
  applyDeleteOrUpdate(std::function<int64_t(DocumentsWriterDeleteQueue *)>
                          &function) ;

  /** If buffered deletes are using too much heap, resolve them and write disk
   * and return true. */
  bool applyAllDeletes(std::shared_ptr<DocumentsWriterDeleteQueue>
                           deleteQueue) ;

public:
  void
  purgeFlushTickets(bool forced,
                    IOUtils::IOConsumer<
                        std::shared_ptr<DocumentsWriterFlushQueue::FlushTicket>>
                        consumer) ;

  /** Returns how many docs are currently buffered in RAM. */
  int getNumDocs();

private:
  void ensureOpen() ;

  /** Called if we hit an exception at a bad time (when
   *  updating the index files) and must discard all
   *  currently buffered docs.  This resets our state,
   *  discarding any docs added since last flush. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void abort() ;

  bool flushOneDWPT() ;

  /** Locks all currently active DWPT and aborts them.
   *  The returned Closeable should be closed once the locks for the aborted
   *  DWPTs can be released. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Closeable> lockAndAbortAll() ;

  /** Returns how many documents were aborted. */
private:
  int abortThreadState(std::shared_ptr<ThreadState> perThread) throw(
      IOException);

  /** returns the maximum sequence number for all previously completed
   * operations */
public:
  int64_t getMaxCompletedSequenceNumber();

  bool anyChanges();

  int getBufferedDeleteTermsSize();

  // for testing
  int getNumBufferedDeleteTerms();

  bool anyDeletions();

  virtual ~DocumentsWriter();

private:
  bool preUpdate() ;

  bool postUpdate(std::shared_ptr<DocumentsWriterPerThread> flushingDWPT,
                  bool hasEvents) ;

  void ensureInitialized(std::shared_ptr<ThreadState> state) ;

public:
  template <typename T1, typename T2>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: long updateDocuments(final Iterable<? extends
  // Iterable<? extends IndexableField>> docs, final
  // org.apache.lucene.analysis.Analyzer analyzer, final
  // DocumentsWriterDeleteQueue.Node<?> delNode) throws java.io.IOException
  int64_t
  updateDocuments(std::deque<T1> const docs,
                  std::shared_ptr<Analyzer> analyzer,
                  std::shared_ptr<DocumentsWriterDeleteQueue::Node<T2>>
                      delNode) ;

  template <typename T1, typename T2>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: long updateDocument(final Iterable<? extends
  // IndexableField> doc, final org.apache.lucene.analysis.Analyzer analyzer,
  // final DocumentsWriterDeleteQueue.Node<?> delNode) throws java.io.IOException
  int64_t updateDocument(std::deque<T1> const doc,
                           std::shared_ptr<Analyzer> analyzer,
                           std::shared_ptr<DocumentsWriterDeleteQueue::Node<T2>>
                               delNode) ;

private:
  bool doFlush(std::shared_ptr<DocumentsWriterPerThread> flushingDWPT) throw(
      IOException);

public:
  class FlushNotifications
  { // TODO maybe we find a better name for this?
    GET_CLASS_NAME(FlushNotifications)

    /**
     * Called when files were written to disk that are not used anymore. It's
     * the implementation's responsibility to clean these files up
     */
  public:
    virtual void
    deleteUnusedFiles(std::shared_ptr<std::deque<std::wstring>> files) = 0;

    /**
     * Called when a segment failed to flush.
     */
    virtual void flushFailed(std::shared_ptr<SegmentInfo> info) = 0;

    /**
     * Called after one or more segments were flushed to disk.
     */
    virtual void afterSegmentsFlushed() = 0;

    /**
     * Should be called if a flush or an indexing operation caused a tragic /
     * unrecoverable event.
     */
    virtual void onTragicEvent(std::runtime_error event_,
                               const std::wstring &message) = 0;

    /**
     * Called once deletes have been applied either after a flush or on a
     * deletes call
     */
    virtual void onDeletesApplied() = 0;

    /**
     * Called once the DocumentsWriter ticket queue has a backlog. This means
     * there is an inner thread that tries to publish flushed segments but can't
     * keep up with the other threads flushing new segments. This likely
     * requires other thread to forcefully purge the buffer to help publishing.
     * This can't be done in-place since we might hold index writer locks when
     * this is called. The caller must ensure that the purge happens without an
     * index writer lock being held.
     *
     * @see DocumentsWriter#purgeFlushTickets(bool, IOUtils.IOConsumer)
     */
    virtual void onTicketBacklog() = 0;
  };

public:
  void subtractFlushedNumDocs(int numFlushed);

  // for asserts
private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile DocumentsWriterDeleteQueue
  // currentFullFlushDelQueue = null;
  std::shared_ptr<DocumentsWriterDeleteQueue> currentFullFlushDelQueue =
      nullptr;

  // for asserts
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool
  setFlushingDeleteQueue(std::shared_ptr<DocumentsWriterDeleteQueue> session);

  bool assertTicketQueueModification(
      std::shared_ptr<DocumentsWriterDeleteQueue> deleteQueue);

  /*
   * FlushAllThreads is synced by IW fullFlushLock. Flushing all threads is a
   * two stage operation; the caller must ensure (in try/finally) that
   * finishFlush is called after this method, to release the flush lock in
   * DWFlushControl
   */
public:
  int64_t flushAllThreads() ;

  void finishFullFlush(bool success) ;

  int64_t ramBytesUsed() override;
};

} // namespace org::apache::lucene::index
