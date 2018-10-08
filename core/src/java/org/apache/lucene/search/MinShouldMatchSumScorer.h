#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DisiWrapper;
}

namespace org::apache::lucene::search
{
class DisiPriorityQueue;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class TwoPhaseIterator;
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DisiPriorityQueue.leftNode;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DisiPriorityQueue.parentNode;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DisiPriorityQueue.rightNode;

/**
 * A {@link Scorer} for {@link BooleanQuery} when
 * {@link BooleanQuery.Builder#setMinimumNumberShouldMatch(int) minShouldMatch}
 * is between 2 and the total number of clauses.
 *
 * This implementation keeps sub scorers in 3 different places:
 *  - lead: a linked deque of scorer that are positioned on the desired doc ID
 *  - tail: a heap that contains at most minShouldMatch - 1 scorers that are
 *    behind the desired doc ID. These scorers are ordered by cost so that we
 *    can advance the least costly ones first.
 *  - head: a heap that contains scorers which are beyond the desired doc ID,
 *    ordered by doc ID in order to move quickly to the next candidate.
 *
 * Finding the next match consists of first setting the desired doc ID to the
 * least entry in 'head' and then advance 'tail' until there is a match.
 */
class MinShouldMatchSumScorer final : public Scorer
{
  GET_CLASS_NAME(MinShouldMatchSumScorer)

public:
  static int64_t cost(std::shared_ptr<LongStream> costs, int numScorers,
                        int minShouldMatch);

private:
  class PriorityQueueAnonymousInnerClass : public PriorityQueue<int64_t>
  {
    GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
  public:
    PriorityQueueAnonymousInnerClass(int numScorers);

  protected:
    bool lessThan(std::optional<int64_t> &a,
                  std::optional<int64_t> &b) override;

  protected:
    std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
          org.apache.lucene.util.PriorityQueue<long>::shared_from_this());
    }
  };

public:
  const int minShouldMatch;

  // deque of scorers which 'lead' the iteration and are currently
  // positioned on 'doc'
  std::shared_ptr<DisiWrapper> lead;
  int doc = 0;  // current doc ID of the leads
  int freq = 0; // number of scorers on the desired doc ID

  // priority queue of scorers that are too advanced compared to the current
  // doc. Ordered by doc ID.
  const std::shared_ptr<DisiPriorityQueue> head;

  // priority queue of scorers which are behind the current doc.
  // Ordered by cost.
  std::deque<std::shared_ptr<DisiWrapper>> const tail;
  int tailSize = 0;

  // C++ NOTE: Fields cannot have the same name as methods:
  const int64_t cost_;

  MinShouldMatchSumScorer(
      std::shared_ptr<Weight> weight,
      std::shared_ptr<std::deque<std::shared_ptr<Scorer>>> scorers,
      int minShouldMatch);

  std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren()  override final;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

private:
  class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<MinShouldMatchSumScorer> outerInstance;

  public:
    DocIdSetIteratorAnonymousInnerClass(
        std::shared_ptr<MinShouldMatchSumScorer> outerInstance);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
          DocIdSetIterator::shared_from_this());
    }
  };

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<MinShouldMatchSumScorer> outerInstance;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<MinShouldMatchSumScorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            approximation);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          TwoPhaseIterator::shared_from_this());
    }
  };

private:
  void addLead(std::shared_ptr<DisiWrapper> lead);

  void pushBackLeads() ;

  void advanceTail(std::shared_ptr<DisiWrapper> top) ;

  void advanceTail() ;

  /** Reinitializes head, freq and doc from 'head' */
  void setDocAndFreq();

  /** Advance tail to the lead until there is a match. */
  int doNext() ;

  /** Move iterators to the tail until the cumulated size of lead+tail is
   *  greater than or equal to minShouldMath */
  int doNextCandidate() ;

  /** Advance all entries from the tail to know about all matches on the
   *  current doc. */
  void updateFreq() ;

public:
  float score()  override;

  int docID() override;

  /** Insert an entry in 'tail' and evict the least-costly scorer if full. */
private:
  std::shared_ptr<DisiWrapper>
  insertTailWithOverFlow(std::shared_ptr<DisiWrapper> s);

  /** Add an entry to 'tail'. Fails if over capacity. */
  void addTail(std::shared_ptr<DisiWrapper> s);

  /** Pop the least-costly scorer from 'tail'. */
  std::shared_ptr<DisiWrapper> popTail();

  /** Heap helpers */

  static void upHeapCost(std::deque<std::shared_ptr<DisiWrapper>> &heap,
                         int i);

  static void downHeapCost(std::deque<std::shared_ptr<DisiWrapper>> &heap,
                           int size);

protected:
  std::shared_ptr<MinShouldMatchSumScorer> shared_from_this()
  {
    return std::static_pointer_cast<MinShouldMatchSumScorer>(
        Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
