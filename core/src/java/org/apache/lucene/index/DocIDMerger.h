#pragma once
#include "stringhelper.h"
#include <memory>
#include <type_traits>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MergeState;
}

namespace org::apache::lucene::index
{
class DocMap;
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

namespace org::apache::lucene::index
{

using PriorityQueue = org::apache::lucene::util::PriorityQueue;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/** Utility class to help merging documents from sub-readers according to either
 * simple concatenated (unsorted) order, or by a specified index-time sort,
 * skipping deleted documents and remapping non-deleted documents. */

template <typename T>
class DocIDMerger : public std::enable_shared_from_this<DocIDMerger>
{
  GET_CLASS_NAME(DocIDMerger)
  static_assert(std::is_base_of<DocIDMerger.Sub, T>::value,
                L"T must inherit from DocIDMerger.Sub");

  /** Represents one sub-reader being merged */
public:
  class Sub : public std::enable_shared_from_this<Sub>
  {
    GET_CLASS_NAME(Sub)
    /** Mapped doc ID */
  public:
    int mappedDocID = 0;

    const std::shared_ptr<MergeState::DocMap> docMap;

    /** Sole constructor */
    Sub(std::shared_ptr<MergeState::DocMap> docMap);

    /** Returns the next document ID from this sub reader, and {@link
     * DocIdSetIterator#NO_MORE_DOCS} when done */
    virtual int nextDoc() = 0;
  };

  /** Construct this from the provided subs, specifying the maximum sub count */
public:
  template <typename T>
  static std::shared_ptr<DocIDMerger<T>>
  of(std::deque<T> &subs, int maxCount, bool indexIsSorted) 
  {
    static_assert(std::is_base_of<DocIDMerger.Sub, T>::value,
                  L"T must inherit from DocIDMerger.Sub");

    if (indexIsSorted && maxCount > 1) {
      return std::make_shared<SortedDocIDMerger<>>(subs, maxCount);
    } else {
      return std::make_shared<SequentialDocIDMerger<>>(subs);
    }
  }

  /** Construct this from the provided subs */
  template <typename T>
  static std::shared_ptr<DocIDMerger<T>>
  of(std::deque<T> &subs, bool indexIsSorted) 
  {
    static_assert(std::is_base_of<DocIDMerger.Sub, T>::value,
                  L"T must inherit from DocIDMerger.Sub");

    return of(subs, subs.size(), indexIsSorted);
  }

  /** Reuse API, currently only used by postings during merge */
  virtual void reset() = 0;

  /** Returns null when done.
   *  <b>NOTE:</b> after the iterator has exhausted you should not call this
   *  method, as it may result in unpredicted behavior. */
  virtual T next() = 0;

private:
  DocIDMerger() {}

private:
  template <typename T>
  class SequentialDocIDMerger : public DocIDMerger<T>
  {
    GET_CLASS_NAME(SequentialDocIDMerger)
    static_assert(std::is_base_of<DocIDMerger.Sub, T>::value,
                  L"T must inherit from DocIDMerger.Sub");

  private:
    const std::deque<T> subs;
    T current;
    int nextIndex = 0;

    SequentialDocIDMerger(std::deque<T> &subs)  : subs(subs)
    {
      reset();
    }

  public:
    void reset()  override
    {
      if (subs.size() > 0) {
        current = subs[0];
        nextIndex = 1;
      } else {
        current.reset();
        nextIndex = 0;
      }
    }

    T next()  override
    {
      while (true) {
        int docID = current->nextDoc();
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          if (nextIndex == subs.size()) {
            current.reset();
            return nullptr;
          }
          current = subs[nextIndex];
          nextIndex++;
          continue;
        }

        int mappedDocID = current->docMap->get(docID);
        if (mappedDocID != -1) {
          current->mappedDocID = mappedDocID;
          return current;
        }
      }
    }

  protected:
    std::shared_ptr<SequentialDocIDMerger> shared_from_this()
    {
      return std::static_pointer_cast<SequentialDocIDMerger>(
          DocIDMerger<T>::shared_from_this());
    }
  };

private:
  template <typename T>
  class SortedDocIDMerger : public DocIDMerger<T>
  {
    GET_CLASS_NAME(SortedDocIDMerger)
    static_assert(std::is_base_of<DocIDMerger.Sub, T>::value,
                  L"T must inherit from DocIDMerger.Sub");

  private:
    const std::deque<T> subs;
    const std::shared_ptr<PriorityQueue<T>> queue;

    SortedDocIDMerger(std::deque<T> &subs, int maxCount) 
        : subs(subs), queue(std::make_shared<PriorityQueue<T>>(maxCount))
    {
      {protected :
           bool lessThan(Sub a, Sub b){assert(a::mappedDocID != b::mappedDocID);
      return a::mappedDocID < b::mappedDocID;
    }
  };
  reset();
}

public : void
         reset()  override
{
  // caller may not have fully consumed the queue:
  queue->clear();
  bool first = true;
  for (auto sub : subs) {
    if (first) {
      // by setting mappedDocID = -1, this entry is guaranteed to be the top of
      // the queue so the first call to next() will advance it
      sub->mappedDocID = -1;
      first = false;
    } else {
      int mappedDocID;
      while (true) {
        int docID = sub->nextDoc();
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          mappedDocID = DocIdSetIterator::NO_MORE_DOCS;
          break;
        }
        mappedDocID = sub->docMap->get(docID);
        if (mappedDocID != -1) {
          break;
        }
      }
      if (mappedDocID == DocIdSetIterator::NO_MORE_DOCS) {
        // all docs in this sub were deleted; do not add it to the queue!
        continue;
      }
      sub->mappedDocID = mappedDocID;
    }
    queue->push_back(sub);
  }
}

T next()  override
{
  T top = queue->top();

  while (true) {
    int docID = top->nextDoc();
    if (docID == DocIdSetIterator::NO_MORE_DOCS) {
      queue->pop();
      top = queue->top();
      break;
    }
    int mappedDocID = top->docMap->get(docID);
    if (mappedDocID == -1) {
      // doc was deleted
      continue;
    } else {
      top->mappedDocID = mappedDocID;
      top = queue->updateTop();
      break;
    }
  }

  return top;
}

protected:
std::shared_ptr<SortedDocIDMerger> shared_from_this()
{
  return std::static_pointer_cast<SortedDocIDMerger>(
      DocIDMerger<T>::shared_from_this());
}
}; // namespace org::apache::lucene::index
}
;
}
