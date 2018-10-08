#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanCollector.h"

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
namespace org::apache::lucene::search::spans
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

/** Matches spans which are near one another.  One can specify <i>slop</i>, the
 * maximum number of intervening unmatched positions, as well as whether
 * matches are required to be in-order.
 */
class SpanNearQuery : public SpanQuery, public Cloneable
{
  GET_CLASS_NAME(SpanNearQuery)

  /**
   * A builder for SpanNearQueries
   */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    const bool ordered;
    const std::wstring field;
    const std::deque<std::shared_ptr<SpanQuery>> clauses =
        std::deque<std::shared_ptr<SpanQuery>>();
    int slop = 0;

    /**
     * Construct a new builder
     * @param field the field to search in
     * @param ordered whether or not clauses must be in-order to match
     */
  public:
    Builder(const std::wstring &field, bool ordered);

    /**
     * Add a new clause
     */
    virtual std::shared_ptr<Builder>
    addClause(std::shared_ptr<SpanQuery> clause);

    /**
     * Add a gap after the previous clause of a defined width
     */
    virtual std::shared_ptr<Builder> addGap(int width);

    /**
     * Set the slop for this query
     */
    virtual std::shared_ptr<Builder> setSlop(int slop);

    /**
     * Build the query
     */
    virtual std::shared_ptr<SpanNearQuery> build();
  };

  /**
   * Returns a {@link Builder} for an ordered query on a particular field
   */
public:
  static std::shared_ptr<Builder>
  newOrderedNearQuery(const std::wstring &field);

  /**
   * Returns a {@link Builder} for an unordered query on a particular field
   */
  static std::shared_ptr<Builder>
  newUnorderedNearQuery(const std::wstring &field);

protected:
  std::deque<std::shared_ptr<SpanQuery>> clauses;
  int slop = 0;
  bool inOrder = false;

  std::wstring field;

  /** Construct a SpanNearQuery.  Matches spans matching a span from each
   * clause, with up to <code>slop</code> total unmatched positions between
   * them.
   * <br>When <code>inOrder</code> is true, the spans from each clause
   * must be in the same order as in <code>clauses</code> and must be
   * non-overlapping. <br>When <code>inOrder</code> is false, the spans from
   * each clause need not be ordered and may overlap.
   * @param clausesIn the clauses to find near each other, in the same field, at
   * least 2.
   * @param slop The slop value
   * @param inOrder true if order is important
   */
public:
  SpanNearQuery(std::deque<std::shared_ptr<SpanQuery>> &clausesIn, int slop,
                bool inOrder);

  /** Return the clauses whose spans are matched. */
  virtual std::deque<std::shared_ptr<SpanQuery>> getClauses();

  /** Return the maximum number of intervening unmatched positions permitted.*/
  virtual int getSlop();

  /** Return true if matches are required to be in-order.*/
  virtual bool isInOrder();

  std::wstring getField() override;
  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

public:
  class SpanNearWeight : public SpanWeight
  {
    GET_CLASS_NAME(SpanNearWeight)
  private:
    std::shared_ptr<SpanNearQuery> outerInstance;

  public:
    const std::deque<std::shared_ptr<SpanWeight>> subWeights;

    SpanNearWeight(std::shared_ptr<SpanNearQuery> outerInstance,
                   std::deque<std::shared_ptr<SpanWeight>> &subWeights,
                   std::shared_ptr<IndexSearcher> searcher,
                   std::unordered_map<std::shared_ptr<Term>,
                                      std::shared_ptr<TermContext>> &terms,
                   float boost) ;

    void extractTermContexts(
        std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &contexts) override;

    std::shared_ptr<Spans>
    getSpans(std::shared_ptr<LeafReaderContext> context,
             Postings requiredPostings)  override;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<SpanNearWeight> shared_from_this()
    {
      return std::static_pointer_cast<SpanNearWeight>(
          SpanWeight::shared_from_this());
    }
  };

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<SpanNearQuery> other);

public:
  virtual int hashCode();

private:
  class SpanGapQuery : public SpanQuery
  {
    GET_CLASS_NAME(SpanGapQuery)

  private:
    const std::wstring field;
    const int width;

  public:
    SpanGapQuery(const std::wstring &field, int width);

    std::wstring getField() override;

    std::wstring toString(const std::wstring &field) override;

    std::shared_ptr<SpanWeight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class SpanGapWeight : public SpanWeight
    {
      GET_CLASS_NAME(SpanGapWeight)
    private:
      std::shared_ptr<SpanNearQuery::SpanGapQuery> outerInstance;

    public:
      SpanGapWeight(std::shared_ptr<SpanNearQuery::SpanGapQuery> outerInstance,
                    std::shared_ptr<IndexSearcher> searcher,
                    float boost) ;

      void extractTermContexts(
          std::unordered_map<std::shared_ptr<Term>,
                             std::shared_ptr<TermContext>> &contexts) override;

      std::shared_ptr<Spans>
      getSpans(std::shared_ptr<LeafReaderContext> ctx,
               Postings requiredPostings)  override;

      void
      extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<SpanGapWeight> shared_from_this()
      {
        return std::static_pointer_cast<SpanGapWeight>(
            SpanWeight::shared_from_this());
      }
    };

  public:
    bool equals(std::any other) override;

  private:
    bool equalsTo(std::shared_ptr<SpanGapQuery> other);

  public:
    virtual int hashCode();

  protected:
    std::shared_ptr<SpanGapQuery> shared_from_this()
    {
      return std::static_pointer_cast<SpanGapQuery>(
          SpanQuery::shared_from_this());
    }
  };

public:
  class GapSpans : public Spans
  {
    GET_CLASS_NAME(GapSpans)

  public:
    int doc = -1;
    int pos = -1;
    // C++ NOTE: Fields cannot have the same name as methods:
    const int width_;

    GapSpans(int width);

    int nextStartPosition()  override;

    virtual int skipToPosition(int position) ;

    int startPosition() override;

    int endPosition() override;

    int width() override;

    void collect(std::shared_ptr<SpanCollector> collector) throw(
        IOException) override;

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

    float positionsCost() override;

  protected:
    std::shared_ptr<GapSpans> shared_from_this()
    {
      return std::static_pointer_cast<GapSpans>(Spans::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SpanNearQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanNearQuery>(
        SpanQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
