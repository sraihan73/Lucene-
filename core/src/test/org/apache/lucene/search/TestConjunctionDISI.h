#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <deque>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}

namespace org::apache::lucene::search
{
class TwoPhaseIterator;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::search
{
class Scorer;
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

using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestConjunctionDISI : public LuceneTestCase
{
  GET_CLASS_NAME(TestConjunctionDISI)

private:
  static std::shared_ptr<TwoPhaseIterator>
  approximation(std::shared_ptr<DocIdSetIterator> iterator,
                std::shared_ptr<FixedBitSet> confirmed);

private:
  class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<FixedBitSet> confirmed;
    std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
        approximation;

  public:
    TwoPhaseIteratorAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator>
            approximation,
        std::shared_ptr<FixedBitSet> confirmed);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
          TwoPhaseIterator::shared_from_this());
    }
  };

  /** Return an anonym class so that ConjunctionDISI cannot optimize it
   *  like it does eg. for BitSetIterators. */
private:
  static std::shared_ptr<DocIdSetIterator>
  anonymizeIterator(std::shared_ptr<DocIdSetIterator> it);

private:
  class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
  {
    GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> it;

  public:
    DocIdSetIteratorAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> it);

    int nextDoc()  override;

    int docID() override;

    int64_t cost() override;

    int advance(int target)  override;

  protected:
    std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
          DocIdSetIterator::shared_from_this());
    }
  };

private:
  static std::shared_ptr<Scorer>
  scorer(std::shared_ptr<TwoPhaseIterator> twoPhaseIterator);

  /**
   * Create a {@link Scorer} that wraps the given {@link DocIdSetIterator}. It
   * also accepts a {@link TwoPhaseIterator} view, which is exposed in
   * {@link Scorer#twoPhaseIterator()}. When the two-phase view is not null,
   * then {@link DocIdSetIterator#nextDoc()} and {@link
   * DocIdSetIterator#advance(int)} will raise an exception in order to make
   * sure that {@link ConjunctionDISI} takes advantage of the {@link
   * TwoPhaseIterator} view.
   */
  static std::shared_ptr<Scorer>
  scorer(std::shared_ptr<DocIdSetIterator> it,
         std::shared_ptr<TwoPhaseIterator> twoPhaseIterator);

private:
  class ScorerAnonymousInnerClass : public Scorer
  {
    GET_CLASS_NAME(ScorerAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> it;
    std::shared_ptr<org::apache::lucene::search::TwoPhaseIterator>
        twoPhaseIterator;

  public:
    ScorerAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::search::DocIdSetIterator> it,
        std::shared_ptr<org::apache::lucene::search::TwoPhaseIterator>
            twoPhaseIterator);

    std::shared_ptr<DocIdSetIterator> iterator() override;

  private:
    class DocIdSetIteratorAnonymousInnerClass2 : public DocIdSetIterator
    {
      GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass2)
    private:
      std::shared_ptr<ScorerAnonymousInnerClass> outerInstance;

    public:
      DocIdSetIteratorAnonymousInnerClass2(
          std::shared_ptr<ScorerAnonymousInnerClass> outerInstance);

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

  public:
    std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

    int docID() override;

    float score()  override;

  protected:
    std::shared_ptr<ScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ScorerAnonymousInnerClass>(
          Scorer::shared_from_this());
    }
  };

private:
  static std::shared_ptr<FixedBitSet> randomSet(int maxDoc);

  static std::shared_ptr<FixedBitSet>
  clearRandomBits(std::shared_ptr<FixedBitSet> other);

  static std::shared_ptr<FixedBitSet>
  intersect(std::deque<std::shared_ptr<FixedBitSet>> &bitSets);

  static std::shared_ptr<FixedBitSet>
  toBitSet(int maxDoc,
           std::shared_ptr<DocIdSetIterator> iterator) ;

  // Test that the conjunction iterator is correct
public:
  virtual void testConjunction() ;

  // Test that the conjunction approximation is correct
  virtual void testConjunctionApproximation() ;

  // This test makes sure that when nesting scorers with ConjunctionDISI,
  // confirmations are pushed to the root.
  virtual void testRecursiveConjunctionApproximation() ;

  virtual void
  testCollapseSubConjunctions(bool wrapWithScorer) ;

  virtual void testCollapseSubConjunctionDISIs() ;

  virtual void testCollapseSubConjunctionScorers() ;

protected:
  std::shared_ptr<TestConjunctionDISI> shared_from_this()
  {
    return std::static_pointer_cast<TestConjunctionDISI>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
