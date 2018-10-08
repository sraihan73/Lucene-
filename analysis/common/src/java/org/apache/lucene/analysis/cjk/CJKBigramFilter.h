#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
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
namespace org::apache::lucene::analysis::cjk
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
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

/**
 * Forms bigrams of CJK terms that are generated from StandardTokenizer
 * or ICUTokenizer.
 * <p>
 * CJK types are set by these tokenizers, but you can also use
 * {@link #CJKBigramFilter(TokenStream, int)} to explicitly control which
 * of the CJK scripts are turned into bigrams.
 * <p>
 * By default, when a CJK character has no adjacent characters to form
 * a bigram, it is output in unigram form. If you want to always output
 * both unigrams and bigrams, set the <code>outputUnigrams</code>
 * flag in {@link CJKBigramFilter#CJKBigramFilter(TokenStream, int, bool)}.
 * This can be used for a combined unigram+bigram approach.
 * <p>
 * In all cases, all non-CJK input is passed thru unmodified.
 */
class CJKBigramFilter final : public TokenFilter
{
  GET_CLASS_NAME(CJKBigramFilter)
  // configuration
  /** bigram flag for Han Ideographs */
public:
  static constexpr int HAN = 1;
  /** bigram flag for Hiragana */
  static constexpr int HIRAGANA = 2;
  /** bigram flag for Katakana */
  static constexpr int KATAKANA = 4;
  /** bigram flag for Hangul */
  static constexpr int HANGUL = 8;

  /** when we emit a bigram, it's then marked as this type */
  static const std::wstring DOUBLE_TYPE;
  /** when we emit a unigram, it's then marked as this type */
  static const std::wstring SINGLE_TYPE;

  // the types from standardtokenizer
private:
  static const std::wstring HAN_TYPE;
  static const std::wstring HIRAGANA_TYPE;
  static const std::wstring KATAKANA_TYPE;
  static const std::wstring HANGUL_TYPE;

  // sentinel value for ignoring a script
  static const std::any NO;

  // these are set to either their type or NO if we want to pass them thru
  const std::any doHan;
  const std::any doHiragana;
  const std::any doKatakana;
  const std::any doHangul;

  // true if we should output unigram tokens always
  const bool outputUnigrams;
  bool ngramState = false; // false = output unigram, true = output bigram

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLengthAtt =
      addAttribute(PositionLengthAttribute::typeid);

  // buffers containing codepoint and offsets in parallel
public:
  std::deque<int> buffer = std::deque<int>(8);
  std::deque<int> startOffset = std::deque<int>(8);
  std::deque<int> endOffset = std::deque<int>(8);
  // length of valid buffer
  int bufferLen = 0;
  // current buffer index
  int index = 0;

  // the last end offset, to determine if we should bigram across tokens
  int lastEndOffset = 0;

private:
  bool exhausted = false;

  /**
   * Calls {@link CJKBigramFilter#CJKBigramFilter(TokenStream, int)
   *       CJKBigramFilter(in, HAN | HIRAGANA | KATAKANA | HANGUL)}
   */
public:
  CJKBigramFilter(std::shared_ptr<TokenStream> in_);

  /**
   * Calls {@link CJKBigramFilter#CJKBigramFilter(TokenStream, int, bool)
   *       CJKBigramFilter(in, flags, false)}
   */
  CJKBigramFilter(std::shared_ptr<TokenStream> in_, int flags);

  /**
   * Create a new CJKBigramFilter, specifying which writing systems should be
   * bigrammed, and whether or not unigrams should also be output.
   * @param flags OR'ed set from {@link CJKBigramFilter#HAN}, {@link
   * CJKBigramFilter#HIRAGANA},
   *        {@link CJKBigramFilter#KATAKANA}, {@link CJKBigramFilter#HANGUL}
   * @param outputUnigrams true if unigrams for the selected writing systems
   * should also be output. when this is false, this is only done when there are
   * no adjacent characters to form a bigram.
   */
  CJKBigramFilter(std::shared_ptr<TokenStream> in_, int flags,
                  bool outputUnigrams);

  /*
   * much of this complexity revolves around handling the special case of a
   * "lone cjk character" where cjktokenizer would output a unigram. this
   * is also the only time we ever have to captureState.
   */
  bool incrementToken()  override;

private:
  std::shared_ptr<State> loneState; // rarely used: only for "lone cjk
                                    // characters", where we emit unigrams

  /**
   * looks at next input token, returning false is none is available
   */
  bool doNext() ;

  /**
   * refills buffers with new data from the current token.
   */
  void refill();

  /**
   * Flushes a bigram token to output from our buffer
   * This is the normal case, e.g. ABC -&gt; AB BC
   */
  void flushBigram();

  /**
   * Flushes a unigram token to output from our buffer.
   * This happens when we encounter isolated CJK characters, either the whole
   * CJK string is a single character, or we encounter a CJK character
   * surrounded by space, punctuation, english, etc, but not beside any other
   * CJK.
   */
  void flushUnigram();

  /**
   * True if we have multiple codepoints sitting in our buffer
   */
  bool hasBufferedBigram();

  /**
   * True if we have a single codepoint sitting in our buffer, where its future
   * (whether it is emitted as unigram or forms a bigram) depends upon
   * not-yet-seen inputs.
   */
  bool hasBufferedUnigram();

public:
  void reset()  override;

protected:
  std::shared_ptr<CJKBigramFilter> shared_from_this()
  {
    return std::static_pointer_cast<CJKBigramFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/cjk/
