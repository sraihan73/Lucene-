#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/CollectionStatistics.h"

#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/TermStatistics.h"
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/IDFStats.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"

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
 * Implementation of {@link Similarity} with the Vector Space Model.
 * <p>
 * Expert: Scoring API.
 * <p>TFIDFSimilarity defines the components of Lucene scoring.
 * Overriding computation of these components is a convenient
 * way to alter Lucene scoring.
 *
 * <p>Suggested reading:
 * <a
 * href="http://nlp.stanford.edu/IR-book/html/htmledition/queries-as-vectors-1.html">
 * Introduction To Information Retrieval, Chapter 6</a>.
 *
 * <p>The following describes how Lucene scoring evolves from
 * underlying information retrieval models to (efficient) implementation.
 * We first brief on <i>VSM Score</i>,
 * then derive from it <i>Lucene's Conceptual Scoring Formula</i>,
 * from which, finally, evolves <i>Lucene's Practical Scoring Function</i>
 * (the latter is connected directly with Lucene classes and methods).
 *
 * <p>Lucene combines
 * <a href="http://en.wikipedia.org/wiki/Standard_Boolean_model">
 * Boolean model (BM) of Information Retrieval</a>
 * with
 * <a href="http://en.wikipedia.org/wiki/Vector_Space_Model">
 * Vector Space Model (VSM) of Information Retrieval</a> -
 * documents "approved" by BM are scored by VSM.
 *
 * <p>In VSM, documents and queries are represented as
 * weighted vectors in a multi-dimensional space,
 * where each distinct index term is a dimension,
 * and weights are
 * <a href="http://en.wikipedia.org/wiki/Tfidf">Tf-idf</a> values.
 *
 * <p>VSM does not require weights to be <i>Tf-idf</i> values,
 * but <i>Tf-idf</i> values are believed to produce search results of high
 * quality, and so Lucene is using <i>Tf-idf</i>. <i>Tf</i> and <i>Idf</i> are
 * described in more detail below, but for now, for completion, let's just say
 * that for given term <i>t</i> and document (or query) <i>x</i>, <i>Tf(t,x)</i>
 * varies with the number of occurrences of term <i>t</i> in <i>x</i> (when one
 * increases so does the other) and <i>idf(t)</i> similarly varies with the
 * inverse of the number of index documents containing term <i>t</i>.
 *
 * <p><i>VSM score</i> of document <i>d</i> for query <i>q</i> is the
 * <a href="http://en.wikipedia.org/wiki/Cosine_similarity">
 * Cosine Similarity</a>
 * of the weighted query vectors <i>V(q)</i> and <i>V(d)</i>:
 *
 *  <br>&nbsp;<br>
 *  <table cellpadding="2" cellspacing="2" border="0" style="width:auto;
 * margin-left:auto; margin-right:auto" summary="formatting only"> <tr><td>
 *    <table cellpadding="1" cellspacing="0" border="1" style="margin-left:auto;
 * margin-right:auto" summary="formatting only"> <tr><td> <table cellpadding="2"
 * cellspacing="2" border="0" style="margin-left:auto; margin-right:auto"
 * summary="cosine similarity formula"> <tr> <td valign="middle" align="right"
 * rowspan="1"> cosine-similarity(q,d) &nbsp; = &nbsp;
 *          </td>
 *          <td valign="middle" align="center">
 *            <table summary="cosine similarity formula">
 *               <tr><td align="center" style="text-align:
 * center"><small>V(q)&nbsp;&middot;&nbsp;V(d)</small></td></tr> <tr><td
 * align="center" style="text-align:
 * center">&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;</td></tr>
 *               <tr><td align="center" style="text-align:
 * center"><small>|V(q)|&nbsp;|V(d)|</small></td></tr>
 *            </table>
 *          </td>
 *        </tr>
 *      </table>
 *      </td></tr>
 *    </table>
 *    </td></tr>
 *    <tr><td>
 *    <center><u>VSM Score</u></center>
 *    </td></tr>
 *  </table>
 *  <br>&nbsp;<br>
 *
 *
 * Where <i>V(q)</i> &middot; <i>V(d)</i> is the
 * <a href="http://en.wikipedia.org/wiki/Dot_product">dot product</a>
 * of the weighted vectors,
 * and <i>|V(q)|</i> and <i>|V(d)|</i> are their
 * <a
 * href="http://en.wikipedia.org/wiki/Euclidean_norm#Euclidean_norm">Euclidean
 * norms</a>.
 *
 * <p>Note: the above equation can be viewed as the dot product of
 * the normalized weighted vectors, in the sense that dividing
 * <i>V(q)</i> by its euclidean norm is normalizing it to a unit deque.
 *
 * <p>Lucene refines <i>VSM score</i> for both search quality and usability:
 * <ul>
 *  <li>Normalizing <i>V(d)</i> to the unit deque is known to be problematic in
 * that it removes all document length information. For some documents removing
 * this info is probably ok, e.g. a document made by duplicating a certain
 * paragraph <i>10</i> times, especially if that paragraph is made of distinct
 * terms. But for a document which contains no duplicated paragraphs, this might
 * be wrong. To avoid this problem, a different document length normalization
 *  factor is used, which normalizes to a deque equal to or larger
 *  than the unit deque: <i>doc-len-norm(d)</i>.
 *  </li>
 *
 *  <li>At indexing, users can specify that certain documents are more
 *  important than others, by assigning a document boost.
 *  For this, the score of each document is also multiplied by its boost value
 *  <i>doc-boost(d)</i>.
 *  </li>
 *
 *  <li>Lucene is field based, hence each query term applies to a single
 *  field, document length normalization is by the length of the certain field,
 *  and in addition to document boost there are also document fields boosts.
 *  </li>
 *
 *  <li>The same field can be added to a document during indexing several times,
 *  and so the boost of that field is the multiplication of the boosts of
 *  the separate additions (or parts) of that field within the document.
 *  </li>
 *
 *  <li>At search time users can specify boosts to each query, sub-query, and
 *  each query term, hence the contribution of a query term to the score of
 *  a document is multiplied by the boost of that query term
 * <i>query-boost(q)</i>.
 *  </li>
 *
 *  <li>A document may match a multi term query without containing all
 *  the terms of that query (this is correct for some of the queries).
 *  </li>
 * </ul>
 *
 * <p>Under the simplifying assumption of a single field in the index,
 * we get <i>Lucene's Conceptual scoring formula</i>:
 *
 *  <br>&nbsp;<br>
 *  <table cellpadding="2" cellspacing="2" border="0" style="width:auto;
 * margin-left:auto; margin-right:auto" summary="formatting only"> <tr><td>
 *    <table cellpadding="1" cellspacing="0" border="1" style="margin-left:auto;
 * margin-right:auto" summary="formatting only"> <tr><td> <table cellpadding="2"
 * cellspacing="2" border="0" style="margin-left:auto; margin-right:auto"
 * summary="formatting only"> <tr> <td valign="middle" align="right"
 * rowspan="1"> score(q,d) &nbsp; = &nbsp; <span style="color:
 * #CCCC00">query-boost(q)</span> &middot; &nbsp;
 *          </td>
 *          <td valign="middle" align="center">
 *            <table summary="Lucene conceptual scoring formula">
 *               <tr><td align="center" style="text-align: center"><small><span
 * style="color: #993399">V(q)&nbsp;&middot;&nbsp;V(d)</span></small></td></tr>
 *               <tr><td align="center" style="text-align:
 * center">&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;</td></tr>
 *               <tr><td align="center" style="text-align: center"><small><span
 * style="color: #FF33CC">|V(q)|</span></small></td></tr>
 *            </table>
 *          </td>
 *          <td valign="middle" align="right" rowspan="1">
 *            &nbsp; &middot; &nbsp; <span style="color:
 * #3399FF">doc-len-norm(d)</span> &nbsp; &middot; &nbsp; <span style="color:
 * #3399FF">doc-boost(d)</span>
 *          </td>
 *        </tr>
 *      </table>
 *      </td></tr>
 *    </table>
 *    </td></tr>
 *    <tr><td>
 *    <center><u>Lucene Conceptual Scoring Formula</u></center>
 *    </td></tr>
 *  </table>
 *  <br>&nbsp;<br>
 *
 * <p>The conceptual formula is a simplification in the sense that (1) terms and
 * documents are fielded and (2) boosts are usually per query term rather than
 * per query.
 *
 * <p>We now describe how Lucene implements this conceptual scoring formula, and
 * derive from it <i>Lucene's Practical Scoring Function</i>.
 *
 * <p>For efficient score computation some scoring components
 * are computed and aggregated in advance:
 *
 * <ul>
 *  <li><i>Query-boost</i> for the query (actually for each query term)
 *  is known when search starts.
 *  </li>
 *
 *  <li>Query Euclidean norm <i>|V(q)|</i> can be computed when search starts,
 *  as it is independent of the document being scored.
 *  From search optimization perspective, it is a valid question
 *  why bother to normalize the query at all, because all
 *  scored documents will be multiplied by the same <i>|V(q)|</i>,
 *  and hence documents ranks (their order by score) will not
 *  be affected by this normalization.
 *  There are two good reasons to keep this normalization:
 *  <ul>
 *   <li>Recall that
 *   <a href="http://en.wikipedia.org/wiki/Cosine_similarity">
 *   Cosine Similarity</a> can be used find how similar
 *   two documents are. One can use Lucene for e.g.
 *   clustering, and use a document as a query to compute
 *   its similarity to other documents.
 *   In this use case it is important that the score of document <i>d3</i>
 *   for query <i>d1</i> is comparable to the score of document <i>d3</i>
 *   for query <i>d2</i>. In other words, scores of a document for two
 *   distinct queries should be comparable.
 *   There are other applications that may require this.
 *   And this is exactly what normalizing the query deque <i>V(q)</i>
 *   provides: comparability (to a certain extent) of two or more queries.
 *   </li>
 *  </ul>
 *  </li>
 *
 *  <li>Document length norm <i>doc-len-norm(d)</i> and document
 *  boost <i>doc-boost(d)</i> are known at indexing time.
 *  They are computed in advance and their multiplication
 *  is saved as a single value in the index: <i>norm(d)</i>.
 *  (In the equations below, <i>norm(t in d)</i> means <i>norm(field(t) in doc
 * d)</i> where <i>field(t)</i> is the field associated with term <i>t</i>.)
 *  </li>
 * </ul>
 *
 * <p><i>Lucene's Practical Scoring Function</i> is derived from the above.
 * The color codes demonstrate how it relates
 * to those of the <i>conceptual</i> formula:
 *
 * <table cellpadding="2" cellspacing="2" border="0" style="width:auto;
 * margin-left:auto; margin-right:auto" summary="formatting only"> <tr><td>
 *  <table cellpadding="" cellspacing="2" border="2" style="margin-left:auto;
 * margin-right:auto" summary="formatting only"> <tr><td> <table cellpadding="2"
 * cellspacing="2" border="0" style="margin-left:auto; margin-right:auto"
 * summary="Lucene conceptual scoring formula"> <tr> <td valign="middle"
 * align="right" rowspan="1"> score(q,d) &nbsp; = &nbsp;
 *       <big><big><big>&sum;</big></big></big>
 *     </td>
 *     <td valign="middle" align="right" rowspan="1">
 *       <big><big>(</big></big>
 *       <A HREF="#formula_tf"><span style="color: #993399">tf(t in
 * d)</span></A> &nbsp;&middot;&nbsp; <A HREF="#formula_idf"><span style="color:
 * #993399">idf(t)</span></A><sup>2</sup> &nbsp;&middot;&nbsp; <A
 * HREF="#formula_termBoost"><span style="color:
 * #CCCC00">t.getBoost()</span></A>&nbsp;&middot;&nbsp; <A
 * HREF="#formula_norm"><span style="color: #3399FF">norm(t,d)</span></A>
 *       <big><big>)</big></big>
 *     </td>
 *   </tr>
 *   <tr valign="top">
 *    <td></td>
 *    <td align="center" style="text-align: center"><small>t in q</small></td>
 *    <td></td>
 *   </tr>
 *   </table>
 *  </td></tr>
 *  </table>
 * </td></tr>
 * <tr><td>
 *  <center><u>Lucene Practical Scoring Function</u></center>
 * </td></tr>
 * </table>
 *
 * <p> where
 * <ol>
 *    <li>
 *      <A NAME="formula_tf"></A>
 *      <b><i>tf(t in d)</i></b>
 *      correlates to the term's <i>frequency</i>,
 *      defined as the number of times term <i>t</i> appears in the currently
 * scored document <i>d</i>. Documents that have more occurrences of a given
 * term receive a higher score. Note that <i>tf(t in q)</i> is assumed to be
 * <i>1</i> and therefore it does not appear in this equation, However if a
 * query contains twice the same term, there will be two term-queries with that
 * same term and hence the computation would still be correct (although not very
 * efficient). The default computation for <i>tf(t in d)</i> in
 *      {@link org.apache.lucene.search.similarities.ClassicSimilarity#tf(float)
 * ClassicSimilarity} is:
 *
 *      <br>&nbsp;<br>
 *      <table cellpadding="2" cellspacing="2" border="0" style="width:auto;
 * margin-left:auto; margin-right:auto" summary="term frequency computation">
 *        <tr>
 *          <td valign="middle" align="right" rowspan="1">
 *            {@link
 * org.apache.lucene.search.similarities.ClassicSimilarity#tf(float) tf(t in d)}
 * &nbsp; = &nbsp;
 *          </td>
 *          <td valign="top" align="center" rowspan="1">
 *               frequency<sup><big>&frac12;</big></sup>
 *          </td>
 *        </tr>
 *      </table>
 *      <br>&nbsp;<br>
 *    </li>
 *
 *    <li>
 *      <A NAME="formula_idf"></A>
 *      <b><i>idf(t)</i></b> stands for Inverse Document Frequency. This value
 *      correlates to the inverse of <i>docFreq</i>
 *      (the number of documents in which the term <i>t</i> appears).
 *      This means rarer terms give higher contribution to the total score.
 *      <i>idf(t)</i> appears for <i>t</i> in both the query and the document,
 *      hence it is squared in the equation.
 *      The default computation for <i>idf(t)</i> in
 *      {@link org.apache.lucene.search.similarities.ClassicSimilarity#idf(long,
 * long) ClassicSimilarity} is:
 *
 *      <br>&nbsp;<br>
 *      <table cellpadding="2" cellspacing="2" border="0" style="width:auto;
 * margin-left:auto; margin-right:auto" summary="inverse document frequency
 * computation"> <tr> <td valign="middle" align="right">
 *            {@link
 * org.apache.lucene.search.similarities.ClassicSimilarity#idf(long, long)
 * idf(t)}&nbsp; = &nbsp;
 *          </td>
 *          <td valign="middle" align="center">
 *            1 + log <big>(</big>
 *          </td>
 *          <td valign="middle" align="center">
 *            <table summary="inverse document frequency computation">
 *               <tr><td align="center" style="text-align:
 * center"><small>docCount+1</small></td></tr> <tr><td align="center"
 * style="text-align:
 * center">&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;&ndash;</td></tr>
 *               <tr><td align="center" style="text-align:
 * center"><small>docFreq+1</small></td></tr>
 *            </table>
 *          </td>
 *          <td valign="middle" align="center">
 *            <big>)</big>
 *          </td>
 *        </tr>
 *      </table>
 *      <br>&nbsp;<br>
 *    </li>
 *
 *    <li>
 *      <A NAME="formula_termBoost"></A>
 *      <b><i>t.getBoost()</i></b>
 *      is a search time boost of term <i>t</i> in the query <i>q</i> as
 *      specified in the query text
 *      (see <A
 * HREF="{@docRoot}/../queryparser/org/apache/lucene/queryparser/classic/package-summary.html#Boosting_a_Term">query
 * syntax</A>), or as set by wrapping with
 *      {@link
 * org.apache.lucene.search.BoostQuery#BoostQuery(org.apache.lucene.search.Query,
 * float) BoostQuery}. Notice that there is really no direct API for accessing a
 * boost of one term in a multi term query, but rather multi terms are
 * represented in a query as multi
 *      {@link org.apache.lucene.search.TermQuery TermQuery} objects,
 *      and so the boost of a term in the query is accessible by calling the
 * sub-query
 *      {@link org.apache.lucene.search.BoostQuery#getBoost() getBoost()}.
 *      <br>&nbsp;<br>
 *    </li>
 *
 *    <li>
 *      <A NAME="formula_norm"></A>
 *      <b><i>norm(t,d)</i></b> is an index-time boost factor that solely
 *      depends on the number of tokens of this field in the document, so
 *      that shorter fields contribute more to the score.
 *    </li>
 * </ol>
 *
 * @see org.apache.lucene.index.IndexWriterConfig#setSimilarity(Similarity)
 * @see IndexSearcher#setSimilarity(Similarity)
 */
class TFIDFSimilarity : public Similarity
{
  GET_CLASS_NAME(TFIDFSimilarity)

  /** Cache of decoded bytes. */
public:
  static std::deque<float> const OLD_NORM_TABLE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TFIDFSimilarity::StaticConstructor staticConstructor;

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  TFIDFSimilarity();

  /**
   * True if overlap tokens (tokens with a position of increment of zero) are
   * discounted from the document's length.
   */
protected:
  bool discountOverlaps = true;

  /** Determines whether overlap tokens (Tokens with
   *  0 position increment) are ignored when computing
   *  norm.  By default this is true, meaning overlap
   *  tokens do not count when computing norms.
   *
   *  @lucene.experimental
   *
   *  @see #computeNorm
   */
public:
  virtual void setDiscountOverlaps(bool v);

  /**
   * Returns true if overlap tokens are discounted from the document's length.
   * @see #setDiscountOverlaps
   */
  virtual bool getDiscountOverlaps();

  /** Computes a score factor based on a term or phrase's frequency in a
   * document.  This value is multiplied by the {@link #idf(long, long)}
   * factor for each term in the query and these products are then summed to
   * form the initial score for a document.
   *
   * <p>Terms and phrases repeated in a document indicate the topic of the
   * document, so implementations of this method usually return larger values
   * when <code>freq</code> is large, and smaller values when <code>freq</code>
   * is small.
   *
   * @param freq the frequency of a term within a document
   * @return a score factor based on a term's within-document frequency
   */
  virtual float tf(float freq) = 0;

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

  /** Computes a score factor based on a term's document frequency (the number
   * of documents which contain the term).  This value is multiplied by the
   * {@link #tf(float)} factor for each term in the query and these products are
   * then summed to form the initial score for a document.
   *
   * <p>Terms that occur in fewer documents are better indicators of topic, so
   * implementations of this method usually return larger values for rare terms,
   * and smaller values for common terms.
   *
   * @param docFreq the number of documents which contain the term
   * @param docCount the total number of documents in the collection
   * @return a score factor based on the term's document frequency
   */
  virtual float idf(int64_t docFreq, int64_t docCount) = 0;

  /**
   * Compute an index-time normalization value for this field instance.
   *
   * @param length the number of terms in the field, optionally {@link
   * #setDiscountOverlaps(bool) discounting overlaps}
   * @return a length normalization value
   */
  virtual float lengthNorm(int length) = 0;

  int64_t computeNorm(std::shared_ptr<FieldInvertState> state) override final;

  /** Computes the amount of a sloppy phrase match, based on an edit distance.
   * This value is summed for each sloppy phrase match in a document to form
   * the frequency to be used in scoring instead of the exact term count.
   *
   * <p>A phrase match with a small edit distance to a document passage more
   * closely matches the document, so implementations of this method usually
   * return larger values when the edit distance is small and smaller values
   * when it is large.
   *
   * @see PhraseQuery#getSlop()
   * @param distance the edit distance of this sloppy phrase match
   * @return the frequency increment for this match
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public abstract float sloppyFreq(int distance);
  virtual float sloppyFreq(int distance) = 0;

  /**
   * Calculate a scoring factor based on the data in the payload.
   * Implementations are responsible for interpreting what is in the payload.
   * Lucene makes no assumptions about what is in the byte array.
   *
   * @param doc The docId currently being scored.
   * @param start The start position of the payload
   * @param end The end position of the payload
   * @param payload The payload byte array to be scored
   * @return An implementation dependent float to be used as a scoring factor
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public abstract float scorePayload(int doc, int
  // start, int end, org.apache.lucene.util.BytesRef payload);
  virtual float scorePayload(int doc, int start, int end,
                             std::shared_ptr<BytesRef> payload) = 0;

  std::shared_ptr<SimWeight>
  computeWeight(float boost,
                std::shared_ptr<CollectionStatistics> collectionStats,
                std::deque<TermStatistics> &termStats) override final;

  std::shared_ptr<SimScorer>
  simScorer(std::shared_ptr<SimWeight> stats,
            std::shared_ptr<LeafReaderContext> context) 
      override final;

private:
  class TFIDFSimScorer final : public SimScorer
  {
    GET_CLASS_NAME(TFIDFSimScorer)
  private:
    std::shared_ptr<TFIDFSimilarity> outerInstance;

    const std::shared_ptr<IDFStats> stats;
    const float weightValue;
    const std::shared_ptr<NumericDocValues> norms;
    std::deque<float> const normTable;

  public:
    TFIDFSimScorer(std::shared_ptr<TFIDFSimilarity> outerInstance,
                   std::shared_ptr<IDFStats> stats,
                   std::shared_ptr<NumericDocValues> norms,
                   std::deque<float> &normTable) ;

    float score(int doc, float freq)  override;

    float computeSlopFactor(int distance) override;

    float computePayloadFactor(int doc, int start, int end,
                               std::shared_ptr<BytesRef> payload) override;

    std::shared_ptr<Explanation>
    explain(int doc,
            std::shared_ptr<Explanation> freq)  override;

  protected:
    std::shared_ptr<TFIDFSimScorer> shared_from_this()
    {
      return std::static_pointer_cast<TFIDFSimScorer>(
          SimScorer::shared_from_this());
    }
  };

  /** std::deque statistics for the TF-IDF model. The only statistic of interest
   * to this model is idf. */
public:
  class IDFStats : public SimWeight
  {
    GET_CLASS_NAME(IDFStats)
  private:
    const std::wstring field;
    /** The idf and its explanation */
    const std::shared_ptr<Explanation> idf;
    const float boost;
    const float queryWeight;

  public:
    std::deque<float> const normTable;

    IDFStats(const std::wstring &field, float boost,
             std::shared_ptr<Explanation> idf, std::deque<float> &normTable);

  protected:
    std::shared_ptr<IDFStats> shared_from_this()
    {
      return std::static_pointer_cast<IDFStats>(SimWeight::shared_from_this());
    }
  };

private:
  std::shared_ptr<Explanation>
  explainField(int doc, std::shared_ptr<Explanation> freq,
               std::shared_ptr<IDFStats> stats,
               std::shared_ptr<NumericDocValues> norms,
               std::deque<float> &normTable) ;

  std::shared_ptr<Explanation>
  explainScore(int doc, std::shared_ptr<Explanation> freq,
               std::shared_ptr<IDFStats> stats,
               std::shared_ptr<NumericDocValues> norms,
               std::deque<float> &normTable) ;

protected:
  std::shared_ptr<TFIDFSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<TFIDFSimilarity>(
        Similarity::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/similarities/
