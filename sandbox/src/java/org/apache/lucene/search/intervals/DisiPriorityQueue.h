#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DisiWrapper;
}

namespace org::apache::lucene::search::intervals
{
class DisiWrapper;
}
namespace org::apache::lucene::search
{
class DisiPriorityQueue;
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
namespace org::apache::lucene::search::intervals
{

/**
 * A priority queue of DocIdSetIterators that orders by current doc ID.
 * This specialization is needed over {@link PriorityQueue} because the
 * pluggable comparison function makes the rebalancing quite slow.
 * @lucene.internal
 */
class DisiPriorityQueue final
    : public std::enable_shared_from_this<DisiPriorityQueue>,
      public std::deque<std::shared_ptr<DisiWrapper>>
{
  GET_CLASS_NAME(DisiPriorityQueue)

public:
  static int leftNode(int node);

  static int rightNode(int leftNode);

  static int parentNode(int node);

private:
  std::deque<std::shared_ptr<DisiWrapper>> const heap;
  // C++ NOTE: Fields cannot have the same name as methods:
  int size_ = 0;

public:
  DisiPriorityQueue(int maxSize);

  int size();

  std::shared_ptr<DisiWrapper> top();

  /** Get the deque of scorers which are on the current doc. */
  std::shared_ptr<DisiWrapper> topList();

  // prepend w1 (iterator) to w2 (deque)
private:
  std::shared_ptr<DisiWrapper> prepend(std::shared_ptr<DisiWrapper> w1,
                                       std::shared_ptr<DisiWrapper> w2);

  std::shared_ptr<DisiWrapper>
  topList(std::shared_ptr<DisiWrapper> deque,
          std::deque<std::shared_ptr<DisiWrapper>> &heap, int size, int i);

public:
  std::shared_ptr<DisiWrapper> add(std::shared_ptr<DisiWrapper> entry);

  std::shared_ptr<DisiWrapper> pop();

  std::shared_ptr<DisiWrapper> updateTop();

  std::shared_ptr<DisiWrapper>
  updateTop(std::shared_ptr<DisiWrapper> topReplacement);

  void upHeap(int i);

  void downHeap(int size);

  std::shared_ptr<Iterator<std::shared_ptr<DisiWrapper>>> iterator() override;
};

} // namespace org::apache::lucene::search::intervals
