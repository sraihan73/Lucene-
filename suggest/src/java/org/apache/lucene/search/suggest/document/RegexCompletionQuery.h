#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/search/suggest/BitsProducer.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"

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

using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;

/**
 * A {@link CompletionQuery} which takes a regular expression
 * as the prefix of the query term.
 *
 * <p>
 * Example usage of querying a prefix of 'sug' and 'sub'
 * as a regular expression against a suggest field 'suggest_field':
 *
 * <pre class="prettyprint">
 *  CompletionQuery query = new RegexCompletionQuery(new Term("suggest_field",
 * "su[g|b]"));
 * </pre>
 *
 * <p>
 * See {@link RegExp} for the supported regular expression
 * syntax
 *
 * @lucene.experimental
 */
class RegexCompletionQuery : public CompletionQuery
{
  GET_CLASS_NAME(RegexCompletionQuery)

private:
  const int flags;
  const int maxDeterminizedStates;

  /**
   * Calls {@link RegexCompletionQuery#RegexCompletionQuery(Term, BitsProducer)}
   * with no filter
   */
public:
  RegexCompletionQuery(std::shared_ptr<Term> term);

  /**
   * Calls {@link RegexCompletionQuery#RegexCompletionQuery(Term, int, int,
   * BitsProducer)} enabling all optional regex syntax and
   * <code>maxDeterminizedStates</code> of
   * {@value Operations#DEFAULT_MAX_DETERMINIZED_STATES}
   */
  RegexCompletionQuery(std::shared_ptr<Term> term,
                       std::shared_ptr<BitsProducer> filter);
  /**
   * Calls {@link RegexCompletionQuery#RegexCompletionQuery(Term, int, int,
   * BitsProducer)} with no filter
   */
  RegexCompletionQuery(std::shared_ptr<Term> term, int flags,
                       int maxDeterminizedStates);

  /**
   * Constructs a regular expression completion query
   *
   * @param term query is run against {@link Term#field()} and {@link
   * Term#text()} is interpreted as a regular expression
   * @param flags used as syntax_flag in {@link RegExp#RegExp(std::wstring, int)}
   * @param maxDeterminizedStates used in {@link RegExp#toAutomaton(int)}
   * @param filter used to query on a sub set of documents
   */
  RegexCompletionQuery(std::shared_ptr<Term> term, int flags,
                       int maxDeterminizedStates,
                       std::shared_ptr<BitsProducer> filter);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  /**
   * Get the regex flags
   */
  virtual int getFlags();

  /**
   * Get the maximum number of states permitted in the determinized automaton
   */
  virtual int getMaxDeterminizedStates();

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<RegexCompletionQuery> shared_from_this()
  {
    return std::static_pointer_cast<RegexCompletionQuery>(
        CompletionQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
