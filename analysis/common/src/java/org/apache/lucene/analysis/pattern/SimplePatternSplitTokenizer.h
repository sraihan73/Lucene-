#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::util::automaton
{
class CharacterRunAutomaton;
}
namespace org::apache::lucene::util::automaton
{
class Automaton;
}
namespace org::apache::lucene::util
{
class AttributeFactory;
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
 * faster.  This is just like {@link SimplePatternTokenizer} except that the
 * pattern shold make valid token separator characters, like
 * {@code std::wstring.split}.  Empty string tokens are never produced.
 *
 * @lucene.experimental
 */

class SimplePatternSplitTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(SimplePatternSplitTokenizer)

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  const std::shared_ptr<CharacterRunAutomaton> runDFA;

  // TODO: this is copied from SimplePatternTokenizer, but there are subtle
  // differences e.g. we track sepUpto an tokenUpto; find a clean way to share
  // it:

  // TODO: we could likely use a single rolling buffer instead of two separate
  // char buffers here.  We could also use PushBackReader but I suspect it's
  // slowish:

  std::deque<wchar_t> pendingChars = std::deque<wchar_t>(8);
  int tokenUpto = 0;
  int pendingLimit = 0;
  int pendingUpto = 0;
  int offset = 0;
  int sepUpto = 0;
  std::deque<wchar_t> const buffer = std::deque<wchar_t>(1024);
  int bufferLimit = 0;
  int bufferNextRead = 0;

  /** See {@link RegExp} for the accepted syntax. */
public:
  SimplePatternSplitTokenizer(const std::wstring &regexp);

  /** Runs a pre-built automaton. */
  SimplePatternSplitTokenizer(std::shared_ptr<Automaton> dfa);

  /** See {@link RegExp} for the accepted syntax. */
  SimplePatternSplitTokenizer(std::shared_ptr<AttributeFactory> factory,
                              const std::wstring &regexp,
                              int maxDeterminizedStates);

  /** Runs a pre-built automaton. */
  SimplePatternSplitTokenizer(std::shared_ptr<AttributeFactory> factory,
                              std::shared_ptr<Automaton> dfa);

private:
  void fillToken(int offsetStart);

public:
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
  std::shared_ptr<SimplePatternSplitTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<SimplePatternSplitTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::pattern
