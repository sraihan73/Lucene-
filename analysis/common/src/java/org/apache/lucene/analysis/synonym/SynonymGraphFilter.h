#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/RollingBuffer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <cctype>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/synonym/SynonymMap.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/util/fst/BytesReader.h"
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include  "core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include  "core/src/java/org/apache/lucene/analysis/synonym/BufferedOutputToken.h"
#include  "core/src/java/org/apache/lucene/analysis/synonym/BufferedInputToken.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/util/State.h"
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

namespace org::apache::lucene::analysis::synonym
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using RollingBuffer = org::apache::lucene::util::RollingBuffer;
using FST = org::apache::lucene::util::fst::FST;

// TODO: maybe we should resolve token -> wordID then run
// FST on wordIDs, for better perf?

// TODO: a more efficient approach would be Aho/Corasick's
// algorithm
// http://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_string_matching_algorithm
// It improves over the current approach here
// because it does not fully re-start matching at every
// token.  For example if one pattern is "a b c x"
// and another is "b c d" and the input is "a b c d", on
// trying to parse "a b c x" but failing when you got to x,
// rather than starting over again your really should
// immediately recognize that "b c d" matches at the next
// input.  I suspect this won't matter that much in
// practice, but it's possible on some set of synonyms it
// will.  We'd have to modify Aho/Corasick to enforce our
// conflict resolving (eg greedy matching) because that algo
// finds all matches.  This really amounts to adding a .*
// closure to the FST and then determinizing it.
//
// Another possible solution is described at
// http://www.cis.uni-muenchen.de/people/Schulz/Pub/dictle5.ps

/** Applies single- or multi-token synonyms from a {@link SynonymMap}
 *  to an incoming {@link TokenStream}, producing a fully correct graph
 *  output.  This is a replacement for {@link SynonymFilter}, which produces
 *  incorrect graphs for multi-token synonyms.
 *
 *  <p>However, if you use this during indexing, you must follow it with
 *  {@link FlattenGraphFilter} to squash tokens on top of one another
 *  like {@link SynonymFilter}, because the indexer can't directly
 *  consume a graph.  To get fully correct positional queries when your
 *  synonym replacements are multiple tokens, you should instead apply
 *  synonyms using this {@code TokenFilter} at query time and translate
 *  the resulting graph to a {@code TermAutomatonQuery} e.g. using
 *  {@code TokenStreamToTermAutomatonQuery}.
 *
 *  <p><b>NOTE</b>: this cannot consume an incoming graph; results will
 *  be undefined.
 *
 *  @lucene.experimental */

class SynonymGraphFilter final : public TokenFilter
{
  GET_CLASS_NAME(SynonymGraphFilter)

public:
  static const std::wstring TYPE_SYNONYM;

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAtt =
      addAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  const std::shared_ptr<SynonymMap> synonyms;
  const bool ignoreCase;

  const std::shared_ptr<FST<std::shared_ptr<BytesRef>>> fst;

  const std::shared_ptr<FST::BytesReader> fstReader;
  const std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> scratchArc;
  const std::shared_ptr<ByteArrayDataInput> bytesReader =
      std::make_shared<ByteArrayDataInput>();
  const std::shared_ptr<BytesRef> scratchBytes = std::make_shared<BytesRef>();
  const std::shared_ptr<CharsRefBuilder> scratchChars =
      std::make_shared<CharsRefBuilder>();
  const std::deque<std::shared_ptr<BufferedOutputToken>> outputBuffer =
      std::deque<std::shared_ptr<BufferedOutputToken>>();

  int nextNodeOut = 0;
  int lastNodeOut = 0;
  int maxLookaheadUsed = 0;

  // For testing:
  int captureCount = 0;

  bool liveToken = false;

  // Start/end offset of the current match:
  int matchStartOffset = 0;
  int matchEndOffset = 0;

  // True once the input TokenStream is exhausted:
  bool finished = false;

  int lookaheadNextRead = 0;
  int lookaheadNextWrite = 0;

  std::shared_ptr<RollingBuffer<std::shared_ptr<BufferedInputToken>>>
      lookahead = std::make_shared<RollingBufferAnonymousInnerClass>();

private:
  class RollingBufferAnonymousInnerClass
      : public RollingBuffer<std::shared_ptr<BufferedInputToken>>
  {
    GET_CLASS_NAME(RollingBufferAnonymousInnerClass)
  public:
    RollingBufferAnonymousInnerClass();

  protected:
    std::shared_ptr<BufferedInputToken> newInstance() override;

  protected:
    std::shared_ptr<RollingBufferAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RollingBufferAnonymousInnerClass>(
          org.apache.lucene.util
              .RollingBuffer<BufferedInputToken>::shared_from_this());
    }
  };

public:
  class BufferedInputToken
      : public std::enable_shared_from_this<BufferedInputToken>,
        public RollingBuffer::Resettable
  {
    GET_CLASS_NAME(BufferedInputToken)
  public:
    const std::shared_ptr<CharsRefBuilder> term =
        std::make_shared<CharsRefBuilder>();
    std::shared_ptr<AttributeSource::State> state;
    int startOffset = -1;
    int endOffset = -1;

    void reset() override;
  };

public:
  class BufferedOutputToken
      : public std::enable_shared_from_this<BufferedOutputToken>
  {
    GET_CLASS_NAME(BufferedOutputToken)
  public:
    const std::wstring term;

    // Non-null if this was an incoming token:
    const std::shared_ptr<AttributeSource::State> state;

    const int startNode;
    const int endNode;

    BufferedOutputToken(std::shared_ptr<AttributeSource::State> state,
                        const std::wstring &term, int startNode, int endNode);
  };

  /**
   * Apply previously built synonyms to incoming tokens.
   * @param input input tokenstream
   * @param synonyms synonym map_obj
   * @param ignoreCase case-folds input for matching with {@link
   * Character#toLowerCase(int)}. Note, if you set this to true, it's your
   * responsibility to lowercase the input entries when you create the {@link
   * SynonymMap}
   */
public:
  SynonymGraphFilter(std::shared_ptr<TokenStream> input,
                     std::shared_ptr<SynonymMap> synonyms, bool ignoreCase);

  bool incrementToken()  override;

private:
  void releaseBufferedToken() ;

  /** Scans the next input token(s) to see if a synonym matches.  Returns true
   *  if a match was found. */
  bool parse() ;

  /** Expands the output graph into the necessary tokens, adding
   *  synonyms as side paths parallel to the input tokens, and
   *  buffers them in the output token buffer. */
  void bufferOutputTokens(std::shared_ptr<BytesRef> bytes,
                          int matchInputLength);

  /** Buffers the current input token into lookahead buffer. */
  void capture();

public:
  void reset()  override;

  // for testing
  int getCaptureCount();

  // for testing
  int getMaxLookaheadUsed();

protected:
  std::shared_ptr<SynonymGraphFilter> shared_from_this()
  {
    return std::static_pointer_cast<SynonymGraphFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/synonym/
