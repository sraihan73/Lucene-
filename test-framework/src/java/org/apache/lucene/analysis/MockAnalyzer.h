#pragma once
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenFilter.h"

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
namespace org::apache::lucene::analysis
{

using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * Analyzer for testing
 * <p>
 * This analyzer is a replacement for Whitespace/Simple/KeywordAnalyzers
 * for unit tests. If you are testing a custom component such as a queryparser
 * or analyzer-wrapper that consumes analysis streams, it's a great idea to test
 * it with this analyzer instead. MockAnalyzer has the following behavior:
 * <ul>
 *   <li>By default, the assertions in {@link MockTokenizer} are turned on for
 * extra checks that the consumer is consuming properly. These checks can be
 * disabled with {@link #setEnableChecks(bool)}. <li>Payload data is randomly
 * injected into the stream for more thorough testing of payloads.
 * </ul>
 * @see MockTokenizer
 */
class MockAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(MockAnalyzer)
private:
  const std::shared_ptr<CharacterRunAutomaton> runAutomaton;
  const bool lowerCase;
  const std::shared_ptr<CharacterRunAutomaton> filter;
  int positionIncrementGap = 0;
  std::optional<int> offsetGap;
  const std::shared_ptr<Random> random;
  std::unordered_map<std::wstring, int> previousMappings =
      std::unordered_map<std::wstring, int>();
  bool enableChecks = true;
  int maxTokenLength = MockTokenizer::DEFAULT_MAX_TOKEN_LENGTH;

  /**
   * Creates a new MockAnalyzer.
   *
   * @param random Random for payloads behavior
   * @param runAutomaton DFA describing how tokenization should happen (e.g.
   * [a-zA-Z]+)
   * @param lowerCase true if the tokenizer should lowercase terms
   * @param filter DFA describing how terms should be filtered (set of
   * stopwords, etc)
   */
public:
  MockAnalyzer(std::shared_ptr<Random> random,
               std::shared_ptr<CharacterRunAutomaton> runAutomaton,
               bool lowerCase, std::shared_ptr<CharacterRunAutomaton> filter);

  /**
   * Calls {@link #MockAnalyzer(Random, CharacterRunAutomaton, bool,
   * CharacterRunAutomaton) MockAnalyzer(random, runAutomaton, lowerCase,
   * MockTokenFilter.EMPTY_STOPSET, false}).
   */
  MockAnalyzer(std::shared_ptr<Random> random,
               std::shared_ptr<CharacterRunAutomaton> runAutomaton,
               bool lowerCase);

  /**
   * Create a Whitespace-lowercasing analyzer with no stopwords removal.
   * <p>
   * Calls {@link #MockAnalyzer(Random, CharacterRunAutomaton, bool,
   * CharacterRunAutomaton) MockAnalyzer(random, MockTokenizer.WHITESPACE, true,
   * MockTokenFilter.EMPTY_STOPSET, false}).
   */
  MockAnalyzer(std::shared_ptr<Random> random);

  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

protected:
  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<TokenFilter> maybePayload(std::shared_ptr<TokenFilter> stream,
                                            const std::wstring &fieldName);

public:
  void setPositionIncrementGap(int positionIncrementGap);

  int getPositionIncrementGap(const std::wstring &fieldName) override;

  /**
   * Set a new offset gap which will then be added to the offset when several
   * fields with the same name are indexed
   * @param offsetGap The offset gap that should be used.
   */
  void setOffsetGap(int offsetGap);

  /**
   * Get the offset gap between tokens in fields if several fields with the same
   * name were added.
   * @param fieldName Currently not used, the same offset gap is returned for
   * each field.
   */
  int getOffsetGap(const std::wstring &fieldName) override;

  /**
   * Toggle consumer workflow checking: if your test consumes tokenstreams
   * normally you should leave this enabled.
   */
  void setEnableChecks(bool enableChecks);

  /**
   * Toggle maxTokenLength for MockTokenizer
   */
  void setMaxTokenLength(int length);

protected:
  std::shared_ptr<MockAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<MockAnalyzer>(Analyzer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
