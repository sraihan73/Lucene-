#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class LongBitSet;
}

namespace org::apache::lucene::index
{
class OrdinalMap;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::util
{
class LongValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
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
namespace org::apache::lucene::search::join
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using OrdinalMap = org::apache::lucene::index::OrdinalMap;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using Term = org::apache::lucene::index::Term;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using LongBitSet = org::apache::lucene::util::LongBitSet;
using LongValues = org::apache::lucene::util::LongValues;

class GlobalOrdinalsQuery final : public Query
{
  GET_CLASS_NAME(GlobalOrdinalsQuery)

  // All the ords of matching docs found with OrdinalsCollector.
private:
  const std::shared_ptr<LongBitSet> foundOrds;
  const std::wstring joinField;
  const std::shared_ptr<OrdinalMap> globalOrds;
  // Is also an approximation of the docs that will match. Can be all docs that
  // have toField or something more specific.
  const std::shared_ptr<Query> toQuery;

  // just for hashcode and equals:
  const std::shared_ptr<Query> fromQuery;
  // id of the context rather than the context itself in order not to hold
  // references to index readers
  const std::any indexReaderContextId;

public:
  GlobalOrdinalsQuery(std::shared_ptr<LongBitSet> foundOrds,
                      const std::wstring &joinField,
                      std::shared_ptr<OrdinalMap> globalOrds,
                      std::shared_ptr<Query> toQuery,
                      std::shared_ptr<Query> fromQuery,
                      std::any indexReaderContextId);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<GlobalOrdinalsQuery> other);

public:
  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

public:
  class W final : public ConstantScoreWeight
  {
    GET_CLASS_NAME(W)
  private:
    std::shared_ptr<GlobalOrdinalsQuery> outerInstance;

    const std::shared_ptr<Weight> approximationWeight;

  public:
    W(std::shared_ptr<GlobalOrdinalsQuery> outerInstance,
      std::shared_ptr<Query> query, std::shared_ptr<Weight> approximationWeight,
      float boost);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;
    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<W> shared_from_this()
    {
      return std::static_pointer_cast<W>(
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

public:
  class OrdinalMapScorer final : public BaseGlobalOrdinalScorer
  {
    GET_CLASS_NAME(OrdinalMapScorer)

  public:
    const std::shared_ptr<LongBitSet> foundOrds;
    const std::shared_ptr<LongValues> segmentOrdToGlobalOrdLookup;

    OrdinalMapScorer(std::shared_ptr<Weight> weight, float score,
                     std::shared_ptr<LongBitSet> foundOrds,
                     std::shared_ptr<SortedDocValues> values,
                     std::shared_ptr<DocIdSetIterator> approximationScorer,
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
    const std::shared_ptr<LongBitSet> foundOrds;

    SegmentOrdinalScorer(std::shared_ptr<Weight> weight, float score,
                         std::shared_ptr<LongBitSet> foundOrds,
                         std::shared_ptr<SortedDocValues> values,
                         std::shared_ptr<DocIdSetIterator> approximationScorer);

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
  std::shared_ptr<GlobalOrdinalsQuery> shared_from_this()
  {
    return std::static_pointer_cast<GlobalOrdinalsQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::join
