#pragma once
#include "stringhelper.h"
#include <memory>
#include <type_traits>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::grouping
{
template <typename C>
class GroupCollector;
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
using Collector = org::apache::lucene::search::Collector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Scorer = org::apache::lucene::search::Scorer;

/**
 * Concrete implementations of this class define what to collect for individual
 * groups during the second-pass of a grouping search.
 *
 * Each group is assigned a Collector returned by {@link #newCollector()}, and
 * {@link LeafCollector#collect(int)} is called for each document that is in
 * a group
 *
 * @see SecondPassGroupingCollector
 *
 * @param <T> the type of the value used for grouping
 * @param <C> the type of {@link Collector} used to reduce each group
 */
template <typename T, typename C>
class GroupReducer : public std::enable_shared_from_this<GroupReducer>
{
  GET_CLASS_NAME(GroupReducer)
  static_assert(std::is_base_of<org.apache.lucene.search.Collector, C>::value,
                L"C must inherit from org.apache.lucene.search.Collector");

private:
  const std::unordered_map<T, GroupCollector<C>> groups =
      std::unordered_map<T, GroupCollector<C>>();

  /**
   * Define which groups should be reduced.
   *
   * Called by {@link SecondPassGroupingCollector}
   */
public:
  virtual void setGroups(std::shared_ptr<std::deque<SearchGroup<T>>> groups)
  {
    for (auto group : groups) {
      this->groups.emplace(group->groupValue,
                           std::make_shared<GroupCollector<>>(newCollector()));
    }
  }

  /**
   * Whether or not this reducer requires collected documents to be scored
   */
  virtual bool needsScores() = 0;

  /**
   * Creates a new Collector for each group
   */
protected:
  virtual C newCollector() = 0;

  /**
   * Get the Collector for a given group
   */
public:
  C getCollector(T value) { return groups[value]->collector; }

  /**
   * Collect a given document into a given group
   * @throws IOException on error
   */
  void collect(T value, int doc) 
  {
    std::shared_ptr<GroupCollector<C>> collector = groups[value];
    collector->leafCollector->collect(doc);
  }

  /**
   * Set the Scorer on all group collectors
   */
  void setScorer(std::shared_ptr<Scorer> scorer) 
  {
    for (auto collector : groups) {
      collector->second.leafCollector.setScorer(scorer);
    }
  }

  /**
   * Called when the parent {@link SecondPassGroupingCollector} moves to a new
   * segment
   */
  void setNextReader(std::shared_ptr<LeafReaderContext> ctx) 
  {
    for (auto collector : groups) {
      collector->second->leafCollector =
          collector->second.collector.getLeafCollector(ctx);
    }
  }

private:
  template <typename C>
  class GroupCollector final
      : public std::enable_shared_from_this<GroupCollector>
  {
    GET_CLASS_NAME(GroupCollector)
    static_assert(std::is_base_of<org.apache.lucene.search.Collector, C>::value,
                  L"C must inherit from org.apache.lucene.search.Collector");

  public:
    const C collector;
    std::shared_ptr<LeafCollector> leafCollector;

  private:
    GroupCollector(C collector) : collector(collector) {}
  };
};

} // namespace org::apache::lucene::search::grouping
