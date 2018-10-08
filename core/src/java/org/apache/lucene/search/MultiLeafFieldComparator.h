#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class LeafFieldComparator;
}

namespace org::apache::lucene::search
{
class Scorer;
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
namespace org::apache::lucene::search
{

class MultiLeafFieldComparator final
    : public std::enable_shared_from_this<MultiLeafFieldComparator>,
      public LeafFieldComparator
{
  GET_CLASS_NAME(MultiLeafFieldComparator)

private:
  std::deque<std::shared_ptr<LeafFieldComparator>> const comparators;
  std::deque<int> const reverseMul;
  // we extract the first comparator to avoid array access in the common case
  // that the first comparator compares worse than the bottom entry in the queue
  const std::shared_ptr<LeafFieldComparator> firstComparator;
  const int firstReverseMul;

public:
  MultiLeafFieldComparator(
      std::deque<std::shared_ptr<LeafFieldComparator>> &comparators,
      std::deque<int> &reverseMul);

  void setBottom(int slot)  override;

  int compareBottom(int doc)  override;

  int compareTop(int doc)  override;

  void copy(int slot, int doc)  override;

  void setScorer(std::shared_ptr<Scorer> scorer)  override;
};

} // namespace org::apache::lucene::search
