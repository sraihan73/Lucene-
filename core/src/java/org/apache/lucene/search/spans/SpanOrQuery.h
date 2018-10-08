#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/search/DisiPriorityQueue.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanPositionQueue.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
#include  "core/src/java/org/apache/lucene/search/DisjunctionDISIApproximation.h"
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

/** Matches the union of its clauses.
 */
class SpanOrQuery final : public SpanQuery
{
  GET_CLASS_NAME(SpanOrQuery)
private:
  std::deque<std::shared_ptr<SpanQuery>> clauses;
  std::wstring field;

  /** Construct a SpanOrQuery merging the provided clauses.
   * All clauses must have the same field.
   */
public:
  SpanOrQuery(std::deque<SpanQuery> &clauses);

  /** Adds a clause to this query */
private:
  void addClause(std::shared_ptr<SpanQuery> clause);

  /** Return the clauses whose spans are matched. */
public:
  std::deque<std::shared_ptr<SpanQuery>> getClauses();

  std::wstring getField() override;
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

  virtual int hashCode();

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

public:
  class SpanOrWeight : public SpanWeight
  {
    GET_CLASS_NAME(SpanOrWeight)
  private:
    std::shared_ptr<SpanOrQuery> outerInstance;

  public:
    const std::deque<std::shared_ptr<SpanWeight>> subWeights;

    SpanOrWeight(std::shared_ptr<SpanOrQuery> outerInstance,
                 std::shared_ptr<IndexSearcher> searcher,
                 std::unordered_map<std::shared_ptr<Term>,
                                    std::shared_ptr<TermContext>> &terms,
                 std::deque<std::shared_ptr<SpanWeight>> &subWeights,
                 float boost) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    void extractTermContexts(
        std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &contexts) override;

    std::shared_ptr<Spans>
    getSpans(std::shared_ptr<LeafReaderContext> context,
             Postings requiredPostings)  override;

  private:
    class SpansAnonymousInnerClass : public Spans
    {
      GET_CLASS_NAME(SpansAnonymousInnerClass)
    private:
      std::shared_ptr<SpanOrWeight> outerInstance;

      std::deque<std::shared_ptr<Spans>> subSpans;
      std::shared_ptr<DisiPriorityQueue> byDocQueue;
      std::shared_ptr<org::apache::lucene::search::spans::SpanPositionQueue>
          byPositionQueue;

    public:
      SpansAnonymousInnerClass(
          std::shared_ptr<SpanOrWeight> outerInstance,
          std::deque<std::shared_ptr<Spans>> &subSpans,
          std::shared_ptr<DisiPriorityQueue> byDocQueue,
          std::shared_ptr<org::apache::lucene::search::spans::SpanPositionQueue>
              byPositionQueue);

      std::shared_ptr<Spans> topPositionSpans;

      int nextDoc()  override;

      int advance(int target)  override;

      int docID() override;

      std::shared_ptr<TwoPhaseIterator> asTwoPhaseIterator() override;

    private:
      class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
      {
        GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
      private:
        std::shared_ptr<SpansAnonymousInnerClass> outerInstance;

        float matchCost = 0;

      public:
        TwoPhaseIteratorAnonymousInnerClass(
            std::shared_ptr<SpansAnonymousInnerClass> outerInstance,
            std::shared_ptr<DisjunctionDISIApproximation> org, float matchCost);

        bool matches()  override;

        float matchCost() override;

      protected:
        std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
              org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
        }
      };

    public:
      float positionsCost = 0;

      void computePositionsCost();

      float positionsCost() override;

      int lastDocTwoPhaseMatched = 0;

      bool twoPhaseCurrentDocMatches() ;

      void fillPositionQueue() ;

      int nextStartPosition()  override;

      int startPosition() override;

      int endPosition() override;

      int width() override;

      void collect(std::shared_ptr<SpanCollector> collector) throw(
          IOException) override;

      virtual std::wstring toString();

      int64_t cost = 0;

      int64_t cost() override;

    protected:
      std::shared_ptr<SpansAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SpansAnonymousInnerClass>(
            Spans::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<SpanOrWeight> shared_from_this()
    {
      return std::static_pointer_cast<SpanOrWeight>(
          SpanWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SpanOrQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanOrQuery>(SpanQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
