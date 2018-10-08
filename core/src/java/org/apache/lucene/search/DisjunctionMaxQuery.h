#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
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
class Matches;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class Explanation;
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
namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;

/**
 * A query that generates the union of documents produced by its subqueries, and
 * that scores each document with the maximum score for that document as
 * produced by any subquery, plus a tie breaking increment for any additional
 * matching subqueries. This is useful when searching for a word in multiple
 * fields with different boost factors (so that the fields cannot be combined
 * equivalently into a single search field).  We want the primary score to be
 * the one associated with the highest boost, not the sum of the field scores
 * (as BooleanQuery would give). If the query is "albino elephant" this ensures
 * that "albino" matching one field and "elephant" matching another gets a
 * higher score than "albino" matching both fields. To get this result, use both
 * BooleanQuery and DisjunctionMaxQuery:  for each term a DisjunctionMaxQuery
 * searches for it in each field, while the set of these DisjunctionMaxQuery's
 * is combined into a BooleanQuery. The tie breaker capability allows results
 * that include the same term in multiple fields to be judged better than
 * results that include this term in only the best of those multiple fields,
 * without confusing this with the better case of two different terms in the
 * multiple fields.
 */
class DisjunctionMaxQuery final : public Query,
                                  public std::deque<std::shared_ptr<Query>>
{
  GET_CLASS_NAME(DisjunctionMaxQuery)

  /* The subqueries */
private:
  std::deque<std::shared_ptr<Query>> const disjuncts;

  /* Multiple of the non-max disjunct scores added into our final score.
   * Non-zero values support tie-breaking. */
  const float tieBreakerMultiplier;

  /**
   * Creates a new DisjunctionMaxQuery
   * @param disjuncts a {@code std::deque<Query>} of all the disjuncts to add
   * @param tieBreakerMultiplier  the score of each non-maximum disjunct for a
   * document is multiplied by this weight and added into the final score.  If
   * non-zero, the value should be small, on the order of 0.1, which says that
   *        10 occurrences of word in a lower-scored field that is also in a
   * higher scored field is just as good as a unique word in the lower scored
   * field (i.e., one that is not in any higher scored field.
   */
public:
  DisjunctionMaxQuery(
      std::shared_ptr<std::deque<std::shared_ptr<Query>>> disjuncts,
      float tieBreakerMultiplier);

  /** @return An {@code Iterator<Query>} over the disjuncts */
  std::shared_ptr<Iterator<std::shared_ptr<Query>>> iterator() override;

  /**
   * @return the disjuncts.
   */
  std::deque<std::shared_ptr<Query>> getDisjuncts();

  /**
   * @return tie breaker value for multiple matches.
   */
  float getTieBreakerMultiplier();

  /**
   * Expert: the Weight for DisjunctionMaxQuery, used to
   * normalize, score and explain these queries.
   *
   * <p>NOTE: this API and implementation is subject to
   * change suddenly in the next release.</p>
   */
protected:
  class DisjunctionMaxWeight : public Weight
  {
    GET_CLASS_NAME(DisjunctionMaxWeight)
  private:
    std::shared_ptr<DisjunctionMaxQuery> outerInstance;

    /** The Weights for our subqueries, in 1-1 correspondence with disjuncts */
  protected:
    const std::deque<std::shared_ptr<Weight>> weights =
        std::deque<std::shared_ptr<Weight>>(); // The Weight's for our
                                                // subqueries, in 1-1
                                                // correspondence with disjuncts
  private:
    const bool needsScores;

    /** Construct the Weight for this Query searched by searcher.  Recursively
     * construct subquery weights. */
  public:
    DisjunctionMaxWeight(std::shared_ptr<DisjunctionMaxQuery> outerInstance,
                         std::shared_ptr<IndexSearcher> searcher,
                         bool needsScores, float boost) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

    /** Create the scorer used to score our associated DisjunctionMaxQuery */
    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    /** Explain the score we computed for doc */
    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

  protected:
    std::shared_ptr<DisjunctionMaxWeight> shared_from_this()
    {
      return std::static_pointer_cast<DisjunctionMaxWeight>(
          Weight::shared_from_this());
    }
  }; // end of DisjunctionMaxWeight inner class

  /** Create the Weight used to score us */
public:
  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  /** Optimize our representation and our subqueries representations
   * @param reader the IndexReader we query
   * @return an optimized copy of us (which may not be a copy if there is
   * nothing to optimize) */
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  /** Prettyprint us.
   * @param field the field to which we are applied
   * @return a string that shows what we do, of the form "(disjunct1 | disjunct2
   * | ... | disjunctn)^boost"
   */
  std::wstring toString(const std::wstring &field) override;

  /** Return true iff we represent the same query as o
   * @param other another object
   * @return true iff o is a DisjunctionMaxQuery with the same boost and the
   * same subqueries, in the same order, as us
   */
  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<DisjunctionMaxQuery> other);

  /** Compute a hash code for hashing us
   * @return the hash code
   */
public:
  virtual int hashCode();

protected:
  std::shared_ptr<DisjunctionMaxQuery> shared_from_this()
  {
    return std::static_pointer_cast<DisjunctionMaxQuery>(
        Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
