#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/InPlaceMergeSorter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis::miscellaneous
{
class WordDelimiterIterator;
}
namespace org::apache::lucene::analysis::miscellaneous
{
class WordDelimiterConcatenation;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::util
{
class AttributeSource;
}
namespace org::apache::lucene::util
{
class State;
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
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;

/**
 * Splits words into subwords and performs optional transformations on subword
 * groups. Words are split into subwords with the following rules:
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
 *
 * One use for {@link WordDelimiterFilter} is to help match words with different
 * subword delimiters. For example, if the source text contained "wi-fi" one may
 * want "wifi" "WiFi" "wi-fi" "wi+fi" queries to all match. One way of doing so
 * is to specify CATENATE options in the analyzer used for indexing, and
 * not in the analyzer used for querying. Given that
 * the current {@link StandardTokenizer} immediately removes many intra-word
 * delimiters, it is recommended that this filter be used after a tokenizer that
 * does not do this (such as {@link WhitespaceTokenizer}).
 *
 * @deprecated Use {@link WordDelimiterGraphFilter} instead: it produces a
 * correct token graph so that e.g. {@link PhraseQuery} works correctly when
 * it's used in the search time analyzer.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public final class WordDelimiterFilter extends
// org.apache.lucene.analysis.TokenFilter
class WordDelimiterFilter final : public TokenFilter
{

public:
  static constexpr int LOWER = 0x01;
  static constexpr int UPPER = 0x02;
  static constexpr int DIGIT = 0x04;
  static constexpr int SUBWORD_DELIM = 0x08;

  // combinations: for testing, not for setting bits
  static constexpr int ALPHA = 0x03;
  static constexpr int ALPHANUM = 0x07;

  /**
   * Causes parts of words to be generated:
   * <p>
   * "PowerShot" =&gt; "Power" "Shot"
   */
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
   * Causes maximum runs of word parts to be catenated:
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
   * If not set, causes case changes to be ignored (subwords will only be
   * generated given SUBWORD_DELIM tokens)
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

  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttribute =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAttribute =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAttribute =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAttribute =
      addAttribute(TypeAttribute::typeid);

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

  // used for accumulating position increment gaps
  int accumPosInc = 0;

  std::deque<wchar_t> savedBuffer = std::deque<wchar_t>(1024);
  int savedStartOffset = 0;
  int savedEndOffset = 0;
  std::wstring savedType;
  bool hasSavedState = false;
  // if length by start + end offsets doesn't match the term text then assume
  // this is a synonym and don't adjust the offsets.
  bool hasIllegalOffsets = false;

  // for a run of the same subword type within a word, have we output anything?
  bool hasOutputToken = false;
  // when preserve original is on, have we output any token following it?
  // this token must have posInc=0!
  bool hasOutputFollowingOriginal = false;

  /**
   * Creates a new WordDelimiterFilter
   *
   * @param in TokenStream to be filtered
   * @param charTypeTable table containing character types
   * @param configurationFlags Flags configuring the filter
   * @param protWords If not null is the set of tokens to protect from being
   * delimited
   */
public:
  WordDelimiterFilter(std::shared_ptr<TokenStream> in_,
                      std::deque<char> &charTypeTable, int configurationFlags,
                      std::shared_ptr<CharArraySet> protWords);

  /**
   * Creates a new WordDelimiterFilter using {@link
   * WordDelimiterIterator#DEFAULT_WORD_DELIM_TABLE} as its charTypeTable
   *
   * @param in TokenStream to be filtered
   * @param configurationFlags Flags configuring the filter
   * @param protWords If not null is the set of tokens to protect from being
   * delimited
   */
  WordDelimiterFilter(std::shared_ptr<TokenStream> in_, int configurationFlags,
                      std::shared_ptr<CharArraySet> protWords);

  bool incrementToken()  override;

  void reset()  override;

  // ================================================= Helper Methods
  // ================================================

private:
  std::deque<std::shared_ptr<AttributeSource::State>> buffered =
      std::deque<std::shared_ptr<AttributeSource::State>>(8);
  std::deque<int> startOff = std::deque<int>(8);
  std::deque<int> posInc = std::deque<int>(8);
  int bufferedLen = 0;
  int bufferedPos = 0;
  bool first = false;

private:
  class OffsetSorter : public InPlaceMergeSorter
  {
    GET_CLASS_NAME(OffsetSorter)
  private:
    std::shared_ptr<WordDelimiterFilter> outerInstance;

  public:
    OffsetSorter(std::shared_ptr<WordDelimiterFilter> outerInstance);

  protected:
    int compare(int i, int j) override;

    void swap(int i, int j) override;

  protected:
    std::shared_ptr<OffsetSorter> shared_from_this()
    {
      return std::static_pointer_cast<OffsetSorter>(
          org.apache.lucene.util.InPlaceMergeSorter::shared_from_this());
    }
  };

public:
  const std::shared_ptr<OffsetSorter> sorter =
      std::make_shared<OffsetSorter>(shared_from_this());

private:
  void buffer();

  /**
   * Saves the existing attribute states
   */
  void saveState();

  /**
   * Flushes the given WordDelimiterConcatenation by either writing its concat
   * and then clearing, or just clearing.
   *
   * @param concatenation WordDelimiterConcatenation that will be flushed
   * @return {@code true} if the concatenation was written before it was
   * cleared, {@code false} otherwise
   */
  bool
  flushConcatenation(std::shared_ptr<WordDelimiterConcatenation> concatenation);

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
   * Generates a word/number part, updating the appropriate attributes
   *
   * @param isSingleWord {@code true} if the generation is occurring from a
   * single word, {@code false} otherwise
   */
  void generatePart(bool isSingleWord);

  /**
   * Get the position increment gap for a subword or concatenation
   *
   * @param inject true if this token wants to be injected
   * @return position increment gap
   */
  int position(bool inject);

  /**
   * Checks if the given word type includes {@link #ALPHA}
   *
   * @param type Word type to check
   * @return {@code true} if the type contains ALPHA, {@code false} otherwise
   */
public:
  static bool isAlpha(int type);

  /**
   * Checks if the given word type includes {@link #DIGIT}
   *
   * @param type Word type to check
   * @return {@code true} if the type contains DIGIT, {@code false} otherwise
   */
  static bool isDigit(int type);

  /**
   * Checks if the given word type includes {@link #SUBWORD_DELIM}
   *
   * @param type Word type to check
   * @return {@code true} if the type contains SUBWORD_DELIM, {@code false}
   * otherwise
   */
  static bool isSubwordDelim(int type);

  /**
   * Checks if the given word type includes {@link #UPPER}
   *
   * @param type Word type to check
   * @return {@code true} if the type contains UPPER, {@code false} otherwise
   */
  static bool isUpper(int type);

  /**
   * Determines whether the given flag is set
   *
   * @param flag Flag to see if set
   * @return {@code true} if flag is set
   */
private:
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
    std::shared_ptr<WordDelimiterFilter> outerInstance;

  public:
    WordDelimiterConcatenation(
        std::shared_ptr<WordDelimiterFilter> outerInstance);

    const std::shared_ptr<StringBuilder> buffer =
        std::make_shared<StringBuilder>();
    int startOffset = 0;
    int endOffset = 0;
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
     * Writes the concatenation to the attributes
     */
    void write();

    /**
     * Determines if the concatenation is empty
     *
     * @return {@code true} if the concatenation is empty, {@code false}
     * otherwise
     */
    bool isEmpty();

    /**
     * Clears the concatenation and resets its state
     */
    void clear();

    /**
     * Convenience method for the common scenario of having to write the
     * concetenation and then clearing its state
     */
    void writeAndClear();
  };
  // questions:
  // negative numbers?  -42 indexed as just 42?
  // dollar sign?  $42
  // percent sign?  33%
  // downsides:  if source text is "powershot" then a query of "PowerShot" won't
  // match!

protected:
  std::shared_ptr<WordDelimiterFilter> shared_from_this()
  {
    return std::static_pointer_cast<WordDelimiterFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
