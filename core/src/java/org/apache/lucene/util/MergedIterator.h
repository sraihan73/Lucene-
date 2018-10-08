#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <type_traits>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
template <typename I>
class SubIterator;
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
namespace org::apache::lucene::util
{

using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Provides a merged sorted view from several sorted iterators.
 * <p>
 * If built with <code>removeDuplicates</code> set to true and an element
 * appears in multiple iterators then it is deduplicated, that is this iterator
 * returns the sorted union of elements.
 * <p>
 * If built with <code>removeDuplicates</code> set to false then all elements
 * in all iterators are returned.
 * <p>
 * Caveats:
 * <ul>
 *   <li>The behavior is undefined if the iterators are not actually sorted.
 *   <li>Null elements are unsupported.
 *   <li>If removeDuplicates is set to true and if a single iterator contains
 *       duplicates then they will not be deduplicated.
 *   <li>When elements are deduplicated it is not defined which one is returned.
 *   <li>If removeDuplicates is set to false then the order in which duplicates
 *       are returned isn't defined.
 * </ul>
 * @lucene.internal
 */
template <typename T>
class MergedIterator final
    : public std::enable_shared_from_this<MergedIterator>,
      public Iterator<T>
{
  GET_CLASS_NAME(MergedIterator)
  static_assert(std::is_base_of<Comparable<T>, T>::value,
                L"T must inherit from Comparable<T>");

private:
  T current;
  const std::shared_ptr<TermMergeQueue<T>> queue;
  std::deque<SubIterator<T>> const top;
  const bool removeDuplicates;
  int numTop = 0;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) public
  // MergedIterator(java.util.Iterator<T>... iterators)
  MergedIterator(std::deque<Iterator<T>> &iterators)
      : MergedIterator(true, iterators)
  {
  }

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) public
  // MergedIterator(bool removeDuplicates, java.util.Iterator<T>... iterators)
  MergedIterator(bool removeDuplicates, std::deque<Iterator<T>> &iterators)
      : queue(std::make_shared<TermMergeQueue<T>>(iterators->length)),
        top(std::deque<std::shared_ptr<SubIterator>>(iterators->length)),
        removeDuplicates(removeDuplicates)
  {
    int index = 0;
    for (std::shared_ptr<Iterator<T>> iterator : iterators) {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      if (iterator->hasNext()) {
        std::shared_ptr<SubIterator<T>> sub =
            std::make_shared<SubIterator<T>>();
        sub->current = *iterator;
        sub->iterator = iterator;
        sub->index = index++;
        queue->add(sub);
      }
    }
  }

  bool hasNext() override
  {
    if (queue->size() > 0) {
      return true;
    }

    for (int i = 0; i < numTop; i++) {
      if (top[i]->iterator->hasNext()) {
        return true;
      }
    }
    return false;
  }

  T next() override
  {
    // restore queue
    pushTop();

    // gather equal top elements
    if (queue->size() > 0) {
      pullTop();
    } else {
      current = nullptr;
    }
    if (current == nullptr) {
      // C++ TODO: The following line could not be converted:
      throw java.util.NoSuchElementException();
    }
    return current;
  }

  void remove() override
  {
    throw std::make_shared<UnsupportedOperationException>();
  }

private:
  void pullTop()
  {
    assert(numTop == 0);
    top[numTop++] = queue->pop();
    if (removeDuplicates) {
      // extract all subs from the queue that have the same top element
      while (queue->size() != 0 &&
             queue->top().current.equals(top[0]->current)) {
        top[numTop++] = queue->pop();
      }
    }
    current = top[0]->current;
  }

  void pushTop()
  {
    // call next() on each top, and put back into queue
    for (int i = 0; i < numTop; i++) {
      if (top[i]->iterator->hasNext()) {
        top[i]->current = top[i]->iterator->next();
        queue->add(top[i]);
      } else {
        // no more elements
        top[i]->current = nullptr;
      }
    }
    numTop = 0;
  }

private:
  template <typename I>
  class SubIterator : public std::enable_shared_from_this<SubIterator>
  {
    GET_CLASS_NAME(SubIterator)
    static_assert(std::is_base_of<Comparable<I>, I>::value,
                  L"I must inherit from Comparable<I>");

  public:
    std::shared_ptr<Iterator<I>> iterator;
    I current;
    int index = 0;
  };

private:
  template <typename C>
  class TermMergeQueue : public PriorityQueue<SubIterator<C>>
  {
    GET_CLASS_NAME(TermMergeQueue)
    static_assert(std::is_base_of<Comparable<C>, C>::value,
                  L"C must inherit from Comparable<C>");

  public:
    TermMergeQueue(int size) : PriorityQueue<SubIterator<C>>(size) {}

  protected:
    bool lessThan(std::shared_ptr<SubIterator<C>> a,
                  std::shared_ptr<SubIterator<C>> b) override
    {
      constexpr int cmp = a->current.compareTo(b->current);
      if (cmp != 0) {
        return cmp < 0;
      } else {
        return a->index < b->index;
      }
    }

  protected:
    std::shared_ptr<TermMergeQueue> shared_from_this()
    {
      return std::static_pointer_cast<TermMergeQueue>(
          org.apache.lucene.util
              .PriorityQueue<SubIterator<C>>::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::util
