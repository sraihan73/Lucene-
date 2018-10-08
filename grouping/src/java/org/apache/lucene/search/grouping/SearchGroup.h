#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
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

using FieldComparator = org::apache::lucene::search::FieldComparator;
using Sort = org::apache::lucene::search::Sort;

/**
 * Represents a group that is found during the first pass search.
 *
 * @lucene.experimental
 */
template <typename T>
class SearchGroup : public std::enable_shared_from_this<SearchGroup>
{
  GET_CLASS_NAME(SearchGroup)

  /** The value that defines this group  */
public:
  T groupValue;

  /** The sort values used during sorting. These are the
   *  groupSort field values of the highest rank document
   *  (by the groupSort) within the group.  Can be
   * <code>null</code> if <code>fillFields=false</code> had
   * been passed to {@link FirstPassGroupingCollector#getTopGroups} */
  std::deque<std::any> sortValues;

  std::wstring toString() override
  {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return (L"SearchGroup(groupValue=" + groupValue + L" sortValues=" +
            Arrays->toString(sortValues) + L")");
  }

  bool equals(std::any o) override
  {
    if (shared_from_this() == o) {
      return true;
    }
    if (o == nullptr || getClass() != o.type()) {
      return false;
    }

    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: SearchGroup<?> that = (SearchGroup<?>) o;
    std::shared_ptr < SearchGroup < ? >> that = std::any_cast < SearchGroup <
                                      ? >> (o);

    if (groupValue == nullptr) {
      if (that->groupValue != nullptr) {
        return false;
      }
    } else if (!groupValue.equals(that->groupValue)) {
      return false;
    }

    return true;
  }

  int hashCode() override
  {
    return groupValue != nullptr ? groupValue.hashCode() : 0;
  }

private:
  template <typename T>
  class ShardIter : public std::enable_shared_from_this<ShardIter>
  {
    GET_CLASS_NAME(ShardIter)
  public:
    const std::shared_ptr<Iterator<SearchGroup<T>>> iter;
    const int shardIndex;

    ShardIter(std::shared_ptr<std::deque<SearchGroup<T>>> shard, int shardIndex)
        : iter(shard->begin()), shardIndex(shardIndex)
    {
      assert(iter->hasNext());
    }

    virtual std::shared_ptr<SearchGroup<T>> next()
    {
      assert(iter->hasNext());
      std::shared_ptr<SearchGroup<T>> *const group = iter->next();
      if (group->sortValues.empty()) {
        throw std::invalid_argument(
            "group.sortValues is null; you must pass fillFields=true to the "
            "first pass collector");
      }
      return group;
    }

    std::wstring toString() override
    {
      return L"ShardIter(shard=" + std::to_wstring(shardIndex) + L")";
    }
  };

  // Holds all shards currently on the same group
private:
  template <typename T>
  class MergedGroup : public std::enable_shared_from_this<MergedGroup>
  {
    GET_CLASS_NAME(MergedGroup)

    // groupValue may be null!
  public:
    const T groupValue;

    std::deque<std::any> topValues;
    const std::deque<ShardIter<T>> shards = std::deque<ShardIter<T>>();
    int minShardIndex = 0;
    bool processed = false;
    bool inQueue = false;

    MergedGroup(T groupValue) : groupValue(groupValue) {}

    // Only for assert
  private:
    bool neverEquals(std::any _other)
    {
      if (std::dynamic_pointer_cast<MergedGroup>(_other) != nullptr) {
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: MergedGroup<?> other = (MergedGroup<?>) _other;
        std::shared_ptr < MergedGroup <
            ? >> other = std::any_cast < MergedGroup < ? >> (_other);
        if (groupValue == nullptr) {
          assert(other->groupValue != nullptr);
        } else {
          assert(!groupValue.equals(other->groupValue));
        }
      }
      return true;
    }

  public:
    bool equals(std::any _other) override
    {
      // We never have another MergedGroup instance with
      // same groupValue
      assert(neverEquals(_other));

      if (std::dynamic_pointer_cast<MergedGroup>(_other) != nullptr) {
        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: MergedGroup<?> other = (MergedGroup<?>) _other;
        std::shared_ptr < MergedGroup <
            ? >> other = std::any_cast < MergedGroup < ? >> (_other);
        if (groupValue == nullptr) {
          return other == nullptr;
        } else {
          return groupValue.equals(other);
        }
      } else {
        return false;
      }
    }

    int hashCode() override
    {
      if (groupValue == nullptr) {
        return 0;
      } else {
        return groupValue.hashCode();
      }
    }
  };

private:
  template <typename T>
  class GroupComparator : public std::enable_shared_from_this<GroupComparator>,
                          public Comparator<MergedGroup<T>>
  {
    GET_CLASS_NAME(GroupComparator)

  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("rawtypes") public final
    // org.apache.lucene.search.FieldComparator[] comparators;
    std::deque<std::shared_ptr<FieldComparator>> const comparators;

    std::deque<int> const reversed;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) public
    // GroupComparator(org.apache.lucene.search.Sort groupSort)
    GroupComparator(std::shared_ptr<Sort> groupSort)
        : comparators(std::deque<std::shared_ptr<FieldComparator>>(
              sortFields->length)),
          reversed(std::deque<int>(sortFields->length))
    {
      std::deque<std::shared_ptr<SortField>> sortFields = groupSort->getSort();
      for (int compIDX = 0; compIDX < sortFields.size(); compIDX++) {
        std::shared_ptr<SortField> *const sortField = sortFields[compIDX];
        comparators[compIDX] = sortField->getComparator(1, compIDX);
        reversed[compIDX] = sortField->getReverse() ? -1 : 1;
      }
    }

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Override @SuppressWarnings({"unchecked","rawtypes"})
    // public int compare(MergedGroup<T> group, MergedGroup<T> other)
    int compare(std::shared_ptr<MergedGroup<T>> group,
                std::shared_ptr<MergedGroup<T>> other) override
    {
      if (group == other) {
        return 0;
      }
      // System.out.println("compare group=" + group + " other=" + other);
      const std::deque<std::any> groupValues = group->topValues;
      const std::deque<std::any> otherValues = other->topValues;
      // System.out.println("  groupValues=" + groupValues + " otherValues=" +
      // otherValues);
      for (int compIDX = 0; compIDX < comparators.size(); compIDX++) {
        constexpr int c =
            reversed[compIDX] * comparators[compIDX]->compareValues(
                                    groupValues[compIDX], otherValues[compIDX]);
        if (c != 0) {
          return c;
        }
      }

      // Tie break by min shard index:
      assert(group->minShardIndex != other->minShardIndex);
      return group->minShardIndex - other->minShardIndex;
    }
  };

private:
  template <typename T>
  class GroupMerger : public std::enable_shared_from_this<GroupMerger>
  {
    GET_CLASS_NAME(GroupMerger)

  private:
    const std::shared_ptr<GroupComparator<T>> groupComp;
    const std::shared_ptr<NavigableSet<MergedGroup<T>>> queue;
    const std::unordered_map<T, MergedGroup<T>> groupsSeen;

  public:
    GroupMerger(std::shared_ptr<Sort> groupSort)
        : groupComp(std::make_shared<GroupComparator<T>>(groupSort)),
          queue(std::set<>(groupComp)), groupsSeen(std::unordered_map<>())
    {
    }

  private:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) private void
    // updateNextGroup(int topN, ShardIter<T> shard)
    void updateNextGroup(int topN, std::shared_ptr<ShardIter<T>> shard)
    {
      while (shard->iter->hasNext()) {
        std::shared_ptr<SearchGroup<T>> *const group = shard->next();
        std::shared_ptr<MergedGroup<T>> mergedGroup =
            groupsSeen[group->groupValue];
        constexpr bool isNew = mergedGroup == nullptr;
        // System.out.println("    next group=" + (group.groupValue == null ?
        // "null" : ((BytesRef) group.groupValue).utf8ToString()) + " sort=" +
        // Arrays.toString(group.sortValues));

        if (isNew) {
          // Start a new group:
          // System.out.println("      new");
          mergedGroup = std::make_shared<MergedGroup<T>>(group->groupValue);
          mergedGroup->minShardIndex = shard->shardIndex;
          assert(group->sortValues.size() > 0);
          mergedGroup->topValues = group->sortValues;
          groupsSeen.emplace(group->groupValue, mergedGroup);
          mergedGroup->inQueue = true;
          queue->add(mergedGroup);
        } else if (mergedGroup->processed) {
          // This shard produced a group that we already
          // processed; move on to next group...
          continue;
        } else {
          // System.out.println("      old");
          bool competes = false;
          for (int compIDX = 0; compIDX < groupComp->comparators.size();
               compIDX++) {
            constexpr int cmp = groupComp->reversed[compIDX] *
                                groupComp->comparators[compIDX]->compareValues(
                                    group->sortValues[compIDX],
                                    mergedGroup->topValues[compIDX]);
            if (cmp < 0) {
              // Definitely competes
              competes = true;
              break;
            } else if (cmp > 0) {
              // Definitely does not compete
              break;
            } else if (compIDX == groupComp->comparators.size() - 1) {
              if (shard->shardIndex < mergedGroup->minShardIndex) {
                competes = true;
              }
            }
          }

          // System.out.println("      competes=" + competes);

          if (competes) {
            // Group's sort changed -- remove & re-insert
            if (mergedGroup->inQueue) {
              queue->remove(mergedGroup);
            }
            mergedGroup->topValues = group->sortValues;
            mergedGroup->minShardIndex = shard->shardIndex;
            queue->add(mergedGroup);
            mergedGroup->inQueue = true;
          }
        }

        mergedGroup->shards.push_back(shard);
        break;
        shard.iter++;
      }

      // Prune un-competitive groups:
      while (queue->size() > topN) {
        std::shared_ptr<MergedGroup<T>> *const group = queue->pollLast();
        // System.out.println("PRUNE: " + group);
        group->inQueue = false;
      }
    }

  public:
    virtual std::shared_ptr<std::deque<SearchGroup<T>>>
    merge(std::deque<std::deque<SearchGroup<T>>> &shards, int offset, int topN)
    {

      constexpr int maxQueueSize = offset + topN;

      // System.out.println("merge");
      // Init queue:
      for (int shardIDX = 0; shardIDX < shards.size(); shardIDX++) {
        std::shared_ptr<std::deque<SearchGroup<T>>> *const shard =
            shards[shardIDX];
        if (!shard->isEmpty()) {
          // System.out.println("  insert shard=" + shardIDX);
          updateNextGroup(maxQueueSize,
                          std::make_shared<ShardIter<>>(shard, shardIDX));
        }
      }

      // Pull merged topN groups:
      const std::deque<SearchGroup<T>> newTopGroups =
          std::deque<SearchGroup<T>>(topN);

      int count = 0;

      while (!queue->isEmpty()) {
        std::shared_ptr<MergedGroup<T>> *const group = queue->pollFirst();
        group->processed = true;
        // System.out.println("  pop: shards=" + group.shards + " group=" +
        // (group.groupValue == null ? "null" : (((BytesRef)
        // group.groupValue).utf8ToString())) + " sortValues=" +
        // Arrays.toString(group.topValues));
        if (count++ >= offset) {
          std::shared_ptr<SearchGroup<T>> *const newGroup =
              std::make_shared<SearchGroup<T>>();
          newGroup->groupValue = group->groupValue;
          newGroup->sortValues = group->topValues;
          newTopGroups.push_back(newGroup);
          if (newTopGroups.size() == topN) {
            break;
          }
          //} else {
          // System.out.println("    skip < offset");
        }

        // Advance all iters in this group:
        for (auto shardIter : group->shards) {
          updateNextGroup(maxQueueSize, shardIter);
        }
      }

      if (newTopGroups.empty()) {
        return nullptr;
      } else {
        return newTopGroups;
      }
    }
  };

  /** Merges multiple collections of top groups, for example
   *  obtained from separate index shards.  The provided
   *  groupSort must match how the groups were sorted, and
   *  the provided SearchGroups must have been computed
   *  with fillFields=true passed to {@link
   *  FirstPassGroupingCollector#getTopGroups}.
   *
   * <p>NOTE: this returns null if the topGroups is empty.
   */
public:
  template <typename T>
  static std::shared_ptr<std::deque<SearchGroup<T>>>
  merge(std::deque<std::deque<SearchGroup<T>>> &topGroups, int offset,
        int topN, std::shared_ptr<Sort> groupSort)
  {
    if (topGroups.empty()) {
      return nullptr;
    } else {
      return (std::make_shared<GroupMerger<T>>(groupSort))
          ->merge(topGroups, offset, topN);
    }
  }
};

} // namespace org::apache::lucene::search::grouping
