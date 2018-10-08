#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}

namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search::spans
{
class Spans;
}
namespace org::apache::lucene::search
{
class TwoPhaseIterator;
}
namespace org::apache::lucene::util
{
class BitSetIterator;
}
namespace org::apache::lucene::util
{
class BitSet;
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

using Spans = org::apache::lucene::search::spans::Spans;
using BitSet = org::apache::lucene::util::BitSet;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;

/** A conjunction of DocIdSetIterators.
 * This iterates over the doc ids that are present in each given
 * DocIdSetIterator. <br>Public only for use in {@link
 * org.apache.lucene.search.spans}.
 * @lucene.internal
 */
class ConjunctionDISI final : public DocIdSetIterator
{
  GET_CLASS_NAME(ConjunctionDISI)

  /** Create a conjunction over the provided {@link Scorer}s. Note that the
   * returned {@link DocIdSetIterator} might leverage two-phase iteration in
   * which case it is possible to retrieve the {@link TwoPhaseIterator} using
   * {@link TwoPhaseIterator#unwrap}. */
public:
  static std::shared_ptr<DocIdSetIterator> intersectScorers(
      std::shared_ptr<std::deque<std::shared_ptr<Scorer>>> scorers);

  /** Create a conjunction over the provided DocIdSetIterators. Note that the
   * returned {@link DocIdSetIterator} might leverage two-phase iteration in
   * which case it is possible to retrieve the {@link TwoPhaseIterator} using
   * {@link TwoPhaseIterator#unwrap}. */
  static std::shared_ptr<DocIdSetIterator>
  intersectIterators(std::deque<std::shared_ptr<DocIdSetIterator>> &iterators);

  /** Create a conjunction over the provided {@link Spans}. Note that the
   * returned {@link DocIdSetIterator} might leverage two-phase iteration in
   * which case it is possible to retrieve the {@link TwoPhaseIterator} using
   * {@link TwoPhaseIterator#unwrap}. */
  static std::shared_ptr<DocIdSetIterator>
  intersectSpans(std::deque<std::shared_ptr<Spans>> &spanList);

  /** Adds the scorer, possibly splitting up into two phases or collapsing if it
   * is another conjunction */
private:
  static void
  addScorer(std::shared_ptr<Scorer> scorer,
            std::deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
            std::deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators);

  /** Adds the Spans. */
  static void
  addSpans(std::shared_ptr<Spans> spans,
           std::deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
           std::deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators);

  static void addIterator(
      std::shared_ptr<DocIdSetIterator> disi,
      std::deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
      std::deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators);

  static void addTwoPhaseIterator(
      std::shared_ptr<TwoPhaseIterator> twoPhaseIter,
      std::deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
      std::deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators);

  static std::shared_ptr<DocIdSetIterator> createConjunction(
      std::deque<std::shared_ptr<DocIdSetIterator>> &allIterators,
      std::deque<std::shared_ptr<TwoPhaseIterator>> &twoPhaseIterators);

public:
  const std::shared_ptr<DocIdSetIterator> lead1, lead2;
  std::deque<std::shared_ptr<DocIdSetIterator>> const others;

private:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: private ConjunctionDISI(java.util.List<? extends
  // DocIdSetIterator> iterators)
  ConjunctionDISI(std::deque<T1> iterators);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<DocIdSetIterator>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<ConjunctionDISI> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<ConjunctionDISI> outerInstance);

    int compare(std::shared_ptr<DocIdSetIterator> o1,
                std::shared_ptr<DocIdSetIterator> o2);
  };

private:
  int doNext(int doc) ;

public:
  int advance(int target)  override;

  int docID() override;

  int nextDoc()  override;

  int64_t cost() override;

  /** Conjunction between a {@link DocIdSetIterator} and one or more {@link
   * BitSetIterator}s. */
private:
  class BitSetConjunctionDISI : public DocIdSetIterator
  {
    GET_CLASS_NAME(BitSetConjunctionDISI)

  private:
    const std::shared_ptr<DocIdSetIterator> lead;
    std::deque<std::shared_ptr<BitSetIterator>> const bitSetIterators;
    std::deque<std::shared_ptr<BitSet>> const bitSets;
    const int minLength;

  public:
    BitSetConjunctionDISI(
        std::shared_ptr<DocIdSetIterator> lead,
        std::shared_ptr<std::deque<std::shared_ptr<BitSetIterator>>>
            bitSetIterators);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

  private:
    int doNext(int doc) ;

  public:
    int64_t cost() override;

  protected:
    std::shared_ptr<BitSetConjunctionDISI> shared_from_this()
    {
      return std::static_pointer_cast<BitSetConjunctionDISI>(
          DocIdSetIterator::shared_from_this());
    }
  };

  /**
   * {@link TwoPhaseIterator} implementing a conjunction.
   */
private:
  class ConjunctionTwoPhaseIterator final : public TwoPhaseIterator
  {
    GET_CLASS_NAME(ConjunctionTwoPhaseIterator)

  private:
    std::deque<std::shared_ptr<TwoPhaseIterator>> const twoPhaseIterators;
    // C++ NOTE: Fields cannot have the same name as methods:
    const float matchCost_;

    template <typename T1>
    // C++ TODO: There is no native C++ template equivalent to this generic
    // constraint: ORIGINAL LINE: private
    // ConjunctionTwoPhaseIterator(DocIdSetIterator approximation,
    // java.util.List<? extends TwoPhaseIterator> twoPhaseIterators)
    ConjunctionTwoPhaseIterator(std::shared_ptr<DocIdSetIterator> approximation,
                                std::deque<T1> twoPhaseIterators);

  private:
    class ComparatorAnonymousInnerClass
        : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
          public Comparator<std::shared_ptr<TwoPhaseIterator>>
    {
      GET_CLASS_NAME(ComparatorAnonymousInnerClass)
    private:
      std::shared_ptr<ConjunctionTwoPhaseIterator> outerInstance;

    public:
      ComparatorAnonymousInnerClass(
          std::shared_ptr<ConjunctionTwoPhaseIterator> outerInstance);

      int compare(std::shared_ptr<TwoPhaseIterator> o1,
                  std::shared_ptr<TwoPhaseIterator> o2);
    };

  public:
    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<ConjunctionTwoPhaseIterator> shared_from_this()
    {
      return std::static_pointer_cast<ConjunctionTwoPhaseIterator>(
          TwoPhaseIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ConjunctionDISI> shared_from_this()
  {
    return std::static_pointer_cast<ConjunctionDISI>(
        DocIdSetIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
