#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStreamToAutomaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TermToBytesRefAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeImpl.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/Operations.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::miscellaneous
{
class BytesRefBuilderTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::util::automaton
{
class LimitedFiniteStringsIterator;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::util::automaton
{
class Automaton;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class CharsRefBuilder;
}
namespace org::apache::lucene::util
{
class AttributeImpl;
}
namespace org::apache::lucene::util
{
class AttributeReflector;
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
namespace org::apache::lucene::analysis::miscellaneous
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenStreamToAutomaton =
    org::apache::lucene::analysis::TokenStreamToAutomaton;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using LimitedFiniteStringsIterator =
    org::apache::lucene::util::automaton::LimitedFiniteStringsIterator;
using Operations = org::apache::lucene::util::automaton::Operations;

/**
 * Concatenates/Joins every incoming token with a separator into one output
 * token for every path through the token stream (which is a graph).  In simple
 * cases this yields one token, but in the presence of any tokens with a zero
 * positionIncrmeent (e.g. synonyms) it will be more.  This filter uses the
 * token bytes, position increment, and position length of the incoming stream.
 * Other attributes are not used or manipulated.
 *
 * @lucene.experimental
 */
class ConcatenateGraphFilter final : public TokenStream
{
  GET_CLASS_NAME(ConcatenateGraphFilter)

  /*
   * Token stream which converts a provided token stream to an automaton.
   * The accepted strings enumeration from the automaton are available through
   * the
   * {@link org.apache.lucene.analysis.tokenattributes.TermToBytesRefAttribute}
   * attribute The token stream uses a {@link
   * org.apache.lucene.analysis.tokenattributes.PayloadAttribute} to store a
   * completion's payload (see {@link
   * ConcatenateGraphFilter#setPayload(org.apache.lucene.util.BytesRef)})
   */

  /**
   * Represents the separation between tokens, if
   * <code>preserveSep</code> is <code>true</code>.
   */
public:
  static constexpr int SEP_LABEL = TokenStreamToAutomaton::POS_SEP;
  static constexpr int DEFAULT_MAX_GRAPH_EXPANSIONS =
      Operations::DEFAULT_MAX_DETERMINIZED_STATES;
  static constexpr bool DEFAULT_PRESERVE_SEP = true;
  static constexpr bool DEFAULT_PRESERVE_POSITION_INCREMENTS = true;

private:
  const std::shared_ptr<BytesRefBuilderTermAttribute> bytesAtt =
      addAttribute(BytesRefBuilderTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  const std::shared_ptr<TokenStream> inputTokenStream;
  const bool preserveSep;
  const bool preservePositionIncrements;
  const int maxGraphExpansions;

  std::shared_ptr<LimitedFiniteStringsIterator> finiteStrings;
  std::shared_ptr<CharTermAttribute> charTermAttribute;
  bool wasReset = false;
  int endOffset = 0;

  /**
   * Creates a token stream to convert <code>input</code> to a token stream
   * of accepted strings by its token stream graph.
   * <p>
   * This constructor uses the default settings of the constants in this class.
   */
public:
  ConcatenateGraphFilter(std::shared_ptr<TokenStream> inputTokenStream);

  /**
   * Creates a token stream to convert <code>input</code> to a token stream
   * of accepted strings by its token stream graph.
   *
   * @param inputTokenStream The input/incoming TokenStream
   * @param preserveSep Whether {@link #SEP_LABEL} should separate the input
   * tokens in the concatenated token
   * @param preservePositionIncrements Whether to add an empty token for missing
   * positions. The effect is a consecutive {@link #SEP_LABEL}. When false, it's
   * as if there were no missing positions (we pretend the surrounding tokens
   * were adjacent).
   * @param maxGraphExpansions If the tokenStream graph has more than this many
   * possible paths through, then we'll throw
   *                           {@link TooComplexToDeterminizeException} to
   * preserve the stability and memory of the machine.
   * @throws TooComplexToDeterminizeException if the tokenStream graph has more
   * than {@code maxGraphExpansions} expansions
   *
   */
  ConcatenateGraphFilter(std::shared_ptr<TokenStream> inputTokenStream,
                         bool preserveSep, bool preservePositionIncrements,
                         int maxGraphExpansions);

  void reset()  override;

  bool incrementToken()  override;

  void end()  override;

  virtual ~ConcatenateGraphFilter();

  /**
   * Converts the tokenStream to an automaton, treating the transition labels as
   * utf-8.  Does *not* close it.
   */
  std::shared_ptr<Automaton> toAutomaton() ;

  /**
   * Converts the tokenStream to an automaton.  Does *not* close it.
   */
  std::shared_ptr<Automaton> toAutomaton(bool unicodeAware) ;

  /**
   * Just escapes the {@link #SEP_LABEL} byte with an extra.
   */
private:
  class EscapingTokenStreamToAutomaton final : public TokenStreamToAutomaton
  {
    GET_CLASS_NAME(EscapingTokenStreamToAutomaton)

  public:
    const std::shared_ptr<BytesRefBuilder> spare =
        std::make_shared<BytesRefBuilder>();
    const char sepLabel;

    EscapingTokenStreamToAutomaton(int sepLabel);

  protected:
    std::shared_ptr<BytesRef>
    changeToken(std::shared_ptr<BytesRef> in_) override;

  protected:
    std::shared_ptr<EscapingTokenStreamToAutomaton> shared_from_this()
    {
      return std::static_pointer_cast<EscapingTokenStreamToAutomaton>(
          org.apache.lucene.analysis
              .TokenStreamToAutomaton::shared_from_this());
    }
  };

  // Replaces SEP with epsilon or remaps them if
  // we were asked to preserve them:
private:
  static std::shared_ptr<Automaton> replaceSep(std::shared_ptr<Automaton> a,
                                               bool preserveSep, int sepLabel);

  /**
   * Attribute providing access to the term builder and UTF-16 conversion
   * @lucene.internal
   */
public:
  class BytesRefBuilderTermAttribute : public TermToBytesRefAttribute
  {
    GET_CLASS_NAME(BytesRefBuilderTermAttribute)
    /**
     * Returns the builder from which the term is derived.
     */
  public:
    virtual std::shared_ptr<BytesRefBuilder> builder() = 0;

    /**
     * Returns the term represented as UTF-16
     */
    virtual std::shared_ptr<std::wstring> toUTF16() = 0;
  };

  /**
   * Implementation of {@link BytesRefBuilderTermAttribute}
   * @lucene.internal
   */
public:
  class BytesRefBuilderTermAttributeImpl final
      : public AttributeImpl,
        public BytesRefBuilderTermAttribute,
        public TermToBytesRefAttribute
  {
    GET_CLASS_NAME(BytesRefBuilderTermAttributeImpl)
  private:
    const std::shared_ptr<BytesRefBuilder> bytes =
        std::make_shared<BytesRefBuilder>();
    std::shared_ptr<CharsRefBuilder> charsRef;

    /**
     * Sole constructor
     * no-op
     */
  public:
    BytesRefBuilderTermAttributeImpl();

    std::shared_ptr<BytesRefBuilder> builder() override;

    std::shared_ptr<BytesRef> getBytesRef() override;

    void clear() override;

    void copyTo(std::shared_ptr<AttributeImpl> target) override;

    std::shared_ptr<AttributeImpl> clone() override;

    void reflectWith(AttributeReflector reflector) override;

    std::shared_ptr<std::wstring> toUTF16() override;

  protected:
    std::shared_ptr<BytesRefBuilderTermAttributeImpl> shared_from_this()
    {
      return std::static_pointer_cast<BytesRefBuilderTermAttributeImpl>(
          org.apache.lucene.util.AttributeImpl::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ConcatenateGraphFilter> shared_from_this()
  {
    return std::static_pointer_cast<ConcatenateGraphFilter>(
        org.apache.lucene.analysis.TokenStream::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
