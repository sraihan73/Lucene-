#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionAnalyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;
using BitsProducer = org::apache::lucene::search::suggest::BitsProducer;

/**
 * A {@link CompletionQuery} which takes an {@link Analyzer}
 * to analyze the prefix of the query term.
 * <p>
 * Example usage of querying an analyzed prefix 'sugg'
 * against a field 'suggest_field' is as follows:
 *
 * <pre class="prettyprint">
 *  CompletionQuery query = new PrefixCompletionQuery(analyzer, new
 * Term("suggest_field", "sugg"));
 * </pre>
 * @lucene.experimental
 */
class PrefixCompletionQuery : public CompletionQuery
{
  GET_CLASS_NAME(PrefixCompletionQuery)
  /** Used to analyze the term text */
protected:
  const std::shared_ptr<CompletionAnalyzer> analyzer;

  /**
   * Calls {@link PrefixCompletionQuery#PrefixCompletionQuery(Analyzer, Term,
   * BitsProducer)} with no filter
   */
public:
  PrefixCompletionQuery(std::shared_ptr<Analyzer> analyzer,
                        std::shared_ptr<Term> term);

  /**
   * Constructs an analyzed prefix completion query
   *
   * @param analyzer used to analyze the provided {@link Term#text()}
   * @param term query is run against {@link Term#field()} and {@link
   * Term#text()} is analyzed with <code>analyzer</code>
   * @param filter used to query on a sub set of documents
   */
  PrefixCompletionQuery(std::shared_ptr<Analyzer> analyzer,
                        std::shared_ptr<Term> term,
                        std::shared_ptr<BitsProducer> filter);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  /**
   * Gets the analyzer used to analyze the prefix.
   */
  virtual std::shared_ptr<Analyzer> getAnalyzer();

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<PrefixCompletionQuery> shared_from_this()
  {
    return std::static_pointer_cast<PrefixCompletionQuery>(
        CompletionQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
