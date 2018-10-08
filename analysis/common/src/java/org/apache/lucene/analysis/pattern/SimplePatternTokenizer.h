#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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
namespace org::apache::lucene::analysis::pattern
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * This tokenizer uses a Lucene {@link RegExp} or (expert usage) a pre-built
 * determinized {@link Automaton}, to locate tokens. The regexp syntax is more
 * limited than {@link PatternTokenizer}, but the tokenization is quite a bit
 * faster.  The provided regex should match valid token characters (not token
 * separator characters, like {@code std::wstring.split}).  The matching is greedy:
 * the longest match at a given start point will be the next token.  Empty
 * string tokens are never produced.
 *
 * @lucene.experimental
 */

// TODO: the matcher here is naive and does have N^2 adversarial cases that are
// unlikely to arise in practice, e.g. if the pattern is aaaaaaaaaab and the
// input is aaaaaaaaaaa, the work we do here is N^2 where N is the number of
// a's.  This is because on failing to match a token, we skip one character
// forward and try again.  A better approach would be to compile something like
// this regexp instead: .* | <pattern>, because that automaton would not
// "forget" all the as it had already seen, and would be a single pass through
// the input.  I think this is the same thing as Aho/Corasick's algorithm
// (http://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_string_matching_algorithm).
// But we cannot implement this (I think?) until/unless Lucene regexps support
// sub-group capture, so we could know which specific characters the pattern
// matched.  SynonymFilter has this same limitation.

class SimplePatternTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(SimplePatternTokenizer)

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  const std::shared_ptr<CharacterRunAutomaton> runDFA;

  // TODO: we could likely use a single rolling buffer instead of two separate
  // char buffers here.  We could also use PushBackReader but I suspect it's
  // slowish:

  std::deque<wchar_t> pendingChars = std::deque<wchar_t>(8);
  int pendingLimit = 0;
  int pendingUpto = 0;
  int offset = 0;
  int tokenUpto = 0;
  std::deque<wchar_t> const buffer = std::deque<wchar_t>(1024);
  int bufferLimit = 0;
  int bufferNextRead = 0;

  /** See {@link RegExp} for the accepted syntax. */
public:
  SimplePatternTokenizer(const std::wstring &regexp);

  /** Runs a pre-built automaton. */
  SimplePatternTokenizer(std::shared_ptr<Automaton> dfa);

  /** See {@link RegExp} for the accepted syntax. */
  SimplePatternTokenizer(std::shared_ptr<AttributeFactory> factory,
                         const std::wstring &regexp, int maxDeterminizedStates);

  /** Runs a pre-built automaton. */
  SimplePatternTokenizer(std::shared_ptr<AttributeFactory> factory,
                         std::shared_ptr<Automaton> dfa);

  bool incrementToken()  override;

  void end()  override;

  void reset()  override;

  /** Pushes back the last {@code count} characters in current token's buffer.
   */
private:
  void pushBack(int count);

  void appendToToken(wchar_t ch);

  int nextCodeUnit() ;

  int nextCodePoint() ;

protected:
  std::shared_ptr<SimplePatternTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<SimplePatternTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/pattern/
