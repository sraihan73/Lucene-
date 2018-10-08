#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/BytesRefHash.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
namespace org::apache::lucene::search::grouping
{
template <typename T>
class SearchGroup;
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
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

/**
 * A GroupSelector implementation that groups via SortedDocValues
 */
class TermGroupSelector : public GroupSelector<std::shared_ptr<BytesRef>>
{
  GET_CLASS_NAME(TermGroupSelector)

private:
  const std::wstring field;
  const std::shared_ptr<BytesRefHash> values = std::make_shared<BytesRefHash>();
  const std::unordered_map<int, int> ordsToGroupIds =
      std::unordered_map<int, int>();

  std::shared_ptr<SortedDocValues> docValues;
  int groupId = 0;

  bool secondPass = false;
  bool includeEmpty = false;

  /**
   * Create a new TermGroupSelector
   * @param field the SortedDocValues field to use for grouping
   */
public:
  TermGroupSelector(const std::wstring &field);

  void setNextReader(std::shared_ptr<LeafReaderContext> readerContext) throw(
      IOException) override;

  State advanceTo(int doc)  override;

private:
  std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();

public:
  std::shared_ptr<BytesRef> currentValue() override;

  std::shared_ptr<BytesRef> copyValue() override;

  void
  setGroups(std::shared_ptr<std::deque<SearchGroup<std::shared_ptr<BytesRef>>>>
                searchGroups) override;

protected:
  std::shared_ptr<TermGroupSelector> shared_from_this()
  {
    return std::static_pointer_cast<TermGroupSelector>(
        GroupSelector<org.apache.lucene.util.BytesRef>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
