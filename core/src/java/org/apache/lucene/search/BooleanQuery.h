#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class BooleanClause;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using Occur = org::apache::lucene::search::BooleanClause::Occur;

/** A Query that matches documents matching bool combinations of other
 * queries, e.g. {@link TermQuery}s, {@link PhraseQuery}s or other
 * BooleanQuerys.
 */
class BooleanQuery : public Query,
                     public std::deque<std::shared_ptr<BooleanClause>>
{
  GET_CLASS_NAME(BooleanQuery)

private:
  static int maxClauseCount;

  /** Thrown when an attempt is made to add more than {@link
   * #getMaxClauseCount()} clauses. This typically happens if
   * a PrefixQuery, FuzzyQuery, WildcardQuery, or TermRangeQuery
   * is expanded to many terms during search.
   */
public:
  class TooManyClauses : public std::runtime_error
  {
    GET_CLASS_NAME(TooManyClauses)
  public:
    TooManyClauses();

  protected:
    std::shared_ptr<TooManyClauses> shared_from_this()
    {
      return std::static_pointer_cast<TooManyClauses>(
          RuntimeException::shared_from_this());
    }
  };

  /** Return the maximum number of clauses permitted, 1024 by default.
   * Attempts to add more than the permitted number of clauses cause {@link
   * TooManyClauses} to be thrown.
   * @see #setMaxClauseCount(int)
   */
public:
  static int getMaxClauseCount();

  /**
   * Set the maximum number of clauses permitted per BooleanQuery.
   * Default value is 1024.
   */
  static void setMaxClauseCount(int maxClauseCount);

  /** A builder for bool queries. */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)

  private:
    int minimumNumberShouldMatch = 0;
    const std::deque<std::shared_ptr<BooleanClause>> clauses =
        std::deque<std::shared_ptr<BooleanClause>>();

    /** Sole constructor. */
  public:
    Builder();

    /**
     * Specifies a minimum number of the optional BooleanClauses
     * which must be satisfied.
     *
     * <p>
     * By default no optional clauses are necessary for a match
     * (unless there are no required clauses).  If this method is used,
     * then the specified number of clauses is required.
     * </p>
     * <p>
     * Use of this method is totally independent of specifying that
     * any specific clauses are required (or prohibited).  This number will
     * only be compared against the number of matching optional clauses.
     * </p>
     *
     * @param min the number of optional clauses that must match
     */
    virtual std::shared_ptr<Builder> setMinimumNumberShouldMatch(int min);

    /**
     * Add a new clause to this {@link Builder}. Note that the order in which
     * clauses are added does not have any impact on matching documents or query
     * performance.
     * @throws TooManyClauses if the new number of clauses exceeds the maximum
     * clause number
     */
    virtual std::shared_ptr<Builder> add(std::shared_ptr<BooleanClause> clause);

    /**
     * Add a new clause to this {@link Builder}. Note that the order in which
     * clauses are added does not have any impact on matching documents or query
     * performance.
     * @throws TooManyClauses if the new number of clauses exceeds the maximum
     * clause number
     */
    virtual std::shared_ptr<Builder> add(std::shared_ptr<Query> query,
                                         Occur occur);

    /** Create a new {@link BooleanQuery} based on the parameters that have
     *  been set on this builder. */
    virtual std::shared_ptr<BooleanQuery> build();
  };

private:
  const int minimumNumberShouldMatch;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::deque<std::shared_ptr<BooleanClause>>
      clauses_; // used for toString() and getClauses()
  const std::unordered_map<Occur, std::deque<std::shared_ptr<Query>>>
      clauseSets; // used for equals/hashcode

  BooleanQuery(int minimumNumberShouldMatch,
               std::deque<std::shared_ptr<BooleanClause>> &clauses);

  /**
   * Gets the minimum number of the optional BooleanClauses
   * which must be satisfied.
   */
public:
  virtual int getMinimumNumberShouldMatch();

  /** Return a deque of the clauses of this {@link BooleanQuery}. */
  virtual std::deque<std::shared_ptr<BooleanClause>> clauses();

  /** Return the collection of queries for the given {@link Occur}. */
  virtual std::shared_ptr<std::deque<std::shared_ptr<Query>>>
  getClauses(Occur occur);

  /** Returns an iterator on the clauses in this query. It implements the {@link
   * Iterable} interface to make it possible to do: <pre class="prettyprint">for
   * (BooleanClause clause : booleanQuery) {}</pre>
   */
  std::shared_ptr<Iterator<std::shared_ptr<BooleanClause>>> iterator() override;

private:
  std::shared_ptr<BooleanQuery> rewriteNoScoring();

public:
  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  /** Prints a user-readable version of this query. */
  std::wstring toString(const std::wstring &field) override;

  /**
   * Compares the specified object with this bool query for equality.
   * Returns true if and only if the provided object<ul>
   * <li>is also a {@link BooleanQuery},</li>
   * <li>has the same value of {@link #getMinimumNumberShouldMatch()}</li>
   * <li>has the same {@link Occur#SHOULD} clauses, regardless of the order</li>
   * <li>has the same {@link Occur#MUST} clauses, regardless of the order</li>
   * <li>has the same set of {@link Occur#FILTER} clauses, regardless of the
   * order and regardless of duplicates</li>
   * <li>has the same set of {@link Occur#MUST_NOT} clauses, regardless of
   * the order and regardless of duplicates</li></ul>
   */
  virtual bool equals(std::any o);

private:
  bool equalsTo(std::shared_ptr<BooleanQuery> other);

  int computeHashCode();

  // cached hash code is ok since bool queries are immutable
  // C++ NOTE: Fields cannot have the same name as methods:
  int hashCode_ = 0;

public:
  virtual int hashCode();

protected:
  std::shared_ptr<BooleanQuery> shared_from_this()
  {
    return std::static_pointer_cast<BooleanQuery>(Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
