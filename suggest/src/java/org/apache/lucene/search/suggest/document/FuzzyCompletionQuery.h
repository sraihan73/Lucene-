#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search::suggest
{
class BitsProducer;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::util
{
class IntsRef;
}
namespace org::apache::lucene::util::automaton
{
class Automaton;
}
namespace org::apache::lucene::search::suggest::document
{
class CompletionQuery;
}

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
using IntsRef = org::apache::lucene::util::IntsRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * A {@link CompletionQuery} that match documents containing terms
 * within an edit distance of the specified prefix.
 * <p>
 * This query boost documents relative to how similar the indexed terms are to
 * the provided prefix. <p> Example usage of querying an analyzed prefix within
 * an edit distance of 1 of 'subg' against a field 'suggest_field' is as
 * follows:
 *
 * <pre class="prettyprint">
 *  CompletionQuery query = new FuzzyCompletionQuery(analyzer, new
 * Term("suggest_field", "subg"));
 * </pre>
 *
 * @lucene.experimental
 */
class FuzzyCompletionQuery : public PrefixCompletionQuery
{
  GET_CLASS_NAME(FuzzyCompletionQuery)

  /**
   * Measure maxEdits, minFuzzyLength, transpositions and nonFuzzyPrefix
   * parameters in Unicode code points (actual letters)
   * instead of bytes.
   * */
public:
  static constexpr bool DEFAULT_UNICODE_AWARE = false;

  /**
   * The default minimum length of the key before any edits are allowed.
   */
  static constexpr int DEFAULT_MIN_FUZZY_LENGTH = 3;

  /**
   * The default prefix length where edits are not allowed.
   */
  static constexpr int DEFAULT_NON_FUZZY_PREFIX = 1;

  /**
   * The default maximum number of edits for fuzzy
   * suggestions.
   */
  static constexpr int DEFAULT_MAX_EDITS = 1;

  /**
   * The default transposition value passed to {@link LevenshteinAutomata}
   */
  static constexpr bool DEFAULT_TRANSPOSITIONS = true;

private:
  const int maxEdits;
  const bool transpositions;
  const int nonFuzzyPrefix;
  const int minFuzzyLength;
  const bool unicodeAware;
  const int maxDeterminizedStates;

  /**
   * Calls {@link FuzzyCompletionQuery#FuzzyCompletionQuery(Analyzer, Term,
   * BitsProducer)} with no filter
   */
public:
  FuzzyCompletionQuery(std::shared_ptr<Analyzer> analyzer,
                       std::shared_ptr<Term> term);

  /**
   * Calls {@link FuzzyCompletionQuery#FuzzyCompletionQuery(Analyzer, Term,
   * BitsProducer, int, bool, int, int, bool, int)} with defaults for
   * <code>maxEdits</code>, <code>transpositions</code>,
   * <code>nonFuzzyPrefix</code>, <code>minFuzzyLength</code>,
   * <code>unicodeAware</code> and <code>maxDeterminizedStates</code>
   *
   * See {@link #DEFAULT_MAX_EDITS}, {@link #DEFAULT_TRANSPOSITIONS},
   * {@link #DEFAULT_NON_FUZZY_PREFIX}, {@link #DEFAULT_MIN_FUZZY_LENGTH},
   * {@link #DEFAULT_UNICODE_AWARE} and {@link
   * Operations#DEFAULT_MAX_DETERMINIZED_STATES} for defaults
   */
  FuzzyCompletionQuery(std::shared_ptr<Analyzer> analyzer,
                       std::shared_ptr<Term> term,
                       std::shared_ptr<BitsProducer> filter);

  /**
   * Constructs an analyzed fuzzy prefix completion query
   *
   * @param analyzer used to analyze the provided {@link Term#text()}
   * @param term query is run against {@link Term#field()} and {@link
   * Term#text()} is analyzed with <code>analyzer</code>
   * @param filter used to query on a sub set of documents
   * @param maxEdits maximum number of acceptable edits
   * @param transpositions value passed to {@link LevenshteinAutomata}
   * @param nonFuzzyPrefix prefix length where edits are not allowed
   * @param minFuzzyLength minimum prefix length before any edits are allowed
   * @param unicodeAware treat prefix as unicode rather than bytes
   * @param maxDeterminizedStates maximum automaton states allowed for {@link
   * LevenshteinAutomata}
   */
  FuzzyCompletionQuery(std::shared_ptr<Analyzer> analyzer,
                       std::shared_ptr<Term> term,
                       std::shared_ptr<BitsProducer> filter, int maxEdits,
                       bool transpositions, int nonFuzzyPrefix,
                       int minFuzzyLength, bool unicodeAware,
                       int maxDeterminizedStates);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  std::shared_ptr<Automaton>
  toLevenshteinAutomata(std::shared_ptr<Automaton> automaton,
                        std::shared_ptr<Set<std::shared_ptr<IntsRef>>> refs);

  /**
   * Get the maximum edit distance for fuzzy matches
   */
public:
  virtual int getMaxEdits();

  /**
   * Return whether transpositions count as a single edit
   */
  virtual bool isTranspositions();

  /**
   * Get the length of a prefix where no edits are permitted
   */
  virtual int getNonFuzzyPrefix();

  /**
   * Get the minimum length of a term considered for matching
   */
  virtual int getMinFuzzyLength();

  /**
   * Return true if lengths are measured in unicode code-points rather than
   * bytes
   */
  virtual bool isUnicodeAware();

  /**
   * Get the maximum number of determinized states permitted
   */
  virtual int getMaxDeterminizedStates();

  std::wstring toString(const std::wstring &field) override;

private:
  class FuzzyCompletionWeight : public CompletionWeight
  {
    GET_CLASS_NAME(FuzzyCompletionWeight)
  private:
    const std::shared_ptr<Set<std::shared_ptr<IntsRef>>> refs;

  public:
    int currentBoost = 0;

    FuzzyCompletionWeight(
        std::shared_ptr<CompletionQuery> query,
        std::shared_ptr<Automaton> automaton,
        std::shared_ptr<Set<std::shared_ptr<IntsRef>>> refs) ;

  protected:
    void setNextMatch(std::shared_ptr<IntsRef> pathPrefix) override;

    float boost() override;

  protected:
    std::shared_ptr<FuzzyCompletionWeight> shared_from_this()
    {
      return std::static_pointer_cast<FuzzyCompletionWeight>(
          CompletionWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<FuzzyCompletionQuery> shared_from_this()
  {
    return std::static_pointer_cast<FuzzyCompletionQuery>(
        PrefixCompletionQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
