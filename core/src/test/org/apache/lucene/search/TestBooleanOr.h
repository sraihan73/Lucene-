#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class TermQuery;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::search
{
class BulkScorer;
}
namespace org::apache::lucene::search
{
class FakeScorer;
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
class BooleanScorer;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBooleanOr : public LuceneTestCase
{
  GET_CLASS_NAME(TestBooleanOr)

private:
  static std::wstring FIELD_T;
  static std::wstring FIELD_C;

  std::shared_ptr<TermQuery> t1 =
      std::make_shared<TermQuery>(std::make_shared<Term>(FIELD_T, L"files"));
  std::shared_ptr<TermQuery> t2 =
      std::make_shared<TermQuery>(std::make_shared<Term>(FIELD_T, L"deleting"));
  std::shared_ptr<TermQuery> c1 = std::make_shared<TermQuery>(
      std::make_shared<Term>(FIELD_C, L"production"));
  std::shared_ptr<TermQuery> c2 =
      std::make_shared<TermQuery>(std::make_shared<Term>(FIELD_C, L"optimize"));

  std::shared_ptr<IndexSearcher> searcher = nullptr;
  std::shared_ptr<Directory> dir;
  std::shared_ptr<IndexReader> reader;

  int64_t search(std::shared_ptr<Query> q) ;

public:
  virtual void testElements() ;

  /**
   * <code>T:files T:deleting C:production C:optimize </code>
   * it works.
   */
  virtual void testFlat() ;

  /**
   * <code>(T:files T:deleting) (+C:production +C:optimize)</code>
   * it works.
   */
  virtual void testParenthesisMust() ;

  /**
   * <code>(T:files T:deleting) +(C:production C:optimize)</code>
   * not working. results NO HIT.
   */
  virtual void testParenthesisMust2() ;

  /**
   * <code>(T:files T:deleting) (C:production C:optimize)</code>
   * not working. results NO HIT.
   */
  virtual void testParenthesisShould() ;

  void setUp()  override;

  void tearDown()  override;

  virtual void testBooleanScorerMax() ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestBooleanOr> outerInstance;

    std::shared_ptr<FixedBitSet> hits;
    std::shared_ptr<AtomicInteger> end;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestBooleanOr> outerInstance,
        std::shared_ptr<FixedBitSet> hits, std::shared_ptr<AtomicInteger> end);

    void collect(int doc) override;

    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

private:
  static std::shared_ptr<BulkScorer> scorer(std::deque<int> &matches);

private:
  class BulkScorerAnonymousInnerClass : public BulkScorer
  {
    GET_CLASS_NAME(BulkScorerAnonymousInnerClass)
  public:
    BulkScorerAnonymousInnerClass();

    const std::shared_ptr<FakeScorer> scorer = std::make_shared<FakeScorer>();
    int i = 0;
    int score(std::shared_ptr<LeafCollector> collector,
              std::shared_ptr<Bits> acceptDocs, int min,
              int max)  override;
    int64_t cost() override;

  protected:
    std::shared_ptr<BulkScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BulkScorerAnonymousInnerClass>(
          BulkScorer::shared_from_this());
    }
  };

  // Make sure that BooleanScorer keeps working even if the sub clauses return
  // next matching docs which are less than the actual next match
public:
  virtual void testSubScorerNextIsNotMatch() ;

private:
  class LeafCollectorAnonymousInnerClass
      : public std::enable_shared_from_this<LeafCollectorAnonymousInnerClass>,
        public LeafCollector
  {
    GET_CLASS_NAME(LeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestBooleanOr> outerInstance;

    std::shared_ptr<org::apache::lucene::search::BooleanScorer> scorer;
    std::deque<int> matches;

  public:
    LeafCollectorAnonymousInnerClass(
        std::shared_ptr<TestBooleanOr> outerInstance,
        std::shared_ptr<org::apache::lucene::search::BooleanScorer> scorer,
        std::deque<int> &matches);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;
    void collect(int doc)  override;
  };

protected:
  std::shared_ptr<TestBooleanOr> shared_from_this()
  {
    return std::static_pointer_cast<TestBooleanOr>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
