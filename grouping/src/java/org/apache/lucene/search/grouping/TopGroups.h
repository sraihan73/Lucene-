#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <optional>
#include <stdexcept>
#include <deque>

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
namespace org::apache::lucene::search::grouping
{

using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;

/** Represents result returned by a grouping search.
 *
 * @lucene.experimental */
template <typename T>
class TopGroups : public std::enable_shared_from_this<TopGroups>
{
  GET_CLASS_NAME(TopGroups)
  /** Number of documents matching the search */
public:
  const int totalHitCount;

  /** Number of documents grouped into the topN groups */
  const int totalGroupedHitCount;

  /** The total number of unique groups. If <code>null</code> this value is not
   * computed. */
  const std::optional<int> totalGroupCount;

  /** Group results in groupSort order */
  std::deque<GroupDocs<T>> const groups;

  /** How groups are sorted against each other */
  std::deque<std::shared_ptr<SortField>> const groupSort;

  /** How docs are sorted within each group */
  std::deque<std::shared_ptr<SortField>> const withinGroupSort;

  /** Highest score across all hits, or
   *  <code>Float.NaN</code> if scores were not computed. */
  const float maxScore;

  TopGroups(std::deque<std::shared_ptr<SortField>> &groupSort,
            std::deque<std::shared_ptr<SortField>> &withinGroupSort,
            int totalHitCount, int totalGroupedHitCount,
            std::deque<GroupDocs<T>> &groups, float maxScore)
      : totalHitCount(totalHitCount),
        totalGroupedHitCount(totalGroupedHitCount),
        totalGroupCount(std::nullopt), groups(groups), groupSort(groupSort),
        withinGroupSort(withinGroupSort), maxScore(maxScore)
  {
  }

  TopGroups(std::shared_ptr<TopGroups<T>> oldTopGroups,
            std::optional<int> &totalGroupCount)
      : totalHitCount(oldTopGroups->totalHitCount),
        totalGroupedHitCount(oldTopGroups->totalGroupedHitCount),
        totalGroupCount(totalGroupCount), groups(oldTopGroups->groups),
        groupSort(oldTopGroups->groupSort),
        withinGroupSort(oldTopGroups->withinGroupSort),
        maxScore(oldTopGroups->maxScore)
  {
  }

  /** How the GroupDocs score (if any) should be merged. */
public:
  enum class ScoreMergeMode {
    GET_CLASS_NAME(ScoreMergeMode)
    /** Set score to Float.NaN */
    None,
    /* Sum score across all shards for this group. */
    Total,
    /* Avg score across all shards for this group. */
    Avg,
  };

  /** Merges an array of TopGroups, for example obtained
   *  from the second-pass collector across multiple
   *  shards.  Each TopGroups must have been sorted by the
   *  same groupSort and docSort, and the top groups passed
   *  to all second-pass collectors must be the same.
   *
   * <b>NOTE</b>: We can't always compute an exact totalGroupCount.
   * Documents belonging to a group may occur on more than
   * one shard and thus the merged totalGroupCount can be
   * higher than the actual totalGroupCount. In this case the
   * totalGroupCount represents a upper bound. If the documents
   * of one group do only reside in one shard then the
   * totalGroupCount is exact.
   *
   * <b>NOTE</b>: the topDocs in each GroupDocs is actually
   * an instance of TopDocsAndShards
   */
public:
  template <typename T>
  static std::shared_ptr<TopGroups<T>>
  merge(std::deque<TopGroups<T>> &shardGroups, std::shared_ptr<Sort> groupSort,
        std::shared_ptr<Sort> docSort, int docOffset, int docTopN,
        ScoreMergeMode scoreMergeMode)
  {

    // System.out.println("TopGroups.merge");

    if (shardGroups.empty()) {
      return nullptr;
    }

    int totalHitCount = 0;
    int totalGroupedHitCount = 0;
    // Optionally merge the totalGroupCount.
    std::optional<int> totalGroupCount = std::nullopt;

    constexpr int numGroups = shardGroups[0]->groups->size();
    for (auto shard : shardGroups) {
      if (numGroups != shard->groups.size()) {
        throw std::invalid_argument(
            "number of groups differs across shards; you must pass same top "
            "groups to all shards' second-pass collector");
      }
      totalHitCount += shard->totalHitCount;
      totalGroupedHitCount += shard->totalGroupedHitCount;
      if (shard->totalGroupCount) {
        if (!totalGroupCount) {
          totalGroupCount = 0;
        }

        totalGroupCount += shard->totalGroupCount;
      }
    }

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
    // GroupDocs<T>[] mergedGroupDocs = new GroupDocs[numGroups];
    std::deque<GroupDocs<T>> mergedGroupDocs =
        std::deque<std::shared_ptr<GroupDocs>>(numGroups);

    std::deque<std::shared_ptr<TopDocs>> shardTopDocs;
    if (docSort->equals(Sort::RELEVANCE)) {
      shardTopDocs = std::deque<std::shared_ptr<TopDocs>>(shardGroups.size());
    } else {
      shardTopDocs =
          std::deque<std::shared_ptr<TopFieldDocs>>(shardGroups.size());
    }
    float totalMaxScore = Float::MIN_VALUE;

    for (int groupIDX = 0; groupIDX < numGroups; groupIDX++) {
      constexpr T groupValue = shardGroups[0]->groups[groupIDX].groupValue;
      // System.out.println("  merge groupValue=" + groupValue + " sortValues="
      // + Arrays.toString(shardGroups[0].groups[groupIDX].groupSortValues));
      float maxScore = Float::MIN_VALUE;
      int totalHits = 0;
      double scoreSum = 0.0;
      for (int shardIDX = 0; shardIDX < shardGroups.size(); shardIDX++) {
        // System.out.println("    shard=" + shardIDX);
        std::shared_ptr<TopGroups<T>> *const shard = shardGroups[shardIDX];
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: final GroupDocs<?> shardGroupDocs =
        // shard.groups[groupIDX];
        std::shared_ptr < GroupDocs < ? >> *const shardGroupDocs =
                                            shard->groups[groupIDX];
        if (groupValue == nullptr) {
          if (shardGroupDocs->groupValue != nullptr) {
            throw std::invalid_argument(
                "group values differ across shards; you must pass same top "
                "groups to all shards' second-pass collector");
          }
        } else if (!groupValue.equals(shardGroupDocs->groupValue)) {
          throw std::invalid_argument(
              "group values differ across shards; you must pass same top "
              "groups to all shards' second-pass collector");
        }

        /*
        for(ScoreDoc sd : shardGroupDocs.scoreDocs) {
          System.out.println("      doc=" + sd.doc);
        }
        */

        if (docSort->equals(Sort::RELEVANCE)) {
          shardTopDocs[shardIDX] = std::make_shared<TopDocs>(
              shardGroupDocs->totalHits, shardGroupDocs->scoreDocs,
              shardGroupDocs->maxScore);
        } else {
          shardTopDocs[shardIDX] = std::make_shared<TopFieldDocs>(
              shardGroupDocs->totalHits, shardGroupDocs->scoreDocs,
              docSort->getSort(), shardGroupDocs->maxScore);
        }
        maxScore = std::max(maxScore, shardGroupDocs->maxScore);
        totalHits += shardGroupDocs->totalHits;
        scoreSum += shardGroupDocs->score;
      }

      std::shared_ptr<TopDocs> *const mergedTopDocs;
      if (docSort->equals(Sort::RELEVANCE)) {
        mergedTopDocs = TopDocs::merge(docOffset + docTopN, shardTopDocs);
      } else {
        mergedTopDocs = TopDocs::merge(
            docSort, docOffset + docTopN,
            static_cast<std::deque<std::shared_ptr<TopFieldDocs>>>(
                shardTopDocs));
      }

      // Slice;
      std::deque<std::shared_ptr<ScoreDoc>> mergedScoreDocs;
      if (docOffset == 0) {
        mergedScoreDocs = mergedTopDocs->scoreDocs;
      } else if (docOffset >= mergedTopDocs->scoreDocs.size()) {
        mergedScoreDocs = std::deque<std::shared_ptr<ScoreDoc>>(0);
      } else {
        mergedScoreDocs = std::deque<std::shared_ptr<ScoreDoc>>(
            mergedTopDocs->scoreDocs.size() - docOffset);
        System::arraycopy(mergedTopDocs->scoreDocs, docOffset, mergedScoreDocs,
                          0, mergedTopDocs->scoreDocs.size() - docOffset);
      }

      constexpr float groupScore;
      switch (scoreMergeMode) {
      case org::apache::lucene::search::grouping::TopGroups::ScoreMergeMode::
          None:
        groupScore = NAN;
        break;
      case org::apache::lucene::search::grouping::TopGroups::ScoreMergeMode::
          Avg:
        if (totalHits > 0) {
          groupScore = static_cast<float>(scoreSum / totalHits);
        } else {
          groupScore = NAN;
        }
        break;
      case org::apache::lucene::search::grouping::TopGroups::ScoreMergeMode::
          Total:
        groupScore = static_cast<float>(scoreSum);
        break;
      default:
        throw std::invalid_argument("can't handle ScoreMergeMode " +
                                    scoreMergeMode);
      }

      // System.out.println("SHARDS=" +
      // Arrays.toString(mergedTopDocs.shardIndex));
      mergedGroupDocs[groupIDX] = std::make_shared<GroupDocs<T>>(
          groupScore, maxScore, totalHits, mergedScoreDocs, groupValue,
          shardGroups[0]->groups[groupIDX].groupSortValues);
      totalMaxScore = std::max(totalMaxScore, maxScore);
    }

    if (totalGroupCount) {
      std::shared_ptr<TopGroups<T>> result = std::make_shared<TopGroups<T>>(
          groupSort->getSort(), docSort->getSort(), totalHitCount,
          totalGroupedHitCount, mergedGroupDocs, totalMaxScore);
      return std::make_shared<TopGroups<T>>(result, totalGroupCount);
    } else {
      return std::make_shared<TopGroups<T>>(
          groupSort->getSort(), docSort->getSort(), totalHitCount,
          totalGroupedHitCount, mergedGroupDocs, totalMaxScore);
    }
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
