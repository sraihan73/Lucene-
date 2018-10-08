#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;

/**
 * SecondPassGroupingCollector runs over an already collected set of
 * groups, further applying a {@link GroupReducer} to each group
 *
 * @see TopGroupsCollector
 * @see DistinctValuesCollector
 *
 * @lucene.experimental
 */
template <typename T>
class SecondPassGroupingCollector : public SimpleCollector
{
  GET_CLASS_NAME(SecondPassGroupingCollector)

protected:
  const std::shared_ptr<GroupSelector<T>> groupSelector;
  const std::shared_ptr<std::deque<SearchGroup<T>>> groups;
  const std::shared_ptr < GroupReducer < T, ? >> groupReducer;

  int totalHitCount = 0;
  int totalGroupedHitCount = 0;

  /**
   * Create a new SecondPassGroupingCollector
   * @param groupSelector   the GroupSelector that defines groups for this
   * search
   * @param groups          the groups to collect documents for
   * @param reducer         the reducer to apply to each group
   */
public:
  template <typename T, typename T1>
  SecondPassGroupingCollector(
      std::shared_ptr<GroupSelector<T>> groupSelector,
      std::shared_ptr<std::deque<SearchGroup<T>>> groups,
      std::shared_ptr<GroupReducer<T1>> reducer)
      : groupSelector(Objects::requireNonNull(groupSelector)),
        groups(Objects::requireNonNull(groups)), groupReducer(reducer)
  {

    // System.out.println("SP init");
    if (groups->isEmpty()) {
      throw std::invalid_argument("no groups to collect (groups is empty)");
    }

    this->groupSelector->setGroups(groups);

    reducer->setGroups(groups);
  }

  /**
   * @return the GroupSelector used in this collector
   */
  virtual std::shared_ptr<GroupSelector<T>> getGroupSelector()
  {
    return groupSelector;
  }

  bool needsScores() override { return groupReducer->needsScores(); }

  void setScorer(std::shared_ptr<Scorer> scorer)  override
  {
    groupReducer->setScorer(scorer);
  }

  void collect(int doc)  override
  {
    totalHitCount++;
    if (groupSelector->advanceTo(doc) == GroupSelector::State::SKIP) {
      return;
    }
    totalGroupedHitCount++;
    T value = groupSelector->currentValue();
    groupReducer->collect(value, doc);
  }

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> readerContext) throw(
      IOException) override
  {
    groupReducer->setNextReader(readerContext);
    groupSelector->setNextReader(readerContext);
  }

protected:
  std::shared_ptr<SecondPassGroupingCollector> shared_from_this()
  {
    return std::static_pointer_cast<SecondPassGroupingCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
