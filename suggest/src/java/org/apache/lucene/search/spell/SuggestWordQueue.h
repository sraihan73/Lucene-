#pragma once
#include "stringhelper.h"
#include <memory>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spell
{
class SuggestWord;
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
namespace org::apache::lucene::search::spell
{

using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Sorts SuggestWord instances
 *
 * @see org.apache.lucene.search.spell.SuggestWordScoreComparator
 * @see org.apache.lucene.search.spell.SuggestWordFrequencyComparator
 *
 */
class SuggestWordQueue final
    : public PriorityQueue<std::shared_ptr<SuggestWord>>
{
  GET_CLASS_NAME(SuggestWordQueue)
  /**
   * Default comparator: score then frequency.
   * @see SuggestWordScoreComparator
   */
public:
  static const std::shared_ptr<Comparator<std::shared_ptr<SuggestWord>>>
      DEFAULT_COMPARATOR;

private:
  std::shared_ptr<Comparator<std::shared_ptr<SuggestWord>>> comparator;

  /**
   * Use the {@link #DEFAULT_COMPARATOR}
   * @param size The size of the queue
   */
public:
  SuggestWordQueue(int size);

  /**
   * Specify the size of the queue and the comparator to use for sorting.
   * @param size The size
   * @param comparator The comparator.
   */
  SuggestWordQueue(
      int size,
      std::shared_ptr<Comparator<std::shared_ptr<SuggestWord>>> comparator);

protected:
  bool lessThan(std::shared_ptr<SuggestWord> wa,
                std::shared_ptr<SuggestWord> wb) override;

protected:
  std::shared_ptr<SuggestWordQueue> shared_from_this()
  {
    return std::static_pointer_cast<SuggestWordQueue>(
        org.apache.lucene.util.PriorityQueue<SuggestWord>::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spell
