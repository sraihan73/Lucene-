#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class BulkScorer;
}

namespace org::apache::lucene::search
{
class LeafCollector;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::search
{
class FakeScorer;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class BooleanWeight;
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

using Bits = org::apache::lucene::util::Bits;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * {@link BulkScorer} that is used for pure disjunctions and disjunctions
 * that have low values of {@link
 * BooleanQuery.Builder#setMinimumNumberShouldMatch(int)} and dense clauses.
 * This scorer scores documents by batches of 2048 docs.
 */
class BooleanScorer final : public BulkScorer
{
  GET_CLASS_NAME(BooleanScorer)

public:
  static constexpr int SHIFT = 11;
  static const int SIZE = 1 << SHIFT;
  static const int MASK = SIZE - 1;
  static const int SET_SIZE = 1 << (SHIFT - 6);
  static const int SET_MASK = SET_SIZE - 1;

public:
  class Bucket : public std::enable_shared_from_this<Bucket>
  {
    GET_CLASS_NAME(Bucket)
  public:
    double score = 0;
    int freq = 0;
  };

private:
  class BulkScorerAndDoc : public std::enable_shared_from_this<BulkScorerAndDoc>
  {
    GET_CLASS_NAME(BulkScorerAndDoc)
  private:
    std::shared_ptr<BooleanScorer> outerInstance;

  public:
    const std::shared_ptr<BulkScorer> scorer;
    const int64_t cost;
    int next = 0;

    BulkScorerAndDoc(std::shared_ptr<BooleanScorer> outerInstance,
                     std::shared_ptr<BulkScorer> scorer);

    virtual void advance(int min) ;

    virtual void score(std::shared_ptr<LeafCollector> collector,
                       std::shared_ptr<Bits> acceptDocs, int min,
                       int max) ;
  };

  // See MinShouldMatchSumScorer for an explanation
private:
  static int64_t
  cost(std::shared_ptr<std::deque<std::shared_ptr<BulkScorer>>> scorers,
       int minShouldMatch);

private:
  class PriorityQueueAnonymousInnerClass
      : public PriorityQueue<std::shared_ptr<BulkScorer>>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
  public:
    PriorityQueueAnonymousInnerClass(std::shared_ptr<UnknownType> size);

  protected:
    bool lessThan(std::shared_ptr<BulkScorer> a,
                  std::shared_ptr<BulkScorer> b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
          org.apache.lucene.util.PriorityQueue<BulkScorer>::shared_from_this());
    }
  };

public:
  class HeadPriorityQueue final
      : public PriorityQueue<std::shared_ptr<BulkScorerAndDoc>>
  {
    GET_CLASS_NAME(HeadPriorityQueue)

  public:
    HeadPriorityQueue(int maxSize);

  protected:
    bool lessThan(std::shared_ptr<BulkScorerAndDoc> a,
                  std::shared_ptr<BulkScorerAndDoc> b) override;

  protected:
    std::shared_ptr<HeadPriorityQueue> shared_from_this()
    {
      return std::static_pointer_cast<HeadPriorityQueue>(
          org.apache.lucene.util
              .PriorityQueue<BulkScorerAndDoc>::shared_from_this());
    }
  };

public:
  class TailPriorityQueue final
      : public PriorityQueue<std::shared_ptr<BulkScorerAndDoc>>
  {
    GET_CLASS_NAME(TailPriorityQueue)

  public:
    TailPriorityQueue(int maxSize);

  protected:
    bool lessThan(std::shared_ptr<BulkScorerAndDoc> a,
                  std::shared_ptr<BulkScorerAndDoc> b) override;

  public:
    std::shared_ptr<BulkScorerAndDoc> get(int i);

  protected:
    std::shared_ptr<TailPriorityQueue> shared_from_this()
    {
      return std::static_pointer_cast<TailPriorityQueue>(
          org.apache.lucene.util
              .PriorityQueue<BulkScorerAndDoc>::shared_from_this());
    }
  };

public:
  std::deque<std::shared_ptr<Bucket>> const buckets =
      std::deque<std::shared_ptr<Bucket>>(SIZE);
  // This is basically an inlined FixedBitSet... seems to help with bound checks
  std::deque<int64_t> const matching = std::deque<int64_t>(SET_SIZE);

  std::deque<std::shared_ptr<BulkScorerAndDoc>> const leads;
  const std::shared_ptr<HeadPriorityQueue> head;
  const std::shared_ptr<TailPriorityQueue> tail;
  const std::shared_ptr<FakeScorer> fakeScorer = std::make_shared<FakeScorer>();
  const int minShouldMatch;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t cost_;

public:
  class OrCollector final : public std::enable_shared_from_this<OrCollector>,
                            public LeafCollector
  {
    GET_CLASS_NAME(OrCollector)
  private:
    std::shared_ptr<BooleanScorer> outerInstance;

  public:
    OrCollector(std::shared_ptr<BooleanScorer> outerInstance);

    std::shared_ptr<Scorer> scorer;

    void setScorer(std::shared_ptr<Scorer> scorer) override;

    void collect(int doc)  override;
  };

public:
  const std::shared_ptr<OrCollector> orCollector =
      std::make_shared<OrCollector>(shared_from_this());

  BooleanScorer(
      std::shared_ptr<BooleanWeight> weight,
      std::shared_ptr<std::deque<std::shared_ptr<BulkScorer>>> scorers,
      int minShouldMatch, bool needsScores);

  int64_t cost() override;

private:
  void scoreDocument(std::shared_ptr<LeafCollector> collector, int base,
                     int i) ;

  void scoreMatches(std::shared_ptr<LeafCollector> collector,
                    int base) ;

  void scoreWindowIntoBitSetAndReplay(
      std::shared_ptr<LeafCollector> collector,
      std::shared_ptr<Bits> acceptDocs, int base, int min, int max,
      std::deque<std::shared_ptr<BulkScorerAndDoc>> &scorers,
      int numScorers) ;

  std::shared_ptr<BulkScorerAndDoc> advance(int min) ;

  void scoreWindowMultipleScorers(std::shared_ptr<LeafCollector> collector,
                                  std::shared_ptr<Bits> acceptDocs,
                                  int windowBase, int windowMin, int windowMax,
                                  int maxFreq) ;

  void scoreWindowSingleScorer(std::shared_ptr<BulkScorerAndDoc> bulkScorer,
                               std::shared_ptr<LeafCollector> collector,
                               std::shared_ptr<Bits> acceptDocs, int windowMin,
                               int windowMax, int max) ;

  std::shared_ptr<BulkScorerAndDoc>
  scoreWindow(std::shared_ptr<BulkScorerAndDoc> top,
              std::shared_ptr<LeafCollector> collector,
              std::shared_ptr<Bits> acceptDocs, int min,
              int max) ;

public:
  int score(std::shared_ptr<LeafCollector> collector,
            std::shared_ptr<Bits> acceptDocs, int min,
            int max)  override;

protected:
  std::shared_ptr<BooleanScorer> shared_from_this()
  {
    return std::static_pointer_cast<BooleanScorer>(
        BulkScorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
