#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/InPlaceMergeSorter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/WordDelimiterIterator.h"
#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/WordDelimiterConcatenation.h"
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
namespace org::apache::lucene::analysis::miscellaneous
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;

/**
 * Splits words into subwords and performs optional transformations on subword
 * groups, producing a correct token graph so that e.g. {@link PhraseQuery} can
 * work correctly when this filter is used in the search-time analyzer.  Unlike
 * the deprecated {@link WordDelimiterFilter}, this token filter produces a
 * correct token graph as output.  However, it cannot consume an input token
 * graph correctly. Processing is suppressed by {@link
 * KeywordAttribute#isKeyword()}=true.
 *
 * <p>
 * Words are split into subwords with the following rules:
 * <ul>
 * <li>split on intra-word delimiters (by default, all non alpha-numeric
 * characters): <code>"Wi-Fi"</code> &#8594; <code>"Wi", "Fi"</code></li>
 * <li>split on case transitions: <code>"PowerShot"</code> &#8594;
 * <code>"Power", "Shot"</code></li>
 * <li>split on letter-number transitions: <code>"SD500"</code> &#8594;
 * <code>"SD", "500"</code></li>
 * <li>leading and trailing intra-word delimiters on each subword are ignored:
 * <code>"//hello---there, 'dude'"</code> &#8594;
 * <code>"hello", "there", "dude"</code></li>
 * <li>trailing "'s" are removed for each subword: <code>"O'Neil's"</code>
 * &#8594; <code>"O", "Neil"</code>
 * <ul>
 * <li>Note: this step isn't performed in a separate filter because of possible
 * subword combinations.</li>
 * </ul>
 * </li>
 * </ul>
 *
 * The <b>GENERATE...</b> options affect how incoming tokens are broken into
 * parts, and the various <b>CATENATE_...</b> parameters affect how those parts
 * are combined.
 *
 * <ul>
 * <li>If no CATENATE option is set, then no subword combinations are generated:
 * <code>"PowerShot"</code> &#8594; <code>0:"Power", 1:"Shot"</code> (0 and 1
 * are the token positions)</li> <li>CATENATE_WORDS means that in addition to
 * the subwords, maximum runs of non-numeric subwords are catenated and produced
 * at the same position of the last subword in the run: <ul>
 * <li><code>"PowerShot"</code> &#8594;
 * <code>0:"Power", 1:"Shot" 1:"PowerShot"</code></li>
 * <li><code>"A's+B's&amp;C's"</code> &gt; <code>0:"A", 1:"B", 2:"C",
 * 2:"ABC"</code>
 * </li>
 * <li><code>"Super-Duper-XL500-42-AutoCoder!"</code> &#8594;
 * <code>0:"Super", 1:"Duper", 2:"XL", 2:"SuperDuperXL", 3:"500" 4:"42",
 * 5:"Auto", 6:"Coder", 6:"AutoCoder"</code>
 * </li>
 * </ul>
 * </li>
 * <li>CATENATE_NUMBERS works like CATENATE_WORDS, but for adjacent digit
 * sequences.</li> <li>CATENATE_ALL smushes together all the token parts without
 * distinguishing numbers and words.</li>
 * </ul>
 * One use for {@link WordDelimiterGraphFilter} is to help match words with
 * different subword delimiters. For example, if the source text contained
 * "wi-fi" one may want "wifi" "WiFi" "wi-fi" "wi+fi" queries to all match. One
 * way of doing so is to specify CATENATE options in the analyzer used for
 * indexing, and not in the analyzer used for querying. Given that the current
 * {@link StandardTokenizer} immediately removes many intra-word delimiters, it
 * is recommended that this filter be used after a tokenizer that does not do
 * this (such as {@link WhitespaceTokenizer}).
 */

class WordDelimiterGraphFilter final : public TokenFilter
{
  GET_CLASS_NAME(WordDelimiterGraphFilter)

  /**
   * Causes parts of words to be generated:
   * <p>
   * "PowerShot" =&gt; "Power" "Shot"
   */
public:
  static constexpr int GENERATE_WORD_PARTS = 1;

  /**
   * Causes number subwords to be generated:
   * <p>
   * "500-42" =&gt; "500" "42"
   */
  static constexpr int GENERATE_NUMBER_PARTS = 2;

  /**
   * Causes maximum runs of word parts to be catenated:
   * <p>
   * "wi-fi" =&gt; "wifi"
   */
  static constexpr int CATENATE_WORDS = 4;

  /**
   * Causes maximum runs of number parts to be catenated:
   * <p>
   * "500-42" =&gt; "50042"
   */
  static constexpr int CATENATE_NUMBERS = 8;

  /**
   * Causes all subword parts to be catenated:
   * <p>
   * "wi-fi-4000" =&gt; "wifi4000"
   */
  static constexpr int CATENATE_ALL = 16;

  /**
   * Causes original words are preserved and added to the subword deque (Defaults
   * to false) <p> "500-42" =&gt; "500" "42" "500-42"
   */
  static constexpr int PRESERVE_ORIGINAL = 32;

  /**
   * Causes lowercase -&gt; uppercase transition to start a new subword.
   */
  static constexpr int SPLIT_ON_CASE_CHANGE = 64;

  /**
   * If not set, causes numeric changes to be ignored (subwords will only be
   * generated given SUBWORD_DELIM tokens).
   */
  static constexpr int SPLIT_ON_NUMERICS = 128;

  /**
   * Causes trailing "'s" to be removed for each subword
   * <p>
   * "O'Neil's" =&gt; "O", "Neil"
   */
  static constexpr int STEM_ENGLISH_POSSESSIVE = 256;

  /**
   * Suppresses processing terms with {@link KeywordAttribute#isKeyword()}=true.
   */
  static constexpr int IGNORE_KEYWORDS = 512;

  /**
   * If not null is the set of tokens to protect from being delimited
   *
   */
  const std::shared_ptr<CharArraySet> protWords;

private:
  const int flags;

  // packs start pos, end pos, start part, end part (= slice of the term text)
  // for each buffered part:
  std::deque<int> bufferedParts = std::deque<int>(16);
  int bufferedLen = 0;
  int bufferedPos = 0;

  // holds text for each buffered part, or null if it's a simple slice of the
  // original term
  std::deque<std::deque<wchar_t>> bufferedTermParts =
      std::deque<std::deque<wchar_t>>(4);

  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttribute =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAttribute =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAttribute =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAttribute =
      addAttribute(PositionLengthAttribute::typeid);

  // used for iterating word delimiter breaks
  const std::shared_ptr<WordDelimiterIterator> iterator;

  // used for concatenating runs of similar typed subwords (word,number)
  const std::shared_ptr<WordDelimiterConcatenation> concat =
      std::make_shared<WordDelimiterConcatenation>(shared_from_this());

  // number of subwords last output by concat.
  int lastConcatCount = 0;

  // used for catenate all
  const std::shared_ptr<WordDelimiterConcatenation> concatAll =
      std::make_shared<WordDelimiterConcatenation>(shared_from_this());

  // used for accumulating position increment gaps so that we preserve incoming
  // holes:
  int accumPosInc = 0;

  std::deque<wchar_t> savedTermBuffer = std::deque<wchar_t>(16);
  int savedTermLength = 0;
  int savedStartOffset = 0;
  int savedEndOffset = 0;
  std::shared_ptr<AttributeSource::State> savedState;
  int lastStartOffset = 0;

  // if length by start + end offsets doesn't match the term text then assume
  // this is a synonym and don't adjust the offsets.
  bool hasIllegalOffsets = false;

  int wordPos = 0;

  /**
   * Creates a new WordDelimiterGraphFilter
   *
   * @param in TokenStream to be filtered
   * @param charTypeTable table containing character types
   * @param configurationFlags Flags configuring the filter
   * @param protWords If not null is the set of tokens to protect from being
   * delimited
   */
public:
  WordDelimiterGraphFilter(std::shared_ptr<TokenStream> in_,
                           std::deque<char> &charTypeTable,
                           int configurationFlags,
                           std::shared_ptr<CharArraySet> protWords);

  /**
   * Creates a new WordDelimiterGraphFilter using {@link
   * WordDelimiterIterator#DEFAULT_WORD_DELIM_TABLE} as its charTypeTable
   *
   * @param in TokenStream to be filtered
   * @param configurationFlags Flags configuring the filter
   * @param protWords If not null is the set of tokens to protect from being
   * delimited
   */
  WordDelimiterGraphFilter(std::shared_ptr<TokenStream> in_,
                           int configurationFlags,
                           std::shared_ptr<CharArraySet> protWords);

  /** Iterates all words parts and concatenations, buffering up the term parts
   * we should return. */
private:
  void bufferWordParts() ;

public:
  bool incrementToken()  override;

  void reset()  override;

  // ================================================= Helper Methods
  // ================================================

private:
  class PositionSorter : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(PositionSorter)
  private:
    std::shared_ptr<WordDelimiterGraphFilter> outerInstance;

  public:
    PositionSorter(std::shared_ptr<WordDelimiterGraphFilter> outerInstance);

  protected:
    int compare(int i, int j) override;

    void swap(int i, int j) override;

  protected:
    std::shared_ptr<PositionSorter> shared_from_this()
    {
      return std::static_pointer_cast<PositionSorter>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

public:
  const std::shared_ptr<PositionSorter> sorter =
      std::make_shared<PositionSorter>(shared_from_this());

  /**
   * startPos, endPos -> graph start/end position
   * startPart, endPart -> slice of the original term for this part
   */

  void buffer(int startPos, int endPos, int startPart, int endPart);

  /**
   * a null termPart means it's a simple slice of the original term
   */
  void buffer(std::deque<wchar_t> &termPart, int startPos, int endPos,
              int startPart, int endPart);

  /**
   * Saves the existing attribute states
   */
private:
  void saveState();

  /**
   * Flushes the given WordDelimiterConcatenation by either writing its concat
   * and then clearing, or just clearing.
   *
   * @param concat WordDelimiterConcatenation that will be flushed
   */
  void flushConcatenation(std::shared_ptr<WordDelimiterConcatenation> concat);

  /**
   * Determines whether to concatenate a word or number if the current word is
   * the given type
   *
   * @param wordType Type of the current word used to determine if it should be
   * concatenated
   * @return {@code true} if concatenation should occur, {@code false} otherwise
   */
  bool shouldConcatenate(int wordType);

  /**
   * Determines whether a word/number part should be generated for a word of the
   * given type
   *
   * @param wordType Type of the word used to determine if a word/number part
   * should be generated
   * @return {@code true} if a word/number part should be generated, {@code
   * false} otherwise
   */
  bool shouldGenerateParts(int wordType);

  /**
   * Concatenates the saved buffer to the given WordDelimiterConcatenation
   *
   * @param concatenation WordDelimiterConcatenation to concatenate the buffer
   * to
   */
  void concatenate(std::shared_ptr<WordDelimiterConcatenation> concatenation);

  /**
   * Determines whether the given flag is set
   *
   * @param flag Flag to see if set
   * @return {@code true} if flag is set
   */
  bool has(int flag);

  // ================================================= Inner Classes
  // =================================================

  /**
   * A WDF concatenated 'run'
   */
public:
  class WordDelimiterConcatenation final
      : public std::enable_shared_from_this<WordDelimiterConcatenation>
  {
    GET_CLASS_NAME(WordDelimiterConcatenation)
  private:
    std::shared_ptr<WordDelimiterGraphFilter> outerInstance;

  public:
    WordDelimiterConcatenation(
        std::shared_ptr<WordDelimiterGraphFilter> outerInstance);

    const std::shared_ptr<StringBuilder> buffer =
        std::make_shared<StringBuilder>();
    int startPart = 0;
    int endPart = 0;
    int startPos = 0;
    int type = 0;
    int subwordCount = 0;

    /**
     * Appends the given text of the given length, to the concetenation at the
     * given offset
     *
     * @param text Text to append
     * @param offset Offset in the concetenation to add the text
     * @param length Length of the text to append
     */
    void append(std::deque<wchar_t> &text, int offset, int length);

    /**
     * Writes the concatenation to part buffer
     */
    void write();

    /**
     * Determines if the concatenation is empty
     *
     * @return {@code true} if the concatenation is empty, {@code false}
     * otherwise
     */
    bool isEmpty();

    bool isNotEmpty();

    /**
     * Clears the concatenation and resets its state
     */
    void clear();
  };

  /** Returns string representation of configuration flags */
public:
  static std::wstring flagsToString(int flags);

  virtual std::wstring toString();

  // questions:
  // negative numbers?  -42 indexed as just 42?
  // dollar sign?  $42
  // percent sign?  33%
  // downsides:  if source text is "powershot" then a query of "PowerShot" won't
  // match!

protected:
  std::shared_ptr<WordDelimiterGraphFilter> shared_from_this()
  {
    return std::static_pointer_cast<WordDelimiterGraphFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
