#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"

#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"
#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/search/BooleanWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::search::similarities
{

using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;

/**
 * Simple similarity that gives terms a score that is equal to their query
 * boost. This similarity is typically used with disabled norms since neither
 * document statistics nor index statistics are used for scoring. That said,
 * if norms are enabled, they will be computed the same way as
 * {@link SimilarityBase} and {@link BM25Similarity} with
 * {@link SimilarityBase#setDiscountOverlaps(bool) discounted overlaps}
 * so that the {@link Similarity} can be changed after the index has been
 * created.
 */
class BooleanSimilarity : public Similarity
{
  GET_CLASS_NAME(BooleanSimilarity)

private:
  static const std::shared_ptr<Similarity> BM25_SIM;

  /** Sole constructor */
public:
  BooleanSimilarity();

  int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

  std::shared_ptr<SimWeight>
  computeWeight(float boost,
                std::shared_ptr<CollectionStatistics> collectionStats,
                std::deque<TermStatistics> &termStats) override;

private:
  class BooleanWeight : public SimWeight
  {
    GET_CLASS_NAME(BooleanWeight)
  public:
    const float boost;

    BooleanWeight(float boost);

  protected:
    std::shared_ptr<BooleanWeight> shared_from_this()
    {
      return std::static_pointer_cast<BooleanWeight>(
          SimWeight::shared_from_this());
    }
  };

public:
  std::shared_ptr<SimScorer> simScorer(
      std::shared_ptr<SimWeight> weight,
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class SimScorerAnonymousInnerClass : public SimScorer
  {
    GET_CLASS_NAME(SimScorerAnonymousInnerClass)
  private:
    std::shared_ptr<BooleanSimilarity> outerInstance;

    float boost = 0;

  public:
    SimScorerAnonymousInnerClass(
        std::shared_ptr<BooleanSimilarity> outerInstance, float boost);

    float score(int doc, float freq)  override;

    std::shared_ptr<Explanation>
    explain(int doc,
            std::shared_ptr<Explanation> freq)  override;

    float computeSlopFactor(int distance) override;

    float computePayloadFactor(int doc, int start, int end,
                               std::shared_ptr<BytesRef> payload) override;

  protected:
    std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
          SimScorer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<BooleanSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<BooleanSimilarity>(
        Similarity::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
