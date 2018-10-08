#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <functional>
#include <limits>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/FieldNumbers.h"
#include  "core/src/java/org/apache/lucene/index/DocumentsWriter.h"
#include  "core/src/java/org/apache/lucene/index/Event.h"
#include  "core/src/java/org/apache/lucene/index/IndexFileDeleter.h"
#include  "core/src/java/org/apache/lucene/store/Lock.h"
#include  "core/src/java/org/apache/lucene/index/MergeScheduler.h"
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/OneMerge.h"
#include  "core/src/java/org/apache/lucene/index/ReaderPool.h"
#include  "core/src/java/org/apache/lucene/index/BufferedUpdatesStream.h"
#include  "core/src/java/org/apache/lucene/index/LiveIndexWriterConfig.h"
#include  "core/src/java/org/apache/lucene/index/FlushNotifications.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/store/AlreadyClosedException.h"
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include  "core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterDeleteQueue.h"
namespace org::apache::lucene::index
{
template <typename T>
class Node;
}
#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/ReadersAndUpdates.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesUpdate.h"
#include  "core/src/java/org/apache/lucene/index/FrozenBufferedUpdates.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesFieldUpdates.h"
#include  "core/src/java/org/apache/lucene/index/Iterator.h"
#include  "core/src/java/org/apache/lucene/index/MergeState.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/store/TrackingDirectoryWrapper.h"
#include  "core/src/java/org/apache/lucene/util/IOUtils.h"
namespace org::apache::lucene::util
{
template <typename T>
class IOConsumer;
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
using Codec = org::apache::lucene::codecs::Codec;
using Field = org::apache::lucene::document::Field;
using FieldNumbers = org::apache::lucene::index::FieldInfos::FieldNumbers;
using Query = org::apache::lucene::search::Query;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Lock = org::apache::lucene::store::Lock;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using Accountable = org::apache::lucene::util::Accountable;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
  An <code>IndexWriter</code> creates and maintains an index.

  <p>The {@link OpenMode} option on
  {@link IndexWriterConfig#setOpenMode(OpenMode)} determines
  whether a new index is created, or whether an existing index is
  opened. Note that you can open an index with {@link OpenMode#CREATE}
  even while readers are using the index. The old readers will
  continue to search the "point in time" snapshot they had opened,
  and won't see the newly created index until they re-open. If
  {@link OpenMode#CREATE_OR_APPEND} is used IndexWriter will create a
  new index if there is not already an index at the provided path
  and otherwise open the existing index.</p>

  <p>In either case, documents are added with {@link #addDocument(Iterable)
  addDocument} and removed with {@link #deleteDocuments(Term...)} or {@link
  #deleteDocuments(Query...)}. A document can be updated with {@link
  #updateDocument(Term, Iterable) updateDocument} (which just deletes
  and then adds the entire document). When finished adding, deleting
  and updating documents, {@link #close() close} should be called.</p>

  <a name="sequence_numbers"></a>
  <p>Each method that changes the index returns a {@code long} sequence number,
  which expresses the effective order in which each change was applied.
  {@link #commit} also returns a sequence number, describing which
  changes are in the commit point and which are not.  Sequence numbers
  are transient (not saved into the index in any way) and only valid
  within a single {@code IndexWriter} instance.</p>

  <a name="flush"></a>
  <p>These changes are buffered in memory and periodically
  flushed to the {@link Directory} (during the above method
  calls). A flush is triggered when there are enough added documents
  since the last flush. Flushing is triggered either by RAM usage of the
  documents (see {@link IndexWriterConfig#setRAMBufferSizeMB}) or the
  number of added documents (see {@link
  IndexWriterConfig#setMaxBufferedDocs(int)}). The default is to flush when RAM
  usage hits
  {@link IndexWriterConfig#DEFAULT_RAM_BUFFER_SIZE_MB} MB. For
  best indexing speed you should flush by RAM usage with a
  large RAM buffer.
  In contrast to the other flush options {@link
  IndexWriterConfig#setRAMBufferSizeMB} and
  {@link IndexWriterConfig#setMaxBufferedDocs(int)}, deleted terms
  won't trigger a segment flush. Note that flushing just moves the
  internal buffered state in IndexWriter into the index, but
  these changes are not visible to IndexReader until either
  {@link #commit()} or {@link #close} is called.  A flush may
  also trigger one or more segment merges which by default
  run with a background thread so as not to block the
  addDocument calls (see <a href="#mergePolicy">below</a>
  for changing the {@link MergeScheduler}).</p>

  <p>Opening an <code>IndexWriter</code> creates a lock file for the directory
  in use. Trying to open another <code>IndexWriter</code> on the same directory
  will lead to a
  {@link LockObtainFailedException}.</p>

  <a name="deletionPolicy"></a>
  <p>Expert: <code>IndexWriter</code> allows an optional
  {@link IndexDeletionPolicy} implementation to be specified.  You
  can use this to control when prior commits are deleted from
  the index.  The default policy is {@link KeepOnlyLastCommitDeletionPolicy}
  which removes all prior commits as soon as a new commit is
  done.  Creating your own policy can allow you to explicitly
  keep previous "point in time" commits alive in the index for
  some time, either because this is useful for your application,
  or to give readers enough time to refresh to the new commit
  without having the old commit deleted out from under them.
  The latter is necessary when multiple computers take turns opening
  their own {@code IndexWriter} and {@code IndexReader}s
  against a single shared index mounted via remote filesystems
  like NFS which do not support "delete on last close" semantics.
  A single computer accessing an index via NFS is fine with the
  default deletion policy since NFS clients emulate "delete on
  last close" locally.  That said, accessing an index via NFS
  will likely result in poor performance compared to a local IO
  device. </p>

  <a name="mergePolicy"></a> <p>Expert:
  <code>IndexWriter</code> allows you to separately change
  the {@link MergePolicy} and the {@link MergeScheduler}.
  The {@link MergePolicy} is invoked whenever there are
  changes to the segments in the index.  Its role is to
  select which merges to do, if any, and return a {@link
  MergePolicy.MergeSpecification} describing the merges.
  The default is {@link LogByteSizeMergePolicy}.  Then, the {@link
  MergeScheduler} is invoked with the requested merges and
  it decides when and how to run the merges.  The default is
  {@link ConcurrentMergeScheduler}. </p>

  <a name="OOME"></a><p><b>NOTE</b>: if you hit a
  VirtualMachineError, or disaster strikes during a checkpoint
  then IndexWriter will close itself.  This is a
  defensive measure in case any internal state (buffered
  documents, deletions, reference counts) were corrupted.
  Any subsequent calls will throw an AlreadyClosedException.</p>

  <a name="thread-safety"></a><p><b>NOTE</b>: {@link
  IndexWriter} instances are completely thread
  safe, meaning multiple threads can call any of its
  methods, concurrently.  If your application requires
  external synchronization, you should <b>not</b>
  synchronize on the <code>IndexWriter</code> instance as
  this may cause deadlock; use your own (non-Lucene) objects
  instead. </p>

  <p><b>NOTE</b>: If you call
  <code>Thread.interrupt()</code> on a thread that's within
  IndexWriter, IndexWriter will try to catch this (eg, if
  it's in a wait() or Thread.sleep()), and will then throw
  the unchecked exception {@link ThreadInterruptedException}
  and <b>clear</b> the interrupt status on the thread.</p>
*/

/*
 * Clarification: Check Points (and commits)
 * IndexWriter writes new index files to the directory without writing a new
 * segments_N file which references these new files. It also means that the
 * state of the in memory SegmentInfos object is different than the most recent
 * segments_N file written to the directory.
 *
 * Each time the SegmentInfos is changed, and matches the (possibly
 * modified) directory files, we have a new "check point".
 * If the modified/new SegmentInfos is written to disk - as a new
 * (generation of) segments_N file - this check point is also an
 * IndexCommit.
 *
 * A new checkpoint always replaces the previous checkpoint and
 * becomes the new "front" of the index. This allows the IndexFileDeleter
 * to delete files that are referenced only by stale checkpoints.
 * (files that were created since the last commit, but are no longer
 * referenced by the "front" of the index). For this, IndexFileDeleter
 * keeps track of the last non commit checkpoint.
 */
class IndexWriter : public std::enable_shared_from_this<IndexWriter>,
                    public TwoPhaseCommit,
                    public Accountable,
                    public MergePolicy::MergeContext
{
  GET_CLASS_NAME(IndexWriter)

  /** Hard limit on maximum number of documents that may be added to the
   *  index.  If you try to add more than this you'll hit {@code
   * IllegalArgumentException}. */
  // We defensively subtract 128 to be well below the lowest
  // ArrayUtil.MAX_ARRAY_LENGTH on "typical" JVMs.  We don't just use
  // ArrayUtil.MAX_ARRAY_LENGTH here because this can vary across JVMs:
public:
  static const int MAX_DOCS = std::numeric_limits<int>::max() - 128;

  /** Maximum value of the token position in an indexed field. */
  static const int MAX_POSITION = std::numeric_limits<int>::max() - 128;

  // Use package-private instance var to enforce the limit so testing
  // can use less electricity:
private:
  static int actualMaxDocs;

  /** Used only for testing. */
public:
  static void setMaxDocs(int maxDocs);

  static int getActualMaxDocs();

  /** Used only for testing. */
private:
  const bool enableTestPoints;

public:
  static constexpr int UNBOUNDED_MAX_MERGE_SEGMENTS = -1;

  /**
   * Name of the write lock in the index.
   */
  static const std::wstring WRITE_LOCK_NAME;

  /** Key for the source of a segment in the {@link SegmentInfo#getDiagnostics()
   * diagnostics}. */
  static const std::wstring SOURCE;
  /** Source of a segment which results from a merge of other segments. */
  static const std::wstring SOURCE_MERGE;
  /** Source of a segment which results from a flush. */
  static const std::wstring SOURCE_FLUSH;
  /** Source of a segment which results from a call to {@link
   * #addIndexes(CodecReader...)}. */
  static const std::wstring SOURCE_ADDINDEXES_READERS;

  /**
   * Absolute hard maximum length for a term, in bytes once
   * encoded as UTF8.  If a term arrives from the analyzer
   * longer than this length, an
   * <code>IllegalArgumentException</code>  is thrown
   * and a message is printed to infoStream, if set (see {@link
   * IndexWriterConfig#setInfoStream(InfoStream)}).
   */
  static const int MAX_TERM_LENGTH =
      DocumentsWriterPerThread::MAX_TERM_LENGTH_UTF8;

  /**
   * Maximum length string for a stored field.
   */
  static const int MAX_STORED_STRING_LENGTH =
      ArrayUtil::MAX_ARRAY_LENGTH / UnicodeUtil::MAX_UTF8_BYTES_PER_CHAR;

  // when unrecoverable disaster strikes, we populate this with the reason that
  // we had to close IndexWriter
  const std::shared_ptr<AtomicReference<std::runtime_error>> tragedy =
      std::make_shared<AtomicReference<std::runtime_error>>(nullptr);

private:
  const std::shared_ptr<Directory> directoryOrig; // original user directory
  const std::shared_ptr<Directory> directory; // wrapped with additional checks
  const std::shared_ptr<Analyzer> analyzer;   // how to analyze text

  const std::shared_ptr<AtomicLong> changeCount =
      std::make_shared<AtomicLong>(); // increments every time a change is
                                      // completed
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long lastCommitChangeCount;
  int64_t lastCommitChangeCount = 0; // last changeCount that was committed

  std::deque<std::shared_ptr<SegmentCommitInfo>>
      rollbackSegments; // deque of segmentInfo we will fallback to if the commit
                        // fails

public:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile SegmentInfos pendingCommit;
  std::shared_ptr<SegmentInfos>
      pendingCommit; // set when a commit is pending (after prepareCommit() &
                     // before commit())
                     // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile long pendingSeqNo;
  int64_t pendingSeqNo = 0;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile long pendingCommitChangeCount;
  int64_t pendingCommitChangeCount = 0;

private:
  std::shared_ptr<std::deque<std::wstring>> filesToCommit;

public:
  const std::shared_ptr<SegmentInfos> segmentInfos; // the segments
  const std::shared_ptr<FieldNumbers> globalFieldNumberMap;

  const std::shared_ptr<DocumentsWriter> docWriter;

private:
  const std::deque<Event> eventQueue =
      std::make_shared<ConcurrentLinkedQueue<Event>>();

public:
  const std::shared_ptr<IndexFileDeleter> deleter;

  // used by forceMerge to note those needing merging
private:
  std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool> segmentsToMerge =
      std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>();
  int mergeMaxNumSegments = 0;

  std::shared_ptr<Lock> writeLock;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed;
  bool closed = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closing;
  bool closing = false;

public:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<AtomicBoolean> maybeMerge_ =
      std::make_shared<AtomicBoolean>();

private:
  std::deque<std::unordered_map::Entry<std::wstring, std::wstring>>
      commitUserData;

  // Holds all SegmentInfo instances currently involved in
  // merges
public:
  std::unordered_set<std::shared_ptr<SegmentCommitInfo>> mergingSegments =
      std::unordered_set<std::shared_ptr<SegmentCommitInfo>>();

private:
  const std::shared_ptr<MergeScheduler> mergeScheduler;
  std::deque<std::shared_ptr<MergePolicy::OneMerge>> pendingMerges =
      std::deque<std::shared_ptr<MergePolicy::OneMerge>>();
  std::shared_ptr<Set<std::shared_ptr<MergePolicy::OneMerge>>> runningMerges =
      std::unordered_set<std::shared_ptr<MergePolicy::OneMerge>>();
  std::deque<std::shared_ptr<MergePolicy::OneMerge>> mergeExceptions =
      std::deque<std::shared_ptr<MergePolicy::OneMerge>>();
  int64_t mergeGen = 0;
  bool stopMerges = false;
  bool didMessageState = false;

public:
  const std::shared_ptr<AtomicInteger> flushCount =
      std::make_shared<AtomicInteger>();

  const std::shared_ptr<AtomicInteger> flushDeletesCount =
      std::make_shared<AtomicInteger>();

private:
  const std::shared_ptr<ReaderPool> readerPool;

public:
  const std::shared_ptr<BufferedUpdatesStream> bufferedUpdatesStream;

  /** Counts how many merges have completed; this is used by {@link
   * FrozenBufferedUpdates#apply} to handle concurrently apply deletes/updates
   * with merges completing. */
  const std::shared_ptr<AtomicLong> mergeFinishedGen =
      std::make_shared<AtomicLong>();

  // The instance that was passed to the constructor. It is saved only in order
  // to allow users to query an IndexWriter settings.
private:
  const std::shared_ptr<LiveIndexWriterConfig> config;

  /** System.nanoTime() when commit started; used to write
   *  an infoStream message about how long commit took. */
  int64_t startCommitTime = 0;

  /** How many documents are in the index, or are in the process of being
   *  added (reserved).  E.g., operations like addIndexes will first reserve
   *  the right to add N docs, before they actually change the index,
   *  much like how hotels place an "authorization hold" on your credit
   *  card to make sure they can later charge you when you check out. */
public:
  const std::shared_ptr<AtomicLong> pendingNumDocs =
      std::make_shared<AtomicLong>();
  const bool softDeletesEnabled;

private:
  const std::shared_ptr<DocumentsWriter::FlushNotifications>
      flushNotifications =
          std::make_shared<FlushNotificationsAnonymousInnerClass>();

private:
  class FlushNotificationsAnonymousInnerClass
      : public std::enable_shared_from_this<
            FlushNotificationsAnonymousInnerClass>,
        public DocumentsWriter::FlushNotifications
  {
    GET_CLASS_NAME(FlushNotificationsAnonymousInnerClass)
  public:
    FlushNotificationsAnonymousInnerClass();

    void
    deleteUnusedFiles(std::shared_ptr<std::deque<std::wstring>> files) override;

    void flushFailed(std::shared_ptr<SegmentInfo> info) override;

    void afterSegmentsFlushed()  override;

    void onTragicEvent(std::runtime_error event_,
                       const std::wstring &message) override;

    void onDeletesApplied() override;

    void onTicketBacklog() override;
  };

public:
  virtual std::shared_ptr<DirectoryReader> getReader() ;

  /**
   * Expert: returns a readonly reader, covering all
   * committed as well as un-committed changes to the index.
   * This provides "near real-time" searching, in that
   * changes made during an IndexWriter session can be
   * quickly made available for searching without closing
   * the writer nor calling {@link #commit}.
   *
   * <p>Note that this is functionally equivalent to calling
   * {#flush} and then opening a new reader.  But the turnaround time of this
   * method should be faster since it avoids the potentially
   * costly {@link #commit}.</p>
   *
   * <p>You must close the {@link IndexReader} returned by
   * this method once you are done using it.</p>
   *
   * <p>It's <i>near</i> real-time because there is no hard
   * guarantee on how quickly you can get a new reader after
   * making changes with IndexWriter.  You'll have to
   * experiment in your situation to determine if it's
   * fast enough.  As this is a new and experimental
   * feature, please report back on your findings so we can
   * learn, improve and iterate.</p>
   *
   * <p>The resulting reader supports {@link
   * DirectoryReader#openIfChanged}, but that call will simply forward
   * back to this method (though this may change in the
   * future).</p>
   *
   * <p>The very first time this method is called, this
   * writer instance will make every effort to pool the
   * readers that it opens for doing merges, applying
   * deletes, etc.  This means additional resources (RAM,
   * file descriptors, CPU time) will be consumed.</p>
   *
   * <p>For lower latency on reopening a reader, you should
   * call {@link IndexWriterConfig#setMergedSegmentWarmer} to
   * pre-warm a newly merged segment before it's committed
   * to the index.  This is important for minimizing
   * index-to-search delay after a large merge.  </p>
   *
   * <p>If an addIndexes* call is running in another thread,
   * then this reader will only search those segments from
   * the foreign index that have been successfully copied
   * over, so far</p>.
   *
   * <p><b>NOTE</b>: Once the writer is closed, any
   * outstanding readers may continue to be used.  However,
   * if you attempt to reopen any of those readers, you'll
   * hit an {@link AlreadyClosedException}.</p>
   *
   * @lucene.experimental
   *
   * @return IndexReader that covers entire index plus all
   * changes made so far by this IndexWriter instance
   *
   * @throws IOException If there is a low-level I/O error
   */
  virtual std::shared_ptr<DirectoryReader>
  getReader(bool applyAllDeletes, bool writeAllDeletes) ;

  int64_t ramBytesUsed() override;

  int64_t getReaderPoolRamBytesUsed();

private:
  const std::shared_ptr<AtomicBoolean> writeDocValuesLock =
      std::make_shared<AtomicBoolean>();

public:
  virtual void writeSomeDocValuesUpdates() ;

  /**
   * Obtain the number of deleted docs for a pooled reader.
   * If the reader isn't being pooled, the segmentInfo's
   * delCount is returned.
   */
  int numDeletedDocs(std::shared_ptr<SegmentCommitInfo> info) override;

  /**
   * Used internally to throw an {@link AlreadyClosedException} if this
   * IndexWriter has been closed or is in the process of closing.
   *
   * @param failIfClosing
   *          if true, also fail when {@code IndexWriter} is in the process of
   *          closing ({@code closing=true}) but not yet done closing (
   *          {@code closed=false})
   * @throws AlreadyClosedException
   *           if this IndexWriter is closed or in the process of closing
   */
protected:
  void ensureOpen(bool failIfClosing) ;

  /**
   * Used internally to throw an {@link
   * AlreadyClosedException} if this IndexWriter has been
   * closed ({@code closed=true}) or is in the process of
   * closing ({@code closing=true}).
   * <p>
   * Calls {@link #ensureOpen(bool) ensureOpen(true)}.
   * @throws AlreadyClosedException if this IndexWriter is closed
   */
  void ensureOpen() ;

public:
  const std::shared_ptr<Codec> codec; // for writing new segments

  /**
   * Constructs a new IndexWriter per the settings given in <code>conf</code>.
   * If you want to make "live" changes to this writer instance, use
   * {@link #getConfig()}.
   *
   * <p>
   * <b>NOTE:</b> after ths writer is created, the given configuration instance
   * cannot be passed to another writer.
   *
   * @param d
   *          the index directory. The index is either created or appended
   *          according <code>conf.getOpenMode()</code>.
   * @param conf
   *          the configuration settings according to which IndexWriter should
   *          be initialized.
   * @throws IOException
   *           if the directory cannot be read/written to, or if it does not
   *           exist and <code>conf.getOpenMode()</code> is
   *           <code>OpenMode.APPEND</code> or if there is any other low-level
   *           IO error
   */
  IndexWriter(std::shared_ptr<Directory> d,
              std::shared_ptr<IndexWriterConfig> conf) ;

  /** Confirms that the incoming index sort (if any) matches the existing index
   * sort (if any). This is unfortunately just best effort, because it could be
   * the old index only has unsorted flushed segments built before {@link
   * Version#LUCENE_6_5_0} (flushed segments are sorted in Lucene 7.0).  */
private:
  void validateIndexSort() ;

  // reads latest field infos for the commit
  // this is used on IW init and addIndexes(Dir) to create/update the global
  // field map_obj.
  // TODO: fix tests abusing this method!
public:
  static std::shared_ptr<FieldInfos>
  readFieldInfos(std::shared_ptr<SegmentCommitInfo> si) ;

  /**
   * Loads or returns the already loaded the global field number map_obj for this
   * {@link SegmentInfos}. If this {@link SegmentInfos} has no global field
   * number map_obj the returned instance is empty
   */
private:
  std::shared_ptr<FieldNumbers> getFieldNumberMap() ;

  /**
   * Returns a {@link LiveIndexWriterConfig}, which can be used to query the
   * IndexWriter current settings, as well as modify "live" ones.
   */
public:
  virtual std::shared_ptr<LiveIndexWriterConfig> getConfig();

private:
  void messageState();

  /**
   * Gracefully closes (commits, waits for merges), but calls rollback
   * if there's an exc so the IndexWriter is always closed.  This is called
   * from {@link #close} when {@link IndexWriterConfig#commitOnClose} is
   * {@code true}.
   */
  void shutdown() ;

  /**
   * Closes all open resources and releases the write lock.
   *
   * If {@link IndexWriterConfig#commitOnClose} is <code>true</code>,
   * this will attempt to gracefully shut down by writing any
   * changes, waiting for any running merges, committing, and closing.
   * In this case, note that:
   * <ul>
   *   <li>If you called prepareCommit but failed to call commit, this
   *       method will throw {@code IllegalStateException} and the {@code
   * IndexWriter} will not be closed.</li> <li>If this method throws any other
   * exception, the {@code IndexWriter} will be closed, but changes may have
   * been lost.</li>
   * </ul>
   *
   * <p>
   * Note that this may be a costly
   * operation, so, try to re-use a single writer instead of
   * closing and opening a new one.  See {@link #commit()} for
   * caveats about write caching done by some IO devices.
   *
   * <p><b>NOTE</b>: You must ensure no other threads are still making
   * changes at the same time that this method is invoked.</p>
   */
public:
  virtual ~IndexWriter();

  // Returns true if this thread should attempt to close, or
  // false if IndexWriter is now closed; else,
  // waits until another thread finishes closing
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool shouldClose(bool waitForClose);

  /** Returns the Directory used by this index. */
public:
  virtual std::shared_ptr<Directory> getDirectory();

  std::shared_ptr<InfoStream> getInfoStream() override;

  /** Returns the analyzer used by this index. */
  virtual std::shared_ptr<Analyzer> getAnalyzer();

  /** Returns total number of docs in this index, including
   *  docs not yet flushed (still in the RAM buffer),
   *  not counting deletions.
   *  @see #numDocs */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int maxDoc();

  /** If {@link SegmentInfos#getVersion} is below {@code newVersion} then update
   * it to this value.
   *
   * @lucene.internal */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void advanceSegmentInfosVersion(int64_t newVersion);

  /** Returns total number of docs in this index, including
   *  docs not yet flushed (still in the RAM buffer), and
   *  including deletions.  <b>NOTE:</b> buffered deletions
   *  are not counted.  If you really need these to be
   *  counted you should call {@link #commit()} first.
   *  @see #numDocs */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int numDocs();

  /**
   * Returns true if this index has deletions (including
   * buffered deletions).  Note that this will return true
   * if there are buffered Term/Query deletions, even if it
   * turns out those buffered deletions don't match any
   * documents.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool hasDeletions();

  /**
   * Adds a document to this index.
   *
   * <p> Note that if an Exception is hit (for example disk full)
   * then the index will be consistent, but this document
   * may not have been added.  Furthermore, it's possible
   * the index will have one segment in non-compound format
   * even when using compound files (when a merge has
   * partially succeeded).</p>
   *
   * <p> This method periodically flushes pending documents
   * to the Directory (see <a href="#flush">above</a>), and
   * also periodically triggers segment merges in the index
   * according to the {@link MergePolicy} in use.</p>
   *
   * <p>Merges temporarily consume space in the
   * directory. The amount of space required is up to 1X the
   * size of all segments being merged, when no
   * readers/searchers are open against the index, and up to
   * 2X the size of all segments being merged when
   * readers/searchers are open against the index (see
   * {@link #forceMerge(int)} for details). The sequence of
   * primitive merge operations performed is governed by the
   * merge policy.
   *
   * <p>Note that each term in the document can be no longer
   * than {@link #MAX_TERM_LENGTH} in bytes, otherwise an
   * IllegalArgumentException will be thrown.</p>
   *
   * <p>Note that it's possible to create an invalid Unicode
   * string in java if a UTF16 surrogate pair is malformed.
   * In this case, the invalid characters are silently
   * replaced with the Unicode replacement character
   * U+FFFD.</p>
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long addDocument(Iterable<? extends
  // IndexableField> doc) throws java.io.IOException
  int64_t addDocument(std::deque<T1> doc) ;

  /**
   * Atomically adds a block of documents with sequentially
   * assigned document IDs, such that an external reader
   * will see all or none of the documents.
   *
   * <p><b>WARNING</b>: the index does not currently record
   * which documents were added as a block.  Today this is
   * fine, because merging will preserve a block. The order of
   * documents within a segment will be preserved, even when child
   * documents within a block are deleted. Most search features
   * (like result grouping and block joining) require you to
   * mark documents; when these documents are deleted these
   * search features will not work as expected. Obviously adding
   * documents to an existing block will require you the reindex
   * the entire block.
   *
   * <p>However it's possible that in the future Lucene may
   * merge more aggressively re-order documents (for example,
   * perhaps to obtain better index compression), in which case
   * you may need to fully re-index your documents at that time.
   *
   * <p>See {@link #addDocument(Iterable)} for details on
   * index and IndexWriter state after an Exception, and
   * flushing/merging temporary free space requirements.</p>
   *
   * <p><b>NOTE</b>: tools that do offline splitting of an index
   * (for example, IndexSplitter in contrib) or
   * re-sorting of documents (for example, IndexSorter in
   * contrib) are not aware of these atomically added documents
   * and will likely break them up.  Use such tools at your
   * own risk!
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   *
   * @lucene.experimental
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long addDocuments(Iterable<? extends
  // Iterable<? extends IndexableField>> docs) throws java.io.IOException
  int64_t addDocuments(std::deque<T1> docs) ;

  /**
   * Atomically deletes documents matching the provided
   * delTerm and adds a block of documents with sequentially
   * assigned document IDs, such that an external reader
   * will see all or none of the documents.
   *
   * See {@link #addDocuments(Iterable)}.
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   *
   * @lucene.experimental
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long updateDocuments(Term delTerm,
  // Iterable<? extends Iterable<? extends IndexableField>> docs) throws
  // java.io.IOException
  int64_t updateDocuments(std::shared_ptr<Term> delTerm,
                            std::deque<T1> docs) ;

private:
  template <typename T1, typename T2>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: private long updateDocuments(final
  // DocumentsWriterDeleteQueue.Node<?> delNode, Iterable<? extends Iterable<?
  // extends IndexableField>> docs) throws java.io.IOException
  int64_t
  updateDocuments(std::shared_ptr<DocumentsWriterDeleteQueue::Node<T1>> delNode,
                  std::deque<T2> docs) ;

  /**
   * Expert:
   * Atomically updates documents matching the provided
   * term with the given doc-values fields
   * and adds a block of documents with sequentially
   * assigned document IDs, such that an external reader
   * will see all or none of the documents.
   *
   * One use of this API is to retain older versions of
   * documents instead of replacing them. The existing
   * documents can be updated to reflect they are no
   * longer current while atomically adding new documents
   * at the same time.
   *
   * In contrast to {@link #updateDocuments(Term, Iterable)}
   * this method will not delete documents in the index
   * matching the given term but instead update them with
   * the given doc-values fields which can be used as a
   * soft-delete mechanism.
   *
   * See {@link #addDocuments(Iterable)}
   * and {@link #updateDocuments(Term, Iterable)}.
   *
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   *
   * @lucene.experimental
   */
public:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long softUpdateDocuments(Term term,
  // Iterable<? extends Iterable<? extends IndexableField>> docs,
  // org.apache.lucene.document.Field... softDeletes) throws java.io.IOException
  int64_t
  softUpdateDocuments(std::shared_ptr<Term> term, std::deque<T1> docs,
                      std::deque<Field> &softDeletes) ;

  /** Expert: attempts to delete by document ID, as long as
   *  the provided reader is a near-real-time reader (from {@link
   *  DirectoryReader#open(IndexWriter)}).  If the
   *  provided reader is an NRT reader obtained from this
   *  writer, and its segment has not been merged away, then
   *  the delete succeeds and this method returns a valid (&gt; 0) sequence
   *  number; else, it returns -1 and the caller must then
   *  separately delete by Term or Query.
   *
   *  <b>NOTE</b>: this method can only delete documents
   *  visible to the currently open NRT reader.  If you need
   *  to delete documents indexed after opening the NRT
   *  reader you must use {@link #deleteDocuments(Term...)}). */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t tryDeleteDocument(std::shared_ptr<IndexReader> readerIn,
                                      int docID) ;

  /** Expert: attempts to update doc values by document ID, as long as
   *  the provided reader is a near-real-time reader (from {@link
   *  DirectoryReader#open(IndexWriter)}).  If the
   *  provided reader is an NRT reader obtained from this
   *  writer, and its segment has not been merged away, then
   *  the update succeeds and this method returns a valid (&gt; 0) sequence
   *  number; else, it returns -1 and the caller must then
   *  either retry the update and resolve the document again.
   *  If a doc values fields data is <code>null</code> the existing
   *  value is removed from all documents matching the term. This can be used
   *  to un-delete a soft-deleted document since this method will apply the
   *  field update even if the document is marked as deleted.
   *
   *  <b>NOTE</b>: this method can only updates documents
   *  visible to the currently open NRT reader.  If you need
   *  to update documents indexed after opening the NRT
   *  reader you must use {@link #updateDocValues(Term, Field...)}. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t
  tryUpdateDocValue(std::shared_ptr<IndexReader> readerIn, int docID,
                    std::deque<Field> &fields) ;

  using DocModifier =
      std::function<void(int docId, ReadersAndUpdates readersAndUpdates)>;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t tryModifyDocument(std::shared_ptr<IndexReader> readerIn, int docID,
                              DocModifier toApply) ;

  /** Drops a segment that has 100% deleted documents. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void dropDeletedSegment(
      std::shared_ptr<SegmentCommitInfo> info) ;

  /**
   * Deletes the document(s) containing any of the
   * terms. All given deletes are applied and flushed atomically
   * at the same time.
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @param terms array of terms to identify the documents
   * to be deleted
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  virtual int64_t
  deleteDocuments(std::deque<Term> &terms) ;

  /**
   * Deletes the document(s) matching any of the provided queries.
   * All given deletes are applied and flushed atomically at the same time.
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @param queries array of queries to identify the documents
   * to be deleted
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  virtual int64_t
  deleteDocuments(std::deque<Query> &queries) ;

  /**
   * Updates a document by first deleting the document(s)
   * containing <code>term</code> and then adding the new
   * document.  The delete and then add are atomic as seen
   * by a reader on the same index (flush may happen only after
   * the add).
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @param term the term to identify the document(s) to be
   * deleted
   * @param doc the document to be added
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long updateDocument(Term term, Iterable<?
  // extends IndexableField> doc) throws java.io.IOException
  int64_t updateDocument(std::shared_ptr<Term> term,
                           std::deque<T1> doc) ;

private:
  template <typename T1, typename T2>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: private long updateDocument(final
  // DocumentsWriterDeleteQueue.Node<?> delNode, Iterable<? extends
  // IndexableField> doc) throws java.io.IOException
  int64_t
  updateDocument(std::shared_ptr<DocumentsWriterDeleteQueue::Node<T1>> delNode,
                 std::deque<T2> doc) ;

  /**
   * Expert:
   * Updates a document by first updating the document(s)
   * containing <code>term</code> with the given doc-values fields
   * and then adding the new document.  The doc-values update and
   * then add are atomic as seen by a reader on the same index
   * (flush may happen only after the add).
   *
   * One use of this API is to retain older versions of
   * documents instead of replacing them. The existing
   * documents can be updated to reflect they are no
   * longer current while atomically adding new documents
   * at the same time.
   *
   * In contrast to {@link #updateDocument(Term, Iterable)}
   * this method will not delete documents in the index
   * matching the given term but instead update them with
   * the given doc-values fields which can be used as a
   * soft-delete mechanism.
   *
   * See {@link #addDocuments(Iterable)}
   * and {@link #updateDocuments(Term, Iterable)}.
   *
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   *
   * @lucene.experimental
   */
public:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long softUpdateDocument(Term term,
  // Iterable<? extends IndexableField> doc, org.apache.lucene.document.Field...
  // softDeletes) throws java.io.IOException
  int64_t
  softUpdateDocument(std::shared_ptr<Term> term, std::deque<T1> doc,
                     std::deque<Field> &softDeletes) ;

  /**
   * Updates a document's {@link NumericDocValues} for <code>field</code> to the
   * given <code>value</code>. You can only update fields that already exist in
   * the index, not add new fields through this method.
   *
   * @param term
   *          the term to identify the document(s) to be updated
   * @param field
   *          field name of the {@link NumericDocValues} field
   * @param value
   *          new value for the field
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException
   *           if the index is corrupt
   * @throws IOException
   *           if there is a low-level IO error
   */
  virtual int64_t updateNumericDocValue(std::shared_ptr<Term> term,
                                          const std::wstring &field,
                                          int64_t value) ;

  /**
   * Updates a document's {@link BinaryDocValues} for <code>field</code> to the
   * given <code>value</code>. You can only update fields that already exist in
   * the index, not add new fields through this method.
   *
   * <p>
   * <b>NOTE:</b> this method currently replaces the existing value of all
   * affected documents with the new value.
   *
   * @param term
   *          the term to identify the document(s) to be updated
   * @param field
   *          field name of the {@link BinaryDocValues} field
   * @param value
   *          new value for the field
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException
   *           if the index is corrupt
   * @throws IOException
   *           if there is a low-level IO error
   */
  virtual int64_t
  updateBinaryDocValue(std::shared_ptr<Term> term, const std::wstring &field,
                       std::shared_ptr<BytesRef> value) ;

  /**
   * Updates documents' DocValues fields to the given values. Each field update
   * is applied to the set of documents that are associated with the
   * {@link Term} to the same value. All updates are atomically applied and
   * flushed together. If a doc values fields data is <code>null</code> the
   * existing value is removed from all documents matching the term.
   *
   *
   * @param updates
   *          the updates to apply
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException
   *           if the index is corrupt
   * @throws IOException
   *           if there is a low-level IO error
   */
  virtual int64_t
  updateDocValues(std::shared_ptr<Term> term,
                  std::deque<Field> &updates) ;

private:
  std::deque<std::shared_ptr<DocValuesUpdate>>
  buildDocValuesUpdate(std::shared_ptr<Term> term,
                       std::deque<std::shared_ptr<Field>> &updates);

  // for test purpose
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int getSegmentCount();

  // for test purpose
  // C++ WARNING: The following method was originally marked 'synchronized':
  int getNumBufferedDocuments();

  // for test purpose
  // C++ WARNING: The following method was originally marked 'synchronized':
  int maxDoc(int i);

  // for test purpose
  int getFlushCount();

  // for test purpose
  int getFlushDeletesCount();

  /**
   * Return an unmodifiable set of all field names as visible
   * from this IndexWriter, across all segments of the index.
   * Useful for knowing which fields exist, before {@link #updateDocValues(Term,
   * Field...)} is attempted. We could phase out this method if
   * {@link #updateDocValues(Term, Field...)} could create the non-existent
   * docValues fields as necessary, instead of throwing
   * IllegalArgumentException for attempts to update non-existent
   * docValues fields.
   * @lucene.internal
   * @lucene.experimental
   */
  virtual std::shared_ptr<Set<std::wstring>> getFieldNames();

  std::wstring newSegmentName();

  /** If enabled, information about merges will be printed to this.
   */
  const std::shared_ptr<InfoStream> infoStream;

  /**
   * Forces merge policy to merge segments until there are
   * {@code <= maxNumSegments}.  The actual merges to be
   * executed are determined by the {@link MergePolicy}.
   *
   * <p>This is a horribly costly operation, especially when
   * you pass a small {@code maxNumSegments}; usually you
   * should only call this if the index is static (will no
   * longer be changed).</p>
   *
   * <p>Note that this requires free space that is proportional
   * to the size of the index in your Directory: 2X if you are
   * not using compound file format, and 3X if you are.
   * For example, if your index size is 10 MB then you need
   * an additional 20 MB free for this to complete (30 MB if
   * you're using compound file format). This is also affected
   * by the {@link Codec} that is used to execute the merge,
   * and may result in even a bigger index. Also, it's best
   * to call {@link #commit()} afterwards, to allow IndexWriter
   * to free up disk space.</p>
   *
   * <p>If some but not all readers re-open while merging
   * is underway, this will cause {@code > 2X} temporary
   * space to be consumed as those new readers will then
   * hold open the temporary segments at that time.  It is
   * best not to re-open readers while merging is running.</p>
   *
   * <p>The actual temporary usage could be much less than
   * these figures (it depends on many factors).</p>
   *
   * <p>In general, once this completes, the total size of the
   * index will be less than the size of the starting index.
   * It could be quite a bit smaller (if there were many
   * pending deletes) or just slightly smaller.</p>
   *
   * <p>If an Exception is hit, for example
   * due to disk full, the index will not be corrupted and no
   * documents will be lost.  However, it may have
   * been partially merged (some segments were merged but
   * not all), and it's possible that one of the segments in
   * the index will be in non-compound format even when
   * using compound file format.  This will occur when the
   * Exception is hit during conversion of the segment into
   * compound format.</p>
   *
   * <p>This call will merge those segments present in
   * the index when the call started.  If other threads are
   * still adding documents and flushing segments, those
   * newly created segments will not be merged unless you
   * call forceMerge again.</p>
   *
   * @param maxNumSegments maximum number of segments left
   * in the index after merging finishes
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   * @see MergePolicy#findMerges
   *
   */
  virtual void forceMerge(int maxNumSegments) ;

  /** Just like {@link #forceMerge(int)}, except you can
   *  specify whether the call should block until
   *  all merging completes.  This is only meaningful with a
   *  {@link MergeScheduler} that is able to run merges in
   *  background threads.
   */
  virtual void forceMerge(int maxNumSegments, bool doWait) ;

  /** Returns true if any merges in pendingMerges or
   *  runningMerges are maxNumSegments merges. */
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool maxNumSegmentsMergesPending();

  /** Just like {@link #forceMergeDeletes()}, except you can
   *  specify whether the call should block until the
   *  operation completes.  This is only meaningful with a
   *  {@link MergeScheduler} that is able to run merges in
   *  background threads. */
public:
  virtual void forceMergeDeletes(bool doWait) ;

  /**
   *  Forces merging of all segments that have deleted
   *  documents.  The actual merges to be executed are
   *  determined by the {@link MergePolicy}.  For example,
   *  the default {@link TieredMergePolicy} will only
   *  pick a segment if the percentage of
   *  deleted docs is over 10%.
   *
   *  <p>This is often a horribly costly operation; rarely
   *  is it warranted.</p>
   *
   *  <p>To see how
   *  many deletions you have pending in your index, call
   *  {@link IndexReader#numDeletedDocs}.</p>
   *
   *  <p><b>NOTE</b>: this method first flushes a new
   *  segment (if there are indexed documents), and applies
   *  all buffered deletes.
   */
  virtual void forceMergeDeletes() ;

  /**
   * Expert: asks the mergePolicy whether any merges are
   * necessary now and if so, runs the requested merges and
   * then iterate (test again if merges are needed) until no
   * more merges are returned by the mergePolicy.
   *
   * Explicit calls to maybeMerge() are usually not
   * necessary. The most common case is when merge policy
   * parameters have changed.
   *
   * This method will call the {@link MergePolicy} with
   * {@link MergeTrigger#EXPLICIT}.
   */
  void maybeMerge() ;

  void maybeMerge(std::shared_ptr<MergePolicy> mergePolicy,
                  MergeTrigger trigger, int maxNumSegments) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool updatePendingMerges(std::shared_ptr<MergePolicy> mergePolicy,
                           MergeTrigger trigger,
                           int maxNumSegments) ;

  /** Expert: to be used by a {@link MergePolicy} to avoid
   *  selecting merges for segments already being merged.
   *  The returned collection is not cloned, and thus is
   *  only safe to access if you hold IndexWriter's lock
   *  (which you do when IndexWriter invokes the
   *  MergePolicy).
   *
   *  <p>The Set is unmodifiable. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>>
  getMergingSegments() override;

  /**
   * Expert: the {@link MergeScheduler} calls this method to retrieve the next
   * merge requested by the MergePolicy
   *
   * @lucene.experimental
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<MergePolicy::OneMerge> getNextMerge();

  /**
   * Expert: returns true if there are merges waiting to be scheduled.
   *
   * @lucene.experimental
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool hasPendingMerges();

  /**
   * Close the <code>IndexWriter</code> without committing
   * any changes that have occurred since the last commit
   * (or since it was opened, if commit hasn't been called).
   * This removes any temporary files that had been created,
   * after which the state of the index will be the same as
   * it was when commit() was last called or when this
   * writer was first opened.  This also clears a previous
   * call to {@link #prepareCommit}.
   * @throws IOException if there is a low-level IO error
   */
  void rollback()  override;

private:
  void rollbackInternal() ;

  void rollbackInternalNoCommit() ;

  /**
   * Delete all documents in the index.
   *
   * <p>
   * This method will drop all buffered documents and will remove all segments
   * from the index. This change will not be visible until a {@link #commit()}
   * has been called. This method can be rolled back using {@link #rollback()}.
   * </p>
   *
   * <p>
   * NOTE: this method is much faster than using deleteDocuments( new
   * MatchAllDocsQuery() ). Yet, this method also has different semantics
   * compared to {@link #deleteDocuments(Query...)} since internal
   * data-structures are cleared as well as all segment information is
   * forcefully dropped anti-viral semantics like omitting norms are reset or
   * doc value types are cleared. Essentially a call to {@link #deleteAll()} is
   * equivalent to creating a new {@link IndexWriter} with
   * {@link OpenMode#CREATE} which a delete query only marks documents as
   * deleted.
   * </p>
   *
   * <p>
   * NOTE: this method will forcefully abort all merges in progress. If other
   * threads are running {@link #forceMerge}, {@link #addIndexes(CodecReader[])}
   * or {@link #forceMergeDeletes} methods, they may receive
   * {@link MergePolicy.MergeAbortedException}s.
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("try") public long deleteAll() throws
  // java.io.IOException
  virtual int64_t deleteAll() ;

  /** Aborts running merges.  Be careful when using this
   *  method: when you abort a long-running merge, you lose
   *  a lot of work that must later be redone. */
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void abortMerges();

  /**
   * Wait for any currently outstanding merges to finish.
   *
   * <p>It is guaranteed that any merges started prior to calling this method
   *    will have completed once this method completes.</p>
   */
public:
  virtual void waitForMerges() ;

  /**
   * Called whenever the SegmentInfos has been updated and
   * the index files referenced exist (correctly) in the
   * index directory.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void checkpoint() ;

  /** Checkpoints with IndexFileDeleter, so it's aware of
   *  new files, and increments changeCount, so on
   *  close/commit we will write a new segments file, but
   *  does NOT bump segmentInfos.version. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void checkpointNoSIS() ;

  /** Called internally if any index state has changed. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void changed();

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t
  publishFrozenUpdates(std::shared_ptr<FrozenBufferedUpdates> packet);

  /**
   * Atomically adds the segment private delete packet and publishes the flushed
   * segments SegmentInfo to the index writer.
   */
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void publishFlushedSegment(
      std::shared_ptr<SegmentCommitInfo> newSegment,
      std::shared_ptr<FieldInfos> fieldInfos,
      std::shared_ptr<FrozenBufferedUpdates> packet,
      std::shared_ptr<FrozenBufferedUpdates> globalPacket,
      std::shared_ptr<Sorter::DocMap> sortMap) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void resetMergeExceptions();

  void noDupDirs(std::deque<Directory> &dirs);

  /** Acquires write locks on all the directories; be sure
   *  to match with a call to {@link IOUtils#close} in a
   *  finally clause. */
  std::deque<std::shared_ptr<Lock>>
  acquireWriteLocks(std::deque<Directory> &dirs) ;

  /**
   * Adds all segments from an array of indexes into this index.
   *
   * <p>This may be used to parallelize batch indexing. A large document
   * collection can be broken into sub-collections. Each sub-collection can be
   * indexed in parallel, on a different thread, process or machine. The
   * complete index can then be created by merging sub-collection indexes
   * with this method.
   *
   * <p>
   * <b>NOTE:</b> this method acquires the write lock in
   * each directory, to ensure that no {@code IndexWriter}
   * is currently open or tries to open while this is
   * running.
   *
   * <p>This method is transactional in how Exceptions are
   * handled: it does not commit a new segments_N file until
   * all indexes are added.  This means if an Exception
   * occurs (for example disk full), then either no indexes
   * will have been added or they all will have been.
   *
   * <p>Note that this requires temporary free space in the
   * {@link Directory} up to 2X the sum of all input indexes
   * (including the starting index). If readers/searchers
   * are open against the starting index, then temporary
   * free space required will be higher by the size of the
   * starting index (see {@link #forceMerge(int)} for details).
   *
   * <p>This requires this index not be among those to be added.
   *
   * <p>All added indexes must have been created by the same
   * Lucene version as this index.
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   * @throws IllegalArgumentException if addIndexes would cause
   *   the index to exceed {@link #MAX_DOCS}, or if the indoming
   *   index sort does not match this index's index sort
   */
public:
  virtual int64_t addIndexes(std::deque<Directory> &dirs) ;

private:
  void validateMergeReader(std::shared_ptr<CodecReader> leaf);

  /**
   * Merges the provided indexes into this index.
   *
   * <p>
   * The provided IndexReaders are not closed.
   *
   * <p>
   * See {@link #addIndexes} for details on transactional semantics, temporary
   * free space required in the Directory, and non-CFS segments on an Exception.
   *
   * <p>
   * <b>NOTE:</b> empty segments are dropped by this method and not added to
   * this index.
   *
   * <p>
   * <b>NOTE:</b> this merges all given {@link LeafReader}s in one
   * merge. If you intend to merge a large number of readers, it may be better
   * to call this method multiple times, each time with a small set of readers.
   * In principle, if you use a merge policy with a {@code mergeFactor} or
   * {@code maxMergeAtOnce} parameter, you should pass that many readers in one
   * call.
   *
   * <p>
   * <b>NOTE:</b> this method does not call or make use of the {@link
   * MergeScheduler}, so any custom bandwidth throttling is at the moment
   * ignored.
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * for this operation
   *
   * @throws CorruptIndexException
   *           if the index is corrupt
   * @throws IOException
   *           if there is a low-level IO error
   * @throws IllegalArgumentException
   *           if addIndexes would cause the index to exceed {@link #MAX_DOCS}
   */
public:
  virtual int64_t
  addIndexes(std::deque<CodecReader> &readers) ;

  /** Copies the segment files as-is into the IndexWriter's directory. */
private:
  std::shared_ptr<SegmentCommitInfo>
  copySegmentAsIs(std::shared_ptr<SegmentCommitInfo> info,
                  const std::wstring &segName,
                  std::shared_ptr<IOContext> context) ;

  /**
   * A hook for extending classes to execute operations after pending added and
   * deleted documents have been flushed to the Directory but before the change
   * is committed (new segments_N file written).
   */
protected:
  virtual void doAfterFlush() ;

  /**
   * A hook for extending classes to execute operations before pending added and
   * deleted documents are flushed to the Directory.
   */
  virtual void doBeforeFlush() ;

  /** <p>Expert: prepare for commit.  This does the
   *  first phase of 2-phase commit. This method does all
   *  steps necessary to commit changes since this writer
   *  was opened: flushes pending added and deleted docs,
   *  syncs the index files, writes most of next segments_N
   *  file.  After calling this you must call either {@link
   *  #commit()} to finish the commit, or {@link
   *  #rollback()} to revert the commit and undo all changes
   *  done since the writer was opened.</p>
   *
   * <p>You can also just call {@link #commit()} directly
   *  without prepareCommit first in which case that method
   *  will internally call prepareCommit.
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * of the last operation in the commit.  All sequence numbers &lt;= this value
   * will be reflected in the commit, and all others will not.
   */
public:
  int64_t prepareCommit()  override;

  /**
   * <p>Expert: Flushes the next pending writer per thread buffer if available
   * or the largest active non-pending writer per thread buffer in the calling
   * thread. This can be used to flush documents to disk outside of an indexing
   * thread. In contrast to {@link #flush()} this won't mark all currently
   * active indexing buffers as flush-pending.
   *
   * Note: this method is best-effort and might not flush any segments to disk.
   * If there is a full flush happening concurrently multiple segments might
   * have been flushed. Users of this API can access the IndexWriters current
   * memory consumption via {@link #ramBytesUsed()}
   * </p>
   * @return <code>true</code> iff this method flushed at least on segment to
   * disk.
   * @lucene.experimental
   */
  bool flushNextBuffer() ;

private:
  int64_t prepareCommitInternal() ;

  /**
   * Ensures that all changes in the reader-pool are written to disk.
   * @param writeDeletes if <code>true</code> if deletes should be written to
   * disk too.
   */
  void writeReaderPool(bool writeDeletes) ;

  /**
   * Sets the iterator to provide the commit user data map_obj at commit time.
   * Calling this method is considered a committable change and will be {@link
   * #commit() committed} even if there are no other changes this writer. Note
   * that you must call this method before {@link #prepareCommit()}.  Otherwise
   * it won't be included in the follow-on {@link #commit()}. <p> <b>NOTE:</b>
   * the iterator is late-binding: it is only visited once all documents for the
   * commit have been written to their segments, before the next segments_N file
   * is written
   */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void setLiveCommitData(
      std::deque<std::unordered_map::Entry<std::wstring, std::wstring>>
          &commitUserData);

  /**
   * Sets the commit user data iterator, controlling whether to advance the
   * {@link SegmentInfos#getVersion}.
   *
   * @see #setLiveCommitData(Iterable)
   *
   * @lucene.internal */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void setLiveCommitData(
      std::deque<std::unordered_map::Entry<std::wstring, std::wstring>>
          &commitUserData,
      bool doIncrementVersion);

  /**
   * Returns the commit user data iterable previously set with {@link
   * #setLiveCommitData(Iterable)}, or null if nothing has been set yet.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::deque<std::unordered_map::Entry<std::wstring, std::wstring>>
  getLiveCommitData();

  // Used only by commit and prepareCommit, below; lock
  // order is commitLock -> IW
private:
  std::mutex commitLock;

  /**
   * <p>Commits all pending changes (added and deleted
   * documents, segment merges, added
   * indexes, etc.) to the index, and syncs all referenced
   * index files, such that a reader will see the changes
   * and the index updates will survive an OS or machine
   * crash or power loss.  Note that this does not wait for
   * any running background merges to finish.  This may be a
   * costly operation, so you should test the cost in your
   * application and do it only when really necessary.</p>
   *
   * <p> Note that this operation calls Directory.sync on
   * the index files.  That call should not return until the
   * file contents and metadata are on stable storage.  For
   * FSDirectory, this calls the OS's fsync.  But, beware:
   * some hardware devices may in fact cache writes even
   * during fsync, and return before the bits are actually
   * on stable storage, to give the appearance of faster
   * performance.  If you have such a device, and it does
   * not have a battery backup (for example) then on power
   * loss it may still lose data.  Lucene cannot guarantee
   * consistency on such devices.  </p>
   *
   * <p> If nothing was committed, because there were no
   * pending changes, this returns -1.  Otherwise, it returns
   * the sequence number such that all indexing operations
   * prior to this sequence will be included in the commit
   * point, and all other operations will not. </p>
   *
   * @see #prepareCommit
   *
   * @return The <a href="#sequence_number">sequence number</a>
   * of the last operation in the commit.  All sequence numbers &lt;= this value
   * will be reflected in the commit, and all others will not.
   */
public:
  int64_t commit()  override;

  /** Returns true if there may be changes that have not been
   *  committed.  There are cases where this may return true
   *  when there are no actual "real" changes to the index,
   *  for example if you've deleted by Term or Query but
   *  that Term or Query does not match any documents.
   *  Also, if a merge kicked off as a result of flushing a
   *  new segment during {@link #commit}, or a concurrent
   *  merged finished, this method may return true right
   *  after you had just called {@link #commit}. */
  bool hasUncommittedChanges();

private:
  int64_t
  commitInternal(std::shared_ptr<MergePolicy> mergePolicy) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("try") private final void finishCommit()
  // throws java.io.IOException
  void finishCommit() ;

  // Ensures only one flush() is actually flushing segments
  // at a time:
  std::mutex fullFlushLock;

  // for assert
public:
  virtual bool holdsFullFlushLock();

  /** Moves all in-memory segments to the {@link Directory}, but does not commit
   *  (fsync) them (call {@link #commit} for that). */
  void flush() ;

  /**
   * Flush all in-memory buffered updates (adds and deletes)
   * to the Directory.
   * @param triggerMerge if true, we may merge segments (if
   *  deletes or docs were flushed) if necessary
   * @param applyAllDeletes whether pending deletes should also
   */
  void flush(bool triggerMerge, bool applyAllDeletes) ;

  /** Returns true a segment was flushed or deletes were applied. */
private:
  bool doFlush(bool applyAllDeletes) ;

public:
  void applyAllDeletesAndUpdates() ;

  // for testing only
  virtual std::shared_ptr<DocumentsWriter> getDocsWriter();

  /** Expert:  Return the number of documents currently
   *  buffered in RAM. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  int numRamDocs();

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void ensureValidMerge(std::shared_ptr<MergePolicy::OneMerge> merge);

  void
  skipDeletedDoc(std::deque<std::shared_ptr<DocValuesFieldUpdates::Iterator>>
                     &updatesIters,
                 int deletedDoc);

  /**
   * Carefully merges deletes and updates for the segments we just merged. This
   * is tricky because, although merging will clear all deletes (compacts the
   * documents) and compact all the updates, new deletes and updates may have
   * been flushed to the segments since the merge was started. This method
   * "carries over" such new deletes and updates onto the newly merged segment,
   * and saves the resulting deletes and updates files (incrementing the delete
   * and DV generations for merge.info). If no deletes were flushed, no new
   * deletes file is saved.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<ReadersAndUpdates> commitMergedDeletesAndUpdates(
      std::shared_ptr<MergePolicy::OneMerge> merge,
      std::shared_ptr<MergeState> mergeState) ;

  /**
   * This method carries over hard-deleted documents that are applied to the
   * source segment during a merge.
   */
  static void carryOverHardDeletes(
      std::shared_ptr<ReadersAndUpdates> mergedReadersAndUpdates, int maxDoc,
      std::shared_ptr<Bits> mergeLiveDocs,
      std::shared_ptr<Bits> prevHardLiveDocs,
      std::shared_ptr<Bits> currentHardLiveDocs,
      std::shared_ptr<MergeState::DocMap> segDocMap,
      std::shared_ptr<MergeState::DocMap> segLeafDocMap) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("try") synchronized private bool
  // commitMerge(MergePolicy.OneMerge merge, MergeState mergeState) throws
  // java.io.IOException C++ WARNING: The following method was originally marked
  // 'synchronized':
  bool commitMerge(std::shared_ptr<MergePolicy::OneMerge> merge,
                   std::shared_ptr<MergeState> mergeState) ;

  void handleMergeException(
      std::runtime_error t,
      std::shared_ptr<MergePolicy::OneMerge> merge) ;

  /**
   * Merges the indicated segments, replacing them in the stack with a
   * single segment.
   *
   * @lucene.experimental
   */
public:
  virtual void
  merge(std::shared_ptr<MergePolicy::OneMerge> merge) ;

  /** Hook that's called when the specified merge is complete. */
  virtual void mergeSuccess(std::shared_ptr<MergePolicy::OneMerge> merge);

  /** Checks whether this merge involves any segments
   *  already participating in a merge.  If not, this merge
   *  is "registered", meaning we record that its segments
   *  are now participating in a merge, and true is
   *  returned.  Else (the merge conflicts) false is
   *  returned. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool registerMerge(std::shared_ptr<MergePolicy::OneMerge> merge) throw(
      IOException);

  /** Does initial setup for a merge, which is fast but holds
   *  the synchronized lock on IndexWriter instance.  */
  void
  mergeInit(std::shared_ptr<MergePolicy::OneMerge> merge) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void
  _mergeInit(std::shared_ptr<MergePolicy::OneMerge> merge) ;

public:
  static void setDiagnostics(std::shared_ptr<SegmentInfo> info,
                             const std::wstring &source);

private:
  static void
  setDiagnostics(std::shared_ptr<SegmentInfo> info, const std::wstring &source,
                 std::unordered_map<std::wstring, std::wstring> &details);

  /** Does fininishing for a merge, which is fast but holds
   *  the synchronized lock on IndexWriter instance. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void mergeFinish(std::shared_ptr<MergePolicy::OneMerge> merge);

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("try") private synchronized void
  // closeMergeReaders(MergePolicy.OneMerge merge, bool suppressExceptions)
  // throws java.io.IOException C++ WARNING: The following method was originally
  // marked 'synchronized':
  void closeMergeReaders(std::shared_ptr<MergePolicy::OneMerge> merge,
                         bool suppressExceptions) ;

  /** Does the actual (time-consuming) work of the merge,
   *  but without holding synchronized lock on IndexWriter
   *  instance */
  int mergeMiddle(std::shared_ptr<MergePolicy::OneMerge> merge,
                  std::shared_ptr<MergePolicy> mergePolicy) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void addMergeException(std::shared_ptr<MergePolicy::OneMerge> merge);

  // For test purposes.
  int getBufferedDeleteTermsSize();

  // For test purposes.
  int getNumBufferedDeleteTerms();

  // utility routines for tests
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<SegmentCommitInfo> newestSegment();

  /** Returns a string description of all segments, for
   *  debugging.
   *
   * @lucene.internal */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::wstring segString();

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::wstring
  segString(std::deque<std::shared_ptr<SegmentCommitInfo>> &infos);

  /** Returns a string description of the specified
   *  segment, for debugging.
   *
   * @lucene.internal */
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::wstring segString(std::shared_ptr<SegmentCommitInfo> info);

  // C++ WARNING: The following method was originally marked 'synchronized':
  void doWait();

  // called only from assert
  bool filesExist(std::shared_ptr<SegmentInfos> toSync) ;

  // For infoStream output
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<SegmentInfos>
  toLiveInfos(std::shared_ptr<SegmentInfos> sis);

  /** Walk through all files referenced by the current
   *  segmentInfos and ask the Directory to sync each file,
   *  if it wasn't already.  If that succeeds, then we
   *  prepare a new segments_N file but do not fully commit
   *  it. */
private:
  void startCommit(std::shared_ptr<SegmentInfos> toSync) ;

  /** If {@link DirectoryReader#open(IndexWriter)} has
   *  been called (ie, this writer is in near real-time
   *  mode), then after a merge completes, this class can be
   *  invoked to warm the reader on the newly merged
   *  segment, before the merge commits.  This is not
   *  required for near real-time search, but will reduce
   *  search latency on opening a new near real-time reader
   *  after a merge completes.
   *
   * @lucene.experimental
   *
   * <p><b>NOTE</b>: {@link #warm(LeafReader)} is called before any
   * deletes have been carried over to the merged segment. */
  using IndexReaderWarmer = std::function<void(LeafReader reader)>;

  /**
   * This method should be called on a tragic event ie. if a downstream class of
   * the writer hits an unrecoverable exception. This method does not rethrow
   * the tragic event exception. Note: This method will not close the writer but
   * can be called from any location without respecting any lock order
   */
public:
  void onTragicEvent(std::runtime_error tragedy, const std::wstring &location);

  /**
   * This method set the tragic exception unless it's already set and closes the
   * writer if necessary. Note this method will not rethrow the throwable passed
   * to it.
   */
private:
  void tragicEvent(std::runtime_error tragedy,
                   const std::wstring &location) ;

  void maybeCloseOnTragicEvent() ;

  /** If this {@code IndexWriter} was closed as a side-effect of a tragic
   * exception, e.g. disk full while flushing a new segment, this returns the
   * root cause exception. Otherwise (no tragic exception has occurred) it
   * returns null. */
public:
  virtual std::runtime_error getTragicException();

  /** Returns {@code true} if this {@code IndexWriter} is still open. */
  virtual bool isOpen();

  // Used for testing.  Current points:
  //   startDoFlush
  //   startCommitMerge
  //   startStartCommit
  //   midStartCommit
  //   midStartCommit2
  //   midStartCommitSuccess
  //   finishStartCommit
  //   startCommitMergeDeletes
  //   startMergeInit
  //   DocumentsWriter.ThreadState.init start
private:
  void testPoint(const std::wstring &message);

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool nrtIsCurrent(std::shared_ptr<SegmentInfos> infos);

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool isClosed();

  /** Expert: remove any index files that are no longer
   *  used.
   *
   *  <p> IndexWriter normally deletes unused files itself,
   *  during indexing.  However, on Windows, which disallows
   *  deletion of open files, if there is a reader open on
   *  the index then those files cannot be deleted.  This is
   *  fine, because IndexWriter will periodically retry
   *  the deletion.</p>
   *
   *  <p> However, IndexWriter doesn't try that often: only
   *  on open, close, flushing a new segment, and finishing
   *  a merge.  If you don't do any of these actions with your
   *  IndexWriter, you'll see the unused files linger.  If
   *  that's a problem, call this method to delete them
   *  (once you've closed the open readers that were
   *  preventing their deletion).
   *
   *  <p> In addition, you can call this method to delete
   *  unreferenced index commits. This might be useful if you
   *  are using an {@link IndexDeletionPolicy} which holds
   *  onto index commits until some criteria are met, but those
   *  commits are no longer needed. Otherwise, those commits will
   *  be deleted the next time commit() is called.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void deleteUnusedFiles() ;

  /**
   * NOTE: this method creates a compound file for all files returned by
   * info.files(). While, generally, this may include separate norms and
   * deletion files, this SegmentInfo must not reference such files when this
   * method is called, because they are not allowed within a compound file.
   */
  static void
  createCompoundFile(std::shared_ptr<InfoStream> infoStream,
                     std::shared_ptr<TrackingDirectoryWrapper> directory,
                     std::shared_ptr<SegmentInfo> info,
                     std::shared_ptr<IOContext> context,
                     IOUtils::IOConsumer<std::deque<std::wstring>>
                         deleteFiles) ;

  /**
   * Tries to delete the given files if unreferenced
   * @param files the files to delete
   * @throws IOException if an {@link IOException} occurs
   * @see IndexFileDeleter#deleteNewFiles(std::deque)
   */
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void deleteNewFiles(std::shared_ptr<std::deque<std::wstring>> files) throw(
      IOException);
  /**
   * Cleans up residuals from a segment that could not be entirely flushed due
   * to an error
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void flushFailed(std::shared_ptr<SegmentInfo> info) ;

  /**
   * Publishes the flushed segment, segment-private deletes (if any) and its
   * associated global delete (if present) to IndexWriter.  The actual
   * publishing operation is synced on {@code IW -> BDS} so that the {@link
   * SegmentInfo}'s delete generation is always GlobalPacket_deleteGeneration +
   * 1
   * @param forced if <code>true</code> this call will block on the ticket queue
   * if the lock is held by another thread. if <code>false</code> the call will
   * try to acquire the queue lock and exits if it's held by another thread.
   *
   */
public:
  virtual void publishFlushedSegments(bool forced) ;

  /** Record that the files referenced by this {@link SegmentInfos} are still in
   * use.
   *
   * @lucene.internal */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void
  incRefDeleter(std::shared_ptr<SegmentInfos> segmentInfos) ;

  /** Record that the files referenced by this {@link SegmentInfos} are no
   * longer in use.  Only call this if you are sure you previously called {@link
   * #incRefDeleter}.
   *
   * @lucene.internal */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void
  decRefDeleter(std::shared_ptr<SegmentInfos> segmentInfos) ;

private:
  void processEvents(bool triggerMerge) ;

  /**
   * Interface for internal atomic events. See {@link DocumentsWriter} for
   * details. Events are executed concurrently and no order is guaranteed. Each
   * event should only rely on the serializeability within its process method.
   * All actions that must happen before or after a certain action must be
   * encoded inside the {@link #process(IndexWriter)} method.
   *
   */
  using Event = std::function<void(IndexWriter writer)>;

  /** Anything that will add N docs to the index should reserve first to
   *  make sure it's allowed.  This will throw {@code
   *  IllegalArgumentException} if it's not allowed. */
private:
  void reserveDocs(int64_t addedNumDocs);

  /** Does a best-effort check, that the current index would accept this many
   * additional docs, but does not actually reserve them.
   *
   * @throws IllegalArgumentException if there would be too many docs */
  void testReserveDocs(int64_t addedNumDocs);

  void tooManyDocs(int64_t addedNumDocs);

  /** Returns the highest <a href="#sequence_number">sequence number</a> across
   *  all completed operations, or 0 if no operations have finished yet.  Still
   *  in-flight operations (in other threads) are not counted until they finish.
   *
   * @lucene.experimental */
public:
  virtual int64_t getMaxCompletedSequenceNumber();

private:
  int64_t adjustPendingNumDocs(int64_t numDocs);

public:
  bool isFullyDeleted(
      std::shared_ptr<ReadersAndUpdates> readersAndUpdates) ;

  /**
   * Returns the number of deletes a merge would claim back if the given segment
   * is merged.
   * @see MergePolicy#numDeletesToMerge(SegmentCommitInfo, int,
   * org.apache.lucene.util.IOSupplier)
   * @param info the segment to get the number of deletes for
   * @lucene.experimental
   */
  int numDeletesToMerge(std::shared_ptr<SegmentCommitInfo> info) throw(
      IOException) override;

  virtual void release(
      std::shared_ptr<ReadersAndUpdates> readersAndUpdates) ;

private:
  void release(std::shared_ptr<ReadersAndUpdates> readersAndUpdates,
               bool assertLiveInfo) ;

public:
  virtual std::shared_ptr<ReadersAndUpdates>
  getPooledInstance(std::shared_ptr<SegmentCommitInfo> info, bool create);

  virtual void finished(std::shared_ptr<FrozenBufferedUpdates> packet);

  virtual int getPendingUpdatesCount();

  /**
   * Tests should override this to enable test points. Default is
   * <code>false</code>.
   */
protected:
  virtual bool isEnableTestPoints();

private:
  void validate(std::shared_ptr<SegmentCommitInfo> info);
};

} // #include  "core/src/java/org/apache/lucene/index/
