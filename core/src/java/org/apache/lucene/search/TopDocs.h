#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include "core/src/java/org/apache/lucene/util/PriorityQueue.h"
#include "core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "core/src/java/org/apache/lucene/search/FieldComparator.h"
#include "core/src/java/org/apache/lucene/search/Sort.h"
#include "core/src/java/org/apache/lucene/search/TopFieldDocs.h"

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

/** Represents hits returned by {@link
 * IndexSearcher#search(Query,int)}. */
class TopDocs : public std::enable_shared_from_this<TopDocs>
{
  GET_CLASS_NAME(TopDocs)

  /** The total number of hits for the query. */
public:
  int64_t totalHits = 0;

  /** The top hits for the query. */
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs;

  /** Stores the maximum score value encountered, needed for normalizing. */
private:
  float maxScore = 0;

  /**
   * Returns the maximum score value encountered. Note that in case
   * scores are not tracked, this returns {@link Float#NaN}.
   */
public:
  virtual float getMaxScore();

  /** Sets the maximum score value encountered. */
  virtual void setMaxScore(float maxScore);

  /** Constructs a TopDocs with a default maxScore=Float.NaN. */
  TopDocs(int64_t totalHits,
          std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs);

  TopDocs(int64_t totalHits,
          std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs, float maxScore);

  // Refers to one hit:
private:
  class ShardRef final : public std::enable_shared_from_this<ShardRef>
  {
    GET_CLASS_NAME(ShardRef)
    // Which shard (index into shardHits[]):
  public:
    const int shardIndex;

    // True if we should use the incoming ScoreDoc.shardIndex for sort order
    const bool useScoreDocIndex;

    // Which hit within the shard:
    int hitIndex = 0;

    ShardRef(int shardIndex, bool useScoreDocIndex);

    virtual std::wstring toString();

    int getShardIndex(std::shared_ptr<ScoreDoc> scoreDoc);
  };

  /**
   * if we need to tie-break since score / sort value are the same we first
   * compare shard index (lower shard wins) and then iff shard index is the same
   * we use the hit index.
   */
public:
  static bool tieBreakLessThan(std::shared_ptr<ShardRef> first,
                               std::shared_ptr<ScoreDoc> firstDoc,
                               std::shared_ptr<ShardRef> second,
                               std::shared_ptr<ScoreDoc> secondDoc);

  // Specialized MergeSortQueue that just merges by
  // relevance score, descending:
private:
  class ScoreMergeSortQueue : public PriorityQueue<std::shared_ptr<ShardRef>>
  {
    GET_CLASS_NAME(ScoreMergeSortQueue)
  public:
    std::deque<std::deque<std::shared_ptr<ScoreDoc>>> const shardHits;

    ScoreMergeSortQueue(std::deque<std::shared_ptr<TopDocs>> &shardHits);

    // Returns true if first is < second
    bool operator < (std::shared_ptr<ShardRef> first,
                     std::shared_ptr<ShardRef> second);

  protected:
    std::shared_ptr<ScoreMergeSortQueue> shared_from_this()
    {
      return std::static_pointer_cast<ScoreMergeSortQueue>(PriorityQueue<ShardRef>::shared_from_this());
    }
  };

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) private static
  // class MergeSortQueue extends org.apache.lucene.util.PriorityQueue<ShardRef>
  class MergeSortQueue : public PriorityQueue<std::shared_ptr<ShardRef>>
  {
    // These are really FieldDoc instances:
  public:
    std::deque<std::deque<std::shared_ptr<ScoreDoc>>> const shardHits;
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: final FieldComparator<?>[] comparators;
    std::deque < FieldComparator < ? >> const comparators;
    std::deque<int> const reverseMul;

    MergeSortQueue(std::shared_ptr<Sort> sort,
                   std::deque<std::shared_ptr<TopDocs>> &shardHits);

    // Returns true if first is < second
    bool lessThan(std::shared_ptr<ShardRef> first,
                  std::shared_ptr<ShardRef> second) override;

  protected:
    std::shared_ptr<MergeSortQueue> shared_from_this()
    {
      return std::static_pointer_cast<MergeSortQueue>(
          org.apache.lucene.util.PriorityQueue<ShardRef>::shared_from_this());
    }
  };

  /** Returns a new TopDocs, containing topN results across
   *  the provided TopDocs, sorting by score. Each {@link TopDocs}
   *  instance must be sorted.
   *
   *  @see #merge(int, int, TopDocs[], bool)
   *  @lucene.experimental */
public:
  static std::shared_ptr<TopDocs>
  merge(int topN, std::deque<std::shared_ptr<TopDocs>> &shardHits);

  /**
   * Same as {@link #merge(int, TopDocs[])} but also ignores the top
   * {@code start} top docs. This is typically useful for pagination.
   *
   * Note: If {@code setShardIndex} is true, this method will assume the
   * incoming order of {@code shardHits} reflects each shard's index and will
   * fill the {@link ScoreDoc#shardIndex}, otherwise it must already be set for
   * all incoming {@code ScoreDoc}s, which can be useful when doing multiple
   * reductions (merges) of TopDocs.
   *
   * @lucene.experimental
   */
  static std::shared_ptr<TopDocs>
  merge(int start, int topN, std::deque<std::shared_ptr<TopDocs>> &shardHits,
        bool setShardIndex);

  /** Returns a new TopFieldDocs, containing topN results across
   *  the provided TopFieldDocs, sorting by the specified {@link
   *  Sort}.  Each of the TopDocs must have been sorted by
   *  the same Sort, and sort field values must have been
   *  filled (ie, <code>fillFields=true</code> must be
   *  passed to {@link TopFieldCollector#create}).
   *  @see #merge(Sort, int, int, TopFieldDocs[], bool)
   * @lucene.experimental */
  static std::shared_ptr<TopFieldDocs>
  merge(std::shared_ptr<Sort> sort, int topN,
        std::deque<std::shared_ptr<TopFieldDocs>> &shardHits);

  /**
   * Same as {@link #merge(Sort, int, TopFieldDocs[])} but also ignores the top
   * {@code start} top docs. This is typically useful for pagination.
   *
   * Note: If {@code setShardIndex} is true, this method will assume the
   * incoming order of {@code shardHits} reflects each shard's index and will
   * fill the {@link ScoreDoc#shardIndex}, otherwise it must already be set for
   * all incoming {@code ScoreDoc}s, which can be useful when doing multiple
   * reductions (merges) of TopDocs.
   *
   * @lucene.experimental
   */
  static std::shared_ptr<TopFieldDocs>
  merge(std::shared_ptr<Sort> sort, int start, int topN,
        std::deque<std::shared_ptr<TopFieldDocs>> &shardHits,
        bool setShardIndex);

  /** Auxiliary method used by the {@link #merge} impls. A sort value of null
   *  is used to indicate that docs should be sorted by score. */
private:
  static std::shared_ptr<TopDocs>
  mergeAux(std::shared_ptr<Sort> sort, int start, int size,
           std::deque<std::shared_ptr<TopDocs>> &shardHits,
           bool setShardIndex);
};

} // namespace org::apache::lucene::search
