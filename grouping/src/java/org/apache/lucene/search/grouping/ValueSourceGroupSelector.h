#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::@ mutable { class MutableValue; }

#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"
#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/queries/function/ValueFiller.h"
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
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;

/**
 * A GroupSelector that groups via a ValueSource
 */
class ValueSourceGroupSelector
    : public GroupSelector<std::shared_ptr<MutableValue>>
{
  GET_CLASS_NAME(ValueSourceGroupSelector)

private:
  const std::shared_ptr<ValueSource> valueSource;
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: private final java.util.Map<?, ?> context;
  const std::unordered_map < ?, ? > context;

  std::shared_ptr<Set<std::shared_ptr<MutableValue>>> secondPassGroups;

  /**
   * Create a new ValueSourceGroupSelector
   * @param valueSource the ValueSource to group by
   * @param context     a context map_obj for the ValueSource
   */
public:
  template <typename T1, typename T1>
  ValueSourceGroupSelector(std::shared_ptr<ValueSource> valueSource,
                           std::unordered_map<T1> context);

private:
  std::shared_ptr<FunctionValues::ValueFiller> filler;

public:
  void setNextReader(std::shared_ptr<LeafReaderContext> readerContext) throw(
      IOException) override;

  State advanceTo(int doc)  override;

  std::shared_ptr<MutableValue> currentValue() override;

  std::shared_ptr<MutableValue> copyValue() override;

  void setGroups(
      std::shared_ptr<std::deque<SearchGroup<std::shared_ptr<MutableValue>>>>
          searchGroups) override;

protected:
  std::shared_ptr<ValueSourceGroupSelector> shared_from_this()
  {
    return std::static_pointer_cast<ValueSourceGroupSelector>(
        GroupSelector<
            org.apache.lucene.util.mutable_.MutableValue>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
