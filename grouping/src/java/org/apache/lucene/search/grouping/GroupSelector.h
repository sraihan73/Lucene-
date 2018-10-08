#pragma once
#include "stringhelper.h"
#include <memory>

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

/**
 * Defines a group, for use by grouping collectors
 *
 * A GroupSelector acts as an iterator over documents.  For each segment,
 * clients should call {@link #setNextReader(LeafReaderContext)}, and then
 * {@link #advanceTo(int)} for each matching document.
 *
 * @param <T> the type of the group value
 */
template <typename T>
class GroupSelector : public std::enable_shared_from_this<GroupSelector>
{
  GET_CLASS_NAME(GroupSelector)

  /**
   * What to do with the current value
   */
public:
  enum class State { GET_CLASS_NAME(State) SKIP, ACCEPT };

  /**
   * Set the LeafReaderContext
   */
public:
  virtual void
  setNextReader(std::shared_ptr<LeafReaderContext> readerContext) = 0;

  /**
   * Advance the GroupSelector's iterator to the given document
   */
  virtual State advanceTo(int doc) = 0;

  /**
   * Get the group value of the current document
   *
   * N.B. this object may be reused, for a persistent version use {@link
   * #copyValue()}
   */
  virtual T currentValue() = 0;

  /**
   * @return a copy of the group value of the current document
   */
  virtual T copyValue() = 0;

  /**
   * Set a restriction on the group values returned by this selector
   *
   * If the selector is positioned on a document whose group value is not
   * contained within this set, then {@link #advanceTo(int)} will return {@link
   * State#SKIP}
   *
   * @param groups a set of {@link SearchGroup} objects to limit selections to
   */
  virtual void
  setGroups(std::shared_ptr<std::deque<SearchGroup<T>>> groups) = 0;
};

} // namespace org::apache::lucene::search::grouping
