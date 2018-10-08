#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.automaton.Automata.makeEmpty;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.automaton.Automata.makeString;

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Operations = org::apache::lucene::util::automaton::Operations;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * A tokenfilter for testing that removes terms accepted by a DFA.
 * <ul>
 *  <li>Union a deque of singletons to act like a stopfilter.
 *  <li>Use the complement to act like a keepwordfilter
 *  <li>Use a regex like <code>.{12,}</code> to act like a lengthfilter
 * </ul>
 */
class MockTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(MockTokenFilter)
  /** Empty set of stopwords */
public:
  static const std::shared_ptr<CharacterRunAutomaton> EMPTY_STOPSET;

  /** Set of common english stopwords */
  static const std::shared_ptr<CharacterRunAutomaton> ENGLISH_STOPSET;

private:
  const std::shared_ptr<CharacterRunAutomaton> filter;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  int skippedPositions = 0;

  /**
   * Create a new MockTokenFilter.
   *
   * @param input TokenStream to filter
   * @param filter DFA representing the terms that should be removed.
   */
public:
  MockTokenFilter(std::shared_ptr<TokenStream> input,
                  std::shared_ptr<CharacterRunAutomaton> filter);

  bool incrementToken()  override;

  void end()  override;

  void reset()  override;

protected:
  std::shared_ptr<MockTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<MockTokenFilter>(
        TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
