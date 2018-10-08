#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"

#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"

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
 * <p>This class implements a {@link MergePolicy} that tries
 * to merge segments into levels of exponentially
 * increasing size, where each level has fewer segments than
 * the value of the merge factor. Whenever extra segments
 * (beyond the merge factor upper bound) are encountered,
 * all segments within the level are merged. You can get or
 * set the merge factor using {@link #getMergeFactor()} and
 * {@link #setMergeFactor(int)} respectively.</p>
 *
 * <p>This class is abstract and requires a subclass to
 * define the {@link #size} method which specifies how a
GET_CLASS_NAME(is)
 * segment's size is determined.  {@link LogDocMergePolicy}
 * is one subclass that measures size by document count in
 * the segment.  {@link LogByteSizeMergePolicy} is another
GET_CLASS_NAME(that)
 * subclass that measures size as the total byte size of the
 * file(s) for the segment.</p>
 */

class LogMergePolicy : public MergePolicy
{
  GET_CLASS_NAME(LogMergePolicy)

  /** Defines the allowed range of log(size) for each
   *  level.  A level is computed by taking the max segment
   *  log size, minus LEVEL_LOG_SPAN, and finding all
   *  segments falling within that range. */
public:
  static constexpr double LEVEL_LOG_SPAN = 0.75;

  /** Default merge factor, which is how many segments are
   *  merged at a time */
  static constexpr int DEFAULT_MERGE_FACTOR = 10;

  /** Default maximum segment size.  A segment of this size
   *  or larger will never be merged.  @see setMaxMergeDocs */
  static const int DEFAULT_MAX_MERGE_DOCS = std::numeric_limits<int>::max();

  /** Default noCFSRatio.  If a merge's size is {@code >= 10%} of
   *  the index, then we disable compound file for it.
   *  @see MergePolicy#setNoCFSRatio */
  static constexpr double DEFAULT_NO_CFS_RATIO = 0.1;

  /** How many segments to merge at a time. */
protected:
  int mergeFactor = DEFAULT_MERGE_FACTOR;

  /** Any segments whose size is smaller than this value
   *  will be rounded up to this value.  This ensures that
   *  tiny segments are aggressively merged. */
  int64_t minMergeSize = 0;

  /** If the size of a segment exceeds this value then it
   *  will never be merged. */
  int64_t maxMergeSize = 0;

  // Although the core MPs set it explicitly, we must default in case someone
  // out there wrote his own LMP ...
  /** If the size of a segment exceeds this value then it
   * will never be merged during {@link IndexWriter#forceMerge}. */
  int64_t maxMergeSizeForForcedMerge = std::numeric_limits<int64_t>::max();

  /** If a segment has more than this many documents then it
   *  will never be merged. */
  int maxMergeDocs = DEFAULT_MAX_MERGE_DOCS;

  /** If true, we pro-rate a segment's size by the
   *  percentage of non-deleted documents. */
  bool calibrateSizeByDeletes = true;

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
public:
  LogMergePolicy();

  /** <p>Returns the number of segments that are merged at
   * once and also controls the total number of segments
   * allowed to accumulate in the index.</p> */
  virtual int getMergeFactor();

  /** Determines how often segment indices are merged by
   * addDocument().  With smaller values, less RAM is used
   * while indexing, and searches are
   * faster, but indexing speed is slower.  With larger
   * values, more RAM is used during indexing, and while
   * searches is slower, indexing is
   * faster.  Thus larger values ({@code > 10}) are best for batch
   * index creation, and smaller values ({@code < 10}) for indices
   * that are interactively maintained. */
  virtual void setMergeFactor(int mergeFactor);

  /** Sets whether the segment size should be calibrated by
   *  the number of deletes when choosing segments for merge. */
  virtual void setCalibrateSizeByDeletes(bool calibrateSizeByDeletes);

  /** Returns true if the segment size should be calibrated
   *  by the number of deletes when choosing segments for merge. */
  virtual bool getCalibrateSizeByDeletes();

  /** Return the number of documents in the provided {@link
   *  SegmentCommitInfo}, pro-rated by percentage of
   *  non-deleted documents if {@link
   *  #setCalibrateSizeByDeletes} is set. */
protected:
  virtual int64_t
  sizeDocs(std::shared_ptr<SegmentCommitInfo> info,
           std::shared_ptr<MergeContext> mergeContext) ;

  /** Return the byte size of the provided {@link
   *  SegmentCommitInfo}, pro-rated by percentage of
   *  non-deleted documents if {@link
   *  #setCalibrateSizeByDeletes} is set. */
  virtual int64_t
  sizeBytes(std::shared_ptr<SegmentCommitInfo> info,
            std::shared_ptr<MergeContext> mergeContext) ;

  /** Returns true if the number of segments eligible for
   *  merging is less than or equal to the specified {@code
   *  maxNumSegments}. */
  virtual bool
  isMerged(std::shared_ptr<SegmentInfos> infos, int maxNumSegments,
           std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
               &segmentsToMerge,
           std::shared_ptr<MergeContext> mergeContext) ;

  /**
   * Returns the merges necessary to merge the index, taking the max merge
   * size or max merge docs into consideration. This method attempts to respect
   * the {@code maxNumSegments} parameter, however it might be, due to size
   * constraints, that more than that number of segments will remain in the
   * index. Also, this method does not guarantee that exactly {@code
   * maxNumSegments} will remain, but &lt;= that number.
   */
private:
  std::shared_ptr<MergeSpecification> findForcedMergesSizeLimit(
      std::shared_ptr<SegmentInfos> infos, int last,
      std::shared_ptr<MergeContext> mergeContext) ;

  /**
   * Returns the merges necessary to forceMerge the index. This method
   * constraints the returned merges only by the {@code maxNumSegments}
   * parameter, and guaranteed that exactly that number of segments will remain
   * in the index.
   */
  std::shared_ptr<MergeSpecification> findForcedMergesMaxNumSegments(
      std::shared_ptr<SegmentInfos> infos, int maxNumSegments, int last,
      std::shared_ptr<MergeContext> mergeContext) ;

  /** Returns the merges necessary to merge the index down
   *  to a specified number of segments.
   *  This respects the {@link #maxMergeSizeForForcedMerge} setting.
   *  By default, and assuming {@code maxNumSegments=1}, only
   *  one segment will be left in the index, where that segment
   *  has no deletions pending nor separate norms, and it is in
   *  compound file format if the current useCompoundFile
   *  setting is true.  This method returns multiple merges
   *  (mergeFactor at a time) so the {@link MergeScheduler}
   *  in use may make use of concurrency. */
public:
  std::shared_ptr<MergeSpecification> findForcedMerges(
      std::shared_ptr<SegmentInfos> infos, int maxNumSegments,
      std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
          &segmentsToMerge,
      std::shared_ptr<MergeContext> mergeContext)  override;

  /**
   * Finds merges necessary to force-merge all deletes from the
   * index.  We simply merge adjacent segments that have
   * deletes, up to mergeFactor at a time.
   */
  std::shared_ptr<MergeSpecification> findForcedDeletesMerges(
      std::shared_ptr<SegmentInfos> segmentInfos,
      std::shared_ptr<MergeContext> mergeContext)  override;

private:
  class SegmentInfoAndLevel
      : public std::enable_shared_from_this<SegmentInfoAndLevel>,
        public Comparable<std::shared_ptr<SegmentInfoAndLevel>>
  {
    GET_CLASS_NAME(SegmentInfoAndLevel)
  public:
    std::shared_ptr<SegmentCommitInfo> info;
    float level = 0;

    SegmentInfoAndLevel(std::shared_ptr<SegmentCommitInfo> info, float level);

    // Sorts largest to smallest
    int compareTo(std::shared_ptr<SegmentInfoAndLevel> other) override;
  };

  /** Checks if any merges are now necessary and returns a
   *  {@link MergePolicy.MergeSpecification} if so.  A merge
   *  is necessary when there are more than {@link
   *  #setMergeFactor} segments at a given level.  When
   *  multiple levels have too many segments, this method
   *  will return multiple merges, allowing the {@link
   *  MergeScheduler} to use concurrency. */
public:
  std::shared_ptr<MergeSpecification> findMerges(
      MergeTrigger mergeTrigger, std::shared_ptr<SegmentInfos> infos,
      std::shared_ptr<MergeContext> mergeContext)  override;

  /** <p>Determines the largest segment (measured by
   * document count) that may be merged with other segments.
   * Small values (e.g., less than 10,000) are best for
   * interactive indexing, as this limits the length of
   * pauses while indexing to a few seconds.  Larger values
   * are best for batched indexing and speedier
   * searches.</p>
   *
   * <p>The default value is {@link Integer#MAX_VALUE}.</p>
   *
   * <p>The default merge policy ({@link
   * LogByteSizeMergePolicy}) also allows you to set this
   * limit by net size (in MB) of the segment, using {@link
   * LogByteSizeMergePolicy#setMaxMergeMB}.</p>
   */
  virtual void setMaxMergeDocs(int maxMergeDocs);

  /** Returns the largest segment (measured by document
   *  count) that may be merged with other segments.
   *  @see #setMaxMergeDocs */
  virtual int getMaxMergeDocs();

  virtual std::wstring toString();

protected:
  std::shared_ptr<LogMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<LogMergePolicy>(
        MergePolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
