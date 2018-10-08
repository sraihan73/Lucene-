#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::grouping
{
template <typename Ttypename R>
class GroupCount;
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
using SimpleCollector = org::apache::lucene::search::SimpleCollector;

/**
 * A second pass grouping collector that keeps track of distinct values for a
 * specified field for the top N group.
 *
 * @lucene.experimental
 */
template <typename T, typename R>
class DistinctValuesCollector : public SecondPassGroupingCollector<T>
{
  GET_CLASS_NAME(DistinctValuesCollector)

  /**
   * Create a DistinctValuesCollector
   * @param groupSelector the group selector to determine the top-level groups
   * @param groups        the top-level groups to collect for
   * @param valueSelector a group selector to determine which values to collect
   * per-group
   */
public:
  DistinctValuesCollector(std::shared_ptr<GroupSelector<T>> groupSelector,
                          std::shared_ptr<std::deque<SearchGroup<T>>> groups,
                          std::shared_ptr<GroupSelector<R>> valueSelector)
      : SecondPassGroupingCollector<T>(
            groupSelector, groups, new DistinctValuesReducer<>(valueSelector))
  {
  }

private:
  template <typename R>
  class ValuesCollector : public SimpleCollector
  {
    GET_CLASS_NAME(ValuesCollector)

  public:
    const std::shared_ptr<GroupSelector<R>> valueSelector;
    const std::shared_ptr<Set<R>> values = std::unordered_set<R>();

  private:
    ValuesCollector(std::shared_ptr<GroupSelector<R>> valueSelector)
        : valueSelector(valueSelector)
    {
    }

  public:
    void collect(int doc)  override
    {
      if (valueSelector->advanceTo(doc) == GroupSelector::State::ACCEPT) {
        R value = valueSelector->currentValue();
        if (values->contains(value) == false) {
          values->add(valueSelector->copyValue());
        }
      } else {
        if (values->contains(nullptr) == false) {
          values->add(nullptr);
        }
      }
    }

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override
    {
      valueSelector->setNextReader(context);
    }

  public:
    bool needsScores() override { return false; }

  protected:
    std::shared_ptr<ValuesCollector> shared_from_this()
    {
      return std::static_pointer_cast<ValuesCollector>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

private:
  template <typename T, typename R>
  class DistinctValuesReducer : public GroupReducer<T, ValuesCollector<R>>
  {
    GET_CLASS_NAME(DistinctValuesReducer)

  public:
    const std::shared_ptr<GroupSelector<R>> valueSelector;

  private:
    DistinctValuesReducer(std::shared_ptr<GroupSelector<R>> valueSelector)
        : valueSelector(valueSelector)
    {
    }

  public:
    bool needsScores() override { return false; }

  protected:
    std::shared_ptr<ValuesCollector<R>> newCollector() override
    {
      return std::make_shared<ValuesCollector<R>>(valueSelector);
    }

  protected:
    std::shared_ptr<DistinctValuesReducer> shared_from_this()
    {
      return std::static_pointer_cast<DistinctValuesReducer>(
          GroupReducer<T, ValuesCollector<R>>::shared_from_this());
    }
  };

  /**
   * Returns all unique values for each top N group.
   *
   * @return all unique values for each top N group
   */
public:
  virtual std::deque<GroupCount<T, R>> getGroups()
  {
    std::deque<GroupCount<T, R>> counts;
    for (auto group : groups) {
      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings("unchecked") ValuesCollector<R> vc =
      // (ValuesCollector<R>) groupReducer.getCollector(group.groupValue);
      std::shared_ptr<ValuesCollector<R>> vc =
          std::static_pointer_cast<ValuesCollector<R>>(
              groupReducer->getCollector(group->groupValue));
      counts.push_back(
          std::make_shared<GroupCount<>>(group->groupValue, vc->values));
    }
    return counts;
  }

  /**
   * Returned by {@link DistinctValuesCollector#getGroups()},
   * representing the value and set of distinct values for the group.
   */
public:
  template <typename T, typename R>
  class GroupCount : public std::enable_shared_from_this<GroupCount>
  {
    GET_CLASS_NAME(GroupCount)

  public:
    const T groupValue;
    const std::shared_ptr<Set<R>> uniqueValues;

    GroupCount(T groupValue, std::shared_ptr<Set<R>> values)
        : groupValue(groupValue), uniqueValues(values)
    {
    }
  };

protected:
  std::shared_ptr<DistinctValuesCollector> shared_from_this()
  {
    return std::static_pointer_cast<DistinctValuesCollector>(
        SecondPassGroupingCollector<T>::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::grouping
