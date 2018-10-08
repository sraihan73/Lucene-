#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::grouping
{
template <typename T>
class CollectedSearchGroup;
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
namespace org::apache::lucene::search::grouping
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using LeafFieldComparator = org::apache::lucene::search::LeafFieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using Sort = org::apache::lucene::search::Sort;

/** FirstPassGroupingCollector is the first of two passes necessary
 *  to collect grouped hits.  This pass gathers the top N sorted
 *  groups. Groups are defined by a {@link GroupSelector}
 *
 *  <p>See {@link org.apache.lucene.search.grouping} for more
 *  details including a full code example.</p>
 *
 * @lucene.experimental
 */
template <typename T>
class FirstPassGroupingCollector : public SimpleCollector
{
  GET_CLASS_NAME(FirstPassGroupingCollector)

private:
  const std::shared_ptr<GroupSelector<T>> groupSelector;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private final org.apache.lucene.search.FieldComparator<?>[]
  // comparators;
  std::deque < FieldComparator < ? >> const comparators;
  std::deque<std::shared_ptr<LeafFieldComparator>> const leafComparators;
  std::deque<int> const reversed;
  const int topNGroups;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool needsScores_;
  const std::unordered_map<T, CollectedSearchGroup<T>> groupMap;
  const int compIDXEnd;

  // Set once we reach topNGroups unique groups:
  /** @lucene.internal */
protected:
  std::set<CollectedSearchGroup<T>> orderedGroups;

private:
  int docBase = 0;
  int spareSlot = 0;

  /**
   * Create the first pass collector.
   *
   * @param groupSelector a GroupSelector used to defined groups
   * @param groupSort The {@link Sort} used to sort the
   *    groups.  The top sorted document within each group
   *    according to groupSort, determines how that group
   *    sorts against other groups.  This must be non-null,
   *    ie, if you want to groupSort by relevance use
   *    Sort.RELEVANCE.
   * @param topNGroups How many top groups to keep.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) public
  // FirstPassGroupingCollector(GroupSelector<T> groupSelector,
  // org.apache.lucene.search.Sort groupSort, int topNGroups)
  FirstPassGroupingCollector(std::shared_ptr<GroupSelector<T>> groupSelector,
                             std::shared_ptr<Sort> groupSort, int topNGroups)
      : groupSelector(groupSelector),
        comparators(
            std::deque<std::shared_ptr<FieldComparator>>(sortFields->length)),
        leafComparators(std::deque<std::shared_ptr<LeafFieldComparator>>(
            sortFields->length)),
        reversed(std::deque<int>(sortFields->length)), topNGroups(topNGroups),
        needsScores(groupSort->needsScores()),
        groupMap(std::unordered_map<>(topNGroups)),
        compIDXEnd(comparators.size() - 1)
  {
    if (topNGroups < 1) {
      throw std::invalid_argument("topNGroups must be >= 1 (got " +
                                  std::to_wstring(topNGroups) + L")");
    }

    // TODO: allow null groupSort to mean "by relevance",
    // and specialize it?

    std::deque<std::shared_ptr<SortField>> sortFields = groupSort->getSort();
    for (int i = 0; i < sortFields.size(); i++) {
      std::shared_ptr<SortField> *const sortField = sortFields[i];

      // use topNGroups + 1 so we have a spare slot to use for comparing
      // (tracked by this.spareSlot):
      comparators[i] = sortField->getComparator(topNGroups + 1, i);
      reversed[i] = sortField->getReverse() ? -1 : 1;
    }

    spareSlot = topNGroups;
  }

  bool needsScores() override { return needsScores_; }

  /**
   * Returns top groups, starting from offset.  This may
   * return null, if no groups were collected, or if the
   * number of unique groups collected is &lt;= offset.
   *
   * @param groupOffset The offset in the collected groups
   * @param fillFields Whether to fill to {@link SearchGroup#sortValues}
   * @return top groups, starting from offset
   */
  virtual std::shared_ptr<std::deque<SearchGroup<T>>>
  getTopGroups(int groupOffset, bool fillFields) 
  {

    // System.out.println("FP.getTopGroups groupOffset=" + groupOffset + "
    // fillFields=" + fillFields + " groupMap.size()=" + groupMap.size());

    if (groupOffset < 0) {
      throw std::invalid_argument("groupOffset must be >= 0 (got " +
                                  std::to_wstring(groupOffset) + L")");
    }

    if (groupMap.size() <= groupOffset) {
      return nullptr;
    }

    if (orderedGroups == nullptr) {
      buildSortedSet();
    }

    std::shared_ptr<std::deque<SearchGroup<T>>> *const result =
        std::deque<SearchGroup<T>>();
    int upto = 0;
    constexpr int sortFieldCount = comparators.size();
    for (auto group : orderedGroups) {
      if (upto++ < groupOffset) {
        continue;
      }
      // System.out.println("  group=" + (group.groupValue == null ? "null" :
      // group.groupValue.toString()));
      std::shared_ptr<SearchGroup<T>> searchGroup =
          std::make_shared<SearchGroup<T>>();
      searchGroup->groupValue = group->groupValue;
      if (fillFields) {
        searchGroup->sortValues = std::deque<std::any>(sortFieldCount);
        for (int sortFieldIDX = 0; sortFieldIDX < sortFieldCount;
             sortFieldIDX++) {
          searchGroup->sortValues[sortFieldIDX] =
              comparators[sortFieldIDX]->value(group->comparatorSlot);
        }
      }
      result->add(searchGroup);
    }
    // System.out.println("  return " + result.size() + " groups");
    return result;
  }

  void setScorer(std::shared_ptr<Scorer> scorer)  override
  {
    for (auto comparator : leafComparators) {
      comparator->setScorer(scorer);
    }
  }

private:
  bool isCompetitive(int doc) 
  {
    // If orderedGroups != null we already have collected N groups and
    // can short circuit by comparing this document to the bottom group,
    // without having to find what group this document belongs to.

    // Even if this document belongs to a group in the top N, we'll know that
    // we don't have to update that group.

    // Downside: if the number of unique groups is very low, this is
    // wasted effort as we will most likely be updating an existing group.
    if (orderedGroups != nullptr) {
      for (int compIDX = 0;; compIDX++) {
        constexpr int c =
            reversed[compIDX] * leafComparators[compIDX]->compareBottom(doc);
        if (c < 0) {
          // Definitely not competitive. So don't even bother to continue
          return false;
        } else if (c > 0) {
          // Definitely competitive.
          break;
        } else if (compIDX == compIDXEnd) {
          // Here c=0. If we're at the last comparator, this doc is not
          // competitive, since docs are visited in doc Id order, which means
          // this doc cannot compete with any other document in the queue.
          return false;
        }
      }
    }
    return true;
  }

public:
  void collect(int doc)  override
  {

    if (isCompetitive(doc) == false) {
      return;
    }

    // TODO: should we add option to mean "ignore docs that
    // don't have the group field" (instead of stuffing them
    // under null group)?
    groupSelector->advanceTo(doc);
    T groupValue = groupSelector->currentValue();

    std::shared_ptr<CollectedSearchGroup<T>> *const group =
        groupMap[groupValue];

    if (group == nullptr) {

      // First time we are seeing this group, or, we've seen
      // it before but it fell out of the top N and is now
      // coming back

      if (groupMap.size() < topNGroups) {

        // Still in startup transient: we have not
        // seen enough unique groups to start pruning them;
        // just keep collecting them

        // Add a new CollectedSearchGroup:
        std::shared_ptr<CollectedSearchGroup<T>> sg =
            std::make_shared<CollectedSearchGroup<T>>();
        sg->groupValue = groupSelector->copyValue();
        sg->comparatorSlot = groupMap.size();
        sg->topDoc = docBase + doc;
        for (auto fc : leafComparators) {
          fc->copy(sg->comparatorSlot, doc);
        }
        groupMap.emplace(sg->groupValue, sg);

        if (groupMap.size() == topNGroups) {
          // End of startup transient: we now have max
          // number of groups; from here on we will drop
          // bottom group when we insert new one:
          buildSortedSet();
        }

        return;
      }

      // We already tested that the document is competitive, so replace
      // the bottom group with this new group.
      std::shared_ptr<CollectedSearchGroup<T>> *const bottomGroup =
          orderedGroups.pollLast();
      assert(orderedGroups.size() == topNGroups - 1);

      groupMap.erase(bottomGroup->groupValue);

      // reuse the removed CollectedSearchGroup
      bottomGroup->groupValue = groupSelector->copyValue();
      bottomGroup->topDoc = docBase + doc;

      for (auto fc : leafComparators) {
        fc->copy(bottomGroup->comparatorSlot, doc);
      }

      groupMap.emplace(bottomGroup->groupValue, bottomGroup);
      orderedGroups.insert(bottomGroup);
      assert(orderedGroups.size() == topNGroups);

      constexpr int lastComparatorSlot = orderedGroups.last().comparatorSlot;
      for (auto fc : leafComparators) {
        fc->setBottom(lastComparatorSlot);
      }

      return;
    }

    // Update existing group:
    for (int compIDX = 0;; compIDX++) {
      leafComparators[compIDX]->copy(spareSlot, doc);

      constexpr int c =
          reversed[compIDX] *
          comparators[compIDX]->compare(group->comparatorSlot, spareSlot);
      if (c < 0) {
        // Definitely not competitive.
        return;
      } else if (c > 0) {
        // Definitely competitive; set remaining comparators:
        for (int compIDX2 = compIDX + 1; compIDX2 < comparators.size();
             compIDX2++) {
          leafComparators[compIDX2]->copy(spareSlot, doc);
        }
        break;
      } else if (compIDX == compIDXEnd) {
        // Here c=0. If we're at the last comparator, this doc is not
        // competitive, since docs are visited in doc Id order, which means
        // this doc cannot compete with any other document in the queue.
        return;
      }
    }

    // Remove before updating the group since lookup is done via comparators
    // TODO: optimize this

    std::shared_ptr<CollectedSearchGroup<T>> *const prevLast;
    if (orderedGroups != nullptr) {
      prevLast = orderedGroups.last();
      orderedGroups.remove(group);
      assert(orderedGroups.size() == topNGroups - 1);
    } else {
      prevLast.reset();
    }

    group->topDoc = docBase + doc;

    // Swap slots
    constexpr int tmp = spareSlot;
    spareSlot = group->comparatorSlot;
    group->comparatorSlot = tmp;

    // Re-add the changed group
    if (orderedGroups != nullptr) {
      orderedGroups.insert(group);
      assert(orderedGroups.size() == topNGroups);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: final CollectedSearchGroup<?> newLast =
      // orderedGroups.last();
      std::shared_ptr < CollectedSearchGroup < ? >> *const newLast =
                                                     orderedGroups.last();
      // If we changed the value of the last group, or changed which group was
      // last, then update bottom:
      if (group == newLast || prevLast != newLast) {
        for (auto fc : leafComparators) {
          fc->setBottom(newLast->comparatorSlot);
        }
      }
    }
  }

private:
  void buildSortedSet() 
  {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final java.util.Comparator<CollectedSearchGroup<?>>
    // comparator = new java.util.Comparator<CollectedSearchGroup<?>>()
    std::shared_ptr < Comparator < CollectedSearchGroup <
        ? >>> *const comparator =
              std::make_shared<ComparatorAnonymousInnerClass>(
                  shared_from_this());

    orderedGroups = std::set<CollectedSearchGroup<T>>(comparator);
    orderedGroups.addAll(groupMap.values());
    assert(orderedGroups.size() > 0);

    for (auto fc : leafComparators) {
      fc->setBottom(orderedGroups.last().comparatorSlot);
    }
  }

private:
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private static class ComparatorAnonymousInnerClass
  // implements java.util.Comparator<CollectedSearchGroup<?>>
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator < CollectedSearchGroup <
        ? >>
  {
  private:
    std::shared_ptr<FirstPassGroupingCollector<std::shared_ptr<T>>>
        outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<FirstPassGroupingCollector<std::shared_ptr<T>>>
            outerInstance);

    template <typename T1, typename T2>
    int compare(std::shared_ptr<CollectedSearchGroup<T1>> o1,
                std::shared_ptr<CollectedSearchGroup<T2>> o2);
  };

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> readerContext) throw(
      IOException) override
  {
    docBase = readerContext->docBase;
    for (int i = 0; i < comparators.size(); i++) {
      leafComparators[i] = comparators[i]->getLeafComparator(readerContext);
    }
    groupSelector->setNextReader(readerContext);
  }

  /**
   * @return the GroupSelector used for this Collector
   */
public:
  virtual std::shared_ptr<GroupSelector<T>> getGroupSelector()
  {
    return groupSelector;
  }

protected:
  std::shared_ptr<FirstPassGroupingCollector> shared_from_this()
  {
    return std::static_pointer_cast<FirstPassGroupingCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
