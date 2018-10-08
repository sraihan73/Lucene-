#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionQuery.h"

#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/BulkScorer.h"
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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
namespace org::apache::lucene::search::suggest::document
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using BulkScorer = org::apache::lucene::search::BulkScorer;
using Explanation = org::apache::lucene::search::Explanation;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using IntsRef = org::apache::lucene::util::IntsRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * Expert: the Weight for CompletionQuery, used to
 * score and explain these queries.
 *
 * Subclasses can override {@link #setNextMatch(IntsRef)},
 * {@link #boost()} and {@link #context()}
GET_CLASS_NAME(es)
 * to calculate the boost and extract the context of
 * a matched path prefix.
 *
 * @lucene.experimental
 */
class CompletionWeight : public Weight
{
  GET_CLASS_NAME(CompletionWeight)
private:
  const std::shared_ptr<CompletionQuery> completionQuery;
  const std::shared_ptr<Automaton> automaton;

  /**
   * Creates a weight for <code>query</code> with an <code>automaton</code>,
   * using the <code>reader</code> for index stats
   */
public:
  CompletionWeight(std::shared_ptr<CompletionQuery> query,
                   std::shared_ptr<Automaton> automaton) ;

  /**
   * Returns the automaton specified
   * by the {@link CompletionQuery}
   *
   * @return query automaton
   */
  virtual std::shared_ptr<Automaton> getAutomaton();

  std::shared_ptr<BulkScorer> bulkScorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  /**
   * Set for every partial path in the index that matched the query
   * automaton.
   *
   * Subclasses should override {@link #boost()} and {@link #context()}
   * to return an appropriate value with respect to the current pathPrefix.
   *
   * @param pathPrefix the prefix of a matched path
   */
protected:
  virtual void setNextMatch(std::shared_ptr<IntsRef> pathPrefix);

  /**
   * Returns the boost of the partial path set by {@link #setNextMatch(IntsRef)}
   *
   * @return suggestion query-time boost
   */
  virtual float boost();

  /**
   * Returns the context of the partial path set by {@link
   * #setNextMatch(IntsRef)}
   *
   * @return suggestion context
   */
  virtual std::shared_ptr<std::wstring> context();

public:
  std::shared_ptr<Scorer> scorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  /**
   * This object can be cached
   *
   * @see
   * org.apache.lucene.search.SegmentCacheable#isCacheable(LeafReaderContext)
   */
  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  void extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> context,
          int doc)  override;

protected:
  std::shared_ptr<CompletionWeight> shared_from_this()
  {
    return std::static_pointer_cast<CompletionWeight>(
        org.apache.lucene.search.Weight::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
