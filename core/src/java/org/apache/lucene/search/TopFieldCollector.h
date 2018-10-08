#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>
#include <queue>

#include  "core/src/java/org/apache/lucene/search/FieldValueHitQueue.h"
//#include  "core/src/java/org/apache/lucene/search/Entry.h"
#include  "core/src/java/org/apache/lucene/search/LeafFieldComparator.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/search/FieldDoc.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include  "core/src/java/org/apache/lucene/util/PriorityQueue.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/search/TopFieldDocs.h"

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Entry = org::apache::lucene::search::FieldValueHitQueue::Entry;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * A {@link Collector} that sorts by {@link SortField} using
 * {@link FieldComparator}s.
 * <p>
 * See the {@link #create(org.apache.lucene.search.Sort, int, bool, bool,
 * bool, bool)} method for instantiating a TopFieldCollector.
 *
 * @lucene.experimental
 */
class TopFieldCollector : public TopDocsCollector<std::shared_ptr<Entry>>
{
  GET_CLASS_NAME(TopFieldCollector)

  // TODO: one optimization we could do is to pre-fill
  // the queue with sentinel value that guaranteed to
  // always compare lower than a real hit; this would
  // save having to check queueFull on each insert

private:
  class MultiComparatorLeafCollector
      : public std::enable_shared_from_this<MultiComparatorLeafCollector>,
        public LeafCollector
  {
    GET_CLASS_NAME(MultiComparatorLeafCollector)

  public:
    const std::shared_ptr<LeafFieldComparator> comparator;
    const int reverseMul;
    const bool mayNeedScoresTwice;
    std::shared_ptr<Scorer> scorer;

    MultiComparatorLeafCollector(
        std::deque<std::shared_ptr<LeafFieldComparator>> &comparators,
        std::deque<int> &reverseMul, bool mayNeedScoresTwice);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
  };

public:
  static bool canEarlyTerminate(std::shared_ptr<Sort> searchSort,
                                std::shared_ptr<Sort> indexSort);

  static int estimateRemainingHits(int hitCount, int doc, int maxDoc);

  /*
   * Implements a TopFieldCollector over one SortField criteria, with tracking
   * document scores and maxScore.
   */
private:
  class SimpleFieldCollector;

  /*
   * Implements a TopFieldCollector when after != null.
   */
private:
  class PagingFieldCollector;

private:
  static std::deque<std::shared_ptr<ScoreDoc>> const EMPTY_SCOREDOCS;

  const bool fillFields;

  /*
   * Stores the maximum score value encountered, needed for normalizing. If
   * document scores are not tracked, this value is initialized to NaN.
   */
public:
  float maxScore = NAN;

  const int numHits;
  std::shared_ptr<FieldValueHitQueue::Entry> bottom = nullptr;
  bool queueFull = false;
  int docBase = 0;
  bool earlyTerminated = false;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool needsScores_;

  // Declaring the constructor private prevents extending this class by anyone
  // else. Note that the class cannot be final since it's extended by the
  // internal versions. If someone will define a constructor with any other
  // visibility, then anyone will be able to extend the class, which is not what
  // we want.
private:
  TopFieldCollector(std::shared_ptr<PriorityQueue<std::shared_ptr<Entry>>> pq,
                    int numHits, bool fillFields, bool needsScores);

public:
  bool needsScores() override;

  /**
   * Creates a new {@link TopFieldCollector} from the given
   * arguments.
   *
   * <p><b>NOTE</b>: The instances returned by this method
   * pre-allocate a full array of length
   * <code>numHits</code>.
   *
   * @param sort
   *          the sort criteria (SortFields).
   * @param numHits
   *          the number of results to collect.
   * @param fillFields
   *          specifies whether the actual field values should be returned on
   *          the results (FieldDoc).
   * @param trackDocScores
   *          specifies whether document scores should be tracked and set on the
   *          results. Note that if set to false, then the results' scores will
   *          be set to Float.NaN. Setting this to true affects performance, as
   *          it incurs the score computation on each competitive result.
   *          Therefore if document scores are not required by the application,
   *          it is recommended to set it to false.
   * @param trackMaxScore
   *          specifies whether the query's maxScore should be tracked and set
   *          on the resulting {@link TopDocs}. Note that if set to false,
   *          {@link TopDocs#getMaxScore()} returns Float.NaN. Setting this to
   *          true affects performance as it incurs the score computation on
   *          each result. Also, setting this true automatically sets
   *          <code>trackDocScores</code> to true as well.
   * @return a {@link TopFieldCollector} instance which will sort the results by
   *         the sort criteria.
   * @deprecated Use {@link #create(Sort, int, bool, bool, bool,
   * bool)}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static TopFieldCollector create(Sort
  // sort, int numHits, bool fillFields, bool trackDocScores, bool
  // trackMaxScore)
  static std::shared_ptr<TopFieldCollector> create(std::shared_ptr<Sort> sort,
                                                   int numHits, bool fillFields,
                                                   bool trackDocScores,
                                                   bool trackMaxScore);

  /**
   * Creates a new {@link TopFieldCollector} from the given
   * arguments.
   *
   * <p><b>NOTE</b>: The instances returned by this method
   * pre-allocate a full array of length
   * <code>numHits</code>.
   *
   * @param sort
   *          the sort criteria (SortFields).
   * @param numHits
   *          the number of results to collect.
   * @param fillFields
   *          specifies whether the actual field values should be returned on
   *          the results (FieldDoc).
   * @param trackDocScores
   *          specifies whether document scores should be tracked and set on the
   *          results. Note that if set to false, then the results' scores will
   *          be set to Float.NaN. Setting this to true affects performance, as
   *          it incurs the score computation on each competitive result.
   *          Therefore if document scores are not required by the application,
   *          it is recommended to set it to false.
   * @param trackMaxScore
   *          specifies whether the query's maxScore should be tracked and set
   *          on the resulting {@link TopDocs}. Note that if set to false,
   *          {@link TopDocs#getMaxScore()} returns Float.NaN. Setting this to
   *          true affects performance as it incurs the score computation on
   *          each result. Also, setting this true automatically sets
   *          <code>trackDocScores</code> to true as well.
   * @param trackTotalHits
   *          specifies whether the total number of hits should be tracked. If
   *          set to false, the value of {@link TopFieldDocs#totalHits} will be
   *          approximated.
   * @return a {@link TopFieldCollector} instance which will sort the results by
   *         the sort criteria.
   */
  static std::shared_ptr<TopFieldCollector>
  create(std::shared_ptr<Sort> sort, int numHits, bool fillFields,
         bool trackDocScores, bool trackMaxScore, bool trackTotalHits);

  /**
   * Creates a new {@link TopFieldCollector} from the given
   * arguments.
   *
   * <p><b>NOTE</b>: The instances returned by this method
   * pre-allocate a full array of length
   * <code>numHits</code>.
   *
   * @param sort
   *          the sort criteria (SortFields).
   * @param numHits
   *          the number of results to collect.
   * @param after
   *          only hits after this FieldDoc will be collected
   * @param fillFields
   *          specifies whether the actual field values should be returned on
   *          the results (FieldDoc).
   * @param trackDocScores
   *          specifies whether document scores should be tracked and set on the
   *          results. Note that if set to false, then the results' scores will
   *          be set to Float.NaN. Setting this to true affects performance, as
   *          it incurs the score computation on each competitive result.
   *          Therefore if document scores are not required by the application,
   *          it is recommended to set it to false.
   * @param trackMaxScore
   *          specifies whether the query's maxScore should be tracked and set
   *          on the resulting {@link TopDocs}. Note that if set to false,
   *          {@link TopDocs#getMaxScore()} returns Float.NaN. Setting this to
   *          true affects performance as it incurs the score computation on
   *          each result. Also, setting this true automatically sets
   *          <code>trackDocScores</code> to true as well.
   * @return a {@link TopFieldCollector} instance which will sort the results by
   *         the sort criteria.
   * @deprecated Use {@link #create(Sort, int, FieldDoc, bool, bool,
   * bool, bool)}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static TopFieldCollector create(Sort
  // sort, int numHits, FieldDoc after, bool fillFields, bool
  // trackDocScores, bool trackMaxScore)
  static std::shared_ptr<TopFieldCollector>
  create(std::shared_ptr<Sort> sort, int numHits,
         std::shared_ptr<FieldDoc> after, bool fillFields, bool trackDocScores,
         bool trackMaxScore);

  /**
   * Creates a new {@link TopFieldCollector} from the given
   * arguments.
   *
   * <p><b>NOTE</b>: The instances returned by this method
   * pre-allocate a full array of length
   * <code>numHits</code>.
   *
   * @param sort
   *          the sort criteria (SortFields).
   * @param numHits
   *          the number of results to collect.
   * @param after
   *          only hits after this FieldDoc will be collected
   * @param fillFields
   *          specifies whether the actual field values should be returned on
   *          the results (FieldDoc).
   * @param trackDocScores
   *          specifies whether document scores should be tracked and set on the
   *          results. Note that if set to false, then the results' scores will
   *          be set to Float.NaN. Setting this to true affects performance, as
   *          it incurs the score computation on each competitive result.
   *          Therefore if document scores are not required by the application,
   *          it is recommended to set it to false.
   * @param trackMaxScore
   *          specifies whether the query's maxScore should be tracked and set
   *          on the resulting {@link TopDocs}. Note that if set to false,
   *          {@link TopDocs#getMaxScore()} returns Float.NaN. Setting this to
   *          true affects performance as it incurs the score computation on
   *          each result. Also, setting this true automatically sets
   *          <code>trackDocScores</code> to true as well.
   * @param trackTotalHits
   *          specifies whether the total number of hits should be tracked. If
   *          set to false, the value of {@link TopFieldDocs#totalHits} will be
   *          approximated.
   * @return a {@link TopFieldCollector} instance which will sort the results by
   *         the sort criteria.
   */
  static std::shared_ptr<TopFieldCollector>
  create(std::shared_ptr<Sort> sort, int numHits,
         std::shared_ptr<FieldDoc> after, bool fillFields, bool trackDocScores,
         bool trackMaxScore, bool trackTotalHits);

  void add(int slot, int doc, float score);

  void updateBottom(int doc);

  void updateBottom(int doc, float score);

  /*
   * Only the following callback methods need to be overridden since
   * topDocs(int, int) calls them to return the results.
   */

protected:
  void populateResults(std::deque<std::shared_ptr<ScoreDoc>> &results,
                       int howMany) override;

  std::shared_ptr<TopDocs>
  newTopDocs(std::deque<std::shared_ptr<ScoreDoc>> &results,
             int start) override;

public:
  std::shared_ptr<TopFieldDocs> topDocs() override;

  /** Return whether collection terminated early. */
  virtual bool isEarlyTerminated();

protected:
  std::shared_ptr<TopFieldCollector> shared_from_this()
  {
    return std::static_pointer_cast<TopFieldCollector>(
        TopDocsCollector<org.apache.lucene.search.FieldValueHitQueue
                             .Entry>::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
class TopFieldCollector::SimpleFieldCollector : public TopFieldCollector
{
  GET_CLASS_NAME(TopFieldCollector::SimpleFieldCollector)

public:
  const std::shared_ptr<Sort> sort;
  const std::shared_ptr<FieldValueHitQueue<std::shared_ptr<Entry>>> queue;
  const bool trackDocScores;
  const bool trackMaxScore;
  const bool mayNeedScoresTwice;
  const bool trackTotalHits;

  SimpleFieldCollector(
      std::shared_ptr<Sort> sort,
      std::shared_ptr<FieldValueHitQueue<std::shared_ptr<Entry>>> queue,
      int numHits, bool fillFields, bool trackDocScores, bool trackMaxScore,
      bool trackTotalHits);

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class MultiComparatorLeafCollectorAnonymousInnerClass
      : public MultiComparatorLeafCollector
  {
    GET_CLASS_NAME(MultiComparatorLeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<SimpleFieldCollector> outerInstance;

    std::shared_ptr<LeafReaderContext> context;
    std::deque<int> reverseMul;
    bool canEarlyTerminate = false;
    int initialTotalHits = 0;

  public:
    MultiComparatorLeafCollectorAnonymousInnerClass(
        std::shared_ptr<SimpleFieldCollector> outerInstance,
        bool mayNeedScoresTwice, std::shared_ptr<LeafReaderContext> context,
        std::deque<int> &reverseMul, bool canEarlyTerminate,
        int initialTotalHits);

    void collect(int doc)  override;

  protected:
    std::shared_ptr<MultiComparatorLeafCollectorAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          MultiComparatorLeafCollectorAnonymousInnerClass>(
          MultiComparatorLeafCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SimpleFieldCollector> shared_from_this()
  {
    return std::static_pointer_cast<SimpleFieldCollector>(
        TopFieldCollector::shared_from_this());
  }
};
class TopFieldCollector::PagingFieldCollector final : public TopFieldCollector
{
  GET_CLASS_NAME(TopFieldCollector::PagingFieldCollector)

public:
  const std::shared_ptr<Sort> sort;
  int collectedHits = 0;
  const std::shared_ptr<FieldValueHitQueue<std::shared_ptr<Entry>>> queue;
  const bool trackDocScores;
  const bool trackMaxScore;
  const std::shared_ptr<FieldDoc> after;
  const bool mayNeedScoresTwice;
  const bool trackTotalHits;

  PagingFieldCollector(
      std::shared_ptr<Sort> sort,
      std::shared_ptr<FieldValueHitQueue<std::shared_ptr<Entry>>> queue,
      std::shared_ptr<FieldDoc> after, int numHits, bool fillFields,
      bool trackDocScores, bool trackMaxScore, bool trackTotalHits);

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class MultiComparatorLeafCollectorAnonymousInnerClass
      : public MultiComparatorLeafCollector
  {
    GET_CLASS_NAME(MultiComparatorLeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<PagingFieldCollector> outerInstance;

    std::shared_ptr<LeafReaderContext> context;
    int afterDoc = 0;
    bool canEarlyTerminate = false;
    int initialTotalHits = 0;

  public:
    MultiComparatorLeafCollectorAnonymousInnerClass(
        std::shared_ptr<PagingFieldCollector> outerInstance,
        std::deque<
            std::shared_ptr<org::apache::lucene::search::LeafFieldComparator>>
            &getComparators,
        std::deque<int> &getReverseMul, bool mayNeedScoresTwice,
        std::shared_ptr<LeafReaderContext> context, int afterDoc,
        bool canEarlyTerminate, int initialTotalHits);

    void collect(int doc)  override;

  protected:
    std::shared_ptr<MultiComparatorLeafCollectorAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          MultiComparatorLeafCollectorAnonymousInnerClass>(
          MultiComparatorLeafCollector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<PagingFieldCollector> shared_from_this()
  {
    return std::static_pointer_cast<PagingFieldCollector>(
        TopFieldCollector::shared_from_this());
  }
};
