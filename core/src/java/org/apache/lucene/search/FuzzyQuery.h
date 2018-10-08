#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class AttributeSource;
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
namespace org::apache::lucene::search
{

using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;

/** Implements the fuzzy search query. The similarity measurement
 * is based on the Damerau-Levenshtein (optimal string alignment) algorithm,
 * though you can explicitly choose classic Levenshtein by passing
 * <code>false</code> to the <code>transpositions</code> parameter.
 *
 * <p>This query uses {@link MultiTermQuery.TopTermsBlendedFreqScoringRewrite}
 * as default. So terms will be collected and scored according to their
 * edit distance. Only the top terms are used for building the {@link
 * BooleanQuery}. It is not recommended to change the rewrite mode for fuzzy
 * queries.
 *
 * <p>At most, this query will match terms up to
 * {@value
 * org.apache.lucene.util.automaton.LevenshteinAutomata#MAXIMUM_SUPPORTED_DISTANCE}
 * edits. Higher distances (especially with transpositions enabled), are
 * generally not useful and will match a significant amount of the term
 * dictionary. If you really want this, consider using an n-gram indexing
 * technique (such as the SpellChecker in the <a
 * href="{@docRoot}/../suggest/overview-summary.html">suggest module</a>)
 * instead.
 *
 * <p>NOTE: terms of length 1 or 2 will sometimes not match because of how the
 * scaled distance between two terms is computed.  For a term to match, the edit
 * distance between the terms must be less than the minimum length term (either
 * the input term, or the candidate term).  For example, FuzzyQuery on term
 * "abcd" with maxEdits=2 will not match an indexed term "ab", and FuzzyQuery on
 * term "a" with maxEdits=2 will not match an indexed term "abc".
 */
class FuzzyQuery : public MultiTermQuery
{
  GET_CLASS_NAME(FuzzyQuery)

public:
  static constexpr int defaultMaxEdits =
      LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE;
  static constexpr int defaultPrefixLength = 0;
  static constexpr int defaultMaxExpansions = 50;
  static constexpr bool defaultTranspositions = true;

private:
  const int maxEdits;
  const int maxExpansions;
  const bool transpositions;
  const int prefixLength;
  const std::shared_ptr<Term> term;

  /**
   * Create a new FuzzyQuery that will match terms with an edit distance
   * of at most <code>maxEdits</code> to <code>term</code>.
   * If a <code>prefixLength</code> &gt; 0 is specified, a common prefix
   * of that length is also required.
   *
   * @param term the term to search for
   * @param maxEdits must be {@code >= 0} and {@code <=} {@link
   * LevenshteinAutomata#MAXIMUM_SUPPORTED_DISTANCE}.
   * @param prefixLength length of common (non-fuzzy) prefix
   * @param maxExpansions the maximum number of terms to match. If this number
   * is greater than {@link BooleanQuery#getMaxClauseCount} when the query is
   * rewritten, then the maxClauseCount will be used instead.
   * @param transpositions true if transpositions should be treated as a
   * primitive edit operation. If this is false, comparisons will implement the
   * classic Levenshtein algorithm.
   */
public:
  FuzzyQuery(std::shared_ptr<Term> term, int maxEdits, int prefixLength,
             int maxExpansions, bool transpositions);

  /**
   * Calls {@link #FuzzyQuery(Term, int, int, int, bool)
   * FuzzyQuery(term, maxEdits, prefixLength, defaultMaxExpansions,
   * defaultTranspositions)}.
   */
  FuzzyQuery(std::shared_ptr<Term> term, int maxEdits, int prefixLength);

  /**
   * Calls {@link #FuzzyQuery(Term, int, int) FuzzyQuery(term, maxEdits,
   * defaultPrefixLength)}.
   */
  FuzzyQuery(std::shared_ptr<Term> term, int maxEdits);

  /**
   * Calls {@link #FuzzyQuery(Term, int) FuzzyQuery(term, defaultMaxEdits)}.
   */
  FuzzyQuery(std::shared_ptr<Term> term);

  /**
   * @return the maximum number of edit distances allowed for this query to
   * match.
   */
  virtual int getMaxEdits();

  /**
   * Returns the non-fuzzy prefix length. This is the number of characters at
   * the start of a term that must be identical (not fuzzy) to the query term if
   * the query is to match that term.
   */
  virtual int getPrefixLength();

  /**
   * Returns true if transpositions should be treated as a primitive edit
   * operation. If this is false, comparisons will implement the classic
   * Levenshtein algorithm.
   */
  virtual bool getTranspositions();

protected:
  std::shared_ptr<TermsEnum> getTermsEnum(
      std::shared_ptr<Terms> terms,
      std::shared_ptr<AttributeSource> atts)  override;

  /**
   * Returns the pattern term.
   */
public:
  virtual std::shared_ptr<Term> getTerm();

  std::wstring toString(const std::wstring &field) override;

  virtual int hashCode();

  bool equals(std::any obj) override;

  /**
   * @deprecated pass integer edit distances instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public final static float defaultMinSimilarity =
  // org.apache.lucene.util.automaton.LevenshteinAutomata.MAXIMUM_SUPPORTED_DISTANCE;
  static constexpr float defaultMinSimilarity =
      LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE;

  /**
   * Helper function to convert from deprecated "minimumSimilarity" fractions
   * to raw edit distances.
   *
   * @param minimumSimilarity scaled similarity
   * @param termLen length (in unicode codepoints) of the term.
   * @return equivalent number of maxEdits
   * @deprecated pass integer edit distances instead.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static int floatToEdits(float
  // minimumSimilarity, int termLen)
  static int floatToEdits(float minimumSimilarity, int termLen);

protected:
  std::shared_ptr<FuzzyQuery> shared_from_this()
  {
    return std::static_pointer_cast<FuzzyQuery>(
        MultiTermQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
