#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/synonym/SynonymMap.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/util/State.h"
#include  "core/src/java/org/apache/lucene/util/CharsRef.h"
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/util/fst/BytesReader.h"
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
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using FST = org::apache::lucene::util::fst::FST;

/**
 * Matches single or multi word synonyms in a token stream.
 * This token stream cannot properly handle position
 * increments != 1, ie, you should place this filter before
 * filtering out stop words.
 *
 * <p>Note that with the current implementation, parsing is
 * greedy, so whenever multiple parses would apply, the rule
 * starting the earliest and parsing the most tokens wins.
 * For example if you have these rules:
 *
 * <pre>
 *   a -&gt; x
 *   a b -&gt; y
 *   b c d -&gt; z
 * </pre>
 *
 * Then input <code>a b c d e</code> parses to <code>y b c
 * d</code>, ie the 2nd rule "wins" because it started
 * earliest and matched the most input tokens of other rules
 * starting at that point.
 *
 * <p>A future improvement to this filter could allow
 * non-greedy parsing, such that the 3rd rule would win, and
 * also separately allow multiple parses, such that all 3
 * rules would match, perhaps even on a rule by rule
 * basis.</p>
 *
 * <p><b>NOTE</b>: when a match occurs, the output tokens
 * associated with the matching rule are "stacked" on top of
 * the input stream (if the rule had
 * <code>keepOrig=true</code>) and also on top of another
 * matched rule's output tokens.  This is not a correct
 * solution, as really the output should be an arbitrary
 * graph/lattice.  For example, with the above match, you
 * would expect an exact <code>PhraseQuery</code> <code>"y b
 * c"</code> to match the parsed tokens, but it will fail to
 * do so.  This limitation is necessary because Lucene's
 * TokenStream (and index) cannot yet represent an arbitrary
 * graph.</p>
 *
 * <p><b>NOTE</b>: If multiple incoming tokens arrive on the
 * same position, only the first token at that position is
 * used for parsing.  Subsequent tokens simply pass through
 * and are not parsed.  A future improvement would be to
 * allow these tokens to also be matched.</p>
 *
 * @deprecated Use {@link SynonymGraphFilter} instead, but be sure to also
 * use {@link FlattenGraphFilter} at index time (not at search time) as well.
 */

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

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public final class SynonymFilter extends
// org.apache.lucene.analysis.TokenFilter
class SynonymFilter final : public TokenFilter
{

public:
  static const std::wstring TYPE_SYNONYM;

private:
  const std::shared_ptr<SynonymMap> synonyms;

  const bool ignoreCase;
  const int rollBufferSize;

  int captureCount = 0;

  // TODO: we should set PositionLengthAttr too...

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

  // How many future input tokens have already been matched
  // to a synonym; because the matching is "greedy" we don't
  // try to do any more matching for such tokens:
  int inputSkipCount = 0;

  // Hold all buffered (read ahead) stacked input tokens for
  // a future position.  When multiple tokens are at the
  // same position, we only store (and match against) the
  // term for the first token at the position, but capture
  // state for (and enumerate) all other tokens at this
  // position:
private:
  class PendingInput : public std::enable_shared_from_this<PendingInput>
  {
    GET_CLASS_NAME(PendingInput)
  public:
    const std::shared_ptr<CharsRefBuilder> term =
        std::make_shared<CharsRefBuilder>();
    std::shared_ptr<AttributeSource::State> state;
    bool keepOrig = false;
    bool matched = false;
    bool consumed = true;
    int startOffset = 0;
    int endOffset = 0;

    virtual void reset();
  };

  // Rolling buffer, holding pending input tokens we had to
  // clone because we needed to look ahead, indexed by
  // position:
private:
  std::deque<std::shared_ptr<PendingInput>> const futureInputs;

  // Holds pending output synonyms for one future position:
private:
  class PendingOutputs : public std::enable_shared_from_this<PendingOutputs>
  {
    GET_CLASS_NAME(PendingOutputs)
  public:
    std::deque<std::shared_ptr<CharsRefBuilder>> outputs;
    std::deque<int> endOffsets;
    std::deque<int> posLengths;
    int upto = 0;
    int count = 0;
    int posIncr = 1;
    int lastEndOffset = 0;
    int lastPosLength = 0;

    PendingOutputs();

    virtual void reset();

    virtual std::shared_ptr<CharsRef> pullNext();

    virtual int getLastEndOffset();

    virtual int getLastPosLength();

    virtual void add(std::deque<wchar_t> &output, int offset, int len,
                     int endOffset, int posLength);
  };

private:
  const std::shared_ptr<ByteArrayDataInput> bytesReader =
      std::make_shared<ByteArrayDataInput>();

  // Rolling buffer, holding stack of pending synonym
  // outputs, indexed by position:
  std::deque<std::shared_ptr<PendingOutputs>> const futureOutputs;

  // Where (in rolling buffers) to write next input saved state:
  int nextWrite = 0;

  // Where (in rolling buffers) to read next input saved state:
  int nextRead = 0;

  // True once we've read last token
  bool finished = false;

  const std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> scratchArc;

  const std::shared_ptr<FST<std::shared_ptr<BytesRef>>> fst;

  const std::shared_ptr<FST::BytesReader> fstReader;

  const std::shared_ptr<BytesRef> scratchBytes = std::make_shared<BytesRef>();
  const std::shared_ptr<CharsRefBuilder> scratchChars =
      std::make_shared<CharsRefBuilder>();

  /**
   * @param input input tokenstream
   * @param synonyms synonym map_obj
   * @param ignoreCase case-folds input for matching with {@link
   * Character#toLowerCase(int)}. Note, if you set this to true, it's your
   * responsibility to lowercase the input entries when you create the {@link
   * SynonymMap}
   */
public:
  SynonymFilter(std::shared_ptr<TokenStream> input,
                std::shared_ptr<SynonymMap> synonyms, bool ignoreCase);

private:
  void capture();

  /*
   This is the core of this TokenFilter: it locates the
   synonym matches and buffers up the results into
   futureInputs/Outputs.

   NOTE: this calls input.incrementToken and does not
   capture the state if no further tokens were checked.  So
   caller must then forward state to our caller, or capture:
  */
  int lastStartOffset = 0;
  int lastEndOffset = 0;

  void parse() ;

  // Interleaves all output tokens onto the futureOutputs:
  void addOutput(std::shared_ptr<BytesRef> bytes, int matchInputLength,
                 int matchEndOffset);

  // ++ mod rollBufferSize
  int rollIncr(int count);

  // for testing
public:
  int getCaptureCount();

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<SynonymFilter> shared_from_this()
  {
    return std::static_pointer_cast<SynonymFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/synonym/
