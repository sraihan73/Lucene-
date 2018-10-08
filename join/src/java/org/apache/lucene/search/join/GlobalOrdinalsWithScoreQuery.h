#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/join/GlobalOrdinalsWithScoreCollector.h"

#include  "core/src/java/org/apache/lucene/index/OrdinalMap.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/util/LongValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
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
namespace org::apache::lucene::search::join
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using FilterWeight = org::apache::lucene::search::FilterWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using LongValues = org::apache::lucene::util::LongValues;

class GlobalOrdinalsWithScoreQuery final : public Query
{
  GET_CLASS_NAME(GlobalOrdinalsWithScoreQuery)

private:
  const std::shared_ptr<GlobalOrdinalsWithScoreCollector> collector;
  const std::wstring joinField;
  const std::shared_ptr<OrdinalMap> globalOrds;
  // Is also an approximation of the docs that will match. Can be all docs that
  // have toField or something more specific.
  const std::shared_ptr<Query> toQuery;

  // just for hashcode and equals:
  const ScoreMode scoreMode;
  const std::shared_ptr<Query> fromQuery;
  const int min;
  const int max;
  // id of the context rather than the context itself in order not to hold
  // references to index readers
  const std::any indexReaderContextId;

public:
  GlobalOrdinalsWithScoreQuery(
      std::shared_ptr<GlobalOrdinalsWithScoreCollector> collector,
      ScoreMode scoreMode, const std::wstring &joinField,
      std::shared_ptr<OrdinalMap> globalOrds, std::shared_ptr<Query> toQuery,
      std::shared_ptr<Query> fromQuery, int min, int max,
      std::any indexReaderContextId);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<GlobalOrdinalsWithScoreQuery> other);

public:
  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

public:
  class W final : public FilterWeight
  {
    GET_CLASS_NAME(W)
  private:
    std::shared_ptr<GlobalOrdinalsWithScoreQuery> outerInstance;

  public:
    W(std::shared_ptr<GlobalOrdinalsWithScoreQuery> outerInstance,
      std::shared_ptr<Query> query,
      std::shared_ptr<Weight> approximationWeight);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;
    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<W> shared_from_this()
    {
      return std::static_pointer_cast<W>(
          org.apache.lucene.search.FilterWeight::shared_from_this());
    }
  };

public:
  class OrdinalMapScorer final : public BaseGlobalOrdinalScorer
  {
    GET_CLASS_NAME(OrdinalMapScorer)

  public:
    const std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup;
    const std::shared_ptr<GlobalOrdinalsWithScoreCollector> collector;

    OrdinalMapScorer(
        std::shared_ptr<Weight> weight,
        std::shared_ptr<GlobalOrdinalsWithScoreCollector> collector,
        std::shared_ptr<SortedDocValues> values,
        std::shared_ptr<DocIdSetIterator> approximation,
        std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup);

  protected:
    std::shared_ptr<TwoPhaseIterator> createTwoPhaseIterator(
        std::shared_ptr<DocIdSetIterator> approximation) override;

  private:
    class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<OrdinalMapScorer> outerInstance;

      std::shared_ptr<DocIdSetIterator> approximation;

    public:
      TwoPhaseIteratorAnonymousInnerClass(
          std::shared_ptr<OrdinalMapScorer> outerInstance,
          std::shared_ptr<DocIdSetIterator> approximation);

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
    std::shared_ptr<OrdinalMapScorer> shared_from_this()
    {
      return std::static_pointer_cast<OrdinalMapScorer>(
          BaseGlobalOrdinalScorer::shared_from_this());
    }
  };

public:
  class SegmentOrdinalScorer final : public BaseGlobalOrdinalScorer
  {
    GET_CLASS_NAME(SegmentOrdinalScorer)

  public:
    const std::shared_ptr<GlobalOrdinalsWithScoreCollector> collector;

    SegmentOrdinalScorer(
        std::shared_ptr<Weight> weight,
        std::shared_ptr<GlobalOrdinalsWithScoreCollector> collector,
        std::shared_ptr<SortedDocValues> values,
        std::shared_ptr<DocIdSetIterator> approximation);

  protected:
    std::shared_ptr<TwoPhaseIterator> createTwoPhaseIterator(
        std::shared_ptr<DocIdSetIterator> approximation) override;

  private:
    class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<SegmentOrdinalScorer> outerInstance;

      std::shared_ptr<DocIdSetIterator> approximation;

    public:
      TwoPhaseIteratorAnonymousInnerClass(
          std::shared_ptr<SegmentOrdinalScorer> outerInstance,
          std::shared_ptr<DocIdSetIterator> approximation);

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
    std::shared_ptr<SegmentOrdinalScorer> shared_from_this()
    {
      return std::static_pointer_cast<SegmentOrdinalScorer>(
          BaseGlobalOrdinalScorer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<GlobalOrdinalsWithScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<GlobalOrdinalsWithScoreQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
