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

/**
 * Expert: representation of a group in {@link FirstPassGroupingCollector},
 * tracking the top doc and {@link FieldComparator} slot.
 * @lucene.internal */
template <typename T>
class CollectedSearchGroup : public SearchGroup<T>
{
  GET_CLASS_NAME(CollectedSearchGroup)
public:
  int topDoc = 0;
  int comparatorSlot = 0;

protected:
  std::shared_ptr<CollectedSearchGroup> shared_from_this()
  {
    return std::static_pointer_cast<CollectedSearchGroup>(
        SearchGroup<T>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/grouping/
