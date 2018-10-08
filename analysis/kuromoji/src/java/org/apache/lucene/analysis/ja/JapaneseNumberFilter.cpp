using namespace std;

#include "JapaneseNumberFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"

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
wchar_t JapaneseNumberFilter::NO_NUMERAL = numeric_limits<wchar_t>::max();
std::deque<wchar_t> JapaneseNumberFilter::numerals;
std::deque<wchar_t> JapaneseNumberFilter::exponents;

JapaneseNumberFilter::StaticConstructor::StaticConstructor()
{
  numerals = std::deque<wchar_t>(0x10000);
  for (int i = 0; i < numerals.size(); i++) {
    numerals[i] = NO_NUMERAL;
  }
  numerals[L'〇'] = 0; // 〇 U+3007 0
  numerals[L'一'] = 1; // 一 U+4E00 1
  numerals[L'二'] = 2; // 二 U+4E8C 2
  numerals[L'三'] = 3; // 三 U+4E09 3
  numerals[L'四'] = 4; // 四 U+56DB 4
  numerals[L'五'] = 5; // 五 U+4E94 5
  numerals[L'六'] = 6; // 六 U+516D 6
  numerals[L'七'] = 7; // 七 U+4E03 7
  numerals[L'八'] = 8; // 八 U+516B 8
  numerals[L'九'] = 9; // 九 U+4E5D 9

  exponents = std::deque<wchar_t>(0x10000);
  for (int i = 0; i < exponents.size(); i++) {
    exponents[i] = 0;
  }
  exponents[L'十'] = 1;  // 十 U+5341 10
  exponents[L'百'] = 2;  // 百 U+767E 100
  exponents[L'千'] = 3;  // 千 U+5343 1,000
  exponents[L'万'] = 4;  // 万 U+4E07 10,000
  exponents[L'億'] = 8;  // 億 U+5104 100,000,000
  exponents[L'兆'] = 12; // 兆 U+5146 1,000,000,000,000
  exponents[L'京'] = 16; // 京 U+4EAC 10,000,000,000,000,000
  exponents[L'垓'] = 20; // 垓 U+5793 100,000,000,000,000,000,000
}

JapaneseNumberFilter::StaticConstructor JapaneseNumberFilter::staticConstructor;

JapaneseNumberFilter::JapaneseNumberFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool JapaneseNumberFilter::incrementToken() 
{

  // Emit previously captured token we read past earlier
  if (state != nullptr) {
    restoreState(state);
    state.reset();
    return true;
  }

  if (exhausted) {
    return false;
  }

  if (!input->incrementToken()) {
    exhausted = true;
    return false;
  }

  if (keywordAttr->isKeyword()) {
    return true;
  }

  if (fallThroughTokens > 0) {
    fallThroughTokens--;
    return true;
  }

  if (posIncrAttr->getPositionIncrement() == 0) {
    fallThroughTokens = posLengthAttr->getPositionLength() - 1;
    return true;
  }

  bool moreTokens = true;
  bool composedNumberToken = false;
  int startOffset = 0;
  int endOffset = 0;
  shared_ptr<State> preCompositionState = captureState();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring term = termAttr->toString();
  bool numeralTerm = isNumeral(term);

  while (moreTokens && numeralTerm) {

    if (!composedNumberToken) {
      startOffset = offsetAttr->startOffset();
      composedNumberToken = true;
    }

    endOffset = offsetAttr->endOffset();
    moreTokens = input->incrementToken();
    if (moreTokens == false) {
      exhausted = true;
    }

    if (posIncrAttr->getPositionIncrement() == 0) {
      // This token is a stacked/synonym token, capture number of tokens "under"
      // this token, except the first token, which we will emit below after
      // restoring state
      fallThroughTokens = posLengthAttr->getPositionLength() - 1;
      state = captureState();
      restoreState(preCompositionState);
      return moreTokens;
    }

    numeral->append(term);

    if (moreTokens) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      term = termAttr->toString();
      numeralTerm = isNumeral(term) || isNumeralPunctuation(term);
    }
  }

  if (composedNumberToken) {
    if (moreTokens) {
      // We have read past all numerals and there are still tokens left, so
      // capture the state of this token and emit it on our next
      // incrementToken()
      state = captureState();
    }

    wstring normalizedNumber = normalizeNumber(numeral->toString());

    termAttr->setEmpty();
    termAttr->append(normalizedNumber);
    offsetAttr->setOffset(startOffset, endOffset);

    numeral = make_shared<StringBuilder>();
    return true;
  }
  return moreTokens;
}

void JapaneseNumberFilter::reset() 
{
  TokenFilter::reset();
  fallThroughTokens = 0;
  numeral = make_shared<StringBuilder>();
  state.reset();
  exhausted = false;
}

wstring JapaneseNumberFilter::normalizeNumber(const wstring &number)
{
  try {
    BigDecimal normalizedNumber =
        parseNumber(make_shared<NumberBuffer>(number));
    if (normalizedNumber == nullptr) {
      return number;
    }
    return normalizedNumber.stripTrailingZeros().toPlainString();
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (NumberFormatException | ArithmeticException e) {
    // Return the source number in case of error, i.e. malformed input
    return number;
  }
}

BigDecimal JapaneseNumberFilter::parseNumber(shared_ptr<NumberBuffer> buffer)
{
  BigDecimal sum = BigDecimal::ZERO;
  BigDecimal result = parseLargePair(buffer);

  if (result == nullptr) {
    return nullptr;
  }

  while (result != nullptr) {
    sum = sum.add(result);
    result = parseLargePair(buffer);
  }

  return sum;
}

BigDecimal JapaneseNumberFilter::parseLargePair(shared_ptr<NumberBuffer> buffer)
{
  BigDecimal first = parseMediumNumber(buffer);
  BigDecimal second = parseLargeKanjiNumeral(buffer);

  if (first == nullptr && second == nullptr) {
    return nullptr;
  }

  if (second == nullptr) {
    // If there's no second factor, we return the first one
    // This can happen if we our number is smaller than 10,000 (万)
    return first;
  }

  if (first == nullptr) {
    // If there's no first factor, just return the second one,
    // which is the same as multiplying by 1, i.e. with 万
    return second;
  }

  return first.multiply(second);
}

BigDecimal
JapaneseNumberFilter::parseMediumNumber(shared_ptr<NumberBuffer> buffer)
{
  BigDecimal sum = BigDecimal::ZERO;
  BigDecimal result = parseMediumPair(buffer);

  if (result == nullptr) {
    return nullptr;
  }

  while (result != nullptr) {
    sum = sum.add(result);
    result = parseMediumPair(buffer);
  }

  return sum;
}

BigDecimal
JapaneseNumberFilter::parseMediumPair(shared_ptr<NumberBuffer> buffer)
{

  BigDecimal first = parseBasicNumber(buffer);
  BigDecimal second = parseMediumKanjiNumeral(buffer);

  if (first == nullptr && second == nullptr) {
    return nullptr;
  }

  if (second == nullptr) {
    // If there's no second factor, we return the first one
    // This can happen if we just have a plain number such as 五
    return first;
  }

  if (first == nullptr) {
    // If there's no first factor, just return the second one,
    // which is the same as multiplying by 1, i.e. with 千
    return second;
  }

  // Return factors multiplied
  return first.multiply(second);
}

BigDecimal
JapaneseNumberFilter::parseBasicNumber(shared_ptr<NumberBuffer> buffer)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  int i = buffer->position();

  while (i < buffer->length()) {
    wchar_t c = buffer->charAt(i);

    if (isArabicNumeral(c)) {
      // Arabic numerals; 0 to 9 or ０ to ９ (full-width)
      builder->append(arabicNumeralValue(c));
    } else if (isKanjiNumeral(c)) {
      // Kanji numerals; 〇, 一, 二, 三, 四, 五, 六, 七, 八, or 九
      builder->append(kanjiNumeralValue(c));
    } else if (isDecimalPoint(c)) {
      builder->append(L".");
    } else if (isThousandSeparator(c)) {
      // Just skip and move to the next character
    } else {
      // We don't have an Arabic nor kanji numeral, nor separation or
      // punctuation, so we'll stop.
      break;
    }

    i++;
    buffer->advance();
  }

  if (builder->length() == 0) {
    // We didn't build anything, so we don't have a number
    return nullptr;
  }

  return BigDecimal(builder->toString());
}

BigDecimal
JapaneseNumberFilter::parseLargeKanjiNumeral(shared_ptr<NumberBuffer> buffer)
{
  int i = buffer->position();

  if (i >= buffer->length()) {
    return nullptr;
  }

  wchar_t c = buffer->charAt(i);
  int power = exponents[c];

  if (power > 3) {
    buffer->advance();
    return BigDecimal::TEN::pow(power);
  }

  return nullptr;
}

BigDecimal
JapaneseNumberFilter::parseMediumKanjiNumeral(shared_ptr<NumberBuffer> buffer)
{
  int i = buffer->position();

  if (i >= buffer->length()) {
    return nullptr;
  }

  wchar_t c = buffer->charAt(i);
  int power = exponents[c];

  if (1 <= power && power <= 3) {
    buffer->advance();
    return BigDecimal::TEN::pow(power);
  }

  return nullptr;
}

bool JapaneseNumberFilter::isNumeral(const wstring &input)
{
  for (int i = 0; i < input.length(); i++) {
    if (!isNumeral(input[i])) {
      return false;
    }
  }
  return true;
}

bool JapaneseNumberFilter::isNumeral(wchar_t c)
{
  return isArabicNumeral(c) || isKanjiNumeral(c) || exponents[c] > 0;
}

bool JapaneseNumberFilter::isNumeralPunctuation(const wstring &input)
{
  for (int i = 0; i < input.length(); i++) {
    if (!isNumeralPunctuation(input[i])) {
      return false;
    }
  }
  return true;
}

bool JapaneseNumberFilter::isNumeralPunctuation(wchar_t c)
{
  return isDecimalPoint(c) || isThousandSeparator(c);
}

bool JapaneseNumberFilter::isArabicNumeral(wchar_t c)
{
  return isHalfWidthArabicNumeral(c) || isFullWidthArabicNumeral(c);
}

bool JapaneseNumberFilter::isHalfWidthArabicNumeral(wchar_t c)
{
  // 0 U+0030 - 9 U+0039
  return L'0' <= c && c <= L'9';
}

bool JapaneseNumberFilter::isFullWidthArabicNumeral(wchar_t c)
{
  // ０ U+FF10 - ９ U+FF19
  return L'０' <= c && c <= L'９';
}

int JapaneseNumberFilter::arabicNumeralValue(wchar_t c)
{
  int offset;
  if (isHalfWidthArabicNumeral(c)) {
    offset = L'0';
  } else {
    offset = L'０';
  }
  return c - offset;
}

bool JapaneseNumberFilter::isKanjiNumeral(wchar_t c)
{
  return numerals[c] != NO_NUMERAL;
}

int JapaneseNumberFilter::kanjiNumeralValue(wchar_t c) { return numerals[c]; }

bool JapaneseNumberFilter::isDecimalPoint(wchar_t c)
{
  return c == L'.' || c == L'．'; // U+FF0E FULLWIDTH FULL STOP
}

bool JapaneseNumberFilter::isThousandSeparator(wchar_t c)
{
  return c == L',' || c == L'，'; // U+FF0C FULLWIDTH COMMA
}

JapaneseNumberFilter::NumberBuffer::NumberBuffer(const wstring &string)
{
  this->string = string;
  this->position_ = 0;
}

wchar_t JapaneseNumberFilter::NumberBuffer::charAt(int index)
{
  return string[index];
}

int JapaneseNumberFilter::NumberBuffer::length() { return string.length(); }

void JapaneseNumberFilter::NumberBuffer::advance() { position_++; }

int JapaneseNumberFilter::NumberBuffer::position() { return position_; }
} // namespace org::apache::lucene::analysis::ja