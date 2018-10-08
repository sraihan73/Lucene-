#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/queries/payloads/PayloadFunction.h"
#include  "core/src/java/org/apache/lucene/queries/payloads/PayloadDecoder.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanScorer.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;
using FilterSpans = org::apache::lucene::search::spans::FilterSpans;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanScorer = org::apache::lucene::search::spans::SpanScorer;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;
using Spans = org::apache::lucene::search::spans::Spans;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A Query class that uses a {@link PayloadFunction} to modify the score of a
 * wrapped SpanQuery
 */
class PayloadScoreQuery : public SpanQuery
{
  GET_CLASS_NAME(PayloadScoreQuery)

private:
  const std::shared_ptr<SpanQuery> wrappedQuery;
  const std::shared_ptr<PayloadFunction> function;
  const std::shared_ptr<PayloadDecoder> decoder;
  const bool includeSpanScore;

  /**
   * Creates a new PayloadScoreQuery
   * @param wrappedQuery the query to wrap
   * @param function a PayloadFunction to use to modify the scores
   * @param decoder a PayloadDecoder to convert payloads into float values
   * @param includeSpanScore include both span score and payload score in the
   * scoring algorithm
   */
public:
  PayloadScoreQuery(std::shared_ptr<SpanQuery> wrappedQuery,
                    std::shared_ptr<PayloadFunction> function,
                    std::shared_ptr<PayloadDecoder> decoder,
                    bool includeSpanScore);

  /**
   * Creates a new PayloadScoreQuery
   * @param wrappedQuery the query to wrap
   * @param function a PayloadFunction to use to modify the scores
   * @param includeSpanScore include both span score and payload score in the
   * scoring algorithm
   * @deprecated Use {@link #PayloadScoreQuery(SpanQuery, PayloadFunction,
   * PayloadDecoder, bool)}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public
  // PayloadScoreQuery(org.apache.lucene.search.spans.SpanQuery wrappedQuery,
  // PayloadFunction function, bool includeSpanScore)
  PayloadScoreQuery(std::shared_ptr<SpanQuery> wrappedQuery,
                    std::shared_ptr<PayloadFunction> function,
                    bool includeSpanScore);

  /**
   * Creates a new PayloadScoreQuery that includes the underlying span scores
   * @param wrappedQuery the query to wrap
   * @param function a PayloadFunction to use to modify the scores
   */
  PayloadScoreQuery(std::shared_ptr<SpanQuery> wrappedQuery,
                    std::shared_ptr<PayloadFunction> function,
                    std::shared_ptr<PayloadDecoder> decoder);

  /**
   * Creates a new PayloadScoreQuery that includes the underlying span scores
   * @param wrappedQuery the query to wrap
   * @param function a PayloadFunction to use to modify the scores
   * @deprecated Use {@link #PayloadScoreQuery(SpanQuery, PayloadFunction,
   * PayloadDecoder)}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public
  // PayloadScoreQuery(org.apache.lucene.search.spans.SpanQuery wrappedQuery,
  // PayloadFunction function)
  PayloadScoreQuery(std::shared_ptr<SpanQuery> wrappedQuery,
                    std::shared_ptr<PayloadFunction> function);

  std::wstring getField() override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<PayloadScoreQuery> other);

public:
  virtual int hashCode();

private:
  class PayloadSpanWeight : public SpanWeight
  {
    GET_CLASS_NAME(PayloadSpanWeight)
  private:
    std::shared_ptr<PayloadScoreQuery> outerInstance;

    const std::shared_ptr<SpanWeight> innerWeight;

  public:
    PayloadSpanWeight(std::shared_ptr<PayloadScoreQuery> outerInstance,
                      std::shared_ptr<IndexSearcher> searcher,
                      std::shared_ptr<SpanWeight> innerWeight,
                      float boost) ;

    void extractTermContexts(
        std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &contexts) override;

    std::shared_ptr<Spans>
    getSpans(std::shared_ptr<LeafReaderContext> ctx,
             Postings requiredPostings)  override;

    std::shared_ptr<SpanScorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

  protected:
    std::shared_ptr<PayloadSpanWeight> shared_from_this()
    {
      return std::static_pointer_cast<PayloadSpanWeight>(
          org.apache.lucene.search.spans.SpanWeight::shared_from_this());
    }
  };

private:
  class PayloadSpans : public FilterSpans, public SpanCollector
  {
    GET_CLASS_NAME(PayloadSpans)
  private:
    std::shared_ptr<PayloadScoreQuery> outerInstance;

    const std::shared_ptr<PayloadDecoder> decoder;

  public:
    int payloadsSeen = 0;
    float payloadScore = 0;

  private:
    PayloadSpans(std::shared_ptr<PayloadScoreQuery> outerInstance,
                 std::shared_ptr<Spans> in_,
                 std::shared_ptr<PayloadDecoder> decoder);

  protected:
    AcceptStatus
    accept(std::shared_ptr<Spans> candidate)  override;

    void doStartCurrentDoc() override;

  public:
    void collectLeaf(std::shared_ptr<PostingsEnum> postings, int position,
                     std::shared_ptr<Term> term)  override;

    void reset() override;

  protected:
    void doCurrentSpans()  override;

  protected:
    std::shared_ptr<PayloadSpans> shared_from_this()
    {
      return std::static_pointer_cast<PayloadSpans>(
          org.apache.lucene.search.spans.FilterSpans::shared_from_this());
    }
  };

private:
  class PayloadSpanScorer : public SpanScorer
  {
    GET_CLASS_NAME(PayloadSpanScorer)
  private:
    std::shared_ptr<PayloadScoreQuery> outerInstance;

    const std::shared_ptr<PayloadSpans> spans;

    PayloadSpanScorer(
        std::shared_ptr<PayloadScoreQuery> outerInstance,
        std::shared_ptr<SpanWeight> weight, std::shared_ptr<PayloadSpans> spans,
        std::shared_ptr<Similarity::SimScorer> docScorer) ;

  protected:
    virtual float getPayloadScore();

    virtual std::shared_ptr<Explanation> getPayloadExplanation();

    virtual float getSpanScore() ;

    float scoreCurrentDoc()  override;

  protected:
    std::shared_ptr<PayloadSpanScorer> shared_from_this()
    {
      return std::static_pointer_cast<PayloadSpanScorer>(
          org.apache.lucene.search.spans.SpanScorer::shared_from_this());
    }
  };

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated private static class SimilarityPayloadDecoder
  // implements PayloadDecoder
  class SimilarityPayloadDecoder
      : public std::enable_shared_from_this<SimilarityPayloadDecoder>,
        public PayloadDecoder
  {

  public:
    const std::shared_ptr<Similarity::SimScorer> docScorer;

    SimilarityPayloadDecoder(std::shared_ptr<Similarity::SimScorer> docScorer);

    float computePayloadFactor(int docID, int startPosition, int endPosition,
                               std::shared_ptr<BytesRef> payload) override;

    float computePayloadFactor(std::shared_ptr<BytesRef> payload) override;
  };

protected:
  std::shared_ptr<PayloadScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<PayloadScoreQuery>(
        org.apache.lucene.search.spans.SpanQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/payloads/
