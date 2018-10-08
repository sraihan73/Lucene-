#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}

namespace org::apache::lucene::index
{
class SegmentInfos;
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

/**
 *  Merges segments of approximately equal size, subject to
 *  an allowed number of segments per tier.  This is similar
 *  to {@link LogByteSizeMergePolicy}, except this merge
 *  policy is able to merge non-adjacent segment, and
 *  separates how many segments are merged at once ({@link
 *  #setMaxMergeAtOnce}) from how many segments are allowed
 *  per tier ({@link #setSegmentsPerTier}).  This merge
 *  policy also does not over-merge (i.e. cascade merges).
 *
 *  <p>For normal merging, this policy first computes a
 *  "budget" of how many segments are allowed to be in the
 *  index.  If the index is over-budget, then the policy
 *  sorts segments by decreasing size (pro-rating by percent
 *  deletes), and then finds the least-cost merge.  Merge
 *  cost is measured by a combination of the "skew" of the
 *  merge (size of largest segment divided by smallest segment),
 *  total merge size and percent deletes reclaimed,
 *  so that merges with lower skew, smaller size
 *  and those reclaiming more deletes, are
 *  favored.
 *
 *  <p>If a merge will produce a segment that's larger than
 *  {@link #setMaxMergedSegmentMB}, then the policy will
 *  merge fewer segments (down to 1 at once, if that one has
 *  deletions) to keep the segment size under budget.
 *
 *  <p><b>NOTE</b>: this policy freely merges non-adjacent
 *  segments; if this is a problem, use {@link
 *  LogMergePolicy}.
 *
 *  <p><b>NOTE</b>: This policy always merges by byte size
 *  of the segments, always pro-rates by percent deletes,
 *  and does not apply any maximum segment size during
 *  forceMerge (unlike {@link LogByteSizeMergePolicy}).
 *
 *  @lucene.experimental
 */

// TODO
//   - we could try to take into account whether a large
//     merge is already running (under CMS) and then bias
//     ourselves towards picking smaller merges if so (or,
//     maybe CMS should do so)

class TieredMergePolicy : public MergePolicy
{
  GET_CLASS_NAME(TieredMergePolicy)
  /** Default noCFSRatio.  If a merge's size is {@code >= 10%} of
   *  the index, then we disable compound file for it.
   *  @see MergePolicy#setNoCFSRatio */
public:
  static constexpr double DEFAULT_NO_CFS_RATIO = 0.1;

private:
  int maxMergeAtOnce = 10;
  int64_t maxMergedSegmentBytes = 5 * 1024 * 1024 * 1024LL;
  int maxMergeAtOnceExplicit = 30;

  int64_t floorSegmentBytes = 2 * 1024 * 1024LL;
  double segsPerTier = 10.0;
  double forceMergeDeletesPctAllowed = 10.0;
  double reclaimDeletesWeight = 2.0;

  /** Sole constructor, setting all settings to their
   *  defaults. */
public:
  TieredMergePolicy();

  /** Maximum number of segments to be merged at a time
   *  during "normal" merging.  For explicit merging (eg,
   *  forceMerge or forceMergeDeletes was called), see {@link
   *  #setMaxMergeAtOnceExplicit}.  Default is 10. */
  virtual std::shared_ptr<TieredMergePolicy> setMaxMergeAtOnce(int v);

  /** Returns the current maxMergeAtOnce setting.
   *
   * @see #setMaxMergeAtOnce */
  virtual int getMaxMergeAtOnce();

  // TODO: should addIndexes do explicit merging, too?  And,
  // if user calls IW.maybeMerge "explicitly"

  /** Maximum number of segments to be merged at a time,
   *  during forceMerge or forceMergeDeletes. Default is 30. */
  virtual std::shared_ptr<TieredMergePolicy> setMaxMergeAtOnceExplicit(int v);

  /** Returns the current maxMergeAtOnceExplicit setting.
   *
   * @see #setMaxMergeAtOnceExplicit */
  virtual int getMaxMergeAtOnceExplicit();

  /** Maximum sized segment to produce during
   *  normal merging.  This setting is approximate: the
   *  estimate of the merged segment size is made by summing
   *  sizes of to-be-merged segments (compensating for
   *  percent deleted docs).  Default is 5 GB. */
  virtual std::shared_ptr<TieredMergePolicy> setMaxMergedSegmentMB(double v);

  /** Returns the current maxMergedSegmentMB setting.
   *
   * @see #setMaxMergedSegmentMB */
  virtual double getMaxMergedSegmentMB();

  /** Controls how aggressively merges that reclaim more
   *  deletions are favored.  Higher values will more
   *  aggressively target merges that reclaim deletions, but
   *  be careful not to go so high that way too much merging
   *  takes place; a value of 3.0 is probably nearly too
   *  high.  A value of 0.0 means deletions don't impact
   *  merge selection. */
  virtual std::shared_ptr<TieredMergePolicy> setReclaimDeletesWeight(double v);

  /** See {@link #setReclaimDeletesWeight}. */
  virtual double getReclaimDeletesWeight();

  /** Segments smaller than this are "rounded up" to this
   *  size, ie treated as equal (floor) size for merge
   *  selection.  This is to prevent frequent flushing of
   *  tiny segments from allowing a long tail in the index.
   *  Default is 2 MB. */
  virtual std::shared_ptr<TieredMergePolicy> setFloorSegmentMB(double v);

  /** Returns the current floorSegmentMB.
   *
   *  @see #setFloorSegmentMB */
  virtual double getFloorSegmentMB();

  /** When forceMergeDeletes is called, we only merge away a
   *  segment if its delete percentage is over this
   *  threshold.  Default is 10%. */
  virtual std::shared_ptr<TieredMergePolicy>
  setForceMergeDeletesPctAllowed(double v);

  /** Returns the current forceMergeDeletesPctAllowed setting.
   *
   * @see #setForceMergeDeletesPctAllowed */
  virtual double getForceMergeDeletesPctAllowed();

  /** Sets the allowed number of segments per tier.  Smaller
   *  values mean more merging but fewer segments.
   *
   *  <p><b>NOTE</b>: this value should be {@code >=} the {@link
   *  #setMaxMergeAtOnce} otherwise you'll force too much
   *  merging to occur.</p>
   *
   *  <p>Default is 10.0.</p> */
  virtual std::shared_ptr<TieredMergePolicy> setSegmentsPerTier(double v);

  /** Returns the current segmentsPerTier setting.
   *
   * @see #setSegmentsPerTier */
  virtual double getSegmentsPerTier();

private:
  class SegmentByteSizeDescending
      : public std::enable_shared_from_this<SegmentByteSizeDescending>,
        public Comparator<std::shared_ptr<SegmentCommitInfo>>
  {
    GET_CLASS_NAME(SegmentByteSizeDescending)
  private:
    std::shared_ptr<TieredMergePolicy> outerInstance;

    const std::unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t>
        sizeInBytes;

  public:
    SegmentByteSizeDescending(
        std::shared_ptr<TieredMergePolicy> outerInstance,
        std::unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t>
            &sizeInBytes);

    int compare(std::shared_ptr<SegmentCommitInfo> o1,
                std::shared_ptr<SegmentCommitInfo> o2) override;
  };

  /** Holds score and explanation for a single candidate
   *  merge. */
protected:
  class MergeScore : public std::enable_shared_from_this<MergeScore>
  {
    GET_CLASS_NAME(MergeScore)
    /** Sole constructor. (For invocation by subclass
     *  constructors, typically implicit.) */
  protected:
    MergeScore();

    /** Returns the score for this merge candidate; lower
     *  scores are better. */
  public:
    virtual double getScore() = 0;

    /** Human readable explanation of how the merge got this
     *  score. */
    virtual std::wstring getExplanation() = 0;
  };

private:
  std::unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t>
  getSegmentSizes(
      std::shared_ptr<MergeContext> mergeContext,
      std::shared_ptr<std::deque<std::shared_ptr<SegmentCommitInfo>>>
          infos) ;

public:
  std::shared_ptr<MergeSpecification> findMerges(
      MergeTrigger mergeTrigger, std::shared_ptr<SegmentInfos> infos,
      std::shared_ptr<MergeContext> mergeContext)  override;

  /** Expert: scores one merge; subclasses can override. */
protected:
  virtual std::shared_ptr<MergeScore>
  score(std::deque<std::shared_ptr<SegmentCommitInfo>> &candidate,
        bool hitTooLarge,
        std::unordered_map<std::shared_ptr<SegmentCommitInfo>, int64_t>
            &sizeInBytes) ;

private:
  class MergeScoreAnonymousInnerClass : public MergeScore
  {
    GET_CLASS_NAME(MergeScoreAnonymousInnerClass)
  private:
    std::shared_ptr<TieredMergePolicy> outerInstance;

    double skew = 0;
    double nonDelRatio = 0;
    double finalMergeScore = 0;

  public:
    MergeScoreAnonymousInnerClass(
        std::shared_ptr<TieredMergePolicy> outerInstance, double skew,
        double nonDelRatio, double finalMergeScore);

    double getScore() override;

    std::wstring getExplanation() override;

  protected:
    std::shared_ptr<MergeScoreAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MergeScoreAnonymousInnerClass>(
          MergeScore::shared_from_this());
    }
  };

public:
  std::shared_ptr<MergeSpecification> findForcedMerges(
      std::shared_ptr<SegmentInfos> infos, int maxSegmentCount,
      std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
          &segmentsToMerge,
      std::shared_ptr<MergeContext> mergeContext)  override;

  std::shared_ptr<MergeSpecification> findForcedDeletesMerges(
      std::shared_ptr<SegmentInfos> infos,
      std::shared_ptr<MergeContext> mergeContext)  override;

private:
  int64_t floorSize(int64_t bytes);

public:
  virtual std::wstring toString();

protected:
  std::shared_ptr<TieredMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<TieredMergePolicy>(
        MergePolicy::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
