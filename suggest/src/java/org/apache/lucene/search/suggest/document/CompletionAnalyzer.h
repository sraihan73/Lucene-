#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
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
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;

/**
 * Wraps an {@link org.apache.lucene.analysis.Analyzer}
 * to provide additional completion-only tuning
 * (e.g. preserving token separators, preserving position increments while
 * converting a token stream to an automaton) <p> Can be used to index {@link
 * SuggestField} and {@link ContextSuggestField} and as a query analyzer to
 * {@link PrefixCompletionQuery} amd {@link FuzzyCompletionQuery} <p> NOTE: In
 * most cases, index and query analyzer should have same values for {@link
 * #preservePositionIncrements()} and {@link #preserveSep()}
 *
 * @lucene.experimental
 */
class CompletionAnalyzer final : public AnalyzerWrapper
{
  GET_CLASS_NAME(CompletionAnalyzer)

  /**
   * Represent a hole character, inserted by {@link
   * org.apache.lucene.analysis.TokenStreamToAutomaton}
   */
public:
  static constexpr int HOLE_CHARACTER = TokenStreamToAutomaton::HOLE;

private:
  const std::shared_ptr<Analyzer> analyzer;

  /**
   * Preserve separation between tokens
   * when converting to an automaton
   * <p>
   * Defaults to <code>true</code>
   */
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool preserveSep_;

  /**
   * Preserve position increments for tokens
   * when converting to an automaton
   * <p>
   * Defaults to <code>true</code>
   */
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool preservePositionIncrements_;

  /**
   * Sets the maximum number of graph expansions of a completion automaton
   * <p>
   * Defaults to <code>-1</code> (no limit)
   */
  const int maxGraphExpansions;

  /**
   * Wraps an analyzer to convert its output token stream to an automaton
   *
   * @param analyzer token stream to be converted to an automaton
   * @param preserveSep Preserve separation between tokens when converting to an
   * automaton
   * @param preservePositionIncrements Preserve position increments for tokens
   * when converting to an automaton
   * @param maxGraphExpansions Sets the maximum number of graph expansions of a
   * completion automaton
   */
public:
  CompletionAnalyzer(std::shared_ptr<Analyzer> analyzer, bool preserveSep,
                     bool preservePositionIncrements, int maxGraphExpansions);

  /**
   * Calls {@link #CompletionAnalyzer(org.apache.lucene.analysis.Analyzer,
   * bool, bool, int)} preserving token separation, position increments
   * and no limit on graph expansions
   */
  CompletionAnalyzer(std::shared_ptr<Analyzer> analyzer);

  /**
   * Calls {@link #CompletionAnalyzer(org.apache.lucene.analysis.Analyzer,
   * bool, bool, int)} with no limit on graph expansions
   */
  CompletionAnalyzer(std::shared_ptr<Analyzer> analyzer, bool preserveSep,
                     bool preservePositionIncrements);

  /**
   * Calls {@link #CompletionAnalyzer(org.apache.lucene.analysis.Analyzer,
   * bool, bool, int)} preserving token separation and position increments
   */
  CompletionAnalyzer(std::shared_ptr<Analyzer> analyzer,
                     int maxGraphExpansions);

  /**
   * Returns true if separation between tokens are preserved when converting
   * the token stream to an automaton
   */
  bool preserveSep();

  /**
   * Returns true if position increments are preserved when converting
   * the token stream to an automaton
   */
  bool preservePositionIncrements();

protected:
  std::shared_ptr<Analyzer>
  getWrappedAnalyzer(const std::wstring &fieldName) override;

  std::shared_ptr<Analyzer::TokenStreamComponents> wrapComponents(
      const std::wstring &fieldName,
      std::shared_ptr<Analyzer::TokenStreamComponents> components) override;

protected:
  std::shared_ptr<CompletionAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<CompletionAnalyzer>(
        org.apache.lucene.analysis.AnalyzerWrapper::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
