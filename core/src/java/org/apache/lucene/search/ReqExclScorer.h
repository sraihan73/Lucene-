#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Scorer;
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

/** A Scorer for queries with a required subscorer
 * and an excluding (prohibited) sub {@link Scorer}.
 */
class ReqExclScorer : public Scorer
{
  GET_CLASS_NAME(ReqExclScorer)

private:
  const std::shared_ptr<Scorer> reqScorer;
  // approximations of the scorers, or the scorers themselves if they don't
  // support approximations
  const std::shared_ptr<DocIdSetIterator> reqApproximation;
  const std::shared_ptr<DocIdSetIterator> exclApproximation;
  // two-phase views of the scorers, or null if they do not support
  // approximations
  const std::shared_ptr<TwoPhaseIterator> reqTwoPhaseIterator;
  const std::shared_ptr<TwoPhaseIterator> exclTwoPhaseIterator;

  /** Construct a <code>ReqExclScorer</code>.
   * @param reqScorer The scorer that must match, except where
   * @param exclScorer indicates exclusion.
   */
public:
  ReqExclScorer(std::shared_ptr<Scorer> reqScorer,
                std::shared_ptr<Scorer> exclScorer);

  /** Confirms whether or not the given {@link TwoPhaseIterator}
   *  matches on the current document. */
private:
  static bool
  matchesOrNull(std::shared_ptr<TwoPhaseIterator> it) ;

public:
  std::shared_ptr<DocIdSetIterator> iterator() override;

  int docID() override;

  float score()  override;

  std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren() override;

  /**
   * Estimation of the number of operations required to call DISI.advance.
   * This is likely completely wrong, especially given that the cost of
   * this method usually depends on how far you want to advance, but it's
   * probably better than nothing.
   */
private:
  static constexpr int ADVANCE_COST = 10;

  static float
  matchCost(std::shared_ptr<DocIdSetIterator> reqApproximation,
            std::shared_ptr<TwoPhaseIterator> reqTwoPhaseIterator,
            std::shared_ptr<DocIdSetIterator> exclApproximation,
            std::shared_ptr<TwoPhaseIterator> exclTwoPhaseIterator);

public:
  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<ReqExclScorer> outerInstance;

    float matchCost = 0;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<ReqExclScorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            reqApproximation,
        float matchCost);

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
  class TwoPhaseIteratorAnonymousInnerClass2 : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<ReqExclScorer> outerInstance;

    float matchCost = 0;

  public:
    TwoPhaseIteratorAnonymousInnerClass2(
        std::shared_ptr<ReqExclScorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            reqApproximation,
        float matchCost);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass2>(
          TwoPhaseIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ReqExclScorer> shared_from_this()
  {
    return std::static_pointer_cast<ReqExclScorer>(Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
