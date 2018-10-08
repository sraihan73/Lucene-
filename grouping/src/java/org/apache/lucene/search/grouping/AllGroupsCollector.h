#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_set>

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
 * A collector that collects all groups that match the
 * query. Only the group value is collected, and the order
 * is undefined.  This collector does not determine
 * the most relevant document of a group.
 *
 * @lucene.experimental
 */
template <typename T>
class AllGroupsCollector : public SimpleCollector
{
  GET_CLASS_NAME(AllGroupsCollector)

private:
  const std::shared_ptr<GroupSelector<T>> groupSelector;

  const std::shared_ptr<Set<T>> groups = std::unordered_set<T>();

  /**
   * Create a new AllGroupsCollector
   * @param groupSelector the GroupSelector to determine groups
   */
public:
  AllGroupsCollector(std::shared_ptr<GroupSelector<T>> groupSelector)
      : groupSelector(groupSelector)
  {
  }

  /**
   * Returns the total number of groups for the executed search.
   * This is a convenience method. The following code snippet has the same
   * effect: <pre>getGroups().size()</pre>
   *
   * @return The total number of groups for the executed search
   */
  virtual int getGroupCount() { return getGroups()->size(); }

  /**
   * Returns the group values
   * <p>
   * This is an unordered collections of group values.
   *
   * @return the group values
   */
  virtual std::shared_ptr<std::deque<T>> getGroups() { return groups; }

  void setScorer(std::shared_ptr<Scorer> scorer)  override {}

protected:
  void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
      IOException) override
  {
    groupSelector->setNextReader(context);
  }

public:
  void collect(int doc)  override
  {
    groupSelector->advanceTo(doc);
    if (groups->contains(groupSelector->currentValue())) {
      return;
    }
    groups->add(groupSelector->copyValue());
  }

  bool needsScores() override
  {
    return false; // the result is unaffected by relevancy
  }

protected:
  std::shared_ptr<AllGroupsCollector> shared_from_this()
  {
    return std::static_pointer_cast<AllGroupsCollector>(
        org.apache.lucene.search.SimpleCollector::shared_from_this());
  }
};
} // namespace org::apache::lucene::search::grouping
