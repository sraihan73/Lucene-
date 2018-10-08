#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cctype>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
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
namespace org::apache::lucene::analysis
{

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;

using com::carrotsearch::randomizedtesting::RandomizedContext;

/**
 * Tokenizer for testing.
 * <p>
 * This tokenizer is a replacement for {@link #WHITESPACE}, {@link #SIMPLE}, and
 * {@link #KEYWORD} tokenizers. If you are writing a component such as a
 * TokenFilter, it's a great idea to test it wrapping this tokenizer instead for
 * extra checks. This tokenizer has the following behavior: <ul> <li>An internal
 * state-machine is used for checking consumer consistency. These checks can be
 * disabled with {@link #setEnableChecks(bool)}. <li>For convenience,
 * optionally lowercases terms that it outputs.
 * </ul>
 */
class MockTokenizer : public Tokenizer
{
  GET_CLASS_NAME(MockTokenizer)
  /** Acts Similar to WhitespaceTokenizer */
public:
  static const std::shared_ptr<CharacterRunAutomaton> WHITESPACE;
  /** Acts Similar to KeywordTokenizer.
   * TODO: Keyword returns an "empty" token for an empty reader...
   */
  static const std::shared_ptr<CharacterRunAutomaton> KEYWORD;
  /** Acts like LetterTokenizer. */
  // the ugly regex below is incomplete Unicode 5.2 [:Letter:]
  static const std::shared_ptr<CharacterRunAutomaton> SIMPLE;

private:
  const std::shared_ptr<CharacterRunAutomaton> runAutomaton;
  const bool lowerCase;
  const int maxTokenLength;

public:
  static const int DEFAULT_MAX_TOKEN_LENGTH = std::numeric_limits<int>::max();

private:
  int state = 0;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

public:
  int off = 0;

  // buffered state (previous codepoint and offset). we replay this once we
  // hit a reject state in case it's permissible as the start of a new term.
  int bufferedCodePoint = -1; // -1 indicates empty buffer
  int bufferedOff = -1;

  // TODO: "register" with LuceneTestCase to ensure all streams are closed() ?
  // currently, we can only check that the lifecycle is correct if someone is
  // reusing, but not for "one-offs".
private:
  enum class State {
    GET_CLASS_NAME(State) SETREADER, // consumer set a reader input either via
                                     // ctor or via reset(Reader)
    RESET,                           // consumer has called reset()
    INCREMENT, // consumer is consuming, has called incrementToken() == true
    INCREMENT_FALSE, // consumer has called incrementToken() which returned
                     // false
    END,  // consumer has called end() to perform end of stream operations
    CLOSE // consumer has called close() to release any resources
  };

private:
  State streamState = State::CLOSE;
  int lastOffset = 0; // only for checks
  bool enableChecks = true;

  // evil: but we don't change the behavior with this random, we only switch up
  // how we read
  const std::shared_ptr<Random> random = std::make_shared<Random>(
      RandomizedContext::current().getRandom().nextLong());

public:
  MockTokenizer(std::shared_ptr<AttributeFactory> factory,
                std::shared_ptr<CharacterRunAutomaton> runAutomaton,
                bool lowerCase, int maxTokenLength);

  MockTokenizer(std::shared_ptr<CharacterRunAutomaton> runAutomaton,
                bool lowerCase, int maxTokenLength);

  MockTokenizer(std::shared_ptr<CharacterRunAutomaton> runAutomaton,
                bool lowerCase);

  /** Calls {@link #MockTokenizer(CharacterRunAutomaton, bool)
   * MockTokenizer(Reader, WHITESPACE, true)} */
  MockTokenizer();

  MockTokenizer(std::shared_ptr<AttributeFactory> factory,
                std::shared_ptr<CharacterRunAutomaton> runAutomaton,
                bool lowerCase);

  /** Calls {@link
   * #MockTokenizer(AttributeFactory,CharacterRunAutomaton,bool)
   *                MockTokenizer(AttributeFactory, Reader, WHITESPACE, true)}
   */
  MockTokenizer(std::shared_ptr<AttributeFactory> factory);

  // we allow some checks (e.g. state machine) to be turned off.
  // turning off checks just means we suppress exceptions from them
private:
  void fail(const std::wstring &message);

  void failAlways(const std::wstring &message);

public:
  bool incrementToken()  override final;

protected:
  virtual int readCodePoint() ;

  virtual int readChar() ;

  virtual bool isTokenChar(int c);

  virtual int normalize(int c);

public:
  void reset()  override;

  virtual ~MockTokenizer();

  void setReaderTestPoint() override;

  void end()  override;

  /**
   * Toggle consumer workflow checking: if your test consumes tokenstreams
   * normally you should leave this enabled.
   */
  virtual void setEnableChecks(bool enableChecks);

protected:
  std::shared_ptr<MockTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<MockTokenizer>(
        Tokenizer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
