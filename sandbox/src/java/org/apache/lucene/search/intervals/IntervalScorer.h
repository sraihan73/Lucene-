#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::intervals
{
class IntervalIterator;
}

namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
class Explanation;
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

namespace org::apache::lucene::search::intervals
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using Similarity = org::apache::lucene::search::similarities::Similarity;

class IntervalScorer : public Scorer
{
  GET_CLASS_NAME(IntervalScorer)

private:
  const std::shared_ptr<IntervalIterator> intervals;
  const std::shared_ptr<Similarity::SimScorer> simScorer;

  // C++ NOTE: Fields cannot have the same name as methods:
  float freq_ = -1;
  int lastScoredDoc = -1;

protected:
  IntervalScorer(std::shared_ptr<Weight> weight,
                 std::shared_ptr<IntervalIterator> intervals,
                 std::shared_ptr<Similarity::SimScorer> simScorer);

public:
  int docID() override;

  float score()  override;

  virtual std::shared_ptr<Explanation>
  explain(const std::wstring &topLevel) ;

  virtual float freq() ;

private:
  void ensureFreq() ;

public:
  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<IntervalScorer> outerInstance;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<IntervalScorer> outerInstance,
        std::shared_ptr<
            org::apache::lucene::search::intervals::IntervalIterator>
            intervals);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<IntervalScorer> shared_from_this()
  {
    return std::static_pointer_cast<IntervalScorer>(
        org.apache.lucene.search.Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals
