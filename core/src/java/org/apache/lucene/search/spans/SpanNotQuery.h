#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/search/TwoPhaseIterator.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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

/** Removes matches which overlap with another SpanQuery or which are
 * within x tokens before or y tokens after another SpanQuery.
 */
class SpanNotQuery final : public SpanQuery
{
  GET_CLASS_NAME(SpanNotQuery)
private:
  std::shared_ptr<SpanQuery> include;
  std::shared_ptr<SpanQuery> exclude;
  const int pre;
  const int post;

  /** Construct a SpanNotQuery matching spans from <code>include</code> which
   * have no overlap with spans from <code>exclude</code>.*/
public:
  SpanNotQuery(std::shared_ptr<SpanQuery> include,
               std::shared_ptr<SpanQuery> exclude);

  /** Construct a SpanNotQuery matching spans from <code>include</code> which
   * have no overlap with spans from <code>exclude</code> within
   * <code>dist</code> tokens of <code>include</code>. Inversely, a negative
   * <code>dist</code> value may be used to specify a certain amount of
   * allowable overlap. */
  SpanNotQuery(std::shared_ptr<SpanQuery> include,
               std::shared_ptr<SpanQuery> exclude, int dist);

  /** Construct a SpanNotQuery matching spans from <code>include</code> which
   * have no overlap with spans from <code>exclude</code> within
   * <code>pre</code> tokens before or <code>post</code> tokens of
   * <code>include</code>. Inversely, negative values for <code>pre</code>
   * and/or <code>post</code> allow a certain amount of overlap to occur. */
  SpanNotQuery(std::shared_ptr<SpanQuery> include,
               std::shared_ptr<SpanQuery> exclude, int pre, int post);

  /** Return the SpanQuery whose matches are filtered. */
  std::shared_ptr<SpanQuery> getInclude();

  /** Return the SpanQuery whose matches must not overlap those returned. */
  std::shared_ptr<SpanQuery> getExclude();

  std::wstring getField() override;
  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

public:
  class SpanNotWeight : public SpanWeight
  {
    GET_CLASS_NAME(SpanNotWeight)
  private:
    std::shared_ptr<SpanNotQuery> outerInstance;

  public:
    const std::shared_ptr<SpanWeight> includeWeight;
    const std::shared_ptr<SpanWeight> excludeWeight;

    SpanNotWeight(std::shared_ptr<SpanNotQuery> outerInstance,
                  std::shared_ptr<IndexSearcher> searcher,
                  std::unordered_map<std::shared_ptr<Term>,
                                     std::shared_ptr<TermContext>> &terms,
                  std::shared_ptr<SpanWeight> includeWeight,
                  std::shared_ptr<SpanWeight> excludeWeight,
                  float boost) ;

    void extractTermContexts(
        std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &contexts) override;

    std::shared_ptr<Spans>
    getSpans(std::shared_ptr<LeafReaderContext> context,
             Postings requiredPostings)  override;

  private:
    class FilterSpansAnonymousInnerClass : public FilterSpans
    {
      GET_CLASS_NAME(FilterSpansAnonymousInnerClass)
    private:
      std::shared_ptr<SpanNotWeight> outerInstance;

      std::shared_ptr<org::apache::lucene::search::spans::Spans> excludeSpans;
      std::shared_ptr<TwoPhaseIterator> excludeTwoPhase;
      std::shared_ptr<DocIdSetIterator> excludeApproximation;

    public:
      FilterSpansAnonymousInnerClass(
          std::shared_ptr<SpanNotWeight> outerInstance,
          std::shared_ptr<org::apache::lucene::search::spans::Spans>
              includeSpans,
          std::shared_ptr<org::apache::lucene::search::spans::Spans>
              excludeSpans,
          std::shared_ptr<TwoPhaseIterator> excludeTwoPhase,
          std::shared_ptr<DocIdSetIterator> excludeApproximation);

      // last document we have checked matches() against for the exclusion, and
      // failed when using approximations, so we don't call it again, and pass
      // thru all inclusions.
      int lastApproxDoc = 0;
      bool lastApproxResult = false;

    protected:
      AcceptStatus
      accept(std::shared_ptr<Spans> candidate)  override;

    protected:
      std::shared_ptr<FilterSpansAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterSpansAnonymousInnerClass>(
            FilterSpans::shared_from_this());
      }
    };

  public:
    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<SpanNotWeight> shared_from_this()
    {
      return std::static_pointer_cast<SpanNotWeight>(
          SpanWeight::shared_from_this());
    }
  };

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;
  /** Returns true iff <code>o</code> is equal to this. */
  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<SpanNotQuery> other);

public:
  virtual int hashCode();

protected:
  std::shared_ptr<SpanNotQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanNotQuery>(
        SpanQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
