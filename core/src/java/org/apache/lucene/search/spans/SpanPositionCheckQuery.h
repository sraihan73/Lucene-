#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/FilterSpans.h"
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
using AcceptStatus =
    org::apache::lucene::search::spans::FilterSpans::AcceptStatus;

/**
 * Base class for filtering a SpanQuery based on the position of a match.
 **/
class SpanPositionCheckQuery : public SpanQuery, public Cloneable
{
  GET_CLASS_NAME(SpanPositionCheckQuery)
protected:
  std::shared_ptr<SpanQuery> match;

public:
  SpanPositionCheckQuery(std::shared_ptr<SpanQuery> match);

  /**
   * @return the SpanQuery whose matches are filtered.
   *
   * */
  virtual std::shared_ptr<SpanQuery> getMatch();

  std::wstring getField() override;

  /**
   * Implementing classes are required to return whether the current position is
a match for the passed in
   * "match" {@link SpanQuery}.
GET_CLASS_NAME(es)
   *
   * This is only called if the underlying last {@link
Spans#nextStartPosition()} for the
   * match indicated a valid start position.
   *
   * @param spans The {@link Spans} instance, positioned at the spot to check
   *
   * @return whether the match is accepted, rejected, or rejected and should
move to the next doc.
   *
   * @see Spans#nextDoc()
   *
   */
protected:
  virtual AcceptStatus acceptPosition(std::shared_ptr<Spans> spans) = 0;

public:
  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

public:
  class SpanPositionCheckWeight : public SpanWeight
  {
    GET_CLASS_NAME(SpanPositionCheckWeight)
  private:
    std::shared_ptr<SpanPositionCheckQuery> outerInstance;

  public:
    const std::shared_ptr<SpanWeight> matchWeight;

    SpanPositionCheckWeight(
        std::shared_ptr<SpanPositionCheckQuery> outerInstance,
        std::shared_ptr<SpanWeight> matchWeight,
        std::shared_ptr<IndexSearcher> searcher,
        std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &terms,
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
    class FilterSpansAnonymousInnerClass : public FilterSpans
    {
      GET_CLASS_NAME(FilterSpansAnonymousInnerClass)
    private:
      std::shared_ptr<SpanPositionCheckWeight> outerInstance;

    public:
      FilterSpansAnonymousInnerClass(
          std::shared_ptr<SpanPositionCheckWeight> outerInstance,
          std::shared_ptr<org::apache::lucene::search::spans::Spans>
              matchSpans);

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

  protected:
    std::shared_ptr<SpanPositionCheckWeight> shared_from_this()
    {
      return std::static_pointer_cast<SpanPositionCheckWeight>(
          SpanWeight::shared_from_this());
    }
  };

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  /** Returns true iff <code>other</code> is equal to this. */
  bool equals(std::any other) override;

  virtual int hashCode();

protected:
  std::shared_ptr<SpanPositionCheckQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanPositionCheckQuery>(
        SpanQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
