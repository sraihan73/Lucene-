#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"

#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
//#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
//#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
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
namespace org::apache::lucene::search::similarities
{

using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Similarity defines the components of Lucene scoring.
 * <p>
 * Expert: Scoring API.
 * <p>
 * This is a low-level API, you should only extend this API if you want to
 * implement an information retrieval <i>model</i>.  If you are instead looking
 * for a convenient way to alter Lucene's scoring, consider extending a
 * higher-level implementation such as {@link TFIDFSimilarity}, which implements
 * the deque space model with this API, or just tweaking the default
 * implementation: {@link BM25Similarity}. <p> Similarity determines how Lucene
 * weights terms, and Lucene interacts with this class at both <a
 * href="#indextime">index-time</a> and <a href="#querytime">query-time</a>. <p>
 * <a name="indextime">Indexing Time</a>
 * At indexing time, the indexer calls {@link #computeNorm(FieldInvertState)},
 * allowing the Similarity implementation to set a per-document value for the
 * field that will be later accessible via {@link
 * org.apache.lucene.index.LeafReader#getNormValues(std::wstring)}.  Lucene makes no
 * assumption about what is in this norm, but it is most useful for encoding
 * length normalization information. <p> Implementations should carefully
 * consider how the normalization is encoded: while Lucene's {@link
 * BM25Similarity} encodes a combination of index-time boost and length
 * normalization information with {@link SmallFloat} into a single byte, this
 * might not be suitable for all purposes.
 * <p>
 * Many formulas require the use of average document length, which can be
 * computed via a combination of {@link CollectionStatistics#sumTotalTermFreq()}
 * and
 * {@link CollectionStatistics#maxDoc()} or {@link
 * CollectionStatistics#docCount()}, depending upon whether the average should
 * reflect field sparsity. <p> Additional scoring factors can be stored in named
 * <code>NumericDocValuesField</code>s and accessed
 * at query-time with {@link
 * org.apache.lucene.index.LeafReader#getNumericDocValues(std::wstring)}. <p> Finally,
 * using index-time boosts (either via folding into the normalization byte or
 * via DocValues), is an inefficient way to boost the scores of different fields
 * if the boost will be the same for every document, instead the Similarity can
 * simply take a constant boost parameter <i>C</i>, and {@link
 * PerFieldSimilarityWrapper} can return different instances with different
 * boosts depending upon field name. <p> <a name="querytime">Query time</a> At
 * query-time, Queries interact with the Similarity via these steps: <ol>
 *   <li>The {@link #computeWeight(float, CollectionStatistics,
 * TermStatistics...)} method is called a single time, allowing the
 * implementation to compute any statistics (such as IDF, average document
 * length, etc) across <i>the entire collection</i>. The {@link TermStatistics}
 * and {@link CollectionStatistics} passed in already contain all of the raw
 * statistics involved, so a Similarity can freely use any combination of
 * statistics without causing any additional I/O. Lucene makes no assumption
 * about what is stored in the returned {@link Similarity.SimWeight} object.
 *   <li>For each segment in the index, the Query creates a {@link
 * #simScorer(SimWeight, org.apache.lucene.index.LeafReaderContext)} The score()
 * method is called for each matching document.
 * </ol>
 * <p>
 * <a name="explaintime">Explanations</a>
 * When {@link IndexSearcher#explain(org.apache.lucene.search.Query, int)} is
 * called, queries consult the Similarity's DocScorer for an explanation of how
 * it computed its score. The query passes in a the document id and an
 * explanation of how the frequency was computed.
 *
 * @see org.apache.lucene.index.IndexWriterConfig#setSimilarity(Similarity)
 * @see IndexSearcher#setSimilarity(Similarity)
 * @lucene.experimental
 */
class Similarity : public std::enable_shared_from_this<Similarity>
{
  GET_CLASS_NAME(Similarity)

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  Similarity();

  /**
   * Computes the normalization value for a field, given the accumulated
   * state of term processing for this field (see {@link FieldInvertState}).
   *
   * <p>Matches in longer fields are less precise, so implementations of this
   * method usually set smaller values when <code>state.getLength()</code> is
   * large, and larger values when <code>state.getLength()</code> is small.
   *
   * @lucene.experimental
   *
   * @param state current processing state for this field
   * @return computed norm value
   */
  virtual int64_t computeNorm(std::shared_ptr<FieldInvertState> state) = 0;

  /**
   * Compute any collection-level weight (e.g. IDF, average document length,
   * etc) needed for scoring a query.
   *
   * @param boost a multiplicative factor to apply to the produces scores
   * @param collectionStats collection-level statistics, such as the number of
   * tokens in the collection.
   * @param termStats term-level statistics, such as the document frequency of a
   * term across the collection.
   * @return SimWeight object with the information this Similarity needs to
   * score a query.
   */
  virtual std::shared_ptr<SimWeight>
  computeWeight(float boost,
                std::shared_ptr<CollectionStatistics> collectionStats,
                std::deque<TermStatistics> &termStats) = 0;

  /**
   * Creates a new {@link Similarity.SimScorer} to score matching documents from
   * a segment of the inverted index.
   * @param weight collection information from {@link #computeWeight(float,
   * CollectionStatistics, TermStatistics...)}
   * @param context segment of the inverted index to be scored.
   * @return SloppySimScorer for scoring documents across <code>context</code>
   * @throws IOException if there is a low-level I/O error
   */
  virtual std::shared_ptr<SimScorer>
  simScorer(std::shared_ptr<SimWeight> weight,
            std::shared_ptr<LeafReaderContext> context) = 0;

  /**
   * API for scoring "sloppy" queries such as {@link TermQuery},
   * {@link SpanQuery}, and {@link PhraseQuery}.
   */
public:
  class SimScorer : public std::enable_shared_from_this<SimScorer>
  {
    GET_CLASS_NAME(SimScorer)

    /**
     * Sole constructor. (For invocation by subclass
     * constructors, typically implicit.)
     */
  public:
    SimScorer();

    /**
     * Score a single document
     * @param doc document id within the inverted index segment
     * @param freq sloppy term frequency
     * @return document's score
     */
    virtual float score(int doc, float freq) = 0;

    /** Computes the amount of a sloppy phrase match, based on an edit distance.
     */
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Deprecated public abstract float computeSlopFactor(int
    // distance);
    virtual float computeSlopFactor(int distance) = 0;

    /** Calculate a scoring factor based on the data in the payload. */
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Deprecated public abstract float computePayloadFactor(int
    // doc, int start, int end, org.apache.lucene.util.BytesRef payload);
    virtual float computePayloadFactor(int doc, int start, int end,
                                       std::shared_ptr<BytesRef> payload) = 0;

    /**
     * Explain the score for a single document
     * @param doc document id within the inverted index segment
     * @param freq Explanation of how the sloppy term frequency was computed
     * @return document's score
     */
    virtual std::shared_ptr<Explanation>
    explain(int doc, std::shared_ptr<Explanation> freq) ;
  };

  /** Stores the weight for a query across the indexed collection. This abstract
   * implementation is empty; descendants of {@code Similarity} should
   * subclass {@code SimWeight} and define the statistics they require in the
   * subclass. Examples include idf, average field length, etc.
   */
public:
  class SimWeight : public std::enable_shared_from_this<SimWeight>
  {
    GET_CLASS_NAME(SimWeight)

    /**
     * Sole constructor. (For invocation by subclass
     * constructors, typically implicit.)
     */
  public:
    SimWeight();
  };
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
