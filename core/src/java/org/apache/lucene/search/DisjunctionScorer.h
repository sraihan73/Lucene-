#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DisiPriorityQueue;
}

namespace org::apache::lucene::search
{
class DisjunctionDISIApproximation;
}
namespace org::apache::lucene::search
{
class TwoPhase;
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
namespace org::apache::lucene::search
{
class DisiWrapper;
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

using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Base class for Scorers that score disjunctions.
 */
class DisjunctionScorer : public Scorer
{
  GET_CLASS_NAME(DisjunctionScorer)

private:
  const bool needsScores;

  const std::shared_ptr<DisiPriorityQueue> subScorers;
  const std::shared_ptr<DisjunctionDISIApproximation> approximation;
  const std::shared_ptr<TwoPhase> twoPhase;

protected:
  DisjunctionScorer(std::shared_ptr<Weight> weight,
                    std::deque<std::shared_ptr<Scorer>> &subScorers,
                    bool needsScores);

public:
  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

private:
  class TwoPhase : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhase)
  private:
    std::shared_ptr<DisjunctionScorer> outerInstance;

    // C++ NOTE: Fields cannot have the same name as methods:
    const float matchCost_;
    // deque of verified matches on the current doc
  public:
    std::shared_ptr<DisiWrapper> verifiedMatches;
    // priority queue of approximations on the current doc that have not been
    // verified yet
    const std::shared_ptr<PriorityQueue<std::shared_ptr<DisiWrapper>>>
        unverifiedMatches;

  private:
    TwoPhase(std::shared_ptr<DisjunctionScorer> outerInstance,
             std::shared_ptr<DocIdSetIterator> approximation, float matchCost);

  public:
    virtual std::shared_ptr<DisiWrapper> getSubMatches() ;

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhase> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhase>(
          TwoPhaseIterator::shared_from_this());
    }
  };

public:
  int docID() override final;

  virtual std::shared_ptr<DisiWrapper> getSubMatches() ;

  float score()  override final;

  /** Compute the score for the given linked deque of scorers. */
protected:
  virtual float score(std::shared_ptr<DisiWrapper> topList) = 0;

public:
  std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren()  override final;

protected:
  std::shared_ptr<DisjunctionScorer> shared_from_this()
  {
    return std::static_pointer_cast<DisjunctionScorer>(
        Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
