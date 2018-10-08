#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}

namespace org::apache::lucene::index
{
class SegmentReader;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class CodecReader;
}
namespace org::apache::lucene::store
{
class MergeInfo;
}
namespace org::apache::lucene::index
{
class MergeAbortedException;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class MergeContext;
}
namespace org::apache::lucene::index
{
class SegmentInfos;
}
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
}
namespace org::apache::lucene::util
{
class InfoStream;
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

using Directory = org::apache::lucene::store::Directory;
using MergeInfo = org::apache::lucene::store::MergeInfo;
using Bits = org::apache::lucene::util::Bits;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * <p>Expert: a MergePolicy determines the sequence of
 * primitive merge operations.</p>
 *
 * <p>Whenever the segments in an index have been altered by
 * {@link IndexWriter}, either the addition of a newly
 * flushed segment, addition of many segments from
 * addIndexes* calls, or a previous merge that may now need
 * to cascade, {@link IndexWriter} invokes {@link
 * #findMerges} to give the MergePolicy a chance to pick
 * merges that are now required.  This method returns a
 * {@link MergeSpecification} instance describing the set of
 * merges that should be done, or null if no merges are
 * necessary.  When IndexWriter.forceMerge is called, it calls
 * {@link #findForcedMerges(SegmentInfos, int, Map, MergeContext)} and the
 * MergePolicy should then return the necessary merges.</p>
 *
 * <p>Note that the policy can return more than one merge at
 * a time.  In this case, if the writer is using {@link
 * SerialMergeScheduler}, the merges will be run
 * sequentially but if it is using {@link
 * ConcurrentMergeScheduler} they will be run concurrently.</p>
 *
 * <p>The default MergePolicy is {@link
 * TieredMergePolicy}.</p>
 *
 * @lucene.experimental
 */
class MergePolicy : public std::enable_shared_from_this<MergePolicy>
{
  GET_CLASS_NAME(MergePolicy)

  /**
   * Progress and state for an executing merge. This class
   * encapsulates the logic to pause and resume the merge thread
   * or to abort the merge entirely.
   *
   * @lucene.experimental */
public:
  class OneMergeProgress : public std::enable_shared_from_this<OneMergeProgress>
  {
    GET_CLASS_NAME(OneMergeProgress)
    /** Reason for pausing the merge thread. */
  public:
    enum class PauseReason {
      GET_CLASS_NAME(PauseReason)
      /** Stopped (because of throughput rate set to 0, typically). */
      STOPPED,
      /** Temporarily paused because of exceeded throughput rate. */
      PAUSED,
      /** Other reason. */
      OTHER
    };

  private:
    const std::shared_ptr<ReentrantLock> pauseLock =
        std::make_shared<ReentrantLock>();
    const std::shared_ptr<Condition> pausing = pauseLock->newCondition();

    /**
     * Pause times (in nanoseconds) for each {@link PauseReason}.
     */
    const std::shared_ptr<EnumMap<PauseReason, std::shared_ptr<AtomicLong>>>
        pauseTimesNS;

    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile bool aborted;
    bool aborted = false;

    /**
     * This field is for sanity-check purposes only. Only the same thread that
     * invoked
     * {@link OneMerge#mergeInit()} is permitted to be calling
     * {@link #pauseNanos}. This is always verified at runtime.
     */
    std::shared_ptr<Thread> owner;

    /** Creates a new merge progress info. */
  public:
    OneMergeProgress();

    /**
     * Abort the merge this progress tracks at the next
     * possible moment.
     */
    virtual void abort();

    /**
     * Return the aborted state of this merge.
     */
    virtual bool isAborted();

    /**
     * Pauses the calling thread for at least <code>pauseNanos</code>
     * nanoseconds unless the merge is aborted or the external condition returns
     * <code>false</code>, in which case control returns immediately.
     *
     * The external condition is required so that other threads can terminate
     * the pausing immediately, before <code>pauseNanos</code> expires. We can't
     * rely on just {@link Condition#awaitNanos(long)} alone because it can
     * return due to spurious wakeups too.
     *
     * @param condition The pause condition that should return false if
     * immediate return from this method is needed. Other threads can wake up
     * any sleeping thread by calling
     *      {@link #wakeup}, but it'd fall to sleep for the remainder of the
     * requested time if this condition
     */
    virtual void
    pauseNanos(int64_t pauseNanos, PauseReason reason,
               std::function<bool()> &condition) ;

    /**
     * Request a wakeup for any threads stalled in {@link #pauseNanos}.
     */
    virtual void wakeup();

    /** Returns pause reasons and associated times in nanoseconds. */
    virtual std::unordered_map<PauseReason, int64_t> getPauseTimes();

    void setMergeThread(std::shared_ptr<Thread> owner);
  };

  /** OneMerge provides the information necessary to perform
   *  an individual primitive merge operation, resulting in
   *  a single new segment.  The merge spec includes the
   *  subset of segments to be merged as well as whether the
   *  new segment should use the compound file format.
   *
   * @lucene.experimental */
public:
  class OneMerge : public std::enable_shared_from_this<OneMerge>
  {
    GET_CLASS_NAME(OneMerge)
  public:
    std::shared_ptr<SegmentCommitInfo> info; // used by IndexWriter
    bool registerDone = false;               // used by IndexWriter
    int64_t mergeGen = 0;                  // used by IndexWriter
    bool isExternal = false;                 // used by IndexWriter
    int maxNumSegments = -1;                 // used by IndexWriter

    /** Estimated size in bytes of the merged segment. */
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: public volatile long estimatedMergeBytes;
    int64_t estimatedMergeBytes = 0; // used by IndexWriter

    // Sum of sizeInBytes of all SegmentInfos; set by IW.mergeInit
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile long totalMergeBytes;
    int64_t totalMergeBytes = 0;

    std::deque<std::shared_ptr<SegmentReader>> readers; // used by IndexWriter
    std::deque<std::shared_ptr<Bits>> hardLiveDocs;     // used by IndexWriter

    /** Segments to be merged. */
    const std::deque<std::shared_ptr<SegmentCommitInfo>> segments;

    /**
     * Control used to pause/stop/resume the merge thread.
     */
  private:
    const std::shared_ptr<OneMergeProgress> mergeProgress;

  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile long mergeStartNS = -1;
    int64_t mergeStartNS = -1;

    /** Total number of documents in segments to be merged, not accounting for
     * deletions. */
    const int totalMaxDoc;
    std::runtime_error error;

    /** Sole constructor.
     * @param segments List of {@link SegmentCommitInfo}s
     *        to be merged. */
    OneMerge(std::deque<std::shared_ptr<SegmentCommitInfo>> &segments);

    /**
     * Called by {@link IndexWriter} after the merge started and from the
     * thread that will be executing the merge.
     */
    virtual void mergeInit() ;

    /** Called by {@link IndexWriter} after the merge is done and all readers
     * have been closed. */
    virtual void mergeFinished() ;

    /** Wrap the reader in order to add/remove information to the merged
     * segment. */
    virtual std::shared_ptr<CodecReader>
    wrapForMerge(std::shared_ptr<CodecReader> reader) ;

    /**
     * Expert: Sets the {@link SegmentCommitInfo} of the merged segment.
     * Allows sub-classes to e.g. set diagnostics properties.
     */
    virtual void setMergeInfo(std::shared_ptr<SegmentCommitInfo> info);

    /**
     * Returns the {@link SegmentCommitInfo} for the merged segment,
     * or null if it hasn't been set yet.
     */
    virtual std::shared_ptr<SegmentCommitInfo> getMergeInfo();

    /** Record that an exception occurred while executing
     *  this merge */
    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual void setException(std::runtime_error error);

    /** Retrieve previous exception set by {@link
     *  #setException}. */
    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual std::runtime_error getException();

    /** Returns a readable description of the current merge
     *  state. */
    virtual std::wstring segString();

    /**
     * Returns the total size in bytes of this merge. Note that this does not
     * indicate the size of the merged segment, but the
     * input total size. This is only set once the merge is
     * initialized by IndexWriter.
     */
    virtual int64_t totalBytesSize();

    /**
     * Returns the total number of documents that are included with this merge.
     * Note that this does not indicate the number of documents after the merge.
     * */
    virtual int totalNumDocs();

    /** Return {@link MergeInfo} describing this merge. */
    virtual std::shared_ptr<MergeInfo> getStoreMergeInfo();

    /** Returns true if this merge was or should be aborted. */
    virtual bool isAborted();

    /** Marks this merge as aborted. The merge thread should terminate at the
     * soonest possible moment. */
    virtual void setAborted();

    /** Checks if merge has been aborted and throws a merge exception if so. */
    virtual void checkAborted() ;

    /**
     * Returns a {@link OneMergeProgress} instance for this merge, which
     * provides statistics of the merge threads (run time vs. sleep time) if
     * merging is throttled.
     */
    virtual std::shared_ptr<OneMergeProgress> getMergeProgress();
  };

  /**
   * A MergeSpecification instance provides the information
   * necessary to perform multiple merges.  It simply
   * contains a deque of {@link OneMerge} instances.
   */

public:
  class MergeSpecification
      : public std::enable_shared_from_this<MergeSpecification>
  {
    GET_CLASS_NAME(MergeSpecification)

    /**
     * The subset of segments to be included in the primitive merge.
     */

  public:
    const std::deque<std::shared_ptr<OneMerge>> merges =
        std::deque<std::shared_ptr<OneMerge>>();

    /** Sole constructor.  Use {@link
     *  #add(MergePolicy.OneMerge)} to add merges. */
    MergeSpecification();

    /** Adds the provided {@link OneMerge} to this
     *  specification. */
    virtual void add(std::shared_ptr<OneMerge> merge);

    /** Returns a description of the merges in this specification. */
    virtual std::wstring segString(std::shared_ptr<Directory> dir);
  };

  /** Exception thrown if there are any problems while executing a merge. */
public:
  class MergeException : public std::runtime_error
  {
    GET_CLASS_NAME(MergeException)
  private:
    std::shared_ptr<Directory> dir;

    /** Create a {@code MergeException}. */
  public:
    MergeException(const std::wstring &message, std::shared_ptr<Directory> dir);

    /** Create a {@code MergeException}. */
    MergeException(std::runtime_error exc, std::shared_ptr<Directory> dir);

    /** Returns the {@link Directory} of the index that hit
     *  the exception. */
    virtual std::shared_ptr<Directory> getDirectory();

  protected:
    std::shared_ptr<MergeException> shared_from_this()
    {
      return std::static_pointer_cast<MergeException>(
          RuntimeException::shared_from_this());
    }
  };

  /** Thrown when a merge was explicitly aborted because
   *  {@link IndexWriter#abortMerges} was called.  Normally
   *  this exception is privately caught and suppressed by
   *  {@link IndexWriter}. */
public:
  class MergeAbortedException : public IOException
  {
    GET_CLASS_NAME(MergeAbortedException)
    /** Create a {@link MergeAbortedException}. */
  public:
    MergeAbortedException();

    /** Create a {@link MergeAbortedException} with a
     *  specified message. */
    MergeAbortedException(const std::wstring &message);

  protected:
    std::shared_ptr<MergeAbortedException> shared_from_this()
    {
      return std::static_pointer_cast<MergeAbortedException>(
          java.io.IOException::shared_from_this());
    }
  };

  /**
   * Default ratio for compound file system usage. Set to <tt>1.0</tt>, always
   * use compound file system.
   */
protected:
  static constexpr double DEFAULT_NO_CFS_RATIO = 1.0;

  /**
   * Default max segment size in order to use compound file system. Set to
   * {@link Long#MAX_VALUE}.
   */
  static const int64_t DEFAULT_MAX_CFS_SEGMENT_SIZE =
      std::numeric_limits<int64_t>::max();

  /** If the size of the merge segment exceeds this ratio of
   *  the total index size then it will remain in
   *  non-compound format */
  double noCFSRatio = DEFAULT_NO_CFS_RATIO;

  /** If the size of the merged segment exceeds
   *  this value then it will not use compound file format. */
  int64_t maxCFSSegmentSize = DEFAULT_MAX_CFS_SEGMENT_SIZE;

  /**
   * Creates a new merge policy instance.
   */
public:
  MergePolicy();

  /**
   * Creates a new merge policy instance with default settings for noCFSRatio
   * and maxCFSSegmentSize. This ctor should be used by subclasses using
different
   * defaults than the {@link MergePolicy}
GET_CLASS_NAME(es)
   */
protected:
  MergePolicy(double defaultNoCFSRatio, int64_t defaultMaxCFSSegmentSize);

  /**
   * Determine what set of merge operations are now necessary on the index.
   * {@link IndexWriter} calls this whenever there is a change to the segments.
   * This call is always synchronized on the {@link IndexWriter} instance so
   * only one thread at a time will call this method.
   * @param mergeTrigger the event that triggered the merge
   * @param segmentInfos
   *          the total set of segments in the index
   * @param mergeContext the IndexWriter to find the merges on
   */
public:
  virtual std::shared_ptr<MergeSpecification>
  findMerges(MergeTrigger mergeTrigger,
             std::shared_ptr<SegmentInfos> segmentInfos,
             std::shared_ptr<MergeContext> mergeContext) = 0;

  /**
   * Determine what set of merge operations is necessary in
   * order to merge to {@code <=} the specified segment count. {@link
   * IndexWriter} calls this when its
   * {@link IndexWriter#forceMerge} method is called. This call is always
   * synchronized on the {@link IndexWriter} instance so only one thread at a
   * time will call this method.
   *  @param segmentInfos
   *          the total set of segments in the index
   * @param maxSegmentCount
   *          requested maximum number of segments in the index (currently this
   *          is always 1)
   * @param segmentsToMerge
   *          contains the specific SegmentInfo instances that must be merged
   *          away. This may be a subset of all
   *          SegmentInfos.  If the value is True for a
   *          given SegmentInfo, that means this segment was
   *          an original segment present in the
   *          to-be-merged index; else, it was a segment
   *          produced by a cascaded merge.
   * @param mergeContext the IndexWriter to find the merges on
   */
  virtual std::shared_ptr<MergeSpecification>
  findForcedMerges(std::shared_ptr<SegmentInfos> segmentInfos,
                   int maxSegmentCount,
                   std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
                       &segmentsToMerge,
                   std::shared_ptr<MergeContext> mergeContext) = 0;

  /**
   * Determine what set of merge operations is necessary in order to expunge all
   * deletes from the index.
   *  @param segmentInfos
   *          the total set of segments in the index
   * @param mergeContext the IndexWriter to find the merges on
   */
  virtual std::shared_ptr<MergeSpecification>
  findForcedDeletesMerges(std::shared_ptr<SegmentInfos> segmentInfos,
                          std::shared_ptr<MergeContext> mergeContext) = 0;

  /**
   * Returns true if a new segment (regardless of its origin) should use the
   * compound file format. The default implementation returns <code>true</code>
   * iff the size of the given mergedInfo is less or equal to
   * {@link #getMaxCFSSegmentSizeMB()} and the size is less or equal to the
   * TotalIndexSize * {@link #getNoCFSRatio()} otherwise <code>false</code>.
   */
  virtual bool useCompoundFile(
      std::shared_ptr<SegmentInfos> infos,
      std::shared_ptr<SegmentCommitInfo> mergedInfo,
      std::shared_ptr<MergeContext> mergeContext) ;

  /** Return the byte size of the provided {@link
   *  SegmentCommitInfo}, pro-rated by percentage of
   *  non-deleted documents is set. */
protected:
  virtual int64_t
  size(std::shared_ptr<SegmentCommitInfo> info,
       std::shared_ptr<MergeContext> mergeContext) ;

  /**
   * Asserts that the delCount for this SegmentCommitInfo is valid
   */
  bool assertDelCount(int delCount, std::shared_ptr<SegmentCommitInfo> info);

  /** Returns true if this single info is already fully merged (has no
   *  pending deletes, is in the same dir as the
   *  writer, and matches the current compound file setting */
  bool isMerged(std::shared_ptr<SegmentInfos> infos,
                std::shared_ptr<SegmentCommitInfo> info,
                std::shared_ptr<MergeContext> mergeContext) ;

  /** Returns current {@code noCFSRatio}.
   *
   *  @see #setNoCFSRatio */
public:
  virtual double getNoCFSRatio();

  /** If a merged segment will be more than this percentage
   *  of the total size of the index, leave the segment as
   *  non-compound file even if compound file is enabled.
   *  Set to 1.0 to always use CFS regardless of merge
   *  size. */
  virtual void setNoCFSRatio(double noCFSRatio);

  /** Returns the largest size allowed for a compound file segment */
  virtual double getMaxCFSSegmentSizeMB();

  /** If a merged segment will be more than this value,
   *  leave the segment as
   *  non-compound file even if compound file is enabled.
   *  Set this to Double.POSITIVE_INFINITY (default) and noCFSRatio to 1.0
   *  to always use CFS regardless of merge size. */
  virtual void setMaxCFSSegmentSizeMB(double v);

  /**
   * Returns true if the segment represented by the given CodecReader should be
   * keep even if it's fully deleted. This is useful for testing of for instance
   * if the merge policy implements retention policies for soft deletes.
   */
  virtual bool keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                                           readerIOSupplier) ;

  /**
   * Returns the number of deletes that a merge would claim on the given
   * segment. This method will by default return the sum of the del count on
   * disk and the pending delete count. Yet, subclasses that wrap merge readers
   * might modify this to reflect deletes that are carried over to the target
   * segment in the case of soft deletes.
   *
   * Soft deletes all deletes to survive across merges in order to control when
   * the soft-deleted data is claimed.
   * @see IndexWriter#softUpdateDocument(Term, Iterable, Field...)
   * @see IndexWriterConfig#setSoftDeletesField(std::wstring)
   * @param info the segment info that identifies the segment
   * @param delCount the number deleted documents for this segment
   * @param readerSupplier a supplier that allows to obtain a {@link
   * CodecReader} for this segment
   */
  virtual int numDeletesToMerge(std::shared_ptr<SegmentCommitInfo> info,
                                int delCount,
                                IOSupplier<std::shared_ptr<CodecReader>>
                                    readerSupplier) ;

  /**
   * Builds a std::wstring representation of the given SegmentCommitInfo instances
   */
protected:
  std::wstring
  segString(std::shared_ptr<MergeContext> mergeContext,
            std::deque<std::shared_ptr<SegmentCommitInfo>> &infos);

  /** Print a debug message to {@link MergeContext}'s {@code
   *  infoStream}. */
  void message(const std::wstring &message,
               std::shared_ptr<MergeContext> mergeContext);

  /**
   * Returns <code>true</code> if the info-stream is in verbose mode
   * @see #message(std::wstring, MergeContext)
   */
  bool verbose(std::shared_ptr<MergeContext> mergeContext);

  /**
   * This interface represents the current context of the merge selection
   * process. It allows to access real-time information like the currently
   * merging segments or how many deletes a segment would claim back if merged.
   * This context might be stateful and change during the execution of a merge
   * policy's selection processes.
   * @lucene.experimental
   */
public:
  class MergeContext
  {
    GET_CLASS_NAME(MergeContext)

    /**
     * Returns the number of deletes a merge would claim back if the given
     * segment is merged.
     * @see MergePolicy#numDeletesToMerge(SegmentCommitInfo, int,
     * org.apache.lucene.util.IOSupplier)
     * @param info the segment to get the number of deletes for
     */
  public:
    virtual int numDeletesToMerge(std::shared_ptr<SegmentCommitInfo> info) = 0;

    /**
     * Returns the number of deleted documents in the given segments.
     */
    virtual int numDeletedDocs(std::shared_ptr<SegmentCommitInfo> info) = 0;

    /**
     * Returns the info stream that can be used to log messages
     */
    virtual std::shared_ptr<InfoStream> getInfoStream() = 0;

    /**
     * Returns an unmodifiable set of segments that are currently merging.
     */
    virtual std::shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>>
    getMergingSegments() = 0;
  };
};

} // namespace org::apache::lucene::index
