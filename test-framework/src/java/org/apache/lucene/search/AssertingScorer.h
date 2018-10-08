#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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

/** Wraps a Scorer with additional checks */
class AssertingScorer : public Scorer
{
  GET_CLASS_NAME(AssertingScorer)

public:
  enum class IteratorState {
    GET_CLASS_NAME(IteratorState) START,
    APPROXIMATING,
    ITERATING,
    FINISHED
  };

public:
  static std::shared_ptr<Scorer> wrap(std::shared_ptr<Random> random,
                                      std::shared_ptr<Scorer> other,
                                      bool canScore);

  const std::shared_ptr<Random> random;
  const std::shared_ptr<Scorer> in_;
  const bool needsScores;

  IteratorState state = IteratorState::START;
  int doc = 0;

private:
  AssertingScorer(std::shared_ptr<Random> random, std::shared_ptr<Scorer> in_,
                  bool needsScores);

public:
  virtual std::shared_ptr<Scorer> getIn();

  virtual bool iterating();

  float score()  override;

  std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren() override;

  int docID() override;

  virtual std::wstring toString();

  std::shared_ptr<DocIdSetIterator> iterator() override;

private:
  class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<AssertingScorer> outerInstance;

    std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> in_;

  public:
    DocIdSetIteratorAnonymousInnerClass(
        std::shared_ptr<AssertingScorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> in_);

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

public:
  std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

private:
  class DocIdSetIteratorAnonymousInnerClass2 : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<AssertingScorer> outerInstance;

    std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
        inApproximation;

  public:
    DocIdSetIteratorAnonymousInnerClass2(
        std::shared_ptr<AssertingScorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            inApproximation);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<DocIdSetIteratorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass2>(
          DocIdSetIterator::shared_from_this());
    }
  };

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<AssertingScorer> outerInstance;

    std::shared_ptr<org::apache::lucene::search::TwoPhaseIterator> in_;
    std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
        inApproximation;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<AssertingScorer> outerInstance,
        std::shared_ptr<org::apache::lucene::search::TwoPhaseIterator> in_,
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            inApproximation);

    bool matches()  override;

    float matchCost() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          TwoPhaseIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingScorer> shared_from_this()
  {
    return std::static_pointer_cast<AssertingScorer>(
        Scorer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
