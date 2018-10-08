#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/LongValues.h"

#include  "core/src/java/org/apache/lucene/search/DisiWrapper.h"
#include  "core/src/java/org/apache/lucene/search/DisiPriorityQueue.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"

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

/** A {@link Scorer} whose number of matches is per-document. */
class CoveringScorer final : public Scorer
{
  GET_CLASS_NAME(CoveringScorer)

public:
  const int numScorers;
  const int maxDoc;
  const std::shared_ptr<LongValues> minMatchValues;

  bool matches = false; // if true then the doc matches, otherwise we don't know
                        // and need to check
  int doc = 0;          // current doc ID
  std::shared_ptr<DisiWrapper> topList; // deque of matches
  int freq = 0;           // number of scorers on the desired doc ID
  int64_t minMatch = 0; // current required number of matches

  // priority queue that stores all scorers
  const std::shared_ptr<DisiPriorityQueue> subScorers;

  const int64_t cost;

  CoveringScorer(std::shared_ptr<Weight> weight,
                 std::shared_ptr<std::deque<std::shared_ptr<Scorer>>> scorers,
                 std::shared_ptr<LongValues> minMatchValues, int maxDoc);

  std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren()  override final;

private:
  const std::shared_ptr<DocIdSetIterator> approximation =
      std::make_shared<DocIdSetIteratorAnonymousInnerClass>();

private:
  class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
  public:
    DocIdSetIteratorAnonymousInnerClass();

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

  private:
    void setMinMatch() ;

  public:
    int64_t cost() override;

  protected:
    std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
          DocIdSetIterator::shared_from_this());
    }
  };

private:
  const std::shared_ptr<TwoPhaseIterator> twoPhase =
      std::make_shared<TwoPhaseIteratorAnonymousInnerClass>(approximation);

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<UnknownType> approximation);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          TwoPhaseIterator::shared_from_this());
    }
  };

public:
  std::shared_ptr<DocIdSetIterator> iterator() override;

  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

private:
  void advanceAll(int target) ;

  void setTopListAndFreq();

  void setTopListAndFreqIfNecessary() ;

public:
  float score()  override;

  int docID() override;

protected:
  std::shared_ptr<CoveringScorer> shared_from_this()
  {
    return std::static_pointer_cast<CoveringScorer>(Scorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
