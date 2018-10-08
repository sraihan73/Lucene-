#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"

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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/** Matches spans containing a term.
 * This should not be used for terms that are indexed at position
 * Integer.MAX_VALUE.
 */
class SpanTermQuery : public SpanQuery
{
  GET_CLASS_NAME(SpanTermQuery)

protected:
  const std::shared_ptr<Term> term;
  const std::shared_ptr<TermContext> termContext;

  /** Construct a SpanTermQuery matching the named term's spans. */
public:
  SpanTermQuery(std::shared_ptr<Term> term);

  /**
   * Expert: Construct a SpanTermQuery matching the named term's spans, using
   * the provided TermContext
   */
  SpanTermQuery(std::shared_ptr<Term> term,
                std::shared_ptr<TermContext> context);

  /** Return the term whose spans are matched. */
  virtual std::shared_ptr<Term> getTerm();

  std::wstring getField() override;
  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

public:
  class SpanTermWeight : public SpanWeight
  {
    GET_CLASS_NAME(SpanTermWeight)
  private:
    std::shared_ptr<SpanTermQuery> outerInstance;

  public:
    const std::shared_ptr<TermContext> termContext;

    SpanTermWeight(std::shared_ptr<SpanTermQuery> outerInstance,
                   std::shared_ptr<TermContext> termContext,
                   std::shared_ptr<IndexSearcher> searcher,
                   std::unordered_map<std::shared_ptr<Term>,
                                      std::shared_ptr<TermContext>> &terms,
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

  protected:
    std::shared_ptr<SpanTermWeight> shared_from_this()
    {
      return std::static_pointer_cast<SpanTermWeight>(
          SpanWeight::shared_from_this());
    }
  };

  /** A guess of
   * the relative cost of dealing with the term positions
   * when using a SpanNearQuery instead of a PhraseQuery.
   */
private:
  static constexpr float PHRASE_TO_SPAN_TERM_POSITIONS_COST = 4.0f;

  static constexpr int TERM_POSNS_SEEK_OPS_PER_DOC = 128;

  static constexpr int TERM_OPS_PER_POS = 7;

  /** Returns an expected cost in simple operations
   *  of processing the occurrences of a term
   *  in a document that contains the term.
   *  <br>This may be inaccurate when {@link TermsEnum#totalTermFreq()} is not
   * available.
   *  @param termsEnum The term is the term at which this TermsEnum is
   * positioned. <p> This is a copy of
   * org.apache.lucene.search.PhraseQuery.termPositionsCost(). <br>
   *  TODO: keep only a single copy of this method and the constants used in it
   *  when SpanTermQuery moves to the o.a.l.search package.
   */
public:
  static float
  termPositionsCost(std::shared_ptr<TermsEnum> termsEnum) ;

  std::wstring toString(const std::wstring &field) override;

  virtual int hashCode();

  bool equals(std::any other) override;

protected:
  std::shared_ptr<SpanTermQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanTermQuery>(
        SpanQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
