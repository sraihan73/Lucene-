#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class PairOutputs;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class Pair;
}
#include  "core/src/java/org/apache/lucene/search/suggest/analyzing/FSTUtil.h"
namespace org::apache::lucene::search::suggest::analyzing
{
template <typename T>
class Path;
}
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamToAutomaton.h"

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
namespace org::apache::lucene::search::suggest::analyzing
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

/**
 * Implements a fuzzy {@link AnalyzingSuggester}. The similarity measurement is
 * based on the Damerau-Levenshtein (optimal string alignment) algorithm, though
 * you can explicitly choose classic Levenshtein by passing <code>false</code>
 * for the <code>transpositions</code> parameter.
 * <p>
 * At most, this query will match terms up to
 * {@value
 * org.apache.lucene.util.automaton.LevenshteinAutomata#MAXIMUM_SUPPORTED_DISTANCE}
 * edits. Higher distances are not supported.  Note that the
 * fuzzy distance is measured in "byte space" on the bytes
 * returned by the {@link TokenStream}'s {@link
 * TermToBytesRefAttribute}, usually UTF8.  By default
 * the analyzed bytes must be at least 3 {@link
 * #DEFAULT_MIN_FUZZY_LENGTH} bytes before any edits are
 * considered.  Furthermore, the first 1 {@link
 * #DEFAULT_NON_FUZZY_PREFIX} byte is not allowed to be
 * edited.  We allow up to 1 (@link
 * #DEFAULT_MAX_EDITS} edit.
 * If {@link #unicodeAware} parameter in the constructor is set to true,
 * maxEdits, minFuzzyLength, transpositions and nonFuzzyPrefix are measured in
 * Unicode code points (actual letters) instead of bytes.
 *
 * <p>
 * NOTE: This suggester does not boost suggestions that
 * required no edits over suggestions that did require
 * edits.  This is a known limitation.
 *
 * <p>
 * Note: complex query analyzers can have a significant impact on the lookup
 * performance. It's recommended to not use analyzers that drop or inject terms
 * like synonyms to keep the complexity of the prefix intersection low for good
 * lookup performance. At index time, complex analyzers can safely be used.
 * </p>
 *
 * @lucene.experimental
 */
class FuzzySuggester final : public AnalyzingSuggester
{
  GET_CLASS_NAME(FuzzySuggester)
private:
  const int maxEdits;
  const bool transpositions;
  const int nonFuzzyPrefix;
  const int minFuzzyLength;
  const bool unicodeAware;

  /** Measure maxEdits, minFuzzyLength, transpositions and nonFuzzyPrefix
   *  parameters in Unicode code points (actual letters)
   *  instead of bytes. */
public:
  static constexpr bool DEFAULT_UNICODE_AWARE = false;

  /**
   * The default minimum length of the key passed to {@link
   * #lookup} before any edits are allowed.
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

  /**
   * Creates a {@link FuzzySuggester} instance initialized with default values.
   *
   * @param analyzer the analyzer used for this suggester
   */
  FuzzySuggester(std::shared_ptr<Directory> tempDir,
                 const std::wstring &tempFileNamePrefix,
                 std::shared_ptr<Analyzer> analyzer);

  /**
   * Creates a {@link FuzzySuggester} instance with an index and query analyzer
   * initialized with default values.
   *
   * @param indexAnalyzer
   *           Analyzer that will be used for analyzing suggestions while
   * building the index.
   * @param queryAnalyzer
   *           Analyzer that will be used for analyzing query text during lookup
   */
  FuzzySuggester(std::shared_ptr<Directory> tempDir,
                 const std::wstring &tempFileNamePrefix,
                 std::shared_ptr<Analyzer> indexAnalyzer,
                 std::shared_ptr<Analyzer> queryAnalyzer);

  /**
   * Creates a {@link FuzzySuggester} instance.
   *
   * @param indexAnalyzer Analyzer that will be used for
   *        analyzing suggestions while building the index.
   * @param queryAnalyzer Analyzer that will be used for
   *        analyzing query text during lookup
   * @param options see {@link #EXACT_FIRST}, {@link #PRESERVE_SEP}
   * @param maxSurfaceFormsPerAnalyzedForm Maximum number of
   *        surface forms to keep for a single analyzed form.
   *        When there are too many surface forms we discard the
   *        lowest weighted ones.
   * @param maxGraphExpansions Maximum number of graph paths
   *        to expand from the analyzed form.  Set this to -1 for
   *        no limit.
   * @param preservePositionIncrements Whether position holes should appear in
   * the automaton
   * @param maxEdits must be &gt;= 0 and &lt;= {@link
   * LevenshteinAutomata#MAXIMUM_SUPPORTED_DISTANCE} .
   * @param transpositions <code>true</code> if transpositions should be treated
   * as a primitive edit operation. If this is false, comparisons will implement
   * the classic Levenshtein algorithm.
   * @param nonFuzzyPrefix length of common (non-fuzzy) prefix (see default
   * {@link #DEFAULT_NON_FUZZY_PREFIX}
   * @param minFuzzyLength minimum length of lookup key before any edits are
   * allowed (see default {@link #DEFAULT_MIN_FUZZY_LENGTH})
   * @param unicodeAware operate Unicode code points instead of bytes.
   */
  FuzzySuggester(std::shared_ptr<Directory> tempDir,
                 const std::wstring &tempFileNamePrefix,
                 std::shared_ptr<Analyzer> indexAnalyzer,
                 std::shared_ptr<Analyzer> queryAnalyzer, int options,
                 int maxSurfaceFormsPerAnalyzedForm, int maxGraphExpansions,
                 bool preservePositionIncrements, int maxEdits,
                 bool transpositions, int nonFuzzyPrefix, int minFuzzyLength,
                 bool unicodeAware);

protected:
  std::deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
  getFullPrefixPaths(
      std::deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
          &prefixPaths,
      std::shared_ptr<Automaton> lookupAutomaton,
      std::shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>>
          fst)  override;

  std::shared_ptr<Automaton>
  convertAutomaton(std::shared_ptr<Automaton> a) override;

public:
  std::shared_ptr<TokenStreamToAutomaton> getTokenStreamToAutomaton() override;

  std::shared_ptr<Automaton>
  toLevenshteinAutomata(std::shared_ptr<Automaton> automaton);

protected:
  std::shared_ptr<FuzzySuggester> shared_from_this()
  {
    return std::static_pointer_cast<FuzzySuggester>(
        AnalyzingSuggester::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/analyzing/
