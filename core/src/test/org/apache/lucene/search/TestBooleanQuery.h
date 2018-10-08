#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class BooleanQuery;
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBooleanQuery : public LuceneTestCase
{
  GET_CLASS_NAME(TestBooleanQuery)

public:
  virtual void testEquality() ;

  virtual void testEqualityDoesNotDependOnOrder();

  virtual void testEqualityOnDuplicateShouldClauses();

  virtual void testEqualityOnDuplicateMustClauses();

  virtual void testEqualityOnDuplicateFilterClauses();

  virtual void testEqualityOnDuplicateMustNotClauses();

  virtual void testHashCodeIsStable();

  virtual void testException();

  // LUCENE-1630
  virtual void testNullOrSubScorer() ;

  virtual void testDeMorgan() ;

  virtual void testBS2DisjunctionNextVsAdvance() ;

  // LUCENE-4477 / LUCENE-4401:
  virtual void testBooleanSpanQuery() ;

  virtual void testMinShouldMatchLeniency() ;

private:
  static std::shared_ptr<BitSet>
  getMatches(std::shared_ptr<IndexSearcher> searcher,
             std::shared_ptr<Query> query) ;

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<BitSet> set;

  public:
    SimpleCollectorAnonymousInnerClass(std::shared_ptr<BitSet> set);

    int docBase = 0;
    bool needsScores() override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void collect(int doc)  override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testFILTERClauseBehavesLikeMUST() ;

private:
  void assertSameScoresWithoutFilters(
      std::shared_ptr<IndexSearcher> searcher,
      std::shared_ptr<BooleanQuery> bq) ;

private:
  class SimpleCollectorAnonymousInnerClass2 : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestBooleanQuery> outerInstance;

    std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher;
    std::shared_ptr<org::apache::lucene::search::BooleanQuery> bq2;
    std::shared_ptr<AtomicBoolean> matched;

  public:
    SimpleCollectorAnonymousInnerClass2(
        std::shared_ptr<TestBooleanQuery> outerInstance,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        std::shared_ptr<org::apache::lucene::search::BooleanQuery> bq2,
        std::shared_ptr<AtomicBoolean> matched);

    int docBase = 0;
    std::shared_ptr<Scorer> scorer;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

    void setScorer(std::shared_ptr<Scorer> scorer)  override;

    void collect(int doc)  override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass2>(
          SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void testFilterClauseDoesNotImpactScore() ;

  virtual void testConjunctionPropagatesApproximations() ;

  virtual void testDisjunctionPropagatesApproximations() ;

  virtual void testBoostedScorerPropagatesApproximations() ;

  virtual void testExclusionPropagatesApproximations() ;

  virtual void testReqOptPropagatesApproximations() ;

  virtual void testToString();

  virtual void testExtractTerms() ;

protected:
  std::shared_ptr<TestBooleanQuery> shared_from_this()
  {
    return std::static_pointer_cast<TestBooleanQuery>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
