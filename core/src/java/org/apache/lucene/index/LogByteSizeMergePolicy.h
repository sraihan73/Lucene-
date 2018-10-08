#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
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

/** This is a {@link LogMergePolicy} that measures size of a
 *  segment as the total byte size of the segment's files. */
class LogByteSizeMergePolicy : public LogMergePolicy
{
  GET_CLASS_NAME(LogByteSizeMergePolicy)

  /** Default minimum segment size.  @see setMinMergeMB */
public:
  static constexpr double DEFAULT_MIN_MERGE_MB = 1.6;

  /** Default maximum segment size.  A segment of this size
   *  or larger will never be merged.  @see setMaxMergeMB */
  static constexpr double DEFAULT_MAX_MERGE_MB = 2048;

  /** Default maximum segment size.  A segment of this size
   *  or larger will never be merged during forceMerge.  @see
   * setMaxMergeMBForForceMerge */
  static const double DEFAULT_MAX_MERGE_MB_FOR_FORCED_MERGE;

  /** Sole constructor, setting all settings to their
   *  defaults. */
  LogByteSizeMergePolicy();

protected:
  int64_t
  size(std::shared_ptr<SegmentCommitInfo> info,
       std::shared_ptr<MergeContext> mergeContext)  override;

  /** <p>Determines the largest segment (measured by total
   *  byte size of the segment's files, in MB) that may be
   *  merged with other segments.  Small values (e.g., less
   *  than 50 MB) are best for interactive indexing, as this
   *  limits the length of pauses while indexing to a few
   *  seconds.  Larger values are best for batched indexing
   *  and speedier searches.</p>
   *
   *  <p>Note that {@link #setMaxMergeDocs} is also
   *  used to check whether a segment is too large for
   *  merging (it's either or).</p>*/
public:
  virtual void setMaxMergeMB(double mb);

  /** Returns the largest segment (measured by total byte
   *  size of the segment's files, in MB) that may be merged
   *  with other segments.
   *  @see #setMaxMergeMB */
  virtual double getMaxMergeMB();

  /** <p>Determines the largest segment (measured by total
   *  byte size of the segment's files, in MB) that may be
   *  merged with other segments during forceMerge. Setting
   *  it low will leave the index with more than 1 segment,
   *  even if {@link IndexWriter#forceMerge} is called.*/
  virtual void setMaxMergeMBForForcedMerge(double mb);

  /** Returns the largest segment (measured by total byte
   *  size of the segment's files, in MB) that may be merged
   *  with other segments during forceMerge.
   *  @see #setMaxMergeMBForForcedMerge */
  virtual double getMaxMergeMBForForcedMerge();

  /** Sets the minimum size for the lowest level segments.
   * Any segments below this size are considered to be on
   * the same level (even if they vary drastically in size)
   * and will be merged whenever there are mergeFactor of
   * them.  This effectively truncates the "long tail" of
   * small segments that would otherwise be created into a
   * single level.  If you set this too large, it could
   * greatly increase the merging cost during indexing (if
   * you flush many small segments). */
  virtual void setMinMergeMB(double mb);

  /** Get the minimum size for a segment to remain
   *  un-merged.
   *  @see #setMinMergeMB **/
  virtual double getMinMergeMB();

protected:
  std::shared_ptr<LogByteSizeMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<LogByteSizeMergePolicy>(
        LogMergePolicy::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
