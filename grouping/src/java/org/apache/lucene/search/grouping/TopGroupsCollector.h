#pragma once
#include "stringhelper.h"
#include <cmath>
#include <functional>
#include <memory>
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
using TopDocsCollector = org::apache::lucene::search::TopDocsCollector;

/**
 * A second-pass collector that collects the TopDocs for each group, and
 * returns them as a {@link TopGroups} object
 *
 * @param <T> the type of the group value
 */
template <typename T>
class TopGroupsCollector : public SecondPassGroupingCollector<T>
{
  GET_CLASS_NAME(TopGroupsCollector)

private:
  const std::shared_ptr<Sort> groupSort;
  const std::shared_ptr<Sort> withinGroupSort;
  const int maxDocsPerGroup;

  /**
   * Create a new TopGroupsCollector
   * @param groupSelector     the group selector used to define groups
   * @param groups            the groups to collect TopDocs for
   * @param groupSort         the order in which groups are returned
   * @param withinGroupSort   the order in which documents are sorted in each
   * group
   * @param maxDocsPerGroup   the maximum number of docs to collect for each
   * group
   * @param getScores         if true, record the scores of all docs in each
   * group
   * @param getMaxScores      if true, record the maximum score for each group
   * @param fillSortFields    if true, record the sort field values for all docs
   */
public:
  TopGroupsCollector(std::shared_ptr<GroupSelector<T>> groupSelector,
                     std::shared_ptr<std::deque<SearchGroup<T>>> groups,
                     std::shared_ptr<Sort> groupSort,
                     std::shared_ptr<Sort> withinGroupSort, int maxDocsPerGroup,
                     bool getScores, bool getMaxScores, bool fillSortFields)
      : SecondPassGroupingCollector<T>(
            groupSelector, groups,
            new TopDocsReducer<>(withinGroupSort, maxDocsPerGroup, getScores,
                                 getMaxScores, fillSortFields)),
        groupSort(Objects::requireNonNull(groupSort)),
        withinGroupSort(Objects::requireNonNull(withinGroupSort)),
        maxDocsPerGroup(maxDocsPerGroup)
  {
  }

private:
  template <typename T>
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: private static class TopDocsReducer<T> extends
      // GroupReducer<T, org.apache.lucene.search.TopDocsCollector<?>>
      class TopDocsReducer : public GroupReducer < T,
      TopDocsCollector < ? >>
  {

  private:
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: private final
    // java.util.function.Supplier<org.apache.lucene.search.TopDocsCollector<?>>
    // supplier;
    const std::function < TopDocsCollector < ? > *() > supplier;
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool needsScores_;

  public:
        TopDocsReducer(std::shared_ptr<Sort> withinGroupSort, int maxDocsPerGroup, bool getScores, bool getMaxScores, bool fillSortFields) : supplier(withinGroupSort == Sort::RELEVANCE ? [&] ()
        {
      TopScoreDocCollector::create(maxDocsPerGroup)
          : [&]() {
              TopFieldCollector::create(withinGroupSort, maxDocsPerGroup,
                                        fillSortFields, getScores, getMaxScores,
                                        true);
            };
        }) / * TODO: disable exact counts? */, needsScores(getScores || getMaxScores || withinGroupSort->needsScores())
        {
        }

        bool needsScores() override { return needsScores_; }

      protected:
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: @Override protected
        // org.apache.lucene.search.TopDocsCollector<?> newCollector()
        std::shared_ptr < TopDocsCollector < ? >> newCollector() override
        {
          return supplier->get();
        }

      protected:
        std::shared_ptr<TopDocsReducer> shared_from_this()
        {
              return std::static_pointer_cast<TopDocsReducer>(GroupReducer<T, org.apache.lucene.search.TopDocsCollector<?>>::shared_from_this());
        }
  };

  /**
   * Get the TopGroups recorded by this collector
   * @param withinGroupOffset the offset within each group to start collecting
   * documents
   */
public:
  virtual std::shared_ptr<TopGroups<T>> getTopGroups(int withinGroupOffset)
  {
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
    // GroupDocs<T>[] groupDocsResult = (GroupDocs<T>[]) new
    // GroupDocs[groups.size()];
    std::deque<GroupDocs<T>> groupDocsResult =
        static_cast<std::deque<GroupDocs<T>>>(
            std::deque<std::shared_ptr<GroupDocs>>(groups->size()));

    int groupIDX = 0;
    float maxScore = Float::MIN_VALUE;
    for (auto group : groups) {
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: org.apache.lucene.search.TopDocsCollector<?> collector =
      // (org.apache.lucene.search.TopDocsCollector<?>)
      // groupReducer.getCollector(group.groupValue);
      std::shared_ptr < TopDocsCollector <
          ? >> collector = std::static_pointer_cast < TopDocsCollector <
            ? >> (groupReducer->getCollector(group->groupValue));
      std::shared_ptr<TopDocs> *const topDocs =
          collector->topDocs(withinGroupOffset, maxDocsPerGroup);
      groupDocsResult[groupIDX++] = std::make_shared<GroupDocs<T>>(
          NAN, topDocs->getMaxScore(), topDocs->totalHits, topDocs->scoreDocs,
          group->groupValue, group->sortValues);
      maxScore = std::max(maxScore, topDocs->getMaxScore());
    }

    return std::make_shared<TopGroups<T>>(
        groupSort->getSort(), withinGroupSort->getSort(), totalHitCount,
        totalGroupedHitCount, groupDocsResult, maxScore);
  }

protected:
  std::shared_ptr<TopGroupsCollector> shared_from_this()
  {
    return std::static_pointer_cast<TopGroupsCollector>(
        SecondPassGroupingCollector<T>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
