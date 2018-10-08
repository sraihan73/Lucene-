#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::tokenattributes
{
class KeywordAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis::ja
{
class NumberBuffer;
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
namespace org::apache::lucene::analysis::ja
{

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

/**
 * A {@link TokenFilter} that normalizes Japanese numbers (kansūji) to regular
 * Arabic decimal numbers in half-width characters. <p> Japanese numbers are
 * often written using a combination of kanji and Arabic numbers with various
 * kinds punctuation. For example, ３．２千 means 3200. This filter does this
 * kind of normalization and allows a search for 3200 to match ３．２千 in text,
 * but can also be used to make range facets based on the normalized numbers and
 * so on. <p> Notice that this analyzer uses a token composition scheme and
 * relies on punctuation tokens being found in the token stream. Please make
 * sure your {@link JapaneseTokenizer} has {@code discardPunctuation} set to
 * false. In case punctuation characters, such as ． (U+FF0E FULLWIDTH FULL
 * STOP), is removed from the token stream, this filter would find input tokens
 * tokens ３ and ２千 and give outputs 3 and 2000 instead of 3200, which is
 * likely not the intended result. If you want to remove punctuation characters
 * from your index that are not part of normalized numbers, add a
 * {@link org.apache.lucene.analysis.StopFilter} with the punctuation you wish
 * to remove after {@link JapaneseNumberFilter} in your analyzer chain. <p>
 * Below are some examples of normalizations this filter supports. The input is
 * untokenized text and the result is the single term attribute emitted for the
 * input. <ul> <li>〇〇七 becomes 7</li> <li>一〇〇〇 becomes 1000</li>
 * <li>三千2百２十三 becomes 3223</li>
 * <li>兆六百万五千一 becomes 1000006005001</li>
 * <li>３．２千 becomes 3200</li>
 * <li>１．２万３４５．６７ becomes 12345.67</li>
 * <li>4,647.100 becomes 4647.1</li>
 * <li>15,7 becomes 157 (be aware of this weakness)</li>
 * </ul>
 * <p>
 * Tokens preceded by a token with {@link PositionIncrementAttribute} of zero
 * are left left untouched and emitted as-is. <p> This filter does not use any
 * part-of-speech information for its normalization and the motivation for this
 * is to also support n-grammed token streams in the future. <p> This filter may
 * in some cases normalize tokens that are not numbers in their context. For
 * example, is 田中京一 is a name and means Tanaka Kyōichi, but 京一 (Kyōichi)
 * out of context can strictly speaking also represent the number
 * 10000000000000001. This filter respects the {@link KeywordAttribute}, which
 * can be used to prevent specific normalizations from happening. <p> Also
 * notice that token attributes such as
 * {@link org.apache.lucene.analysis.ja.tokenattributes.PartOfSpeechAttribute},
 * {@link org.apache.lucene.analysis.ja.tokenattributes.ReadingAttribute},
 * {@link org.apache.lucene.analysis.ja.tokenattributes.InflectionAttribute} and
 * {@link org.apache.lucene.analysis.ja.tokenattributes.BaseFormAttribute} are
 * left unchanged and will inherit the values of the last token used to compose
 * the normalized number and can be wrong. Hence, for １０万 (10000), we will
 * have
 * {@link org.apache.lucene.analysis.ja.tokenattributes.ReadingAttribute}
 * set to マン. This is a known issue and is subject to a future improvement.
 * <p>
 * Japanese formal numbers (daiji), accounting numbers and decimal fractions are
 * currently not supported.
 */
class JapaneseNumberFilter : public TokenFilter
{
  GET_CLASS_NAME(JapaneseNumberFilter)

private:
  const std::shared_ptr<CharTermAttribute> termAttr =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAttr =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAttr =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAttr =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLengthAttr =
      addAttribute(PositionLengthAttribute::typeid);

  static wchar_t NO_NUMERAL;

  static std::deque<wchar_t> numerals;

  static std::deque<wchar_t> exponents;

  std::shared_ptr<State> state;

  std::shared_ptr<StringBuilder> numeral;

  int fallThroughTokens = 0;

  bool exhausted = false;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static JapaneseNumberFilter::StaticConstructor staticConstructor;

public:
  JapaneseNumberFilter(std::shared_ptr<TokenStream> input);

  bool incrementToken()  override final;

  void reset()  override;

  /**
   * Normalizes a Japanese number
   *
   * @param number number or normalize
   * @return normalized number, or number to normalize on error (no op)
   */
  virtual std::wstring normalizeNumber(const std::wstring &number);

  /**
   * Parses a Japanese number
   *
   * @param buffer buffer to parse
   * @return parsed number, or null on error or end of input
   */
private:
  BigDecimal parseNumber(std::shared_ptr<NumberBuffer> buffer);

  /**
   * Parses a pair of large numbers, i.e. large kanji factor is 10,000（万）or
   * larger
   *
   * @param buffer buffer to parse
   * @return parsed pair, or null on error or end of input
   */
  BigDecimal parseLargePair(std::shared_ptr<NumberBuffer> buffer);

  /**
   * Parses a "medium sized" number, typically less than 10,000（万）, but might
   * be larger due to a larger factor from {link parseBasicNumber}.
   *
   * @param buffer buffer to parse
   * @return parsed number, or null on error or end of input
   */
  BigDecimal parseMediumNumber(std::shared_ptr<NumberBuffer> buffer);

  /**
   * Parses a pair of "medium sized" numbers, i.e. large kanji factor is at most
   * 1,000（千）
   *
   * @param buffer buffer to parse
   * @return parsed pair, or null on error or end of input
   */
  BigDecimal parseMediumPair(std::shared_ptr<NumberBuffer> buffer);

  /**
   * Parse a basic number, which is a sequence of Arabic numbers or a sequence
   * or 0-9 kanji numerals (〇 to 九).
   *
   * @param buffer buffer to parse
   * @return parsed number, or null on error or end of input
   */
  BigDecimal parseBasicNumber(std::shared_ptr<NumberBuffer> buffer);

  /**
   * Parse large kanji numerals (ten thousands or larger)
   *
   * @param buffer buffer to parse
   * @return parsed number, or null on error or end of input
   */
public:
  virtual BigDecimal
  parseLargeKanjiNumeral(std::shared_ptr<NumberBuffer> buffer);

  /**
   * Parse medium kanji numerals (tens, hundreds or thousands)
   *
   * @param buffer buffer to parse
   * @return parsed number or null on error
   */
  virtual BigDecimal
  parseMediumKanjiNumeral(std::shared_ptr<NumberBuffer> buffer);

  /**
   * Numeral predicate
   *
   * @param input string to test
   * @return true if and only if input is a numeral
   */
  virtual bool isNumeral(const std::wstring &input);

  /**
   * Numeral predicate
   *
   * @param c character to test
   * @return true if and only if c is a numeral
   */
  virtual bool isNumeral(wchar_t c);

  /**
   * Numeral punctuation predicate
   *
   * @param input string to test
   * @return true if and only if c is a numeral punctuation string
   */
  virtual bool isNumeralPunctuation(const std::wstring &input);

  /**
   * Numeral punctuation predicate
   *
   * @param c character to test
   * @return true if and only if c is a numeral punctuation character
   */
  virtual bool isNumeralPunctuation(wchar_t c);

  /**
   * Arabic numeral predicate. Both half-width and full-width characters are
   * supported
   *
   * @param c character to test
   * @return true if and only if c is an Arabic numeral
   */
  virtual bool isArabicNumeral(wchar_t c);

  /**
   * Arabic half-width numeral predicate
   *
   * @param c character to test
   * @return true if and only if c is a half-width Arabic numeral
   */
private:
  bool isHalfWidthArabicNumeral(wchar_t c);

  /**
   * Arabic full-width numeral predicate
   *
   * @param c character to test
   * @return true if and only if c is a full-width Arabic numeral
   */
  bool isFullWidthArabicNumeral(wchar_t c);

  /**
   * Returns the numeric value for the specified character Arabic numeral.
   * Behavior is undefined if a non-Arabic numeral is provided
   *
   * @param c arabic numeral character
   * @return numeral value
   */
  int arabicNumeralValue(wchar_t c);

  /**
   * Kanji numeral predicate that tests if the provided character is one of 〇,
   * 一, 二, 三, 四, 五, 六, 七, 八, or 九. Larger number kanji gives a false
   * value.
   *
   * @param c character to test
   * @return true if and only is character is one of 〇, 一, 二, 三, 四, 五, 六,
   * 七, 八, or 九 (0 to 9)
   */
  bool isKanjiNumeral(wchar_t c);

  /**
   * Returns the value for the provided kanji numeral. Only numeric values for
   * the characters where {link isKanjiNumeral} return true are supported -
   * behavior is undefined for other characters.
   *
   * @param c kanji numeral character
   * @return numeral value
   * @see #isKanjiNumeral(char)
   */
  int kanjiNumeralValue(wchar_t c);

  /**
   * Decimal point predicate
   *
   * @param c character to test
   * @return true if and only if c is a decimal point
   */
  bool isDecimalPoint(wchar_t c);

  /**
   * Thousand separator predicate
   *
   * @param c character to test
   * @return true if and only if c is a thousand separator predicate
   */
  bool isThousandSeparator(wchar_t c);

  /**
   * Buffer that holds a Japanese number string and a position index used as a
   * parsed-to marker
   */
public:
  class NumberBuffer : public std::enable_shared_from_this<NumberBuffer>
  {
    GET_CLASS_NAME(NumberBuffer)

  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    int position_ = 0;

    std::wstring string;

  public:
    NumberBuffer(const std::wstring &string);

    virtual wchar_t charAt(int index);

    virtual int length();

    virtual void advance();

    virtual int position();
  };

protected:
  std::shared_ptr<JapaneseNumberFilter> shared_from_this()
  {
    return std::static_pointer_cast<JapaneseNumberFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ja
