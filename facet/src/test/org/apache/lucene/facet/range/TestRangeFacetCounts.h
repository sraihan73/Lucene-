#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/facet/Facets.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"
#include  "core/src/java/org/apache/lucene/facet/range/DoubleRange.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"

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
namespace org::apache::lucene::facet::range
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

class TestRangeFacetCounts : public FacetTestCase
{
  GET_CLASS_NAME(TestRangeFacetCounts)

public:
  virtual void testBasicLong() ;

  virtual void testLongGetAllDims() ;

  virtual void testUselessRange();

  virtual void testLongMinMax() ;

  virtual void testOverlappedEndStart() ;

  /** Tests single request that mixes Range and non-Range
   *  faceting, with DrillSideways and taxonomy. */
  virtual void testMixedRangeAndNonRangeTaxonomy() ;

private:
  class DrillSidewaysAnonymousInnerClass : public DrillSideways
  {
    GET_CLASS_NAME(DrillSidewaysAnonymousInnerClass)
  private:
    std::shared_ptr<TestRangeFacetCounts> outerInstance;

    std::shared_ptr<FacetsConfig> config;

  public:
    DrillSidewaysAnonymousInnerClass(
        std::shared_ptr<TestRangeFacetCounts> outerInstance,
        std::shared_ptr<IndexSearcher> s, std::shared_ptr<FacetsConfig> config);

  protected:
    std::shared_ptr<Facets> buildFacetsResult(
        std::shared_ptr<FacetsCollector> drillDowns,
        std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
        std::deque<std::wstring> &drillSidewaysDims) 
        override;

    bool scoreSubDocsAtOnce() override;

  protected:
    std::shared_ptr<DrillSidewaysAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DrillSidewaysAnonymousInnerClass>(
          org.apache.lucene.facet.DrillSideways::shared_from_this());
    }
  };

public:
  virtual void testBasicDouble() ;

  virtual void testRandomLongs() ;

  virtual void testRandomDoubles() ;

  // LUCENE-5178
  virtual void testMissingValues() ;

private:
  class UsedQuery : public Query
  {
    GET_CLASS_NAME(UsedQuery)

  private:
    const std::shared_ptr<AtomicBoolean> used;
    const std::shared_ptr<Query> in_;

  public:
    UsedQuery(std::shared_ptr<Query> in_, std::shared_ptr<AtomicBoolean> used);

    bool equals(std::any other) override;

    virtual int hashCode();

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class FilterWeightAnonymousInnerClass : public FilterWeight
    {
      GET_CLASS_NAME(FilterWeightAnonymousInnerClass)
    private:
      std::shared_ptr<UsedQuery> outerInstance;

      std::shared_ptr<Weight> in_;

    public:
      FilterWeightAnonymousInnerClass(std::shared_ptr<UsedQuery> outerInstance,
                                      std::shared_ptr<Weight> in_);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    protected:
      std::shared_ptr<FilterWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterWeightAnonymousInnerClass>(
            org.apache.lucene.search.FilterWeight::shared_from_this());
      }
    };

  public:
    std::wstring toString(const std::wstring &field) override;

  protected:
    std::shared_ptr<UsedQuery> shared_from_this()
    {
      return std::static_pointer_cast<UsedQuery>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

private:
  class PlusOneValuesSource : public DoubleValuesSource
  {
    GET_CLASS_NAME(PlusOneValuesSource)

  public:
    std::shared_ptr<DoubleValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class DoubleValuesAnonymousInnerClass : public DoubleValues
    {
      GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
    private:
      std::shared_ptr<PlusOneValuesSource> outerInstance;

    public:
      DoubleValuesAnonymousInnerClass(
          std::shared_ptr<PlusOneValuesSource> outerInstance);

      int doc = 0;
      double doubleValue()  override;

      bool advanceExact(int doc)  override;

    protected:
      std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
            org.apache.lucene.search.DoubleValues::shared_from_this());
      }
    };

  public:
    bool needsScores() override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
            std::shared_ptr<Explanation> scoreExplanation) 
        override;

    std::shared_ptr<DoubleValuesSource> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

    virtual int hashCode();

    bool equals(std::any obj) override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<PlusOneValuesSource> shared_from_this()
    {
      return std::static_pointer_cast<PlusOneValuesSource>(
          org.apache.lucene.search.DoubleValuesSource::shared_from_this());
    }
  };

public:
  virtual void testCustomDoubleValuesSource() ;

private:
  class DrillSidewaysAnonymousInnerClass : public DrillSideways
  {
    GET_CLASS_NAME(DrillSidewaysAnonymousInnerClass)
  private:
    std::shared_ptr<TestRangeFacetCounts> outerInstance;

    std::shared_ptr<DoubleValuesSource> vs;
    std::deque<std::shared_ptr<org::apache::lucene::facet::range::DoubleRange>>
        ranges;
    std::shared_ptr<Query> fastMatchFilter;

  public:
    DrillSidewaysAnonymousInnerClass(
        std::shared_ptr<TestRangeFacetCounts> outerInstance,
        std::shared_ptr<IndexSearcher> s, std::shared_ptr<FacetsConfig> config,
        std::shared_ptr<TaxonomyReader> org);

  protected:
    std::shared_ptr<Facets> buildFacetsResult(
        std::shared_ptr<FacetsCollector> drillDowns,
        std::deque<std::shared_ptr<FacetsCollector>> &drillSideways,
        std::deque<std::wstring> &drillSidewaysDims) 
        override;

    bool scoreSubDocsAtOnce() override;

  protected:
    std::shared_ptr<DrillSidewaysAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DrillSidewaysAnonymousInnerClass>(
          org.apache.lucene.facet.DrillSideways::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestRangeFacetCounts> shared_from_this()
  {
    return std::static_pointer_cast<TestRangeFacetCounts>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/range/
