#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::join
{
class BitSetProducer;
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
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class ScorerSupplier;
}
namespace org::apache::lucene::util
{
class BitSet;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class Matches;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search
{
class TwoPhaseIterator;
}
namespace org::apache::lucene::search
{
class ChildScorer;
}
namespace org::apache::lucene::index
{
class IndexReader;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using FilterWeight = org::apache::lucene::search::FilterWeight;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Matches = org::apache::lucene::search::Matches;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using ScorerSupplier = org::apache::lucene::search::ScorerSupplier;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using BitSet = org::apache::lucene::util::BitSet;

/**
 * This query requires that you index
 * children and parent docs as a single block, using the
 * {@link IndexWriter#addDocuments IndexWriter.addDocuments()} or {@link
 * IndexWriter#updateDocuments IndexWriter.updateDocuments()} API.  In each
 * block, the child documents must appear first, ending with the parent
 * document.  At search time you provide a Filter
 * identifying the parents, however this Filter must provide
 * an {@link BitSet} per sub-reader.
 *
 * <p>Once the block index is built, use this query to wrap
 * any sub-query matching only child docs and join matches in that
 * child document space up to the parent document space.
 * You can then use this Query as a clause with
 * other queries in the parent document space.</p>
 *
 * <p>See {@link ToChildBlockJoinQuery} if you need to join
 * in the reverse order.
 *
 * <p>The child documents must be orthogonal to the parent
 * documents: the wrapped child query must never
 * return a parent document.</p>
 *
 * <p>See {@link org.apache.lucene.search.join} for an
 * overview. </p>
 *
 * @lucene.experimental
 */
class ToParentBlockJoinQuery : public Query
{
  GET_CLASS_NAME(ToParentBlockJoinQuery)

private:
  const std::shared_ptr<BitSetProducer> parentsFilter;
  const std::shared_ptr<Query> childQuery;
  const ScoreMode scoreMode;

  /** Create a ToParentBlockJoinQuery.
   *
   * @param childQuery Query matching child documents.
   * @param parentsFilter Filter identifying the parent documents.
   * @param scoreMode How to aggregate multiple child scores
   * into a single parent score.
   **/
public:
  ToParentBlockJoinQuery(std::shared_ptr<Query> childQuery,
                         std::shared_ptr<BitSetProducer> parentsFilter,
                         ScoreMode scoreMode);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  /** Return our child query. */
  virtual std::shared_ptr<Query> getChildQuery();

private:
  class BlockJoinWeight : public FilterWeight
  {
    GET_CLASS_NAME(BlockJoinWeight)
  private:
    const std::shared_ptr<BitSetProducer> parentsFilter;
    const ScoreMode scoreMode;

  public:
    BlockJoinWeight(std::shared_ptr<Query> joinQuery,
                    std::shared_ptr<Weight> childWeight,
                    std::shared_ptr<BitSetProducer> parentsFilter,
                    ScoreMode scoreMode);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    // NOTE: acceptDocs applies (and is checked) only in the
    // parent document space
    std::shared_ptr<ScorerSupplier> scorerSupplier(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ScorerSupplierAnonymousInnerClass : public ScorerSupplier
    {
      GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
    private:
      std::shared_ptr<BlockJoinWeight> outerInstance;

      std::shared_ptr<ScorerSupplier> childScorerSupplier;
      std::shared_ptr<BitSet> parents;

    public:
      ScorerSupplierAnonymousInnerClass(
          std::shared_ptr<BlockJoinWeight> outerInstance,
          std::shared_ptr<ScorerSupplier> childScorerSupplier,
          std::shared_ptr<BitSet> parents);

      std::shared_ptr<Scorer>
      get(int64_t leadCost)  override;

      int64_t cost() override;

    protected:
      std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
            org.apache.lucene.search.ScorerSupplier::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

  protected:
    std::shared_ptr<BlockJoinWeight> shared_from_this()
    {
      return std::static_pointer_cast<BlockJoinWeight>(
          org.apache.lucene.search.FilterWeight::shared_from_this());
    }
  };

private:
  class ParentApproximation : public DocIdSetIterator
  {
    GET_CLASS_NAME(ParentApproximation)

  private:
    const std::shared_ptr<DocIdSetIterator> childApproximation;
    const std::shared_ptr<BitSet> parentBits;
    int doc = -1;

  public:
    ParentApproximation(std::shared_ptr<DocIdSetIterator> childApproximation,
                        std::shared_ptr<BitSet> parentBits);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<ParentApproximation> shared_from_this()
    {
      return std::static_pointer_cast<ParentApproximation>(
          org.apache.lucene.search.DocIdSetIterator::shared_from_this());
    }
  };

private:
  class ParentTwoPhase : public TwoPhaseIterator
  {
    GET_CLASS_NAME(ParentTwoPhase)

  private:
    const std::shared_ptr<ParentApproximation> parentApproximation;
    const std::shared_ptr<DocIdSetIterator> childApproximation;
    const std::shared_ptr<TwoPhaseIterator> childTwoPhase;

  public:
    ParentTwoPhase(std::shared_ptr<ParentApproximation> parentApproximation,
                   std::shared_ptr<TwoPhaseIterator> childTwoPhase);

    bool matches()  override;

    float matchCost() override;

  protected:
    std::shared_ptr<ParentTwoPhase> shared_from_this()
    {
      return std::static_pointer_cast<ParentTwoPhase>(
          org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
    }
  };

public:
  class BlockJoinScorer : public Scorer
  {
    GET_CLASS_NAME(BlockJoinScorer)
  private:
    const std::shared_ptr<Scorer> childScorer;
    const std::shared_ptr<BitSet> parentBits;
    const ScoreMode scoreMode;
    const std::shared_ptr<DocIdSetIterator> childApproximation;
    const std::shared_ptr<TwoPhaseIterator> childTwoPhase;
    const std::shared_ptr<ParentApproximation> parentApproximation;
    const std::shared_ptr<ParentTwoPhase> parentTwoPhase;
    // C++ NOTE: Fields cannot have the same name as methods:
    float score_ = 0;

  public:
    BlockJoinScorer(std::shared_ptr<Weight> weight,
                    std::shared_ptr<Scorer> childScorer,
                    std::shared_ptr<BitSet> parentBits, ScoreMode scoreMode);

    std::shared_ptr<std::deque<std::shared_ptr<Scorer::ChildScorer>>>
    getChildren() override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

    std::shared_ptr<TwoPhaseIterator> twoPhaseIterator() override;

    int docID() override;

    float score()  override;

  private:
    void setScoreAndFreq() ;

  public:
    virtual std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            std::shared_ptr<Weight> childWeight) ;

  protected:
    std::shared_ptr<BlockJoinScorer> shared_from_this()
    {
      return std::static_pointer_cast<BlockJoinScorer>(
          org.apache.lucene.search.Scorer::shared_from_this());
    }
  };

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<ToParentBlockJoinQuery> other);

public:
  virtual int hashCode();

protected:
  std::shared_ptr<ToParentBlockJoinQuery> shared_from_this()
  {
    return std::static_pointer_cast<ToParentBlockJoinQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::join
