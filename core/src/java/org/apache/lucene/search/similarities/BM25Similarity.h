#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::search
{
class CollectionStatistics;
}
namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class TermStatistics;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::similarities
{
class BM25Stats;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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
 * BM25 Similarity. Introduced in Stephen E. Robertson, Steve Walker,
 * Susan Jones, Micheline Hancock-Beaulieu, and Mike Gatford. Okapi at TREC-3.
 * In Proceedings of the Third <b>T</b>ext <b>RE</b>trieval <b>C</b>onference
 * (TREC 1994). Gaithersburg, USA, November 1994.
 */
class BM25Similarity : public Similarity
{
  GET_CLASS_NAME(BM25Similarity)
private:
  const float k1;
  const float b;

  /**
   * BM25 with the supplied parameter values.
   * @param k1 Controls non-linear term frequency normalization (saturation).
   * @param b Controls to what degree document length normalizes tf values.
   * @throws IllegalArgumentException if {@code k1} is infinite or negative, or
   * if {@code b} is not within the range {@code [0..1]}
   */
public:
  BM25Similarity(float k1, float b);

  /** BM25 with these default values:
   * <ul>
   *   <li>{@code k1 = 1.2}</li>
   *   <li>{@code b = 0.75}</li>
   * </ul>
   */
  BM25Similarity();

  /** Implemented as <code>log(1 + (docCount - docFreq + 0.5)/(docFreq +
   * 0.5))</code>. */
protected:
  virtual float idf(int64_t docFreq, int64_t docCount);

  /** Implemented as <code>1 / (distance + 1)</code>. */
  virtual float sloppyFreq(int distance);

  /** The default implementation returns <code>1</code> */
  virtual float scorePayload(int doc, int start, int end,
                             std::shared_ptr<BytesRef> payload);

  /** The default implementation computes the average as <code>sumTotalTermFreq
   * / docCount</code> */
  virtual float
  avgFieldLength(std::shared_ptr<CollectionStatistics> collectionStats);

  /**
   * True if overlap tokens (tokens with a position of increment of zero) are
   * discounted from the document's length.
   */
  bool discountOverlaps = true;

  /** Sets whether overlap tokens (Tokens with 0 position increment) are
   *  ignored when computing norm.  By default this is true, meaning overlap
   *  tokens do not count when computing norms. */
public:
  virtual void setDiscountOverlaps(bool v);

  /**
   * Returns true if overlap tokens are discounted from the document's length.
   * @see #setDiscountOverlaps
   */
  virtual bool getDiscountOverlaps();

  /** Cache of decoded bytes. */
private:
  static std::deque<float> const OLD_LENGTH_TABLE;
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
  static BM25Similarity::StaticConstructor staticConstructor;

public:
  int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override final;

  /**
   * Computes a score factor for a simple term and returns an explanation
   * for that score factor.
   *
   * <p>
   * The default implementation uses:
   *
   * <pre class="prettyprint">
   * idf(docFreq, docCount);
   * </pre>
   *
   * Note that {@link CollectionStatistics#docCount()} is used instead of
   * {@link org.apache.lucene.index.IndexReader#numDocs() IndexReader#numDocs()}
   because also
   * {@link TermStatistics#docFreq()} is used, and when the latter
   * is inaccurate, so is {@link CollectionStatistics#docCount()}, and in the
   same direction.
   * In addition, {@link CollectionStatistics#docCount()} does not skew when
   fields are sparse.
   *
   * @param collectionStats collection-level statistics
   * @param termStats term-level statistics for the term
   * @return an Explain object that includes both an idf score factor
             and an explanation for the term.
   */
  virtual std::shared_ptr<Explanation>
  idfExplain(std::shared_ptr<CollectionStatistics> collectionStats,
             std::shared_ptr<TermStatistics> termStats);

  /**
   * Computes a score factor for a phrase.
   *
   * <p>
   * The default implementation sums the idf factor for
   * each term in the phrase.
   *
   * @param collectionStats collection-level statistics
   * @param termStats term-level statistics for the terms in the phrase
   * @return an Explain object that includes both an idf
   *         score factor for the phrase and an explanation
   *         for each term.
   */
  virtual std::shared_ptr<Explanation>
  idfExplain(std::shared_ptr<CollectionStatistics> collectionStats,
             std::deque<std::shared_ptr<TermStatistics>> &termStats);

  std::shared_ptr<SimWeight>
  computeWeight(float boost,
                std::shared_ptr<CollectionStatistics> collectionStats,
                std::deque<TermStatistics> &termStats) override final;

  std::shared_ptr<SimScorer>
  simScorer(std::shared_ptr<SimWeight> stats,
            std::shared_ptr<LeafReaderContext> context) 
      override final;

private:
  class BM25DocScorer : public SimScorer
  {
    GET_CLASS_NAME(BM25DocScorer)
  private:
    std::shared_ptr<BM25Similarity> outerInstance;

    const std::shared_ptr<BM25Stats> stats;
    const float weightValue; // boost * idf * (k1 + 1)
    const std::shared_ptr<NumericDocValues> norms;
    /** precomputed cache for all length values */
    std::deque<float> const lengthCache;
    /** precomputed norm[256] with k1 * ((1 - b) + b * dl / avgdl) */
    std::deque<float> const cache;

  public:
    BM25DocScorer(std::shared_ptr<BM25Similarity> outerInstance,
                  std::shared_ptr<BM25Stats> stats,
                  int indexCreatedVersionMajor,
                  std::shared_ptr<NumericDocValues> norms) ;

    float score(int doc, float freq)  override;

    std::shared_ptr<Explanation>
    explain(int doc,
            std::shared_ptr<Explanation> freq)  override;

    float computeSlopFactor(int distance) override;

    float computePayloadFactor(int doc, int start, int end,
                               std::shared_ptr<BytesRef> payload) override;

  protected:
    std::shared_ptr<BM25DocScorer> shared_from_this()
    {
      return std::static_pointer_cast<BM25DocScorer>(
          SimScorer::shared_from_this());
    }
  };

  /** std::deque statistics for the BM25 model. */
private:
  class BM25Stats : public SimWeight
  {
    GET_CLASS_NAME(BM25Stats)
    /** BM25's idf */
  private:
    const std::shared_ptr<Explanation> idf;
    /** The average document length. */
    const float avgdl;
    /** query boost */
    const float boost;
    /** weight (idf * boost) */
    const float weight;
    /** field name, for pulling norms */
    const std::wstring field;
    /** precomputed norm[256] with k1 * ((1 - b) + b * dl / avgdl)
     *  for both OLD_LENGTH_TABLE and LENGTH_TABLE */
    std::deque<float> const oldCache, cache;

  public:
    BM25Stats(const std::wstring &field, float boost,
              std::shared_ptr<Explanation> idf, float avgdl,
              std::deque<float> &oldCache, std::deque<float> &cache);

  protected:
    std::shared_ptr<BM25Stats> shared_from_this()
    {
      return std::static_pointer_cast<BM25Stats>(SimWeight::shared_from_this());
    }
  };

private:
  std::shared_ptr<Explanation>
  explainTFNorm(int doc, std::shared_ptr<Explanation> freq,
                std::shared_ptr<BM25Stats> stats,
                std::shared_ptr<NumericDocValues> norms,
                std::deque<float> &lengthCache) ;

  std::shared_ptr<Explanation>
  explainScore(int doc, std::shared_ptr<Explanation> freq,
               std::shared_ptr<BM25Stats> stats,
               std::shared_ptr<NumericDocValues> norms,
               std::deque<float> &lengthCache) ;

public:
  virtual std::wstring toString();

  /**
   * Returns the <code>k1</code> parameter
   * @see #BM25Similarity(float, float)
   */
  float getK1();

  /**
   * Returns the <code>b</code> parameter
   * @see #BM25Similarity(float, float)
   */
  float getB();

protected:
  std::shared_ptr<BM25Similarity> shared_from_this()
  {
    return std::static_pointer_cast<BM25Similarity>(
        Similarity::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
