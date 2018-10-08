#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Field.h"

#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"
#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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

using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests the use of indexdocvalues in scoring.
 *
 * In the example, a docvalues field is used as a per-document boost (separate
 * from the norm)
 * @lucene.experimental
 */
class TestDocValuesScoring : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocValuesScoring)
private:
  static constexpr float SCORE_EPSILON = 0.001f; // for comparing floats

public:
  virtual void testSimple() ;

private:
  class PerFieldSimilarityWrapperAnonymousInnerClass
      : public PerFieldSimilarityWrapper
  {
    GET_CLASS_NAME(PerFieldSimilarityWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocValuesScoring> outerInstance;

    std::shared_ptr<Field> field;
    std::shared_ptr<Similarity> base;

  public:
    PerFieldSimilarityWrapperAnonymousInnerClass(
        std::shared_ptr<TestDocValuesScoring> outerInstance,
        std::shared_ptr<Field> field, std::shared_ptr<Similarity> base);

    const std::shared_ptr<Similarity> fooSim;

    std::shared_ptr<Similarity> get(const std::wstring &field) override;

  protected:
    std::shared_ptr<PerFieldSimilarityWrapperAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          PerFieldSimilarityWrapperAnonymousInnerClass>(
          org.apache.lucene.search.similarities
              .PerFieldSimilarityWrapper::shared_from_this());
    }
  };

  /**
   * Similarity that wraps another similarity and boosts the final score
   * according to whats in a docvalues field.
   *
   * @lucene.experimental
   */
public:
  class BoostingSimilarity : public Similarity
  {
    GET_CLASS_NAME(BoostingSimilarity)
  private:
    const std::shared_ptr<Similarity> sim;
    const std::wstring boostField;

  public:
    BoostingSimilarity(std::shared_ptr<Similarity> sim,
                       const std::wstring &boostField);

    int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override;

    std::shared_ptr<Similarity::SimWeight>
    computeWeight(float boost,
                  std::shared_ptr<CollectionStatistics> collectionStats,
                  std::deque<TermStatistics> &termStats) override;

    std::shared_ptr<Similarity::SimScorer> simScorer(
        std::shared_ptr<Similarity::SimWeight> stats,
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class SimScorerAnonymousInnerClass : public SimScorer
    {
      GET_CLASS_NAME(SimScorerAnonymousInnerClass)
    private:
      std::shared_ptr<BoostingSimilarity> outerInstance;

      std::shared_ptr<Similarity::SimScorer> sub;
      std::shared_ptr<NumericDocValues> values;

    public:
      SimScorerAnonymousInnerClass(
          std::shared_ptr<BoostingSimilarity> outerInstance,
          std::shared_ptr<Similarity::SimScorer> sub,
          std::shared_ptr<NumericDocValues> values);

    private:
      float getValueForDoc(int doc) ;

    public:
      float score(int doc, float freq)  override;

      float computeSlopFactor(int distance) override;

      float computePayloadFactor(int doc, int start, int end,
                                 std::shared_ptr<BytesRef> payload) override;

      std::shared_ptr<Explanation>
      explain(int doc,
              std::shared_ptr<Explanation> freq)  override;

    protected:
      std::shared_ptr<SimScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SimScorerAnonymousInnerClass>(
            SimScorer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<BoostingSimilarity> shared_from_this()
    {
      return std::static_pointer_cast<BoostingSimilarity>(
          org.apache.lucene.search.similarities.Similarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDocValuesScoring> shared_from_this()
  {
    return std::static_pointer_cast<TestDocValuesScoring>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
