#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/PhraseMatcher.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;

/** A Query that matches documents containing a particular sequence of terms.
 * A PhraseQuery is built by QueryParser for input like <code>"new york"</code>.
 *
 * <p>This query may be combined with other terms or queries with a {@link
 * BooleanQuery}.
 *
 * <p><b>NOTE</b>:
 * All terms in the phrase must match, even those at the same position. If you
 * have terms at the same position, perhaps synonyms, you probably want {@link
 * MultiPhraseQuery} instead which only requires one term at a position to
 * match. <br >Also, Leading holes don't have any particular meaning for this
 * query and will be ignored. For instance this query: <pre class="prettyprint">
 * PhraseQuery.Builder builder = new PhraseQuery.Builder();
 * builder.add(new Term("body", "one"), 4);
 * builder.add(new Term("body", "two"), 5);
 * PhraseQuery pq = builder.build();
 * </pre>
 * is equivalent to the below query:
 * <pre class="prettyprint">
 * PhraseQuery.Builder builder = new PhraseQuery.Builder();
 * builder.add(new Term("body", "one"), 0);
 * builder.add(new Term("body", "two"), 1);
 * PhraseQuery pq = builder.build();
 * </pre>
 */
class PhraseQuery : public Query
{
  GET_CLASS_NAME(PhraseQuery)

  /** A builder for phrase queries. */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)

  private:
    int slop = 0;
    const std::deque<std::shared_ptr<Term>> terms;
    const std::deque<int> positions;

    /** Sole constructor. */
  public:
    Builder();

    /**
     * Set the slop.
     * @see PhraseQuery#getSlop()
     */
    virtual std::shared_ptr<Builder> setSlop(int slop);

    /**
     * Adds a term to the end of the query phrase.
     * The relative position of the term is the one immediately after the last
     * term added.
     */
    virtual std::shared_ptr<Builder> add(std::shared_ptr<Term> term);

    /**
     * Adds a term to the end of the query phrase.
     * The relative position of the term within the phrase is specified
     * explicitly, but must be greater than or equal to that of the previously
     * added term. A greater position allows phrases with gaps (e.g. in
     * connection with stopwords). If the position is equal, you most likely
     * should be using
     * {@link MultiPhraseQuery} instead which only requires one term at each
     * position to match; this class requires all of them.
     */
    virtual std::shared_ptr<Builder> add(std::shared_ptr<Term> term,
                                         int position);

    /**
     * Build a phrase query based on the terms that have been added.
     */
    virtual std::shared_ptr<PhraseQuery> build();
  };

private:
  const int slop;
  const std::wstring field;
  std::deque<std::shared_ptr<Term>> const terms;
  std::deque<int> const positions;

  PhraseQuery(int slop, std::deque<std::shared_ptr<Term>> &terms,
              std::deque<int> &positions);

  static std::deque<int> incrementalPositions(int length);

  static std::deque<std::shared_ptr<Term>>
  toTerms(const std::wstring &field, std::deque<std::wstring> &termStrings);

  static std::deque<std::shared_ptr<Term>>
  toTerms(const std::wstring &field, std::deque<BytesRef> &termBytes);

  /**
   * Create a phrase query which will match documents that contain the given
   * deque of terms at consecutive positions in {@code field}, and at a
   * maximum edit distance of {@code slop}. For more complicated use-cases,
   * use {@link PhraseQuery.Builder}.
   * @see #getSlop()
   */
public:
  PhraseQuery(int slop, const std::wstring &field,
              std::deque<std::wstring> &terms);

  /**
   * Create a phrase query which will match documents that contain the given
   * deque of terms at consecutive positions in {@code field}.
   */
  PhraseQuery(const std::wstring &field, std::deque<std::wstring> &terms);

  /**
   * Create a phrase query which will match documents that contain the given
   * deque of terms at consecutive positions in {@code field}, and at a
   * maximum edit distance of {@code slop}. For more complicated use-cases,
   * use {@link PhraseQuery.Builder}.
   * @see #getSlop()
   */
  PhraseQuery(int slop, const std::wstring &field,
              std::deque<BytesRef> &terms);

  /**
   * Create a phrase query which will match documents that contain the given
   * deque of terms at consecutive positions in {@code field}.
   */
  PhraseQuery(const std::wstring &field, std::deque<BytesRef> &terms);

  /**
   * Return the slop for this {@link PhraseQuery}.
   *
   * <p>The slop is an edit distance between respective positions of terms as
   * defined in this {@link PhraseQuery} and the positions of terms in a
   * document.
   *
   * <p>For instance, when searching for {@code "quick fox"}, it is expected
   * that the difference between the positions of {@code fox} and {@code quick}
   * is 1. So {@code "a quick brown fox"} would be at an edit distance of 1
   * since the difference of the positions of {@code fox} and {@code quick}
   * is 2. Similarly, {@code "the fox is quick"} would be at an edit distance of
   * 3 since the difference of the positions of {@code fox} and {@code quick} is
   * -2. The slop defines the maximum edit distance for a document to match.
   *
   * <p>More exact matches are scored higher than sloppier matches, thus search
   * results are sorted by exactness.
   */
  virtual int getSlop();

  /** Returns the deque of terms in this phrase. */
  virtual std::deque<std::shared_ptr<Term>> getTerms();

  /**
   * Returns the relative positions of terms in this phrase.
   */
  virtual std::deque<int> getPositions();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

public:
  class PostingsAndFreq : public std::enable_shared_from_this<PostingsAndFreq>,
                          public Comparable<std::shared_ptr<PostingsAndFreq>>
  {
    GET_CLASS_NAME(PostingsAndFreq)
  public:
    const std::shared_ptr<PostingsEnum> postings;
    const int position;
    std::deque<std::shared_ptr<Term>> const terms;
    const int nTerms; // for faster comparisons

    PostingsAndFreq(std::shared_ptr<PostingsEnum> postings, int position,
                    std::deque<Term> &terms);

    int compareTo(std::shared_ptr<PostingsAndFreq> other) override;

    virtual int hashCode();

    bool equals(std::any obj) override;
  };

  /** A guess of
   * the average number of simple operations for the initial seek and buffer
   * refill per document for the positions of a term. See also {@link
   * Lucene50PostingsReader.BlockPostingsEnum#nextPosition()}. <p> Aside:
   * Instead of being constant this could depend among others on
   * {@link Lucene50PostingsFormat#BLOCK_SIZE},
   * {@link TermsEnum#docFreq()},
   * {@link TermsEnum#totalTermFreq()},
   * {@link DocIdSetIterator#cost()} (expected number of matching docs),
   * {@link LeafReader#maxDoc()} (total number of docs in the segment),
   * and the seek time and block size of the device storing the index.
   */
private:
  static constexpr int TERM_POSNS_SEEK_OPS_PER_DOC = 128;

  /** Number of simple operations in {@link
   * Lucene50PostingsReader.BlockPostingsEnum#nextPosition()} when no seek or
   * buffer refill is done.
   */
  static constexpr int TERM_OPS_PER_POS = 7;

  /** Returns an expected cost in simple operations
   *  of processing the occurrences of a term
   *  in a document that contains the term.
   *  This is for use by {@link TwoPhaseIterator#matchCost} implementations.
   *  <br>This may be inaccurate when {@link TermsEnum#totalTermFreq()} is not
   * available.
   *  @param termsEnum The term is the term at which this TermsEnum is
   * positioned.
   */
public:
  static float
  termPositionsCost(std::shared_ptr<TermsEnum> termsEnum) ;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class PhraseWeightAnonymousInnerClass : public PhraseWeight
  {
    GET_CLASS_NAME(PhraseWeightAnonymousInnerClass)
  private:
    std::shared_ptr<PhraseQuery> outerInstance;

    std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher;
    bool needsScores = false;
    float boost = 0;

  public:
    PhraseWeightAnonymousInnerClass(
        std::shared_ptr<PhraseQuery> outerInstance, const std::wstring &field,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        bool needsScores, float boost);

  private:
    std::deque<std::shared_ptr<TermContext>> states;

  protected:
    std::shared_ptr<Similarity::SimWeight> getStats(
        std::shared_ptr<IndexSearcher> searcher)  override;

    std::shared_ptr<PhraseMatcher>
    getPhraseMatcher(std::shared_ptr<LeafReaderContext> context,
                     bool exposeOffsets)  override;

  public:
    void extractTerms(
        std::shared_ptr<Set<std::shared_ptr<Term>>> queryTerms) override;

  protected:
    std::shared_ptr<PhraseWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PhraseWeightAnonymousInnerClass>(
          PhraseWeight::shared_from_this());
    }
  };

  // only called from assert
private:
  static bool termNotInReader(std::shared_ptr<LeafReader> reader,
                              std::shared_ptr<Term> term) ;

  /** Prints a user-readable version of this query. */
public:
  std::wstring toString(const std::wstring &f) override;

  /** Returns true iff <code>o</code> is equal to this. */
  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<PhraseQuery> other);

  /** Returns a hash code value for this object.*/
public:
  virtual int hashCode();

protected:
  std::shared_ptr<PhraseQuery> shared_from_this()
  {
    return std::static_pointer_cast<PhraseQuery>(Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
