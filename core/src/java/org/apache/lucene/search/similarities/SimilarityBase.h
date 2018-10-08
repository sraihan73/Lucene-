#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class CollectionStatistics;
}

namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::search::similarities
{
class BasicStats;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::search::similarities
{

using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A subclass of {@code Similarity} that provides a simplified API for its
 * descendants. Subclasses are only required to implement the {@link #score}
GET_CLASS_NAME(of)
 * and {@link #toString()} methods. Implementing
 * {@link #explain(List, BasicStats, int, float, float)} is optional,
 * inasmuch as SimilarityBase already provides a basic explanation of the score
 * and the term frequency. However, implementers of a subclass are encouraged to
 * include as much detail about the scoring method as possible.
 * <p>
 * Note: multi-word queries such as phrase queries are scored in a different way
 * than Lucene's default ranking algorithm: whereas it "fakes" an IDF value for
 * the phrase as a whole (since it does not know it), this class instead scores
 * phrases as a summation of the individual term scores.
 * @lucene.experimental
 */
class SimilarityBase : public Similarity
{
  GET_CLASS_NAME(SimilarityBase)
  /** For {@link #log2(double)}. Precomputed for efficiency reasons. */
private:
  static const double LOG_2;

  /**
   * True if overlap tokens (tokens with a position of increment of zero) are
   * discounted from the document's length.
   */
protected:
  bool discountOverlaps = true;

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  SimilarityBase();

  /** Determines whether overlap tokens (Tokens with
   *  0 position increment) are ignored when computing
   *  norm.  By default this is true, meaning overlap
   *  tokens do not count when computing norms.
   *
   *  @lucene.experimental
   *
   *  @see #computeNorm
   */
  virtual void setDiscountOverlaps(bool v);

  /**
   * Returns true if overlap tokens are discounted from the document's length.
   * @see #setDiscountOverlaps
   */
  virtual bool getDiscountOverlaps();

  std::shared_ptr<SimWeight>
  computeWeight(float boost,
                std::shared_ptr<CollectionStatistics> collectionStats,
                std::deque<TermStatistics> &termStats) override final;

  /** Factory method to return a custom stats object */
protected:
  virtual std::shared_ptr<BasicStats> newStats(const std::wstring &field,
                                               float boost);

  /** Fills all member fields defined in {@code BasicStats} in {@code stats}.
   *  Subclasses can override this method to fill additional stats. */
  virtual void
  fillBasicStats(std::shared_ptr<BasicStats> stats,
                 std::shared_ptr<CollectionStatistics> collectionStats,
                 std::shared_ptr<TermStatistics> termStats);

  /**
   * Scores the document {@code doc}.
   * <p>Subclasses must apply their scoring formula in this class.</p>
   * @param stats the corpus level statistics.
   * @param freq the term frequency.
   * @param docLen the document length.
   * @return the score.
   */
  virtual float score(std::shared_ptr<BasicStats> stats, float freq,
                      float docLen) = 0;

  /**
   * Subclasses should implement this method to explain the score. {@code expl}
   * already contains the score, the name of the class and the doc id, as well
   * as the term frequency and its explanation; subclasses can add additional
   * clauses to explain details of their scoring formulae.
   * <p>The default implementation does nothing.</p>
   *
   * @param subExpls the deque of details of the explanation to extend
   * @param stats the corpus level statistics.
   * @param doc the document id.
   * @param freq the term frequency.
   * @param docLen the document length.
   */
  virtual void explain(std::deque<std::shared_ptr<Explanation>> &subExpls,
                       std::shared_ptr<BasicStats> stats, int doc, float freq,
                       float docLen);

  /**
   * Explains the score. The implementation here provides a basic explanation
   * in the format <em>score(name-of-similarity, doc=doc-id,
   * freq=term-frequency), computed from:</em>, and
   * attaches the score (computed via the {@link #score(BasicStats, float,
   * float)} method) and the explanation for the term frequency. Subclasses
   * content with this format may add additional details in
   * {@link #explain(List, BasicStats, int, float, float)}.
   *
   * @param stats the corpus level statistics.
   * @param doc the document id.
   * @param freq the term frequency and its explanation.
   * @param docLen the document length.
   * @return the explanation.
   */
  virtual std::shared_ptr<Explanation>
  explain(std::shared_ptr<BasicStats> stats, int doc,
          std::shared_ptr<Explanation> freq, float docLen);

public:
  std::shared_ptr<SimScorer>
  simScorer(std::shared_ptr<SimWeight> stats,
            std::shared_ptr<LeafReaderContext> context) 
      override final;

  /**
   * Subclasses must override this method to return the name of the Similarity
   * and preferably the values of parameters (if any) as well.
   */
  std::wstring toString() = 0;
  override

      // ------------------------------ Norm handling
      // ------------------------------

      /** Cache of decoded bytes. */
      private : static std::deque<float> const OLD_LENGTH_TABLE;
  static std::deque<float> const LENGTH_TABLE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static SimilarityBase::StaticConstructor staticConstructor;

  /** Encodes the document length in the same way as {@link BM25Similarity}. */
public:
  int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override final;

  // ----------------------------- Static methods ------------------------------

  /** Returns the base two logarithm of {@code x}. */
  static double log2(double x);

  // --------------------------------- Classes ---------------------------------

  /** Delegates the {@link #score(int, float)} and
   * {@link #explain(int, Explanation)} methods to
   * {@link SimilarityBase#score(BasicStats, float, float)} and
   * {@link SimilarityBase#explain(BasicStats, int, Explanation, float)},
   * respectively.
   */
public:
  class BasicSimScorer final : public SimScorer
  {
    GET_CLASS_NAME(BasicSimScorer)
  private:
    std::shared_ptr<SimilarityBase> outerInstance;

    const std::shared_ptr<BasicStats> stats;
    const std::shared_ptr<NumericDocValues> norms;
    std::deque<float> const normCache;

  public:
    BasicSimScorer(std::shared_ptr<SimilarityBase> outerInstance,
                   std::shared_ptr<BasicStats> stats,
                   int indexCreatedVersionMajor,
                   std::shared_ptr<NumericDocValues> norms) ;

    float getLengthValue(int doc) ;

    float score(int doc, float freq)  override;

    std::shared_ptr<Explanation>
    explain(int doc,
            std::shared_ptr<Explanation> freq)  override;

    float computeSlopFactor(int distance) override;

    float computePayloadFactor(int doc, int start, int end,
                               std::shared_ptr<BytesRef> payload) override;

  protected:
    std::shared_ptr<BasicSimScorer> shared_from_this()
    {
      return std::static_pointer_cast<BasicSimScorer>(
          SimScorer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SimilarityBase> shared_from_this()
  {
    return std::static_pointer_cast<SimilarityBase>(
        Similarity::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
