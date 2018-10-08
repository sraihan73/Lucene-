#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <type_traits>
#include <deque>
#include <queue>
#include <core/src/java/org/apache/lucene/search/Collector.h>
#include <core/src/java/org/apache/lucene/search/TopDocs.h>

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

//using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * A base class for all collectors that return a {@link TopDocs} output. This
 * collector allows easy extension by providing a single constructor which
 * accepts a {@link PriorityQueue} as well as protected members for that
 * priority queue and a counter of the number of total hits.<br>
 * Extending classes can override any of the methods to provide their own
 * implementation, as well as avoid the use of the priority queue entirely by
 * passing null to {@link #TopDocsCollector(PriorityQueue)}. In that case
 * however, you might want to consider overriding all methods, in order to avoid
 * a NullPointerException.
 */
template <typename T>
class TopDocsCollector : public std::enable_shared_from_this<TopDocsCollector<T>>,
                         public Collector
{
  GET_CLASS_NAME(TopDocsCollector)
  static_assert(std::is_base_of<ScoreDoc, T>::value,
                L"T must inherit from ScoreDoc");

  /** This is used in case topDocs() is called with illegal parameters, or there
   *  simply aren't (enough) results. */
protected:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<TopDocs> EMPTY_TOPDOCS =
      std::make_shared<TopDocs>(0, std::deque<std::shared_ptr<ScoreDoc>>(0),
                                NAN);

  /**
   * The priority queue which holds the top documents. Note that different
   * implementations of PriorityQueue give different meaning to 'top documents'.
   * HitQueue for example aggregates the top scoring documents, while other PQ
   * implementations may hold documents sorted by other criteria.
   */
  std::shared_ptr<PriorityQueue<T>> pq;

  /** The total number of documents that the collector encountered. */
  int totalHits = 0;

  TopDocsCollector(std::shared_ptr<PriorityQueue<T>> pq) { this->pq = pq; }

  /**
   * Populates the results array with the ScoreDoc instances. This can be
   * overridden in case a different ScoreDoc type should be returned.
   */
  virtual void populateResults(std::deque<std::shared_ptr<ScoreDoc>> &results,
                               int howMany)
  {
    for (int i = howMany - 1; i >= 0; i--) {
      results[i] = pq->pop();
    }
  }

  /**
   * Returns a {@link TopDocs} instance containing the given results. If
   * <code>results</code> is null it means there are no results to return,
   * either because there were 0 calls to collect() or because the arguments to
   * topDocs were invalid.
   */
  virtual std::shared_ptr<TopDocs>
  newTopDocs(std::deque<std::shared_ptr<ScoreDoc>> &results, int start)
  {
    return results.empty() ? EMPTY_TOPDOCS
                           : std::make_shared<TopDocs>(totalHits, results);
  }

  /** The total number of documents that matched this query. */
public:
  virtual int getTotalHits() { return totalHits; }

  /** The number of valid PQ entries */
protected:
  virtual int topDocsSize()
  {
    // In case pq was populated with sentinel values, there might be less
    // results than pq.size(). Therefore return all results until either
    // pq.size() or totalHits.
    return totalHits < pq->size() ? totalHits : pq->size();
  }

  /** Returns the top docs that were collected by this collector. */
public:
  virtual std::shared_ptr<TopDocs> topDocs()
  {
    // In case pq was populated with sentinel values, there might be less
    // results than pq.size(). Therefore return all results until either
    // pq.size() or totalHits.
    return topDocs(0, topDocsSize());
  }

  /**
   * Returns the documents in the range [start .. pq.size()) that were collected
   * by this collector. Note that if {@code start >= pq.size()}, an empty
   * TopDocs is returned.<br> This method is convenient to call if the
   * application always asks for the last results, starting from the last
   * 'page'.<br> <b>NOTE:</b> you cannot call this method more than once for
   * each search execution. If you need to call it more than once, passing each
   * time a different <code>start</code>, you should call {@link #topDocs()} and
   * work with the returned {@link TopDocs} object, which will contain all the
   * results this search execution collected.
   */
  virtual std::shared_ptr<TopDocs> topDocs(int start)
  {
    // In case pq was populated with sentinel values, there might be less
    // results than pq.size(). Therefore return all results until either
    // pq.size() or totalHits.
    return topDocs(start, topDocsSize());
  }

  /**
   * Returns the documents in the range [start .. start+howMany) that were
   * collected by this collector. Note that if {@code start >= pq.size()}, an
   * empty TopDocs is returned, and if pq.size() - start &lt; howMany, then only
   * the available documents in [start .. pq.size()) are returned.<br> This
   * method is useful to call in case pagination of search results is allowed by
   * the search application, as well as it attempts to optimize the memory used
   * by allocating only as much as requested by howMany.<br> <b>NOTE:</b> you
   * cannot call this method more than once for each search execution. If you
   * need to call it more than once, passing each time a different range, you
   * should call {@link #topDocs()} and work with the returned {@link TopDocs}
   * object, which will contain all the results this search execution collected.
   */
  virtual std::shared_ptr<TopDocs> topDocs(int start, int howMany)
  {

    // In case pq was populated with sentinel values, there might be less
    // results than pq.size(). Therefore return all results until either
    // pq.size() or totalHits.
    int size = topDocsSize();

    // Don't bother to throw an exception, just return an empty TopDocs in case
    // the parameters are invalid or out of range.
    // TODO: shouldn't we throw IAE if apps give bad params here so they dont
    // have sneaky silent bugs?
    if (start < 0 || start >= size || howMany <= 0) {
      return newTopDocs(nullptr, start);
    }

    // We know that start < pqsize, so just fix howMany.
    howMany = std::min(size - start, howMany);
    std::deque<std::shared_ptr<ScoreDoc>> results(howMany);

    // pq's pop() returns the 'least' element in the queue, therefore need
    // to discard the first ones, until we reach the requested range.
    // Note that this loop will usually not be executed, since the common usage
    // should be that the caller asks for the last howMany results. However it's
    // needed here for completeness.
    for (int i = pq->size() - start - howMany; i > 0; i--) {
      pq->pop();
    }

    // Get the requested results from pq.
    populateResults(results, howMany);

    return newTopDocs(results, start);
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
