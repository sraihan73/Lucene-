#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/queries/payloads/PayloadChecker.h"
#include  "core/src/java/org/apache/lucene/search/spans/FilterSpans.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanScorer.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"

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
namespace org::apache::lucene::queries::payloads
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using FilterSpans = org::apache::lucene::search::spans::FilterSpans;
using AcceptStatus =
    org::apache::lucene::search::spans::FilterSpans::AcceptStatus;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanScorer = org::apache::lucene::search::spans::SpanScorer;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using Spans = org::apache::lucene::search::spans::Spans;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Only return those matches that have a specific payload at the given position.
 */
class SpanPayloadCheckQuery : public SpanQuery
{
  GET_CLASS_NAME(SpanPayloadCheckQuery)

protected:
  const std::deque<std::shared_ptr<BytesRef>> payloadToMatch;
  const std::shared_ptr<SpanQuery> match;

  /**
   * @param match The underlying {@link
   * org.apache.lucene.search.spans.SpanQuery} to check
   * @param payloadToMatch The {@link java.util.List} of payloads to match
   */
public:
  SpanPayloadCheckQuery(std::shared_ptr<SpanQuery> match,
                        std::deque<std::shared_ptr<BytesRef>> &payloadToMatch);

  std::wstring getField() override;

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  /**
   * Weight that pulls its Spans using a PayloadSpanCollector
   */
public:
  class SpanPayloadCheckWeight : public SpanWeight
  {
    GET_CLASS_NAME(SpanPayloadCheckWeight)
  private:
    std::shared_ptr<SpanPayloadCheckQuery> outerInstance;

  public:
    const std::shared_ptr<SpanWeight> matchWeight;

    SpanPayloadCheckWeight(
        std::shared_ptr<SpanPayloadCheckQuery> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &termContexts,
        std::shared_ptr<SpanWeight> matchWeight,
        float boost) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

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
      std::shared_ptr<SpanPayloadCheckWeight> outerInstance;

      std::shared_ptr<org::apache::lucene::queries::payloads::
                          SpanPayloadCheckQuery::PayloadChecker>
          collector;

    public:
      FilterSpansAnonymousInnerClass(
          std::shared_ptr<SpanPayloadCheckWeight> outerInstance,
          std::shared_ptr<Spans> matchSpans,
          std::shared_ptr<org::apache::lucene::queries::payloads::
                              SpanPayloadCheckQuery::PayloadChecker>
              collector);

    protected:
      FilterSpans::AcceptStatus
      accept(std::shared_ptr<Spans> candidate)  override;

    protected:
      std::shared_ptr<FilterSpansAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterSpansAnonymousInnerClass>(
            org.apache.lucene.search.spans.FilterSpans::shared_from_this());
      }
    };

  public:
    std::shared_ptr<SpanScorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<SpanPayloadCheckWeight> shared_from_this()
    {
      return std::static_pointer_cast<SpanPayloadCheckWeight>(
          org.apache.lucene.search.spans.SpanWeight::shared_from_this());
    }
  };

private:
  class PayloadChecker : public std::enable_shared_from_this<PayloadChecker>,
                         public SpanCollector
  {
    GET_CLASS_NAME(PayloadChecker)
  private:
    std::shared_ptr<SpanPayloadCheckQuery> outerInstance;

  public:
    PayloadChecker(std::shared_ptr<SpanPayloadCheckQuery> outerInstance);

    int upto = 0;
    bool matches = true;

    void collectLeaf(std::shared_ptr<PostingsEnum> postings, int position,
                     std::shared_ptr<Term> term)  override;

    virtual FilterSpans::AcceptStatus match();

    void reset() override;
  };

public:
  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

  virtual int hashCode();

protected:
  std::shared_ptr<SpanPayloadCheckQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanPayloadCheckQuery>(
        org.apache.lucene.search.spans.SpanQuery::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/queries/payloads/
