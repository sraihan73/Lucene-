#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/facet/DrillSideways.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/sortedset/SortedSetDocValuesReaderState.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include  "core/src/java/org/apache/lucene/facet/Facets.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/facet/DrillSidewaysResult.h"

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
namespace org::apache::lucene::facet
{

using DrillSidewaysResult =
    org::apache::lucene::facet::DrillSideways::DrillSidewaysResult;
using SortedSetDocValuesReaderState =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesReaderState;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

class TestDrillSideways : public FacetTestCase
{
  GET_CLASS_NAME(TestDrillSideways)

protected:
  virtual std::shared_ptr<DrillSideways>
  getNewDrillSideways(std::shared_ptr<IndexSearcher> searcher,
                      std::shared_ptr<FacetsConfig> config,
                      std::shared_ptr<SortedSetDocValuesReaderState> state);

  virtual std::shared_ptr<DrillSideways>
  getNewDrillSideways(std::shared_ptr<IndexSearcher> searcher,
                      std::shared_ptr<FacetsConfig> config,
                      std::shared_ptr<TaxonomyReader> taxoReader);

  virtual std::shared_ptr<DrillSideways> getNewDrillSidewaysScoreSubdocsAtOnce(
      std::shared_ptr<IndexSearcher> searcher,
      std::shared_ptr<FacetsConfig> config,
      std::shared_ptr<TaxonomyReader> taxoReader);

private:
  class DrillSidewaysAnonymousInnerClass : public DrillSideways
  {
    GET_CLASS_NAME(DrillSidewaysAnonymousInnerClass)
  private:
    std::shared_ptr<TestDrillSideways> outerInstance;

  public:
    DrillSidewaysAnonymousInnerClass(
        std::shared_ptr<TestDrillSideways> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<org::apache::lucene::facet::FacetsConfig> config,
        std::shared_ptr<TaxonomyReader> taxoReader);

  protected:
    bool scoreSubDocsAtOnce() override;

  protected:
    std::shared_ptr<DrillSidewaysAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DrillSidewaysAnonymousInnerClass>(
          DrillSideways::shared_from_this());
    }
  };

protected:
  virtual std::shared_ptr<DrillSideways> getNewDrillSidewaysBuildFacetsResult(
      std::shared_ptr<IndexSearcher> searcher,
      std::shared_ptr<FacetsConfig> config,
      std::shared_ptr<TaxonomyReader> taxoReader);

private:
  class DrillSidewaysAnonymousInnerClass2 : public DrillSideways
  {
    GET_CLASS_NAME(DrillSidewaysAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDrillSideways> outerInstance;

    std::shared_ptr<org::apache::lucene::facet::FacetsConfig> config;
    std::shared_ptr<TaxonomyReader> taxoReader;

  public:
    DrillSidewaysAnonymousInnerClass2(
        std::shared_ptr<TestDrillSideways> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<org::apache::lucene::facet::FacetsConfig> config,
        std::shared_ptr<TaxonomyReader> taxoReader);

  protected:
    std::shared_ptr<Facets> buildFacetsResult(
        std::shared_ptr<FacetsCollector> drillDowns,
        std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
        std::deque<std::wstring> &drillSidewaysDims) 
        override;

  protected:
    std::shared_ptr<DrillSidewaysAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DrillSidewaysAnonymousInnerClass2>(
          DrillSideways::shared_from_this());
    }
  };

public:
  virtual void testBasic() ;

  virtual void testSometimesInvalidDrillDown() ;

  virtual void testMultipleRequestsPerDim() ;

private:
  class Doc : public std::enable_shared_from_this<Doc>,
              public Comparable<std::shared_ptr<Doc>>
  {
    GET_CLASS_NAME(Doc)
  public:
    std::wstring id;
    std::wstring contentToken;

    Doc();

    // -1 if the doc is missing this dim, else the index
    // -into the values for this dim:
    std::deque<int> dims;

    // 2nd value per dim for the doc (so we test
    // multi-valued fields):
    std::deque<int> dims2;
    bool deleted = false;

    int compareTo(std::shared_ptr<Doc> other) override;
  };

private:
  double aChance = 0, bChance = 0, cChance = 0;

  std::wstring randomContentToken(bool isQuery);

public:
  virtual void testRandom() ;

private:
  class QueryAnonymousInnerClass : public Query
  {
    GET_CLASS_NAME(QueryAnonymousInnerClass)
  private:
    std::shared_ptr<TestDrillSideways> outerInstance;

  public:
    QueryAnonymousInnerClass(std::shared_ptr<TestDrillSideways> outerInstance);

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<QueryAnonymousInnerClass> outerInstance;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<QueryAnonymousInnerClass> outerInstance, float boost);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    private:
      class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
      {
        GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
      private:
        std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

        std::shared_ptr<LeafReaderContext> context;
        std::shared_ptr<DocIdSetIterator> approximation;

      public:
        TwoPhaseIteratorAnonymousInnerClass(
            std::shared_ptr<ConstantScoreWeightAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<DocIdSetIterator> approximation,
            std::shared_ptr<LeafReaderContext> context);

        bool matches()  override;

        float matchCost() override;

      protected:
        std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
              org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
        }
      };

    public:
      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
            org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
      }
    };

  public:
    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::wstring Term::toString(const std::wstring &field) override;

    virtual bool equals(std::any o);

    virtual int hashCode();

  protected:
    std::shared_ptr<QueryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<QueryAnonymousInnerClass>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestDrillSideways> outerInstance;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<TestDrillSideways> outerInstance);

    int lastDocID = 0;

    void collect(int doc) override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

private:
  class Counters : public std::enable_shared_from_this<Counters>
  {
    GET_CLASS_NAME(Counters)
  public:
    std::deque<std::deque<int>> counts;

    Counters(std::deque<std::deque<std::wstring>> &dimValues);

    virtual void inc(std::deque<int> &dims, std::deque<int> &dims2);

    virtual void inc(std::deque<int> &dims, std::deque<int> &dims2,
                     int onlyDim);
  };

private:
  class TestFacetResult : public std::enable_shared_from_this<TestFacetResult>
  {
    GET_CLASS_NAME(TestFacetResult)
  public:
    std::deque<std::shared_ptr<Doc>> hits;
    std::deque<std::deque<int>> counts;
    std::deque<int> uniqueCounts;

    TestFacetResult();
  };

private:
  std::deque<int> getTopNOrds(std::deque<int> &counts,
                               std::deque<std::wstring> &values, int topN);

private:
  class InPlaceMergeSorterAnonymousInnerClass : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(InPlaceMergeSorterAnonymousInnerClass)
  private:
    std::shared_ptr<TestDrillSideways> outerInstance;

    std::deque<int> counts;
    std::deque<std::wstring> values;
    std::deque<int> ids;

  public:
    InPlaceMergeSorterAnonymousInnerClass(
        std::shared_ptr<TestDrillSideways> outerInstance,
        std::deque<int> &counts, std::deque<std::wstring> &values,
        std::deque<int> &ids);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

  protected:
    std::shared_ptr<InPlaceMergeSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<InPlaceMergeSorterAnonymousInnerClass>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

private:
  std::shared_ptr<TestFacetResult> slowDrillSidewaysSearch(
      std::shared_ptr<IndexSearcher> s, std::deque<std::shared_ptr<Doc>> &docs,
      const std::wstring &contentToken,
      std::deque<std::deque<std::wstring>> &drillDowns,
      std::deque<std::deque<std::wstring>> &dimValues,
      std::shared_ptr<Query> onlyEven) ;

public:
  virtual void verifyEquals(std::deque<std::deque<std::wstring>> &dimValues,
                            std::shared_ptr<IndexSearcher> s,
                            std::shared_ptr<TestFacetResult> expected,
                            std::shared_ptr<DrillSidewaysResult> actual,
                            std::unordered_map<std::wstring, float> &scores,
                            bool isSortedSetDV) ;

  virtual void testEmptyIndex() ;

  virtual void testScorer() ;

protected:
  std::shared_ptr<TestDrillSideways> shared_from_this()
  {
    return std::static_pointer_cast<TestDrillSideways>(
        FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
