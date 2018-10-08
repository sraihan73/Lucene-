using namespace std;

#include "ASCIIFoldingFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

ASCIIFoldingFilter::ASCIIFoldingFilter(shared_ptr<TokenStream> input)
    : ASCIIFoldingFilter(input, false)
{
}

ASCIIFoldingFilter::ASCIIFoldingFilter(shared_ptr<TokenStream> input,
                                       bool preserveOriginal)
    : org::apache::lucene::analysis::TokenFilter(input),
      preserveOriginal(preserveOriginal)
{
}

bool ASCIIFoldingFilter::isPreserveOriginal() { return preserveOriginal; }

bool ASCIIFoldingFilter::incrementToken() 
{
  if (state != nullptr) {
    assert((preserveOriginal,
            L"state should only be captured if preserveOriginal is true"));
    restoreState(state);
    posIncAttr->setPositionIncrement(0);
    state.reset();
    return true;
  }
  if (input->incrementToken()) {
    const std::deque<wchar_t> buffer = termAtt->buffer();
    constexpr int length = termAtt->length();

    // If no characters actually require rewriting then we
    // just return token as-is:
    for (int i = 0; i < length; ++i) {
      constexpr wchar_t c = buffer[i];
      if (c >= L'\u0080') {
        foldToASCII(buffer, length);
        termAtt->copyBuffer(output, 0, outputPos);
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}

void ASCIIFoldingFilter::reset() 
{
  TokenFilter::reset();
  state.reset();
}

void ASCIIFoldingFilter::foldToASCII(std::deque<wchar_t> &input, int length)
{
  // Worst-case length required:
  constexpr int maxSizeNeeded = 4 * length;
  if (output.size() < maxSizeNeeded) {
    output = std::deque<wchar_t>(
        ArrayUtil::oversize(maxSizeNeeded, Character::BYTES));
  }

  outputPos = foldToASCII(input, 0, output, 0, length);
  if (preserveOriginal && needToPreserve(input, length)) {
    state = captureState();
  }
}

bool ASCIIFoldingFilter::needToPreserve(std::deque<wchar_t> &input,
                                        int inputLength)
{
  if (inputLength != outputPos) {
    return true;
  }
  for (int i = 0; i < inputLength; i++) {
    if (input[i] != output[i]) {
      return true;
    }
  }
  return false;
}

int ASCIIFoldingFilter::foldToASCII(std::deque<wchar_t> &input, int inputPos,
                                    std::deque<wchar_t> &output, int outputPos,
                                    int length)
{
  constexpr int end = inputPos + length;
  for (int pos = inputPos; pos < end; ++pos) {
    constexpr wchar_t c = input[pos];

    // Quick test: if it's not in range then just keep current character
    if (c < L'\u0080') {
      output[outputPos++] = c;
    } else {
      switch (c) {
      case L'\u00C0': // À  [LATIN CAPITAL LETTER A WITH GRAVE]
      case L'\u00C1': // Á  [LATIN CAPITAL LETTER A WITH ACUTE]
      case L'\u00C2': // Â  [LATIN CAPITAL LETTER A WITH CIRCUMFLEX]
      case L'\u00C3': // Ã  [LATIN CAPITAL LETTER A WITH TILDE]
      case L'\u00C4': // Ä  [LATIN CAPITAL LETTER A WITH DIAERESIS]
      case L'\u00C5': // Å  [LATIN CAPITAL LETTER A WITH RING ABOVE]
      case L'\u0100': // Ā  [LATIN CAPITAL LETTER A WITH MACRON]
      case L'\u0102': // Ă  [LATIN CAPITAL LETTER A WITH BREVE]
      case L'\u0104': // Ą  [LATIN CAPITAL LETTER A WITH OGONEK]
      case L'\u018F': // Ə  http://en.wikipedia.org/wiki/Schwa  [LATIN CAPITAL
                      // LETTER SCHWA]
      case L'\u01CD': // Ǎ  [LATIN CAPITAL LETTER A WITH CARON]
      case L'\u01DE': // Ǟ  [LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON]
      case L'\u01E0': // Ǡ  [LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON]
      case L'\u01FA': // Ǻ  [LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE]
      case L'\u0200': // Ȁ  [LATIN CAPITAL LETTER A WITH DOUBLE GRAVE]
      case L'\u0202': // Ȃ  [LATIN CAPITAL LETTER A WITH INVERTED BREVE]
      case L'\u0226': // Ȧ  [LATIN CAPITAL LETTER A WITH DOT ABOVE]
      case L'\u023A': // Ⱥ  [LATIN CAPITAL LETTER A WITH STROKE]
      case L'\u1D00': // ᴀ  [LATIN LETTER SMALL CAPITAL A]
      case L'\u1E00': // Ḁ  [LATIN CAPITAL LETTER A WITH RING BELOW]
      case L'\u1EA0': // Ạ  [LATIN CAPITAL LETTER A WITH DOT BELOW]
      case L'\u1EA2': // Ả  [LATIN CAPITAL LETTER A WITH HOOK ABOVE]
      case L'\u1EA4': // Ấ  [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE]
      case L'\u1EA6': // Ầ  [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE]
      case L'\u1EA8': // Ẩ  [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK
                      // ABOVE]
      case L'\u1EAA': // Ẫ  [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE]
      case L'\u1EAC': // Ậ  [LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT
                      // BELOW]
      case L'\u1EAE': // Ắ  [LATIN CAPITAL LETTER A WITH BREVE AND ACUTE]
      case L'\u1EB0': // Ằ  [LATIN CAPITAL LETTER A WITH BREVE AND GRAVE]
      case L'\u1EB2': // Ẳ  [LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE]
      case L'\u1EB4': // Ẵ  [LATIN CAPITAL LETTER A WITH BREVE AND TILDE]
      case L'\u1EB6': // Ặ  [LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW]
      case L'\u24B6': // Ⓐ  [CIRCLED LATIN CAPITAL LETTER A]
      case L'\uFF21': // Ａ  [FULLWIDTH LATIN CAPITAL LETTER A]
        output[outputPos++] = L'A';
        break;
      case L'\u00E0': // à  [LATIN SMALL LETTER A WITH GRAVE]
      case L'\u00E1': // á  [LATIN SMALL LETTER A WITH ACUTE]
      case L'\u00E2': // â  [LATIN SMALL LETTER A WITH CIRCUMFLEX]
      case L'\u00E3': // ã  [LATIN SMALL LETTER A WITH TILDE]
      case L'\u00E4': // ä  [LATIN SMALL LETTER A WITH DIAERESIS]
      case L'\u00E5': // å  [LATIN SMALL LETTER A WITH RING ABOVE]
      case L'\u0101': // ā  [LATIN SMALL LETTER A WITH MACRON]
      case L'\u0103': // ă  [LATIN SMALL LETTER A WITH BREVE]
      case L'\u0105': // ą  [LATIN SMALL LETTER A WITH OGONEK]
      case L'\u01CE': // ǎ  [LATIN SMALL LETTER A WITH CARON]
      case L'\u01DF': // ǟ  [LATIN SMALL LETTER A WITH DIAERESIS AND MACRON]
      case L'\u01E1': // ǡ  [LATIN SMALL LETTER A WITH DOT ABOVE AND MACRON]
      case L'\u01FB': // ǻ  [LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE]
      case L'\u0201': // ȁ  [LATIN SMALL LETTER A WITH DOUBLE GRAVE]
      case L'\u0203': // ȃ  [LATIN SMALL LETTER A WITH INVERTED BREVE]
      case L'\u0227': // ȧ  [LATIN SMALL LETTER A WITH DOT ABOVE]
      case L'\u0250': // ɐ  [LATIN SMALL LETTER TURNED A]
      case L'\u0259': // ə  [LATIN SMALL LETTER SCHWA]
      case L'\u025A': // ɚ  [LATIN SMALL LETTER SCHWA WITH HOOK]
      case L'\u1D8F': // ᶏ  [LATIN SMALL LETTER A WITH RETROFLEX HOOK]
      case L'\u1D95': // ᶕ  [LATIN SMALL LETTER SCHWA WITH RETROFLEX HOOK]
      case L'\u1E01': // ạ  [LATIN SMALL LETTER A WITH RING BELOW]
      case L'\u1E9A': // ả  [LATIN SMALL LETTER A WITH RIGHT HALF RING]
      case L'\u1EA1': // ạ  [LATIN SMALL LETTER A WITH DOT BELOW]
      case L'\u1EA3': // ả  [LATIN SMALL LETTER A WITH HOOK ABOVE]
      case L'\u1EA5': // ấ  [LATIN SMALL LETTER A WITH CIRCUMFLEX AND ACUTE]
      case L'\u1EA7': // ầ  [LATIN SMALL LETTER A WITH CIRCUMFLEX AND GRAVE]
      case L'\u1EA9': // ẩ  [LATIN SMALL LETTER A WITH CIRCUMFLEX AND HOOK
                      // ABOVE]
      case L'\u1EAB': // ẫ  [LATIN SMALL LETTER A WITH CIRCUMFLEX AND TILDE]
      case L'\u1EAD': // ậ  [LATIN SMALL LETTER A WITH CIRCUMFLEX AND DOT BELOW]
      case L'\u1EAF': // ắ  [LATIN SMALL LETTER A WITH BREVE AND ACUTE]
      case L'\u1EB1': // ằ  [LATIN SMALL LETTER A WITH BREVE AND GRAVE]
      case L'\u1EB3': // ẳ  [LATIN SMALL LETTER A WITH BREVE AND HOOK ABOVE]
      case L'\u1EB5': // ẵ  [LATIN SMALL LETTER A WITH BREVE AND TILDE]
      case L'\u1EB7': // ặ  [LATIN SMALL LETTER A WITH BREVE AND DOT BELOW]
      case L'\u2090': // ₐ  [LATIN SUBSCRIPT SMALL LETTER A]
      case L'\u2094': // ₔ  [LATIN SUBSCRIPT SMALL LETTER SCHWA]
      case L'\u24D0': // ⓐ  [CIRCLED LATIN SMALL LETTER A]
      case L'\u2C65': // ⱥ  [LATIN SMALL LETTER A WITH STROKE]
      case L'\u2C6F': // Ɐ  [LATIN CAPITAL LETTER TURNED A]
      case L'\uFF41': // ａ  [FULLWIDTH LATIN SMALL LETTER A]
        output[outputPos++] = L'a';
        break;
      case L'\uA732': // Ꜳ  [LATIN CAPITAL LETTER AA]
        output[outputPos++] = L'A';
        output[outputPos++] = L'A';
        break;
      case L'\u00C6': // Æ  [LATIN CAPITAL LETTER AE]
      case L'\u01E2': // Ǣ  [LATIN CAPITAL LETTER AE WITH MACRON]
      case L'\u01FC': // Ǽ  [LATIN CAPITAL LETTER AE WITH ACUTE]
      case L'\u1D01': // ᴁ  [LATIN LETTER SMALL CAPITAL AE]
        output[outputPos++] = L'A';
        output[outputPos++] = L'E';
        break;
      case L'\uA734': // Ꜵ  [LATIN CAPITAL LETTER AO]
        output[outputPos++] = L'A';
        output[outputPos++] = L'O';
        break;
      case L'\uA736': // Ꜷ  [LATIN CAPITAL LETTER AU]
        output[outputPos++] = L'A';
        output[outputPos++] = L'U';
        break;
      case L'\uA738': // Ꜹ  [LATIN CAPITAL LETTER AV]
      case L'\uA73A': // Ꜻ  [LATIN CAPITAL LETTER AV WITH HORIZONTAL BAR]
        output[outputPos++] = L'A';
        output[outputPos++] = L'V';
        break;
      case L'\uA73C': // Ꜽ  [LATIN CAPITAL LETTER AY]
        output[outputPos++] = L'A';
        output[outputPos++] = L'Y';
        break;
      case L'\u249C': // ⒜  [PARENTHESIZED LATIN SMALL LETTER A]
        output[outputPos++] = L'(';
        output[outputPos++] = L'a';
        output[outputPos++] = L')';
        break;
      case L'\uA733': // ꜳ  [LATIN SMALL LETTER AA]
        output[outputPos++] = L'a';
        output[outputPos++] = L'a';
        break;
      case L'\u00E6': // æ  [LATIN SMALL LETTER AE]
      case L'\u01E3': // ǣ  [LATIN SMALL LETTER AE WITH MACRON]
      case L'\u01FD': // ǽ  [LATIN SMALL LETTER AE WITH ACUTE]
      case L'\u1D02': // ᴂ  [LATIN SMALL LETTER TURNED AE]
        output[outputPos++] = L'a';
        output[outputPos++] = L'e';
        break;
      case L'\uA735': // ꜵ  [LATIN SMALL LETTER AO]
        output[outputPos++] = L'a';
        output[outputPos++] = L'o';
        break;
      case L'\uA737': // ꜷ  [LATIN SMALL LETTER AU]
        output[outputPos++] = L'a';
        output[outputPos++] = L'u';
        break;
      case L'\uA739': // ꜹ  [LATIN SMALL LETTER AV]
      case L'\uA73B': // ꜻ  [LATIN SMALL LETTER AV WITH HORIZONTAL BAR]
        output[outputPos++] = L'a';
        output[outputPos++] = L'v';
        break;
      case L'\uA73D': // ꜽ  [LATIN SMALL LETTER AY]
        output[outputPos++] = L'a';
        output[outputPos++] = L'y';
        break;
      case L'\u0181': // Ɓ  [LATIN CAPITAL LETTER B WITH HOOK]
      case L'\u0182': // Ƃ  [LATIN CAPITAL LETTER B WITH TOPBAR]
      case L'\u0243': // Ƀ  [LATIN CAPITAL LETTER B WITH STROKE]
      case L'\u0299': // ʙ  [LATIN LETTER SMALL CAPITAL B]
      case L'\u1D03': // ᴃ  [LATIN LETTER SMALL CAPITAL BARRED B]
      case L'\u1E02': // Ḃ  [LATIN CAPITAL LETTER B WITH DOT ABOVE]
      case L'\u1E04': // Ḅ  [LATIN CAPITAL LETTER B WITH DOT BELOW]
      case L'\u1E06': // Ḇ  [LATIN CAPITAL LETTER B WITH LINE BELOW]
      case L'\u24B7': // Ⓑ  [CIRCLED LATIN CAPITAL LETTER B]
      case L'\uFF22': // Ｂ  [FULLWIDTH LATIN CAPITAL LETTER B]
        output[outputPos++] = L'B';
        break;
      case L'\u0180': // ƀ  [LATIN SMALL LETTER B WITH STROKE]
      case L'\u0183': // ƃ  [LATIN SMALL LETTER B WITH TOPBAR]
      case L'\u0253': // ɓ  [LATIN SMALL LETTER B WITH HOOK]
      case L'\u1D6C': // ᵬ  [LATIN SMALL LETTER B WITH MIDDLE TILDE]
      case L'\u1D80': // ᶀ  [LATIN SMALL LETTER B WITH PALATAL HOOK]
      case L'\u1E03': // ḃ  [LATIN SMALL LETTER B WITH DOT ABOVE]
      case L'\u1E05': // ḅ  [LATIN SMALL LETTER B WITH DOT BELOW]
      case L'\u1E07': // ḇ  [LATIN SMALL LETTER B WITH LINE BELOW]
      case L'\u24D1': // ⓑ  [CIRCLED LATIN SMALL LETTER B]
      case L'\uFF42': // ｂ  [FULLWIDTH LATIN SMALL LETTER B]
        output[outputPos++] = L'b';
        break;
      case L'\u249D': // ⒝  [PARENTHESIZED LATIN SMALL LETTER B]
        output[outputPos++] = L'(';
        output[outputPos++] = L'b';
        output[outputPos++] = L')';
        break;
      case L'\u00C7': // Ç  [LATIN CAPITAL LETTER C WITH CEDILLA]
      case L'\u0106': // Ć  [LATIN CAPITAL LETTER C WITH ACUTE]
      case L'\u0108': // Ĉ  [LATIN CAPITAL LETTER C WITH CIRCUMFLEX]
      case L'\u010A': // Ċ  [LATIN CAPITAL LETTER C WITH DOT ABOVE]
      case L'\u010C': // Č  [LATIN CAPITAL LETTER C WITH CARON]
      case L'\u0187': // Ƈ  [LATIN CAPITAL LETTER C WITH HOOK]
      case L'\u023B': // Ȼ  [LATIN CAPITAL LETTER C WITH STROKE]
      case L'\u0297': // ʗ  [LATIN LETTER STRETCHED C]
      case L'\u1D04': // ᴄ  [LATIN LETTER SMALL CAPITAL C]
      case L'\u1E08': // Ḉ  [LATIN CAPITAL LETTER C WITH CEDILLA AND ACUTE]
      case L'\u24B8': // Ⓒ  [CIRCLED LATIN CAPITAL LETTER C]
      case L'\uFF23': // Ｃ  [FULLWIDTH LATIN CAPITAL LETTER C]
        output[outputPos++] = L'C';
        break;
      case L'\u00E7': // ç  [LATIN SMALL LETTER C WITH CEDILLA]
      case L'\u0107': // ć  [LATIN SMALL LETTER C WITH ACUTE]
      case L'\u0109': // ĉ  [LATIN SMALL LETTER C WITH CIRCUMFLEX]
      case L'\u010B': // ċ  [LATIN SMALL LETTER C WITH DOT ABOVE]
      case L'\u010D': // č  [LATIN SMALL LETTER C WITH CARON]
      case L'\u0188': // ƈ  [LATIN SMALL LETTER C WITH HOOK]
      case L'\u023C': // ȼ  [LATIN SMALL LETTER C WITH STROKE]
      case L'\u0255': // ɕ  [LATIN SMALL LETTER C WITH CURL]
      case L'\u1E09': // ḉ  [LATIN SMALL LETTER C WITH CEDILLA AND ACUTE]
      case L'\u2184': // ↄ  [LATIN SMALL LETTER REVERSED C]
      case L'\u24D2': // ⓒ  [CIRCLED LATIN SMALL LETTER C]
      case L'\uA73E': // Ꜿ  [LATIN CAPITAL LETTER REVERSED C WITH DOT]
      case L'\uA73F': // ꜿ  [LATIN SMALL LETTER REVERSED C WITH DOT]
      case L'\uFF43': // ｃ  [FULLWIDTH LATIN SMALL LETTER C]
        output[outputPos++] = L'c';
        break;
      case L'\u249E': // ⒞  [PARENTHESIZED LATIN SMALL LETTER C]
        output[outputPos++] = L'(';
        output[outputPos++] = L'c';
        output[outputPos++] = L')';
        break;
      case L'\u00D0': // Ð  [LATIN CAPITAL LETTER ETH]
      case L'\u010E': // Ď  [LATIN CAPITAL LETTER D WITH CARON]
      case L'\u0110': // Đ  [LATIN CAPITAL LETTER D WITH STROKE]
      case L'\u0189': // Ɖ  [LATIN CAPITAL LETTER AFRICAN D]
      case L'\u018A': // Ɗ  [LATIN CAPITAL LETTER D WITH HOOK]
      case L'\u018B': // Ƌ  [LATIN CAPITAL LETTER D WITH TOPBAR]
      case L'\u1D05': // ᴅ  [LATIN LETTER SMALL CAPITAL D]
      case L'\u1D06': // ᴆ  [LATIN LETTER SMALL CAPITAL ETH]
      case L'\u1E0A': // Ḋ  [LATIN CAPITAL LETTER D WITH DOT ABOVE]
      case L'\u1E0C': // Ḍ  [LATIN CAPITAL LETTER D WITH DOT BELOW]
      case L'\u1E0E': // Ḏ  [LATIN CAPITAL LETTER D WITH LINE BELOW]
      case L'\u1E10': // Ḑ  [LATIN CAPITAL LETTER D WITH CEDILLA]
      case L'\u1E12': // Ḓ  [LATIN CAPITAL LETTER D WITH CIRCUMFLEX BELOW]
      case L'\u24B9': // Ⓓ  [CIRCLED LATIN CAPITAL LETTER D]
      case L'\uA779': // Ꝺ  [LATIN CAPITAL LETTER INSULAR D]
      case L'\uFF24': // Ｄ  [FULLWIDTH LATIN CAPITAL LETTER D]
        output[outputPos++] = L'D';
        break;
      case L'\u00F0': // ð  [LATIN SMALL LETTER ETH]
      case L'\u010F': // ď  [LATIN SMALL LETTER D WITH CARON]
      case L'\u0111': // đ  [LATIN SMALL LETTER D WITH STROKE]
      case L'\u018C': // ƌ  [LATIN SMALL LETTER D WITH TOPBAR]
      case L'\u0221': // ȡ  [LATIN SMALL LETTER D WITH CURL]
      case L'\u0256': // ɖ  [LATIN SMALL LETTER D WITH TAIL]
      case L'\u0257': // ɗ  [LATIN SMALL LETTER D WITH HOOK]
      case L'\u1D6D': // ᵭ  [LATIN SMALL LETTER D WITH MIDDLE TILDE]
      case L'\u1D81': // ᶁ  [LATIN SMALL LETTER D WITH PALATAL HOOK]
      case L'\u1D91': // ᶑ  [LATIN SMALL LETTER D WITH HOOK AND TAIL]
      case L'\u1E0B': // ḋ  [LATIN SMALL LETTER D WITH DOT ABOVE]
      case L'\u1E0D': // ḍ  [LATIN SMALL LETTER D WITH DOT BELOW]
      case L'\u1E0F': // ḏ  [LATIN SMALL LETTER D WITH LINE BELOW]
      case L'\u1E11': // ḑ  [LATIN SMALL LETTER D WITH CEDILLA]
      case L'\u1E13': // ḓ  [LATIN SMALL LETTER D WITH CIRCUMFLEX BELOW]
      case L'\u24D3': // ⓓ  [CIRCLED LATIN SMALL LETTER D]
      case L'\uA77A': // ꝺ  [LATIN SMALL LETTER INSULAR D]
      case L'\uFF44': // ｄ  [FULLWIDTH LATIN SMALL LETTER D]
        output[outputPos++] = L'd';
        break;
      case L'\u01C4': // Ǆ  [LATIN CAPITAL LETTER DZ WITH CARON]
      case L'\u01F1': // Ǳ  [LATIN CAPITAL LETTER DZ]
        output[outputPos++] = L'D';
        output[outputPos++] = L'Z';
        break;
      case L'\u01C5': // ǅ  [LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH
                      // CARON]
      case L'\u01F2': // ǲ  [LATIN CAPITAL LETTER D WITH SMALL LETTER Z]
        output[outputPos++] = L'D';
        output[outputPos++] = L'z';
        break;
      case L'\u249F': // ⒟  [PARENTHESIZED LATIN SMALL LETTER D]
        output[outputPos++] = L'(';
        output[outputPos++] = L'd';
        output[outputPos++] = L')';
        break;
      case L'\u0238': // ȸ  [LATIN SMALL LETTER DB DIGRAPH]
        output[outputPos++] = L'd';
        output[outputPos++] = L'b';
        break;
      case L'\u01C6': // ǆ  [LATIN SMALL LETTER DZ WITH CARON]
      case L'\u01F3': // ǳ  [LATIN SMALL LETTER DZ]
      case L'\u02A3': // ʣ  [LATIN SMALL LETTER DZ DIGRAPH]
      case L'\u02A5': // ʥ  [LATIN SMALL LETTER DZ DIGRAPH WITH CURL]
        output[outputPos++] = L'd';
        output[outputPos++] = L'z';
        break;
      case L'\u00C8': // È  [LATIN CAPITAL LETTER E WITH GRAVE]
      case L'\u00C9': // É  [LATIN CAPITAL LETTER E WITH ACUTE]
      case L'\u00CA': // Ê  [LATIN CAPITAL LETTER E WITH CIRCUMFLEX]
      case L'\u00CB': // Ë  [LATIN CAPITAL LETTER E WITH DIAERESIS]
      case L'\u0112': // Ē  [LATIN CAPITAL LETTER E WITH MACRON]
      case L'\u0114': // Ĕ  [LATIN CAPITAL LETTER E WITH BREVE]
      case L'\u0116': // Ė  [LATIN CAPITAL LETTER E WITH DOT ABOVE]
      case L'\u0118': // Ę  [LATIN CAPITAL LETTER E WITH OGONEK]
      case L'\u011A': // Ě  [LATIN CAPITAL LETTER E WITH CARON]
      case L'\u018E': // Ǝ  [LATIN CAPITAL LETTER REVERSED E]
      case L'\u0190': // Ɛ  [LATIN CAPITAL LETTER OPEN E]
      case L'\u0204': // Ȅ  [LATIN CAPITAL LETTER E WITH DOUBLE GRAVE]
      case L'\u0206': // Ȇ  [LATIN CAPITAL LETTER E WITH INVERTED BREVE]
      case L'\u0228': // Ȩ  [LATIN CAPITAL LETTER E WITH CEDILLA]
      case L'\u0246': // Ɇ  [LATIN CAPITAL LETTER E WITH STROKE]
      case L'\u1D07': // ᴇ  [LATIN LETTER SMALL CAPITAL E]
      case L'\u1E14': // Ḕ  [LATIN CAPITAL LETTER E WITH MACRON AND GRAVE]
      case L'\u1E16': // Ḗ  [LATIN CAPITAL LETTER E WITH MACRON AND ACUTE]
      case L'\u1E18': // Ḙ  [LATIN CAPITAL LETTER E WITH CIRCUMFLEX BELOW]
      case L'\u1E1A': // Ḛ  [LATIN CAPITAL LETTER E WITH TILDE BELOW]
      case L'\u1E1C': // Ḝ  [LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE]
      case L'\u1EB8': // Ẹ  [LATIN CAPITAL LETTER E WITH DOT BELOW]
      case L'\u1EBA': // Ẻ  [LATIN CAPITAL LETTER E WITH HOOK ABOVE]
      case L'\u1EBC': // Ẽ  [LATIN CAPITAL LETTER E WITH TILDE]
      case L'\u1EBE': // Ế  [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE]
      case L'\u1EC0': // Ề  [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE]
      case L'\u1EC2': // Ể  [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK
                      // ABOVE]
      case L'\u1EC4': // Ễ  [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE]
      case L'\u1EC6': // Ệ  [LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT
                      // BELOW]
      case L'\u24BA': // Ⓔ  [CIRCLED LATIN CAPITAL LETTER E]
      case L'\u2C7B': // ⱻ  [LATIN LETTER SMALL CAPITAL TURNED E]
      case L'\uFF25': // Ｅ  [FULLWIDTH LATIN CAPITAL LETTER E]
        output[outputPos++] = L'E';
        break;
      case L'\u00E8': // è  [LATIN SMALL LETTER E WITH GRAVE]
      case L'\u00E9': // é  [LATIN SMALL LETTER E WITH ACUTE]
      case L'\u00EA': // ê  [LATIN SMALL LETTER E WITH CIRCUMFLEX]
      case L'\u00EB': // ë  [LATIN SMALL LETTER E WITH DIAERESIS]
      case L'\u0113': // ē  [LATIN SMALL LETTER E WITH MACRON]
      case L'\u0115': // ĕ  [LATIN SMALL LETTER E WITH BREVE]
      case L'\u0117': // ė  [LATIN SMALL LETTER E WITH DOT ABOVE]
      case L'\u0119': // ę  [LATIN SMALL LETTER E WITH OGONEK]
      case L'\u011B': // ě  [LATIN SMALL LETTER E WITH CARON]
      case L'\u01DD': // ǝ  [LATIN SMALL LETTER TURNED E]
      case L'\u0205': // ȅ  [LATIN SMALL LETTER E WITH DOUBLE GRAVE]
      case L'\u0207': // ȇ  [LATIN SMALL LETTER E WITH INVERTED BREVE]
      case L'\u0229': // ȩ  [LATIN SMALL LETTER E WITH CEDILLA]
      case L'\u0247': // ɇ  [LATIN SMALL LETTER E WITH STROKE]
      case L'\u0258': // ɘ  [LATIN SMALL LETTER REVERSED E]
      case L'\u025B': // ɛ  [LATIN SMALL LETTER OPEN E]
      case L'\u025C': // ɜ  [LATIN SMALL LETTER REVERSED OPEN E]
      case L'\u025D': // ɝ  [LATIN SMALL LETTER REVERSED OPEN E WITH HOOK]
      case L'\u025E': // ɞ  [LATIN SMALL LETTER CLOSED REVERSED OPEN E]
      case L'\u029A': // ʚ  [LATIN SMALL LETTER CLOSED OPEN E]
      case L'\u1D08': // ᴈ  [LATIN SMALL LETTER TURNED OPEN E]
      case L'\u1D92': // ᶒ  [LATIN SMALL LETTER E WITH RETROFLEX HOOK]
      case L'\u1D93': // ᶓ  [LATIN SMALL LETTER OPEN E WITH RETROFLEX HOOK]
      case L'\u1D94': // ᶔ  [LATIN SMALL LETTER REVERSED OPEN E WITH RETROFLEX
                      // HOOK]
      case L'\u1E15': // ḕ  [LATIN SMALL LETTER E WITH MACRON AND GRAVE]
      case L'\u1E17': // ḗ  [LATIN SMALL LETTER E WITH MACRON AND ACUTE]
      case L'\u1E19': // ḙ  [LATIN SMALL LETTER E WITH CIRCUMFLEX BELOW]
      case L'\u1E1B': // ḛ  [LATIN SMALL LETTER E WITH TILDE BELOW]
      case L'\u1E1D': // ḝ  [LATIN SMALL LETTER E WITH CEDILLA AND BREVE]
      case L'\u1EB9': // ẹ  [LATIN SMALL LETTER E WITH DOT BELOW]
      case L'\u1EBB': // ẻ  [LATIN SMALL LETTER E WITH HOOK ABOVE]
      case L'\u1EBD': // ẽ  [LATIN SMALL LETTER E WITH TILDE]
      case L'\u1EBF': // ế  [LATIN SMALL LETTER E WITH CIRCUMFLEX AND ACUTE]
      case L'\u1EC1': // ề  [LATIN SMALL LETTER E WITH CIRCUMFLEX AND GRAVE]
      case L'\u1EC3': // ể  [LATIN SMALL LETTER E WITH CIRCUMFLEX AND HOOK
                      // ABOVE]
      case L'\u1EC5': // ễ  [LATIN SMALL LETTER E WITH CIRCUMFLEX AND TILDE]
      case L'\u1EC7': // ệ  [LATIN SMALL LETTER E WITH CIRCUMFLEX AND DOT BELOW]
      case L'\u2091': // ₑ  [LATIN SUBSCRIPT SMALL LETTER E]
      case L'\u24D4': // ⓔ  [CIRCLED LATIN SMALL LETTER E]
      case L'\u2C78': // ⱸ  [LATIN SMALL LETTER E WITH NOTCH]
      case L'\uFF45': // ｅ  [FULLWIDTH LATIN SMALL LETTER E]
        output[outputPos++] = L'e';
        break;
      case L'\u24A0': // ⒠  [PARENTHESIZED LATIN SMALL LETTER E]
        output[outputPos++] = L'(';
        output[outputPos++] = L'e';
        output[outputPos++] = L')';
        break;
      case L'\u0191': // Ƒ  [LATIN CAPITAL LETTER F WITH HOOK]
      case L'\u1E1E': // Ḟ  [LATIN CAPITAL LETTER F WITH DOT ABOVE]
      case L'\u24BB': // Ⓕ  [CIRCLED LATIN CAPITAL LETTER F]
      case L'\uA730': // ꜰ  [LATIN LETTER SMALL CAPITAL F]
      case L'\uA77B': // Ꝼ  [LATIN CAPITAL LETTER INSULAR F]
      case L'\uA7FB': // ꟻ  [LATIN EPIGRAPHIC LETTER REVERSED F]
      case L'\uFF26': // Ｆ  [FULLWIDTH LATIN CAPITAL LETTER F]
        output[outputPos++] = L'F';
        break;
      case L'\u0192': // ƒ  [LATIN SMALL LETTER F WITH HOOK]
      case L'\u1D6E': // ᵮ  [LATIN SMALL LETTER F WITH MIDDLE TILDE]
      case L'\u1D82': // ᶂ  [LATIN SMALL LETTER F WITH PALATAL HOOK]
      case L'\u1E1F': // ḟ  [LATIN SMALL LETTER F WITH DOT ABOVE]
      case L'\u1E9B': // ẛ  [LATIN SMALL LETTER LONG S WITH DOT ABOVE]
      case L'\u24D5': // ⓕ  [CIRCLED LATIN SMALL LETTER F]
      case L'\uA77C': // ꝼ  [LATIN SMALL LETTER INSULAR F]
      case L'\uFF46': // ｆ  [FULLWIDTH LATIN SMALL LETTER F]
        output[outputPos++] = L'f';
        break;
      case L'\u24A1': // ⒡  [PARENTHESIZED LATIN SMALL LETTER F]
        output[outputPos++] = L'(';
        output[outputPos++] = L'f';
        output[outputPos++] = L')';
        break;
      case L'\uFB00': // ﬀ  [LATIN SMALL LIGATURE FF]
        output[outputPos++] = L'f';
        output[outputPos++] = L'f';
        break;
      case L'\uFB03': // ﬃ  [LATIN SMALL LIGATURE FFI]
        output[outputPos++] = L'f';
        output[outputPos++] = L'f';
        output[outputPos++] = L'i';
        break;
      case L'\uFB04': // ﬄ  [LATIN SMALL LIGATURE FFL]
        output[outputPos++] = L'f';
        output[outputPos++] = L'f';
        output[outputPos++] = L'l';
        break;
      case L'\uFB01': // ﬁ  [LATIN SMALL LIGATURE FI]
        output[outputPos++] = L'f';
        output[outputPos++] = L'i';
        break;
      case L'\uFB02': // ﬂ  [LATIN SMALL LIGATURE FL]
        output[outputPos++] = L'f';
        output[outputPos++] = L'l';
        break;
      case L'\u011C': // Ĝ  [LATIN CAPITAL LETTER G WITH CIRCUMFLEX]
      case L'\u011E': // Ğ  [LATIN CAPITAL LETTER G WITH BREVE]
      case L'\u0120': // Ġ  [LATIN CAPITAL LETTER G WITH DOT ABOVE]
      case L'\u0122': // Ģ  [LATIN CAPITAL LETTER G WITH CEDILLA]
      case L'\u0193': // Ɠ  [LATIN CAPITAL LETTER G WITH HOOK]
      case L'\u01E4': // Ǥ  [LATIN CAPITAL LETTER G WITH STROKE]
      case L'\u01E5': // ǥ  [LATIN SMALL LETTER G WITH STROKE]
      case L'\u01E6': // Ǧ  [LATIN CAPITAL LETTER G WITH CARON]
      case L'\u01E7': // ǧ  [LATIN SMALL LETTER G WITH CARON]
      case L'\u01F4': // Ǵ  [LATIN CAPITAL LETTER G WITH ACUTE]
      case L'\u0262': // ɢ  [LATIN LETTER SMALL CAPITAL G]
      case L'\u029B': // ʛ  [LATIN LETTER SMALL CAPITAL G WITH HOOK]
      case L'\u1E20': // Ḡ  [LATIN CAPITAL LETTER G WITH MACRON]
      case L'\u24BC': // Ⓖ  [CIRCLED LATIN CAPITAL LETTER G]
      case L'\uA77D': // Ᵹ  [LATIN CAPITAL LETTER INSULAR G]
      case L'\uA77E': // Ꝿ  [LATIN CAPITAL LETTER TURNED INSULAR G]
      case L'\uFF27': // Ｇ  [FULLWIDTH LATIN CAPITAL LETTER G]
        output[outputPos++] = L'G';
        break;
      case L'\u011D': // ĝ  [LATIN SMALL LETTER G WITH CIRCUMFLEX]
      case L'\u011F': // ğ  [LATIN SMALL LETTER G WITH BREVE]
      case L'\u0121': // ġ  [LATIN SMALL LETTER G WITH DOT ABOVE]
      case L'\u0123': // ģ  [LATIN SMALL LETTER G WITH CEDILLA]
      case L'\u01F5': // ǵ  [LATIN SMALL LETTER G WITH ACUTE]
      case L'\u0260': // ɠ  [LATIN SMALL LETTER G WITH HOOK]
      case L'\u0261': // ɡ  [LATIN SMALL LETTER SCRIPT G]
      case L'\u1D77': // ᵷ  [LATIN SMALL LETTER TURNED G]
      case L'\u1D79': // ᵹ  [LATIN SMALL LETTER INSULAR G]
      case L'\u1D83': // ᶃ  [LATIN SMALL LETTER G WITH PALATAL HOOK]
      case L'\u1E21': // ḡ  [LATIN SMALL LETTER G WITH MACRON]
      case L'\u24D6': // ⓖ  [CIRCLED LATIN SMALL LETTER G]
      case L'\uA77F': // ꝿ  [LATIN SMALL LETTER TURNED INSULAR G]
      case L'\uFF47': // ｇ  [FULLWIDTH LATIN SMALL LETTER G]
        output[outputPos++] = L'g';
        break;
      case L'\u24A2': // ⒢  [PARENTHESIZED LATIN SMALL LETTER G]
        output[outputPos++] = L'(';
        output[outputPos++] = L'g';
        output[outputPos++] = L')';
        break;
      case L'\u0124': // Ĥ  [LATIN CAPITAL LETTER H WITH CIRCUMFLEX]
      case L'\u0126': // Ħ  [LATIN CAPITAL LETTER H WITH STROKE]
      case L'\u021E': // Ȟ  [LATIN CAPITAL LETTER H WITH CARON]
      case L'\u029C': // ʜ  [LATIN LETTER SMALL CAPITAL H]
      case L'\u1E22': // Ḣ  [LATIN CAPITAL LETTER H WITH DOT ABOVE]
      case L'\u1E24': // Ḥ  [LATIN CAPITAL LETTER H WITH DOT BELOW]
      case L'\u1E26': // Ḧ  [LATIN CAPITAL LETTER H WITH DIAERESIS]
      case L'\u1E28': // Ḩ  [LATIN CAPITAL LETTER H WITH CEDILLA]
      case L'\u1E2A': // Ḫ  [LATIN CAPITAL LETTER H WITH BREVE BELOW]
      case L'\u24BD': // Ⓗ  [CIRCLED LATIN CAPITAL LETTER H]
      case L'\u2C67': // Ⱨ  [LATIN CAPITAL LETTER H WITH DESCENDER]
      case L'\u2C75': // Ⱶ  [LATIN CAPITAL LETTER HALF H]
      case L'\uFF28': // Ｈ  [FULLWIDTH LATIN CAPITAL LETTER H]
        output[outputPos++] = L'H';
        break;
      case L'\u0125': // ĥ  [LATIN SMALL LETTER H WITH CIRCUMFLEX]
      case L'\u0127': // ħ  [LATIN SMALL LETTER H WITH STROKE]
      case L'\u021F': // ȟ  [LATIN SMALL LETTER H WITH CARON]
      case L'\u0265': // ɥ  [LATIN SMALL LETTER TURNED H]
      case L'\u0266': // ɦ  [LATIN SMALL LETTER H WITH HOOK]
      case L'\u02AE': // ʮ  [LATIN SMALL LETTER TURNED H WITH FISHHOOK]
      case L'\u02AF': // ʯ  [LATIN SMALL LETTER TURNED H WITH FISHHOOK AND TAIL]
      case L'\u1E23': // ḣ  [LATIN SMALL LETTER H WITH DOT ABOVE]
      case L'\u1E25': // ḥ  [LATIN SMALL LETTER H WITH DOT BELOW]
      case L'\u1E27': // ḧ  [LATIN SMALL LETTER H WITH DIAERESIS]
      case L'\u1E29': // ḩ  [LATIN SMALL LETTER H WITH CEDILLA]
      case L'\u1E2B': // ḫ  [LATIN SMALL LETTER H WITH BREVE BELOW]
      case L'\u1E96': // ẖ  [LATIN SMALL LETTER H WITH LINE BELOW]
      case L'\u24D7': // ⓗ  [CIRCLED LATIN SMALL LETTER H]
      case L'\u2C68': // ⱨ  [LATIN SMALL LETTER H WITH DESCENDER]
      case L'\u2C76': // ⱶ  [LATIN SMALL LETTER HALF H]
      case L'\uFF48': // ｈ  [FULLWIDTH LATIN SMALL LETTER H]
        output[outputPos++] = L'h';
        break;
      case L'\u01F6': // Ƕ  http://en.wikipedia.org/wiki/Hwair  [LATIN CAPITAL
                      // LETTER HWAIR]
        output[outputPos++] = L'H';
        output[outputPos++] = L'V';
        break;
      case L'\u24A3': // ⒣  [PARENTHESIZED LATIN SMALL LETTER H]
        output[outputPos++] = L'(';
        output[outputPos++] = L'h';
        output[outputPos++] = L')';
        break;
      case L'\u0195': // ƕ  [LATIN SMALL LETTER HV]
        output[outputPos++] = L'h';
        output[outputPos++] = L'v';
        break;
      case L'\u00CC': // Ì  [LATIN CAPITAL LETTER I WITH GRAVE]
      case L'\u00CD': // Í  [LATIN CAPITAL LETTER I WITH ACUTE]
      case L'\u00CE': // Î  [LATIN CAPITAL LETTER I WITH CIRCUMFLEX]
      case L'\u00CF': // Ï  [LATIN CAPITAL LETTER I WITH DIAERESIS]
      case L'\u0128': // Ĩ  [LATIN CAPITAL LETTER I WITH TILDE]
      case L'\u012A': // Ī  [LATIN CAPITAL LETTER I WITH MACRON]
      case L'\u012C': // Ĭ  [LATIN CAPITAL LETTER I WITH BREVE]
      case L'\u012E': // Į  [LATIN CAPITAL LETTER I WITH OGONEK]
      case L'\u0130': // İ  [LATIN CAPITAL LETTER I WITH DOT ABOVE]
      case L'\u0196': // Ɩ  [LATIN CAPITAL LETTER IOTA]
      case L'\u0197': // Ɨ  [LATIN CAPITAL LETTER I WITH STROKE]
      case L'\u01CF': // Ǐ  [LATIN CAPITAL LETTER I WITH CARON]
      case L'\u0208': // Ȉ  [LATIN CAPITAL LETTER I WITH DOUBLE GRAVE]
      case L'\u020A': // Ȋ  [LATIN CAPITAL LETTER I WITH INVERTED BREVE]
      case L'\u026A': // ɪ  [LATIN LETTER SMALL CAPITAL I]
      case L'\u1D7B': // ᵻ  [LATIN SMALL CAPITAL LETTER I WITH STROKE]
      case L'\u1E2C': // Ḭ  [LATIN CAPITAL LETTER I WITH TILDE BELOW]
      case L'\u1E2E': // Ḯ  [LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE]
      case L'\u1EC8': // Ỉ  [LATIN CAPITAL LETTER I WITH HOOK ABOVE]
      case L'\u1ECA': // Ị  [LATIN CAPITAL LETTER I WITH DOT BELOW]
      case L'\u24BE': // Ⓘ  [CIRCLED LATIN CAPITAL LETTER I]
      case L'\uA7FE': // ꟾ  [LATIN EPIGRAPHIC LETTER I LONGA]
      case L'\uFF29': // Ｉ  [FULLWIDTH LATIN CAPITAL LETTER I]
        output[outputPos++] = L'I';
        break;
      case L'\u00EC': // ì  [LATIN SMALL LETTER I WITH GRAVE]
      case L'\u00ED': // í  [LATIN SMALL LETTER I WITH ACUTE]
      case L'\u00EE': // î  [LATIN SMALL LETTER I WITH CIRCUMFLEX]
      case L'\u00EF': // ï  [LATIN SMALL LETTER I WITH DIAERESIS]
      case L'\u0129': // ĩ  [LATIN SMALL LETTER I WITH TILDE]
      case L'\u012B': // ī  [LATIN SMALL LETTER I WITH MACRON]
      case L'\u012D': // ĭ  [LATIN SMALL LETTER I WITH BREVE]
      case L'\u012F': // į  [LATIN SMALL LETTER I WITH OGONEK]
      case L'\u0131': // ı  [LATIN SMALL LETTER DOTLESS I]
      case L'\u01D0': // ǐ  [LATIN SMALL LETTER I WITH CARON]
      case L'\u0209': // ȉ  [LATIN SMALL LETTER I WITH DOUBLE GRAVE]
      case L'\u020B': // ȋ  [LATIN SMALL LETTER I WITH INVERTED BREVE]
      case L'\u0268': // ɨ  [LATIN SMALL LETTER I WITH STROKE]
      case L'\u1D09': // ᴉ  [LATIN SMALL LETTER TURNED I]
      case L'\u1D62': // ᵢ  [LATIN SUBSCRIPT SMALL LETTER I]
      case L'\u1D7C': // ᵼ  [LATIN SMALL LETTER IOTA WITH STROKE]
      case L'\u1D96': // ᶖ  [LATIN SMALL LETTER I WITH RETROFLEX HOOK]
      case L'\u1E2D': // ḭ  [LATIN SMALL LETTER I WITH TILDE BELOW]
      case L'\u1E2F': // ḯ  [LATIN SMALL LETTER I WITH DIAERESIS AND ACUTE]
      case L'\u1EC9': // ỉ  [LATIN SMALL LETTER I WITH HOOK ABOVE]
      case L'\u1ECB': // ị  [LATIN SMALL LETTER I WITH DOT BELOW]
      case L'\u2071': // ⁱ  [SUPERSCRIPT LATIN SMALL LETTER I]
      case L'\u24D8': // ⓘ  [CIRCLED LATIN SMALL LETTER I]
      case L'\uFF49': // ｉ  [FULLWIDTH LATIN SMALL LETTER I]
        output[outputPos++] = L'i';
        break;
      case L'\u0132': // Ĳ  [LATIN CAPITAL LIGATURE IJ]
        output[outputPos++] = L'I';
        output[outputPos++] = L'J';
        break;
      case L'\u24A4': // ⒤  [PARENTHESIZED LATIN SMALL LETTER I]
        output[outputPos++] = L'(';
        output[outputPos++] = L'i';
        output[outputPos++] = L')';
        break;
      case L'\u0133': // ĳ  [LATIN SMALL LIGATURE IJ]
        output[outputPos++] = L'i';
        output[outputPos++] = L'j';
        break;
      case L'\u0134': // Ĵ  [LATIN CAPITAL LETTER J WITH CIRCUMFLEX]
      case L'\u0248': // Ɉ  [LATIN CAPITAL LETTER J WITH STROKE]
      case L'\u1D0A': // ᴊ  [LATIN LETTER SMALL CAPITAL J]
      case L'\u24BF': // Ⓙ  [CIRCLED LATIN CAPITAL LETTER J]
      case L'\uFF2A': // Ｊ  [FULLWIDTH LATIN CAPITAL LETTER J]
        output[outputPos++] = L'J';
        break;
      case L'\u0135': // ĵ  [LATIN SMALL LETTER J WITH CIRCUMFLEX]
      case L'\u01F0': // ǰ  [LATIN SMALL LETTER J WITH CARON]
      case L'\u0237': // ȷ  [LATIN SMALL LETTER DOTLESS J]
      case L'\u0249': // ɉ  [LATIN SMALL LETTER J WITH STROKE]
      case L'\u025F': // ɟ  [LATIN SMALL LETTER DOTLESS J WITH STROKE]
      case L'\u0284': // ʄ  [LATIN SMALL LETTER DOTLESS J WITH STROKE AND HOOK]
      case L'\u029D': // ʝ  [LATIN SMALL LETTER J WITH CROSSED-TAIL]
      case L'\u24D9': // ⓙ  [CIRCLED LATIN SMALL LETTER J]
      case L'\u2C7C': // ⱼ  [LATIN SUBSCRIPT SMALL LETTER J]
      case L'\uFF4A': // ｊ  [FULLWIDTH LATIN SMALL LETTER J]
        output[outputPos++] = L'j';
        break;
      case L'\u24A5': // ⒥  [PARENTHESIZED LATIN SMALL LETTER J]
        output[outputPos++] = L'(';
        output[outputPos++] = L'j';
        output[outputPos++] = L')';
        break;
      case L'\u0136': // Ķ  [LATIN CAPITAL LETTER K WITH CEDILLA]
      case L'\u0198': // Ƙ  [LATIN CAPITAL LETTER K WITH HOOK]
      case L'\u01E8': // Ǩ  [LATIN CAPITAL LETTER K WITH CARON]
      case L'\u1D0B': // ᴋ  [LATIN LETTER SMALL CAPITAL K]
      case L'\u1E30': // Ḱ  [LATIN CAPITAL LETTER K WITH ACUTE]
      case L'\u1E32': // Ḳ  [LATIN CAPITAL LETTER K WITH DOT BELOW]
      case L'\u1E34': // Ḵ  [LATIN CAPITAL LETTER K WITH LINE BELOW]
      case L'\u24C0': // Ⓚ  [CIRCLED LATIN CAPITAL LETTER K]
      case L'\u2C69': // Ⱪ  [LATIN CAPITAL LETTER K WITH DESCENDER]
      case L'\uA740': // Ꝁ  [LATIN CAPITAL LETTER K WITH STROKE]
      case L'\uA742': // Ꝃ  [LATIN CAPITAL LETTER K WITH DIAGONAL STROKE]
      case L'\uA744': // Ꝅ  [LATIN CAPITAL LETTER K WITH STROKE AND DIAGONAL
                      // STROKE]
      case L'\uFF2B': // Ｋ  [FULLWIDTH LATIN CAPITAL LETTER K]
        output[outputPos++] = L'K';
        break;
      case L'\u0137': // ķ  [LATIN SMALL LETTER K WITH CEDILLA]
      case L'\u0199': // ƙ  [LATIN SMALL LETTER K WITH HOOK]
      case L'\u01E9': // ǩ  [LATIN SMALL LETTER K WITH CARON]
      case L'\u029E': // ʞ  [LATIN SMALL LETTER TURNED K]
      case L'\u1D84': // ᶄ  [LATIN SMALL LETTER K WITH PALATAL HOOK]
      case L'\u1E31': // ḱ  [LATIN SMALL LETTER K WITH ACUTE]
      case L'\u1E33': // ḳ  [LATIN SMALL LETTER K WITH DOT BELOW]
      case L'\u1E35': // ḵ  [LATIN SMALL LETTER K WITH LINE BELOW]
      case L'\u24DA': // ⓚ  [CIRCLED LATIN SMALL LETTER K]
      case L'\u2C6A': // ⱪ  [LATIN SMALL LETTER K WITH DESCENDER]
      case L'\uA741': // ꝁ  [LATIN SMALL LETTER K WITH STROKE]
      case L'\uA743': // ꝃ  [LATIN SMALL LETTER K WITH DIAGONAL STROKE]
      case L'\uA745': // ꝅ  [LATIN SMALL LETTER K WITH STROKE AND DIAGONAL
                      // STROKE]
      case L'\uFF4B': // ｋ  [FULLWIDTH LATIN SMALL LETTER K]
        output[outputPos++] = L'k';
        break;
      case L'\u24A6': // ⒦  [PARENTHESIZED LATIN SMALL LETTER K]
        output[outputPos++] = L'(';
        output[outputPos++] = L'k';
        output[outputPos++] = L')';
        break;
      case L'\u0139': // Ĺ  [LATIN CAPITAL LETTER L WITH ACUTE]
      case L'\u013B': // Ļ  [LATIN CAPITAL LETTER L WITH CEDILLA]
      case L'\u013D': // Ľ  [LATIN CAPITAL LETTER L WITH CARON]
      case L'\u013F': // Ŀ  [LATIN CAPITAL LETTER L WITH MIDDLE DOT]
      case L'\u0141': // Ł  [LATIN CAPITAL LETTER L WITH STROKE]
      case L'\u023D': // Ƚ  [LATIN CAPITAL LETTER L WITH BAR]
      case L'\u029F': // ʟ  [LATIN LETTER SMALL CAPITAL L]
      case L'\u1D0C': // ᴌ  [LATIN LETTER SMALL CAPITAL L WITH STROKE]
      case L'\u1E36': // Ḷ  [LATIN CAPITAL LETTER L WITH DOT BELOW]
      case L'\u1E38': // Ḹ  [LATIN CAPITAL LETTER L WITH DOT BELOW AND MACRON]
      case L'\u1E3A': // Ḻ  [LATIN CAPITAL LETTER L WITH LINE BELOW]
      case L'\u1E3C': // Ḽ  [LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW]
      case L'\u24C1': // Ⓛ  [CIRCLED LATIN CAPITAL LETTER L]
      case L'\u2C60': // Ⱡ  [LATIN CAPITAL LETTER L WITH DOUBLE BAR]
      case L'\u2C62': // Ɫ  [LATIN CAPITAL LETTER L WITH MIDDLE TILDE]
      case L'\uA746': // Ꝇ  [LATIN CAPITAL LETTER BROKEN L]
      case L'\uA748': // Ꝉ  [LATIN CAPITAL LETTER L WITH HIGH STROKE]
      case L'\uA780': // Ꞁ  [LATIN CAPITAL LETTER TURNED L]
      case L'\uFF2C': // Ｌ  [FULLWIDTH LATIN CAPITAL LETTER L]
        output[outputPos++] = L'L';
        break;
      case L'\u013A': // ĺ  [LATIN SMALL LETTER L WITH ACUTE]
      case L'\u013C': // ļ  [LATIN SMALL LETTER L WITH CEDILLA]
      case L'\u013E': // ľ  [LATIN SMALL LETTER L WITH CARON]
      case L'\u0140': // ŀ  [LATIN SMALL LETTER L WITH MIDDLE DOT]
      case L'\u0142': // ł  [LATIN SMALL LETTER L WITH STROKE]
      case L'\u019A': // ƚ  [LATIN SMALL LETTER L WITH BAR]
      case L'\u0234': // ȴ  [LATIN SMALL LETTER L WITH CURL]
      case L'\u026B': // ɫ  [LATIN SMALL LETTER L WITH MIDDLE TILDE]
      case L'\u026C': // ɬ  [LATIN SMALL LETTER L WITH BELT]
      case L'\u026D': // ɭ  [LATIN SMALL LETTER L WITH RETROFLEX HOOK]
      case L'\u1D85': // ᶅ  [LATIN SMALL LETTER L WITH PALATAL HOOK]
      case L'\u1E37': // ḷ  [LATIN SMALL LETTER L WITH DOT BELOW]
      case L'\u1E39': // ḹ  [LATIN SMALL LETTER L WITH DOT BELOW AND MACRON]
      case L'\u1E3B': // ḻ  [LATIN SMALL LETTER L WITH LINE BELOW]
      case L'\u1E3D': // ḽ  [LATIN SMALL LETTER L WITH CIRCUMFLEX BELOW]
      case L'\u24DB': // ⓛ  [CIRCLED LATIN SMALL LETTER L]
      case L'\u2C61': // ⱡ  [LATIN SMALL LETTER L WITH DOUBLE BAR]
      case L'\uA747': // ꝇ  [LATIN SMALL LETTER BROKEN L]
      case L'\uA749': // ꝉ  [LATIN SMALL LETTER L WITH HIGH STROKE]
      case L'\uA781': // ꞁ  [LATIN SMALL LETTER TURNED L]
      case L'\uFF4C': // ｌ  [FULLWIDTH LATIN SMALL LETTER L]
        output[outputPos++] = L'l';
        break;
      case L'\u01C7': // Ǉ  [LATIN CAPITAL LETTER LJ]
        output[outputPos++] = L'L';
        output[outputPos++] = L'J';
        break;
      case L'\u1EFA': // Ỻ  [LATIN CAPITAL LETTER MIDDLE-WELSH LL]
        output[outputPos++] = L'L';
        output[outputPos++] = L'L';
        break;
      case L'\u01C8': // ǈ  [LATIN CAPITAL LETTER L WITH SMALL LETTER J]
        output[outputPos++] = L'L';
        output[outputPos++] = L'j';
        break;
      case L'\u24A7': // ⒧  [PARENTHESIZED LATIN SMALL LETTER L]
        output[outputPos++] = L'(';
        output[outputPos++] = L'l';
        output[outputPos++] = L')';
        break;
      case L'\u01C9': // ǉ  [LATIN SMALL LETTER LJ]
        output[outputPos++] = L'l';
        output[outputPos++] = L'j';
        break;
      case L'\u1EFB': // ỻ  [LATIN SMALL LETTER MIDDLE-WELSH LL]
        output[outputPos++] = L'l';
        output[outputPos++] = L'l';
        break;
      case L'\u02AA': // ʪ  [LATIN SMALL LETTER LS DIGRAPH]
        output[outputPos++] = L'l';
        output[outputPos++] = L's';
        break;
      case L'\u02AB': // ʫ  [LATIN SMALL LETTER LZ DIGRAPH]
        output[outputPos++] = L'l';
        output[outputPos++] = L'z';
        break;
      case L'\u019C': // Ɯ  [LATIN CAPITAL LETTER TURNED M]
      case L'\u1D0D': // ᴍ  [LATIN LETTER SMALL CAPITAL M]
      case L'\u1E3E': // Ḿ  [LATIN CAPITAL LETTER M WITH ACUTE]
      case L'\u1E40': // Ṁ  [LATIN CAPITAL LETTER M WITH DOT ABOVE]
      case L'\u1E42': // Ṃ  [LATIN CAPITAL LETTER M WITH DOT BELOW]
      case L'\u24C2': // Ⓜ  [CIRCLED LATIN CAPITAL LETTER M]
      case L'\u2C6E': // Ɱ  [LATIN CAPITAL LETTER M WITH HOOK]
      case L'\uA7FD': // ꟽ  [LATIN EPIGRAPHIC LETTER INVERTED M]
      case L'\uA7FF': // ꟿ  [LATIN EPIGRAPHIC LETTER ARCHAIC M]
      case L'\uFF2D': // Ｍ  [FULLWIDTH LATIN CAPITAL LETTER M]
        output[outputPos++] = L'M';
        break;
      case L'\u026F': // ɯ  [LATIN SMALL LETTER TURNED M]
      case L'\u0270': // ɰ  [LATIN SMALL LETTER TURNED M WITH LONG LEG]
      case L'\u0271': // ɱ  [LATIN SMALL LETTER M WITH HOOK]
      case L'\u1D6F': // ᵯ  [LATIN SMALL LETTER M WITH MIDDLE TILDE]
      case L'\u1D86': // ᶆ  [LATIN SMALL LETTER M WITH PALATAL HOOK]
      case L'\u1E3F': // ḿ  [LATIN SMALL LETTER M WITH ACUTE]
      case L'\u1E41': // ṁ  [LATIN SMALL LETTER M WITH DOT ABOVE]
      case L'\u1E43': // ṃ  [LATIN SMALL LETTER M WITH DOT BELOW]
      case L'\u24DC': // ⓜ  [CIRCLED LATIN SMALL LETTER M]
      case L'\uFF4D': // ｍ  [FULLWIDTH LATIN SMALL LETTER M]
        output[outputPos++] = L'm';
        break;
      case L'\u24A8': // ⒨  [PARENTHESIZED LATIN SMALL LETTER M]
        output[outputPos++] = L'(';
        output[outputPos++] = L'm';
        output[outputPos++] = L')';
        break;
      case L'\u00D1': // Ñ  [LATIN CAPITAL LETTER N WITH TILDE]
      case L'\u0143': // Ń  [LATIN CAPITAL LETTER N WITH ACUTE]
      case L'\u0145': // Ņ  [LATIN CAPITAL LETTER N WITH CEDILLA]
      case L'\u0147': // Ň  [LATIN CAPITAL LETTER N WITH CARON]
      case L'\u014A': // Ŋ  http://en.wikipedia.org/wiki/Eng_(letter)  [LATIN
                      // CAPITAL LETTER ENG]
      case L'\u019D': // Ɲ  [LATIN CAPITAL LETTER N WITH LEFT HOOK]
      case L'\u01F8': // Ǹ  [LATIN CAPITAL LETTER N WITH GRAVE]
      case L'\u0220': // Ƞ  [LATIN CAPITAL LETTER N WITH LONG RIGHT LEG]
      case L'\u0274': // ɴ  [LATIN LETTER SMALL CAPITAL N]
      case L'\u1D0E': // ᴎ  [LATIN LETTER SMALL CAPITAL REVERSED N]
      case L'\u1E44': // Ṅ  [LATIN CAPITAL LETTER N WITH DOT ABOVE]
      case L'\u1E46': // Ṇ  [LATIN CAPITAL LETTER N WITH DOT BELOW]
      case L'\u1E48': // Ṉ  [LATIN CAPITAL LETTER N WITH LINE BELOW]
      case L'\u1E4A': // Ṋ  [LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW]
      case L'\u24C3': // Ⓝ  [CIRCLED LATIN CAPITAL LETTER N]
      case L'\uFF2E': // Ｎ  [FULLWIDTH LATIN CAPITAL LETTER N]
        output[outputPos++] = L'N';
        break;
      case L'\u00F1': // ñ  [LATIN SMALL LETTER N WITH TILDE]
      case L'\u0144': // ń  [LATIN SMALL LETTER N WITH ACUTE]
      case L'\u0146': // ņ  [LATIN SMALL LETTER N WITH CEDILLA]
      case L'\u0148': // ň  [LATIN SMALL LETTER N WITH CARON]
      case L'\u0149': // ŉ  [LATIN SMALL LETTER N PRECEDED BY APOSTROPHE]
      case L'\u014B': // ŋ  http://en.wikipedia.org/wiki/Eng_(letter)  [LATIN
                      // SMALL LETTER ENG]
      case L'\u019E': // ƞ  [LATIN SMALL LETTER N WITH LONG RIGHT LEG]
      case L'\u01F9': // ǹ  [LATIN SMALL LETTER N WITH GRAVE]
      case L'\u0235': // ȵ  [LATIN SMALL LETTER N WITH CURL]
      case L'\u0272': // ɲ  [LATIN SMALL LETTER N WITH LEFT HOOK]
      case L'\u0273': // ɳ  [LATIN SMALL LETTER N WITH RETROFLEX HOOK]
      case L'\u1D70': // ᵰ  [LATIN SMALL LETTER N WITH MIDDLE TILDE]
      case L'\u1D87': // ᶇ  [LATIN SMALL LETTER N WITH PALATAL HOOK]
      case L'\u1E45': // ṅ  [LATIN SMALL LETTER N WITH DOT ABOVE]
      case L'\u1E47': // ṇ  [LATIN SMALL LETTER N WITH DOT BELOW]
      case L'\u1E49': // ṉ  [LATIN SMALL LETTER N WITH LINE BELOW]
      case L'\u1E4B': // ṋ  [LATIN SMALL LETTER N WITH CIRCUMFLEX BELOW]
      case L'\u207F': // ⁿ  [SUPERSCRIPT LATIN SMALL LETTER N]
      case L'\u24DD': // ⓝ  [CIRCLED LATIN SMALL LETTER N]
      case L'\uFF4E': // ｎ  [FULLWIDTH LATIN SMALL LETTER N]
        output[outputPos++] = L'n';
        break;
      case L'\u01CA': // Ǌ  [LATIN CAPITAL LETTER NJ]
        output[outputPos++] = L'N';
        output[outputPos++] = L'J';
        break;
      case L'\u01CB': // ǋ  [LATIN CAPITAL LETTER N WITH SMALL LETTER J]
        output[outputPos++] = L'N';
        output[outputPos++] = L'j';
        break;
      case L'\u24A9': // ⒩  [PARENTHESIZED LATIN SMALL LETTER N]
        output[outputPos++] = L'(';
        output[outputPos++] = L'n';
        output[outputPos++] = L')';
        break;
      case L'\u01CC': // ǌ  [LATIN SMALL LETTER NJ]
        output[outputPos++] = L'n';
        output[outputPos++] = L'j';
        break;
      case L'\u00D2': // Ò  [LATIN CAPITAL LETTER O WITH GRAVE]
      case L'\u00D3': // Ó  [LATIN CAPITAL LETTER O WITH ACUTE]
      case L'\u00D4': // Ô  [LATIN CAPITAL LETTER O WITH CIRCUMFLEX]
      case L'\u00D5': // Õ  [LATIN CAPITAL LETTER O WITH TILDE]
      case L'\u00D6': // Ö  [LATIN CAPITAL LETTER O WITH DIAERESIS]
      case L'\u00D8': // Ø  [LATIN CAPITAL LETTER O WITH STROKE]
      case L'\u014C': // Ō  [LATIN CAPITAL LETTER O WITH MACRON]
      case L'\u014E': // Ŏ  [LATIN CAPITAL LETTER O WITH BREVE]
      case L'\u0150': // Ő  [LATIN CAPITAL LETTER O WITH DOUBLE ACUTE]
      case L'\u0186': // Ɔ  [LATIN CAPITAL LETTER OPEN O]
      case L'\u019F': // Ɵ  [LATIN CAPITAL LETTER O WITH MIDDLE TILDE]
      case L'\u01A0': // Ơ  [LATIN CAPITAL LETTER O WITH HORN]
      case L'\u01D1': // Ǒ  [LATIN CAPITAL LETTER O WITH CARON]
      case L'\u01EA': // Ǫ  [LATIN CAPITAL LETTER O WITH OGONEK]
      case L'\u01EC': // Ǭ  [LATIN CAPITAL LETTER O WITH OGONEK AND MACRON]
      case L'\u01FE': // Ǿ  [LATIN CAPITAL LETTER O WITH STROKE AND ACUTE]
      case L'\u020C': // Ȍ  [LATIN CAPITAL LETTER O WITH DOUBLE GRAVE]
      case L'\u020E': // Ȏ  [LATIN CAPITAL LETTER O WITH INVERTED BREVE]
      case L'\u022A': // Ȫ  [LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON]
      case L'\u022C': // Ȭ  [LATIN CAPITAL LETTER O WITH TILDE AND MACRON]
      case L'\u022E': // Ȯ  [LATIN CAPITAL LETTER O WITH DOT ABOVE]
      case L'\u0230': // Ȱ  [LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON]
      case L'\u1D0F': // ᴏ  [LATIN LETTER SMALL CAPITAL O]
      case L'\u1D10': // ᴐ  [LATIN LETTER SMALL CAPITAL OPEN O]
      case L'\u1E4C': // Ṍ  [LATIN CAPITAL LETTER O WITH TILDE AND ACUTE]
      case L'\u1E4E': // Ṏ  [LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS]
      case L'\u1E50': // Ṑ  [LATIN CAPITAL LETTER O WITH MACRON AND GRAVE]
      case L'\u1E52': // Ṓ  [LATIN CAPITAL LETTER O WITH MACRON AND ACUTE]
      case L'\u1ECC': // Ọ  [LATIN CAPITAL LETTER O WITH DOT BELOW]
      case L'\u1ECE': // Ỏ  [LATIN CAPITAL LETTER O WITH HOOK ABOVE]
      case L'\u1ED0': // Ố  [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE]
      case L'\u1ED2': // Ồ  [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE]
      case L'\u1ED4': // Ổ  [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK
                      // ABOVE]
      case L'\u1ED6': // Ỗ  [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE]
      case L'\u1ED8': // Ộ  [LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT
                      // BELOW]
      case L'\u1EDA': // Ớ  [LATIN CAPITAL LETTER O WITH HORN AND ACUTE]
      case L'\u1EDC': // Ờ  [LATIN CAPITAL LETTER O WITH HORN AND GRAVE]
      case L'\u1EDE': // Ở  [LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE]
      case L'\u1EE0': // Ỡ  [LATIN CAPITAL LETTER O WITH HORN AND TILDE]
      case L'\u1EE2': // Ợ  [LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW]
      case L'\u24C4': // Ⓞ  [CIRCLED LATIN CAPITAL LETTER O]
      case L'\uA74A': // Ꝋ  [LATIN CAPITAL LETTER O WITH LONG STROKE OVERLAY]
      case L'\uA74C': // Ꝍ  [LATIN CAPITAL LETTER O WITH LOOP]
      case L'\uFF2F': // Ｏ  [FULLWIDTH LATIN CAPITAL LETTER O]
        output[outputPos++] = L'O';
        break;
      case L'\u00F2': // ò  [LATIN SMALL LETTER O WITH GRAVE]
      case L'\u00F3': // ó  [LATIN SMALL LETTER O WITH ACUTE]
      case L'\u00F4': // ô  [LATIN SMALL LETTER O WITH CIRCUMFLEX]
      case L'\u00F5': // õ  [LATIN SMALL LETTER O WITH TILDE]
      case L'\u00F6': // ö  [LATIN SMALL LETTER O WITH DIAERESIS]
      case L'\u00F8': // ø  [LATIN SMALL LETTER O WITH STROKE]
      case L'\u014D': // ō  [LATIN SMALL LETTER O WITH MACRON]
      case L'\u014F': // ŏ  [LATIN SMALL LETTER O WITH BREVE]
      case L'\u0151': // ő  [LATIN SMALL LETTER O WITH DOUBLE ACUTE]
      case L'\u01A1': // ơ  [LATIN SMALL LETTER O WITH HORN]
      case L'\u01D2': // ǒ  [LATIN SMALL LETTER O WITH CARON]
      case L'\u01EB': // ǫ  [LATIN SMALL LETTER O WITH OGONEK]
      case L'\u01ED': // ǭ  [LATIN SMALL LETTER O WITH OGONEK AND MACRON]
      case L'\u01FF': // ǿ  [LATIN SMALL LETTER O WITH STROKE AND ACUTE]
      case L'\u020D': // ȍ  [LATIN SMALL LETTER O WITH DOUBLE GRAVE]
      case L'\u020F': // ȏ  [LATIN SMALL LETTER O WITH INVERTED BREVE]
      case L'\u022B': // ȫ  [LATIN SMALL LETTER O WITH DIAERESIS AND MACRON]
      case L'\u022D': // ȭ  [LATIN SMALL LETTER O WITH TILDE AND MACRON]
      case L'\u022F': // ȯ  [LATIN SMALL LETTER O WITH DOT ABOVE]
      case L'\u0231': // ȱ  [LATIN SMALL LETTER O WITH DOT ABOVE AND MACRON]
      case L'\u0254': // ɔ  [LATIN SMALL LETTER OPEN O]
      case L'\u0275': // ɵ  [LATIN SMALL LETTER BARRED O]
      case L'\u1D16': // ᴖ  [LATIN SMALL LETTER TOP HALF O]
      case L'\u1D17': // ᴗ  [LATIN SMALL LETTER BOTTOM HALF O]
      case L'\u1D97': // ᶗ  [LATIN SMALL LETTER OPEN O WITH RETROFLEX HOOK]
      case L'\u1E4D': // ṍ  [LATIN SMALL LETTER O WITH TILDE AND ACUTE]
      case L'\u1E4F': // ṏ  [LATIN SMALL LETTER O WITH TILDE AND DIAERESIS]
      case L'\u1E51': // ṑ  [LATIN SMALL LETTER O WITH MACRON AND GRAVE]
      case L'\u1E53': // ṓ  [LATIN SMALL LETTER O WITH MACRON AND ACUTE]
      case L'\u1ECD': // ọ  [LATIN SMALL LETTER O WITH DOT BELOW]
      case L'\u1ECF': // ỏ  [LATIN SMALL LETTER O WITH HOOK ABOVE]
      case L'\u1ED1': // ố  [LATIN SMALL LETTER O WITH CIRCUMFLEX AND ACUTE]
      case L'\u1ED3': // ồ  [LATIN SMALL LETTER O WITH CIRCUMFLEX AND GRAVE]
      case L'\u1ED5': // ổ  [LATIN SMALL LETTER O WITH CIRCUMFLEX AND HOOK
                      // ABOVE]
      case L'\u1ED7': // ỗ  [LATIN SMALL LETTER O WITH CIRCUMFLEX AND TILDE]
      case L'\u1ED9': // ộ  [LATIN SMALL LETTER O WITH CIRCUMFLEX AND DOT BELOW]
      case L'\u1EDB': // ớ  [LATIN SMALL LETTER O WITH HORN AND ACUTE]
      case L'\u1EDD': // ờ  [LATIN SMALL LETTER O WITH HORN AND GRAVE]
      case L'\u1EDF': // ở  [LATIN SMALL LETTER O WITH HORN AND HOOK ABOVE]
      case L'\u1EE1': // ỡ  [LATIN SMALL LETTER O WITH HORN AND TILDE]
      case L'\u1EE3': // ợ  [LATIN SMALL LETTER O WITH HORN AND DOT BELOW]
      case L'\u2092': // ₒ  [LATIN SUBSCRIPT SMALL LETTER O]
      case L'\u24DE': // ⓞ  [CIRCLED LATIN SMALL LETTER O]
      case L'\u2C7A': // ⱺ  [LATIN SMALL LETTER O WITH LOW RING INSIDE]
      case L'\uA74B': // ꝋ  [LATIN SMALL LETTER O WITH LONG STROKE OVERLAY]
      case L'\uA74D': // ꝍ  [LATIN SMALL LETTER O WITH LOOP]
      case L'\uFF4F': // ｏ  [FULLWIDTH LATIN SMALL LETTER O]
        output[outputPos++] = L'o';
        break;
      case L'\u0152': // Œ  [LATIN CAPITAL LIGATURE OE]
      case L'\u0276': // ɶ  [LATIN LETTER SMALL CAPITAL OE]
        output[outputPos++] = L'O';
        output[outputPos++] = L'E';
        break;
      case L'\uA74E': // Ꝏ  [LATIN CAPITAL LETTER OO]
        output[outputPos++] = L'O';
        output[outputPos++] = L'O';
        break;
      case L'\u0222': // Ȣ  http://en.wikipedia.org/wiki/OU  [LATIN CAPITAL
                      // LETTER OU]
      case L'\u1D15': // ᴕ  [LATIN LETTER SMALL CAPITAL OU]
        output[outputPos++] = L'O';
        output[outputPos++] = L'U';
        break;
      case L'\u24AA': // ⒪  [PARENTHESIZED LATIN SMALL LETTER O]
        output[outputPos++] = L'(';
        output[outputPos++] = L'o';
        output[outputPos++] = L')';
        break;
      case L'\u0153': // œ  [LATIN SMALL LIGATURE OE]
      case L'\u1D14': // ᴔ  [LATIN SMALL LETTER TURNED OE]
        output[outputPos++] = L'o';
        output[outputPos++] = L'e';
        break;
      case L'\uA74F': // ꝏ  [LATIN SMALL LETTER OO]
        output[outputPos++] = L'o';
        output[outputPos++] = L'o';
        break;
      case L'\u0223': // ȣ  http://en.wikipedia.org/wiki/OU  [LATIN SMALL LETTER
                      // OU]
        output[outputPos++] = L'o';
        output[outputPos++] = L'u';
        break;
      case L'\u01A4': // Ƥ  [LATIN CAPITAL LETTER P WITH HOOK]
      case L'\u1D18': // ᴘ  [LATIN LETTER SMALL CAPITAL P]
      case L'\u1E54': // Ṕ  [LATIN CAPITAL LETTER P WITH ACUTE]
      case L'\u1E56': // Ṗ  [LATIN CAPITAL LETTER P WITH DOT ABOVE]
      case L'\u24C5': // Ⓟ  [CIRCLED LATIN CAPITAL LETTER P]
      case L'\u2C63': // Ᵽ  [LATIN CAPITAL LETTER P WITH STROKE]
      case L'\uA750': // Ꝑ  [LATIN CAPITAL LETTER P WITH STROKE THROUGH
                      // DESCENDER]
      case L'\uA752': // Ꝓ  [LATIN CAPITAL LETTER P WITH FLOURISH]
      case L'\uA754': // Ꝕ  [LATIN CAPITAL LETTER P WITH SQUIRREL TAIL]
      case L'\uFF30': // Ｐ  [FULLWIDTH LATIN CAPITAL LETTER P]
        output[outputPos++] = L'P';
        break;
      case L'\u01A5': // ƥ  [LATIN SMALL LETTER P WITH HOOK]
      case L'\u1D71': // ᵱ  [LATIN SMALL LETTER P WITH MIDDLE TILDE]
      case L'\u1D7D': // ᵽ  [LATIN SMALL LETTER P WITH STROKE]
      case L'\u1D88': // ᶈ  [LATIN SMALL LETTER P WITH PALATAL HOOK]
      case L'\u1E55': // ṕ  [LATIN SMALL LETTER P WITH ACUTE]
      case L'\u1E57': // ṗ  [LATIN SMALL LETTER P WITH DOT ABOVE]
      case L'\u24DF': // ⓟ  [CIRCLED LATIN SMALL LETTER P]
      case L'\uA751': // ꝑ  [LATIN SMALL LETTER P WITH STROKE THROUGH DESCENDER]
      case L'\uA753': // ꝓ  [LATIN SMALL LETTER P WITH FLOURISH]
      case L'\uA755': // ꝕ  [LATIN SMALL LETTER P WITH SQUIRREL TAIL]
      case L'\uA7FC': // ꟼ  [LATIN EPIGRAPHIC LETTER REVERSED P]
      case L'\uFF50': // ｐ  [FULLWIDTH LATIN SMALL LETTER P]
        output[outputPos++] = L'p';
        break;
      case L'\u24AB': // ⒫  [PARENTHESIZED LATIN SMALL LETTER P]
        output[outputPos++] = L'(';
        output[outputPos++] = L'p';
        output[outputPos++] = L')';
        break;
      case L'\u024A': // Ɋ  [LATIN CAPITAL LETTER SMALL Q WITH HOOK TAIL]
      case L'\u24C6': // Ⓠ  [CIRCLED LATIN CAPITAL LETTER Q]
      case L'\uA756': // Ꝗ  [LATIN CAPITAL LETTER Q WITH STROKE THROUGH
                      // DESCENDER]
      case L'\uA758': // Ꝙ  [LATIN CAPITAL LETTER Q WITH DIAGONAL STROKE]
      case L'\uFF31': // Ｑ  [FULLWIDTH LATIN CAPITAL LETTER Q]
        output[outputPos++] = L'Q';
        break;
      case L'\u0138': // ĸ  http://en.wikipedia.org/wiki/Kra_(letter)  [LATIN
                      // SMALL LETTER KRA]
      case L'\u024B': // ɋ  [LATIN SMALL LETTER Q WITH HOOK TAIL]
      case L'\u02A0': // ʠ  [LATIN SMALL LETTER Q WITH HOOK]
      case L'\u24E0': // ⓠ  [CIRCLED LATIN SMALL LETTER Q]
      case L'\uA757': // ꝗ  [LATIN SMALL LETTER Q WITH STROKE THROUGH DESCENDER]
      case L'\uA759': // ꝙ  [LATIN SMALL LETTER Q WITH DIAGONAL STROKE]
      case L'\uFF51': // ｑ  [FULLWIDTH LATIN SMALL LETTER Q]
        output[outputPos++] = L'q';
        break;
      case L'\u24AC': // ⒬  [PARENTHESIZED LATIN SMALL LETTER Q]
        output[outputPos++] = L'(';
        output[outputPos++] = L'q';
        output[outputPos++] = L')';
        break;
      case L'\u0239': // ȹ  [LATIN SMALL LETTER QP DIGRAPH]
        output[outputPos++] = L'q';
        output[outputPos++] = L'p';
        break;
      case L'\u0154': // Ŕ  [LATIN CAPITAL LETTER R WITH ACUTE]
      case L'\u0156': // Ŗ  [LATIN CAPITAL LETTER R WITH CEDILLA]
      case L'\u0158': // Ř  [LATIN CAPITAL LETTER R WITH CARON]
      case L'\u0210': // Ȓ  [LATIN CAPITAL LETTER R WITH DOUBLE GRAVE]
      case L'\u0212': // Ȓ  [LATIN CAPITAL LETTER R WITH INVERTED BREVE]
      case L'\u024C': // Ɍ  [LATIN CAPITAL LETTER R WITH STROKE]
      case L'\u0280': // ʀ  [LATIN LETTER SMALL CAPITAL R]
      case L'\u0281': // ʁ  [LATIN LETTER SMALL CAPITAL INVERTED R]
      case L'\u1D19': // ᴙ  [LATIN LETTER SMALL CAPITAL REVERSED R]
      case L'\u1D1A': // ᴚ  [LATIN LETTER SMALL CAPITAL TURNED R]
      case L'\u1E58': // Ṙ  [LATIN CAPITAL LETTER R WITH DOT ABOVE]
      case L'\u1E5A': // Ṛ  [LATIN CAPITAL LETTER R WITH DOT BELOW]
      case L'\u1E5C': // Ṝ  [LATIN CAPITAL LETTER R WITH DOT BELOW AND MACRON]
      case L'\u1E5E': // Ṟ  [LATIN CAPITAL LETTER R WITH LINE BELOW]
      case L'\u24C7': // Ⓡ  [CIRCLED LATIN CAPITAL LETTER R]
      case L'\u2C64': // Ɽ  [LATIN CAPITAL LETTER R WITH TAIL]
      case L'\uA75A': // Ꝛ  [LATIN CAPITAL LETTER R ROTUNDA]
      case L'\uA782': // Ꞃ  [LATIN CAPITAL LETTER INSULAR R]
      case L'\uFF32': // Ｒ  [FULLWIDTH LATIN CAPITAL LETTER R]
        output[outputPos++] = L'R';
        break;
      case L'\u0155': // ŕ  [LATIN SMALL LETTER R WITH ACUTE]
      case L'\u0157': // ŗ  [LATIN SMALL LETTER R WITH CEDILLA]
      case L'\u0159': // ř  [LATIN SMALL LETTER R WITH CARON]
      case L'\u0211': // ȑ  [LATIN SMALL LETTER R WITH DOUBLE GRAVE]
      case L'\u0213': // ȓ  [LATIN SMALL LETTER R WITH INVERTED BREVE]
      case L'\u024D': // ɍ  [LATIN SMALL LETTER R WITH STROKE]
      case L'\u027C': // ɼ  [LATIN SMALL LETTER R WITH LONG LEG]
      case L'\u027D': // ɽ  [LATIN SMALL LETTER R WITH TAIL]
      case L'\u027E': // ɾ  [LATIN SMALL LETTER R WITH FISHHOOK]
      case L'\u027F': // ɿ  [LATIN SMALL LETTER REVERSED R WITH FISHHOOK]
      case L'\u1D63': // ᵣ  [LATIN SUBSCRIPT SMALL LETTER R]
      case L'\u1D72': // ᵲ  [LATIN SMALL LETTER R WITH MIDDLE TILDE]
      case L'\u1D73': // ᵳ  [LATIN SMALL LETTER R WITH FISHHOOK AND MIDDLE
                      // TILDE]
      case L'\u1D89': // ᶉ  [LATIN SMALL LETTER R WITH PALATAL HOOK]
      case L'\u1E59': // ṙ  [LATIN SMALL LETTER R WITH DOT ABOVE]
      case L'\u1E5B': // ṛ  [LATIN SMALL LETTER R WITH DOT BELOW]
      case L'\u1E5D': // ṝ  [LATIN SMALL LETTER R WITH DOT BELOW AND MACRON]
      case L'\u1E5F': // ṟ  [LATIN SMALL LETTER R WITH LINE BELOW]
      case L'\u24E1': // ⓡ  [CIRCLED LATIN SMALL LETTER R]
      case L'\uA75B': // ꝛ  [LATIN SMALL LETTER R ROTUNDA]
      case L'\uA783': // ꞃ  [LATIN SMALL LETTER INSULAR R]
      case L'\uFF52': // ｒ  [FULLWIDTH LATIN SMALL LETTER R]
        output[outputPos++] = L'r';
        break;
      case L'\u24AD': // ⒭  [PARENTHESIZED LATIN SMALL LETTER R]
        output[outputPos++] = L'(';
        output[outputPos++] = L'r';
        output[outputPos++] = L')';
        break;
      case L'\u015A': // Ś  [LATIN CAPITAL LETTER S WITH ACUTE]
      case L'\u015C': // Ŝ  [LATIN CAPITAL LETTER S WITH CIRCUMFLEX]
      case L'\u015E': // Ş  [LATIN CAPITAL LETTER S WITH CEDILLA]
      case L'\u0160': // Š  [LATIN CAPITAL LETTER S WITH CARON]
      case L'\u0218': // Ș  [LATIN CAPITAL LETTER S WITH COMMA BELOW]
      case L'\u1E60': // Ṡ  [LATIN CAPITAL LETTER S WITH DOT ABOVE]
      case L'\u1E62': // Ṣ  [LATIN CAPITAL LETTER S WITH DOT BELOW]
      case L'\u1E64': // Ṥ  [LATIN CAPITAL LETTER S WITH ACUTE AND DOT ABOVE]
      case L'\u1E66': // Ṧ  [LATIN CAPITAL LETTER S WITH CARON AND DOT ABOVE]
      case L'\u1E68': // Ṩ  [LATIN CAPITAL LETTER S WITH DOT BELOW AND DOT
                      // ABOVE]
      case L'\u24C8': // Ⓢ  [CIRCLED LATIN CAPITAL LETTER S]
      case L'\uA731': // ꜱ  [LATIN LETTER SMALL CAPITAL S]
      case L'\uA785': // ꞅ  [LATIN SMALL LETTER INSULAR S]
      case L'\uFF33': // Ｓ  [FULLWIDTH LATIN CAPITAL LETTER S]
        output[outputPos++] = L'S';
        break;
      case L'\u015B': // ś  [LATIN SMALL LETTER S WITH ACUTE]
      case L'\u015D': // ŝ  [LATIN SMALL LETTER S WITH CIRCUMFLEX]
      case L'\u015F': // ş  [LATIN SMALL LETTER S WITH CEDILLA]
      case L'\u0161': // š  [LATIN SMALL LETTER S WITH CARON]
      case L'\u017F': // ſ  http://en.wikipedia.org/wiki/Long_S  [LATIN SMALL
                      // LETTER LONG S]
      case L'\u0219': // ș  [LATIN SMALL LETTER S WITH COMMA BELOW]
      case L'\u023F': // ȿ  [LATIN SMALL LETTER S WITH SWASH TAIL]
      case L'\u0282': // ʂ  [LATIN SMALL LETTER S WITH HOOK]
      case L'\u1D74': // ᵴ  [LATIN SMALL LETTER S WITH MIDDLE TILDE]
      case L'\u1D8A': // ᶊ  [LATIN SMALL LETTER S WITH PALATAL HOOK]
      case L'\u1E61': // ṡ  [LATIN SMALL LETTER S WITH DOT ABOVE]
      case L'\u1E63': // ṣ  [LATIN SMALL LETTER S WITH DOT BELOW]
      case L'\u1E65': // ṥ  [LATIN SMALL LETTER S WITH ACUTE AND DOT ABOVE]
      case L'\u1E67': // ṧ  [LATIN SMALL LETTER S WITH CARON AND DOT ABOVE]
      case L'\u1E69': // ṩ  [LATIN SMALL LETTER S WITH DOT BELOW AND DOT ABOVE]
      case L'\u1E9C': // ẜ  [LATIN SMALL LETTER LONG S WITH DIAGONAL STROKE]
      case L'\u1E9D': // ẝ  [LATIN SMALL LETTER LONG S WITH HIGH STROKE]
      case L'\u24E2': // ⓢ  [CIRCLED LATIN SMALL LETTER S]
      case L'\uA784': // Ꞅ  [LATIN CAPITAL LETTER INSULAR S]
      case L'\uFF53': // ｓ  [FULLWIDTH LATIN SMALL LETTER S]
        output[outputPos++] = L's';
        break;
      case L'\u1E9E': // ẞ  [LATIN CAPITAL LETTER SHARP S]
        output[outputPos++] = L'S';
        output[outputPos++] = L'S';
        break;
      case L'\u24AE': // ⒮  [PARENTHESIZED LATIN SMALL LETTER S]
        output[outputPos++] = L'(';
        output[outputPos++] = L's';
        output[outputPos++] = L')';
        break;
      case L'\u00DF': // ß  [LATIN SMALL LETTER SHARP S]
        output[outputPos++] = L's';
        output[outputPos++] = L's';
        break;
      case L'\uFB06': // ﬆ  [LATIN SMALL LIGATURE ST]
        output[outputPos++] = L's';
        output[outputPos++] = L't';
        break;
      case L'\u0162': // Ţ  [LATIN CAPITAL LETTER T WITH CEDILLA]
      case L'\u0164': // Ť  [LATIN CAPITAL LETTER T WITH CARON]
      case L'\u0166': // Ŧ  [LATIN CAPITAL LETTER T WITH STROKE]
      case L'\u01AC': // Ƭ  [LATIN CAPITAL LETTER T WITH HOOK]
      case L'\u01AE': // Ʈ  [LATIN CAPITAL LETTER T WITH RETROFLEX HOOK]
      case L'\u021A': // Ț  [LATIN CAPITAL LETTER T WITH COMMA BELOW]
      case L'\u023E': // Ⱦ  [LATIN CAPITAL LETTER T WITH DIAGONAL STROKE]
      case L'\u1D1B': // ᴛ  [LATIN LETTER SMALL CAPITAL T]
      case L'\u1E6A': // Ṫ  [LATIN CAPITAL LETTER T WITH DOT ABOVE]
      case L'\u1E6C': // Ṭ  [LATIN CAPITAL LETTER T WITH DOT BELOW]
      case L'\u1E6E': // Ṯ  [LATIN CAPITAL LETTER T WITH LINE BELOW]
      case L'\u1E70': // Ṱ  [LATIN CAPITAL LETTER T WITH CIRCUMFLEX BELOW]
      case L'\u24C9': // Ⓣ  [CIRCLED LATIN CAPITAL LETTER T]
      case L'\uA786': // Ꞇ  [LATIN CAPITAL LETTER INSULAR T]
      case L'\uFF34': // Ｔ  [FULLWIDTH LATIN CAPITAL LETTER T]
        output[outputPos++] = L'T';
        break;
      case L'\u0163': // ţ  [LATIN SMALL LETTER T WITH CEDILLA]
      case L'\u0165': // ť  [LATIN SMALL LETTER T WITH CARON]
      case L'\u0167': // ŧ  [LATIN SMALL LETTER T WITH STROKE]
      case L'\u01AB': // ƫ  [LATIN SMALL LETTER T WITH PALATAL HOOK]
      case L'\u01AD': // ƭ  [LATIN SMALL LETTER T WITH HOOK]
      case L'\u021B': // ț  [LATIN SMALL LETTER T WITH COMMA BELOW]
      case L'\u0236': // ȶ  [LATIN SMALL LETTER T WITH CURL]
      case L'\u0287': // ʇ  [LATIN SMALL LETTER TURNED T]
      case L'\u0288': // ʈ  [LATIN SMALL LETTER T WITH RETROFLEX HOOK]
      case L'\u1D75': // ᵵ  [LATIN SMALL LETTER T WITH MIDDLE TILDE]
      case L'\u1E6B': // ṫ  [LATIN SMALL LETTER T WITH DOT ABOVE]
      case L'\u1E6D': // ṭ  [LATIN SMALL LETTER T WITH DOT BELOW]
      case L'\u1E6F': // ṯ  [LATIN SMALL LETTER T WITH LINE BELOW]
      case L'\u1E71': // ṱ  [LATIN SMALL LETTER T WITH CIRCUMFLEX BELOW]
      case L'\u1E97': // ẗ  [LATIN SMALL LETTER T WITH DIAERESIS]
      case L'\u24E3': // ⓣ  [CIRCLED LATIN SMALL LETTER T]
      case L'\u2C66': // ⱦ  [LATIN SMALL LETTER T WITH DIAGONAL STROKE]
      case L'\uFF54': // ｔ  [FULLWIDTH LATIN SMALL LETTER T]
        output[outputPos++] = L't';
        break;
      case L'\u00DE': // Þ  [LATIN CAPITAL LETTER THORN]
      case L'\uA766': // Ꝧ  [LATIN CAPITAL LETTER THORN WITH STROKE THROUGH
                      // DESCENDER]
        output[outputPos++] = L'T';
        output[outputPos++] = L'H';
        break;
      case L'\uA728': // Ꜩ  [LATIN CAPITAL LETTER TZ]
        output[outputPos++] = L'T';
        output[outputPos++] = L'Z';
        break;
      case L'\u24AF': // ⒯  [PARENTHESIZED LATIN SMALL LETTER T]
        output[outputPos++] = L'(';
        output[outputPos++] = L't';
        output[outputPos++] = L')';
        break;
      case L'\u02A8': // ʨ  [LATIN SMALL LETTER TC DIGRAPH WITH CURL]
        output[outputPos++] = L't';
        output[outputPos++] = L'c';
        break;
      case L'\u00FE': // þ  [LATIN SMALL LETTER THORN]
      case L'\u1D7A': // ᵺ  [LATIN SMALL LETTER TH WITH STRIKETHROUGH]
      case L'\uA767': // ꝧ  [LATIN SMALL LETTER THORN WITH STROKE THROUGH
                      // DESCENDER]
        output[outputPos++] = L't';
        output[outputPos++] = L'h';
        break;
      case L'\u02A6': // ʦ  [LATIN SMALL LETTER TS DIGRAPH]
        output[outputPos++] = L't';
        output[outputPos++] = L's';
        break;
      case L'\uA729': // ꜩ  [LATIN SMALL LETTER TZ]
        output[outputPos++] = L't';
        output[outputPos++] = L'z';
        break;
      case L'\u00D9': // Ù  [LATIN CAPITAL LETTER U WITH GRAVE]
      case L'\u00DA': // Ú  [LATIN CAPITAL LETTER U WITH ACUTE]
      case L'\u00DB': // Û  [LATIN CAPITAL LETTER U WITH CIRCUMFLEX]
      case L'\u00DC': // Ü  [LATIN CAPITAL LETTER U WITH DIAERESIS]
      case L'\u0168': // Ũ  [LATIN CAPITAL LETTER U WITH TILDE]
      case L'\u016A': // Ū  [LATIN CAPITAL LETTER U WITH MACRON]
      case L'\u016C': // Ŭ  [LATIN CAPITAL LETTER U WITH BREVE]
      case L'\u016E': // Ů  [LATIN CAPITAL LETTER U WITH RING ABOVE]
      case L'\u0170': // Ű  [LATIN CAPITAL LETTER U WITH DOUBLE ACUTE]
      case L'\u0172': // Ų  [LATIN CAPITAL LETTER U WITH OGONEK]
      case L'\u01AF': // Ư  [LATIN CAPITAL LETTER U WITH HORN]
      case L'\u01D3': // Ǔ  [LATIN CAPITAL LETTER U WITH CARON]
      case L'\u01D5': // Ǖ  [LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON]
      case L'\u01D7': // Ǘ  [LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE]
      case L'\u01D9': // Ǚ  [LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON]
      case L'\u01DB': // Ǜ  [LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE]
      case L'\u0214': // Ȕ  [LATIN CAPITAL LETTER U WITH DOUBLE GRAVE]
      case L'\u0216': // Ȗ  [LATIN CAPITAL LETTER U WITH INVERTED BREVE]
      case L'\u0244': // Ʉ  [LATIN CAPITAL LETTER U BAR]
      case L'\u1D1C': // ᴜ  [LATIN LETTER SMALL CAPITAL U]
      case L'\u1D7E': // ᵾ  [LATIN SMALL CAPITAL LETTER U WITH STROKE]
      case L'\u1E72': // Ṳ  [LATIN CAPITAL LETTER U WITH DIAERESIS BELOW]
      case L'\u1E74': // Ṵ  [LATIN CAPITAL LETTER U WITH TILDE BELOW]
      case L'\u1E76': // Ṷ  [LATIN CAPITAL LETTER U WITH CIRCUMFLEX BELOW]
      case L'\u1E78': // Ṹ  [LATIN CAPITAL LETTER U WITH TILDE AND ACUTE]
      case L'\u1E7A': // Ṻ  [LATIN CAPITAL LETTER U WITH MACRON AND DIAERESIS]
      case L'\u1EE4': // Ụ  [LATIN CAPITAL LETTER U WITH DOT BELOW]
      case L'\u1EE6': // Ủ  [LATIN CAPITAL LETTER U WITH HOOK ABOVE]
      case L'\u1EE8': // Ứ  [LATIN CAPITAL LETTER U WITH HORN AND ACUTE]
      case L'\u1EEA': // Ừ  [LATIN CAPITAL LETTER U WITH HORN AND GRAVE]
      case L'\u1EEC': // Ử  [LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE]
      case L'\u1EEE': // Ữ  [LATIN CAPITAL LETTER U WITH HORN AND TILDE]
      case L'\u1EF0': // Ự  [LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW]
      case L'\u24CA': // Ⓤ  [CIRCLED LATIN CAPITAL LETTER U]
      case L'\uFF35': // Ｕ  [FULLWIDTH LATIN CAPITAL LETTER U]
        output[outputPos++] = L'U';
        break;
      case L'\u00F9': // ù  [LATIN SMALL LETTER U WITH GRAVE]
      case L'\u00FA': // ú  [LATIN SMALL LETTER U WITH ACUTE]
      case L'\u00FB': // û  [LATIN SMALL LETTER U WITH CIRCUMFLEX]
      case L'\u00FC': // ü  [LATIN SMALL LETTER U WITH DIAERESIS]
      case L'\u0169': // ũ  [LATIN SMALL LETTER U WITH TILDE]
      case L'\u016B': // ū  [LATIN SMALL LETTER U WITH MACRON]
      case L'\u016D': // ŭ  [LATIN SMALL LETTER U WITH BREVE]
      case L'\u016F': // ů  [LATIN SMALL LETTER U WITH RING ABOVE]
      case L'\u0171': // ű  [LATIN SMALL LETTER U WITH DOUBLE ACUTE]
      case L'\u0173': // ų  [LATIN SMALL LETTER U WITH OGONEK]
      case L'\u01B0': // ư  [LATIN SMALL LETTER U WITH HORN]
      case L'\u01D4': // ǔ  [LATIN SMALL LETTER U WITH CARON]
      case L'\u01D6': // ǖ  [LATIN SMALL LETTER U WITH DIAERESIS AND MACRON]
      case L'\u01D8': // ǘ  [LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE]
      case L'\u01DA': // ǚ  [LATIN SMALL LETTER U WITH DIAERESIS AND CARON]
      case L'\u01DC': // ǜ  [LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE]
      case L'\u0215': // ȕ  [LATIN SMALL LETTER U WITH DOUBLE GRAVE]
      case L'\u0217': // ȗ  [LATIN SMALL LETTER U WITH INVERTED BREVE]
      case L'\u0289': // ʉ  [LATIN SMALL LETTER U BAR]
      case L'\u1D64': // ᵤ  [LATIN SUBSCRIPT SMALL LETTER U]
      case L'\u1D99': // ᶙ  [LATIN SMALL LETTER U WITH RETROFLEX HOOK]
      case L'\u1E73': // ṳ  [LATIN SMALL LETTER U WITH DIAERESIS BELOW]
      case L'\u1E75': // ṵ  [LATIN SMALL LETTER U WITH TILDE BELOW]
      case L'\u1E77': // ṷ  [LATIN SMALL LETTER U WITH CIRCUMFLEX BELOW]
      case L'\u1E79': // ṹ  [LATIN SMALL LETTER U WITH TILDE AND ACUTE]
      case L'\u1E7B': // ṻ  [LATIN SMALL LETTER U WITH MACRON AND DIAERESIS]
      case L'\u1EE5': // ụ  [LATIN SMALL LETTER U WITH DOT BELOW]
      case L'\u1EE7': // ủ  [LATIN SMALL LETTER U WITH HOOK ABOVE]
      case L'\u1EE9': // ứ  [LATIN SMALL LETTER U WITH HORN AND ACUTE]
      case L'\u1EEB': // ừ  [LATIN SMALL LETTER U WITH HORN AND GRAVE]
      case L'\u1EED': // ử  [LATIN SMALL LETTER U WITH HORN AND HOOK ABOVE]
      case L'\u1EEF': // ữ  [LATIN SMALL LETTER U WITH HORN AND TILDE]
      case L'\u1EF1': // ự  [LATIN SMALL LETTER U WITH HORN AND DOT BELOW]
      case L'\u24E4': // ⓤ  [CIRCLED LATIN SMALL LETTER U]
      case L'\uFF55': // ｕ  [FULLWIDTH LATIN SMALL LETTER U]
        output[outputPos++] = L'u';
        break;
      case L'\u24B0': // ⒰  [PARENTHESIZED LATIN SMALL LETTER U]
        output[outputPos++] = L'(';
        output[outputPos++] = L'u';
        output[outputPos++] = L')';
        break;
      case L'\u1D6B': // ᵫ  [LATIN SMALL LETTER UE]
        output[outputPos++] = L'u';
        output[outputPos++] = L'e';
        break;
      case L'\u01B2': // Ʋ  [LATIN CAPITAL LETTER V WITH HOOK]
      case L'\u0245': // Ʌ  [LATIN CAPITAL LETTER TURNED V]
      case L'\u1D20': // ᴠ  [LATIN LETTER SMALL CAPITAL V]
      case L'\u1E7C': // Ṽ  [LATIN CAPITAL LETTER V WITH TILDE]
      case L'\u1E7E': // Ṿ  [LATIN CAPITAL LETTER V WITH DOT BELOW]
      case L'\u1EFC': // Ỽ  [LATIN CAPITAL LETTER MIDDLE-WELSH V]
      case L'\u24CB': // Ⓥ  [CIRCLED LATIN CAPITAL LETTER V]
      case L'\uA75E': // Ꝟ  [LATIN CAPITAL LETTER V WITH DIAGONAL STROKE]
      case L'\uA768': // Ꝩ  [LATIN CAPITAL LETTER VEND]
      case L'\uFF36': // Ｖ  [FULLWIDTH LATIN CAPITAL LETTER V]
        output[outputPos++] = L'V';
        break;
      case L'\u028B': // ʋ  [LATIN SMALL LETTER V WITH HOOK]
      case L'\u028C': // ʌ  [LATIN SMALL LETTER TURNED V]
      case L'\u1D65': // ᵥ  [LATIN SUBSCRIPT SMALL LETTER V]
      case L'\u1D8C': // ᶌ  [LATIN SMALL LETTER V WITH PALATAL HOOK]
      case L'\u1E7D': // ṽ  [LATIN SMALL LETTER V WITH TILDE]
      case L'\u1E7F': // ṿ  [LATIN SMALL LETTER V WITH DOT BELOW]
      case L'\u24E5': // ⓥ  [CIRCLED LATIN SMALL LETTER V]
      case L'\u2C71': // ⱱ  [LATIN SMALL LETTER V WITH RIGHT HOOK]
      case L'\u2C74': // ⱴ  [LATIN SMALL LETTER V WITH CURL]
      case L'\uA75F': // ꝟ  [LATIN SMALL LETTER V WITH DIAGONAL STROKE]
      case L'\uFF56': // ｖ  [FULLWIDTH LATIN SMALL LETTER V]
        output[outputPos++] = L'v';
        break;
      case L'\uA760': // Ꝡ  [LATIN CAPITAL LETTER VY]
        output[outputPos++] = L'V';
        output[outputPos++] = L'Y';
        break;
      case L'\u24B1': // ⒱  [PARENTHESIZED LATIN SMALL LETTER V]
        output[outputPos++] = L'(';
        output[outputPos++] = L'v';
        output[outputPos++] = L')';
        break;
      case L'\uA761': // ꝡ  [LATIN SMALL LETTER VY]
        output[outputPos++] = L'v';
        output[outputPos++] = L'y';
        break;
      case L'\u0174': // Ŵ  [LATIN CAPITAL LETTER W WITH CIRCUMFLEX]
      case L'\u01F7': // Ƿ  http://en.wikipedia.org/wiki/Wynn  [LATIN CAPITAL
                      // LETTER WYNN]
      case L'\u1D21': // ᴡ  [LATIN LETTER SMALL CAPITAL W]
      case L'\u1E80': // Ẁ  [LATIN CAPITAL LETTER W WITH GRAVE]
      case L'\u1E82': // Ẃ  [LATIN CAPITAL LETTER W WITH ACUTE]
      case L'\u1E84': // Ẅ  [LATIN CAPITAL LETTER W WITH DIAERESIS]
      case L'\u1E86': // Ẇ  [LATIN CAPITAL LETTER W WITH DOT ABOVE]
      case L'\u1E88': // Ẉ  [LATIN CAPITAL LETTER W WITH DOT BELOW]
      case L'\u24CC': // Ⓦ  [CIRCLED LATIN CAPITAL LETTER W]
      case L'\u2C72': // Ⱳ  [LATIN CAPITAL LETTER W WITH HOOK]
      case L'\uFF37': // Ｗ  [FULLWIDTH LATIN CAPITAL LETTER W]
        output[outputPos++] = L'W';
        break;
      case L'\u0175': // ŵ  [LATIN SMALL LETTER W WITH CIRCUMFLEX]
      case L'\u01BF': // ƿ  http://en.wikipedia.org/wiki/Wynn  [LATIN LETTER
                      // WYNN]
      case L'\u028D': // ʍ  [LATIN SMALL LETTER TURNED W]
      case L'\u1E81': // ẁ  [LATIN SMALL LETTER W WITH GRAVE]
      case L'\u1E83': // ẃ  [LATIN SMALL LETTER W WITH ACUTE]
      case L'\u1E85': // ẅ  [LATIN SMALL LETTER W WITH DIAERESIS]
      case L'\u1E87': // ẇ  [LATIN SMALL LETTER W WITH DOT ABOVE]
      case L'\u1E89': // ẉ  [LATIN SMALL LETTER W WITH DOT BELOW]
      case L'\u1E98': // ẘ  [LATIN SMALL LETTER W WITH RING ABOVE]
      case L'\u24E6': // ⓦ  [CIRCLED LATIN SMALL LETTER W]
      case L'\u2C73': // ⱳ  [LATIN SMALL LETTER W WITH HOOK]
      case L'\uFF57': // ｗ  [FULLWIDTH LATIN SMALL LETTER W]
        output[outputPos++] = L'w';
        break;
      case L'\u24B2': // ⒲  [PARENTHESIZED LATIN SMALL LETTER W]
        output[outputPos++] = L'(';
        output[outputPos++] = L'w';
        output[outputPos++] = L')';
        break;
      case L'\u1E8A': // Ẋ  [LATIN CAPITAL LETTER X WITH DOT ABOVE]
      case L'\u1E8C': // Ẍ  [LATIN CAPITAL LETTER X WITH DIAERESIS]
      case L'\u24CD': // Ⓧ  [CIRCLED LATIN CAPITAL LETTER X]
      case L'\uFF38': // Ｘ  [FULLWIDTH LATIN CAPITAL LETTER X]
        output[outputPos++] = L'X';
        break;
      case L'\u1D8D': // ᶍ  [LATIN SMALL LETTER X WITH PALATAL HOOK]
      case L'\u1E8B': // ẋ  [LATIN SMALL LETTER X WITH DOT ABOVE]
      case L'\u1E8D': // ẍ  [LATIN SMALL LETTER X WITH DIAERESIS]
      case L'\u2093': // ₓ  [LATIN SUBSCRIPT SMALL LETTER X]
      case L'\u24E7': // ⓧ  [CIRCLED LATIN SMALL LETTER X]
      case L'\uFF58': // ｘ  [FULLWIDTH LATIN SMALL LETTER X]
        output[outputPos++] = L'x';
        break;
      case L'\u24B3': // ⒳  [PARENTHESIZED LATIN SMALL LETTER X]
        output[outputPos++] = L'(';
        output[outputPos++] = L'x';
        output[outputPos++] = L')';
        break;
      case L'\u00DD': // Ý  [LATIN CAPITAL LETTER Y WITH ACUTE]
      case L'\u0176': // Ŷ  [LATIN CAPITAL LETTER Y WITH CIRCUMFLEX]
      case L'\u0178': // Ÿ  [LATIN CAPITAL LETTER Y WITH DIAERESIS]
      case L'\u01B3': // Ƴ  [LATIN CAPITAL LETTER Y WITH HOOK]
      case L'\u0232': // Ȳ  [LATIN CAPITAL LETTER Y WITH MACRON]
      case L'\u024E': // Ɏ  [LATIN CAPITAL LETTER Y WITH STROKE]
      case L'\u028F': // ʏ  [LATIN LETTER SMALL CAPITAL Y]
      case L'\u1E8E': // Ẏ  [LATIN CAPITAL LETTER Y WITH DOT ABOVE]
      case L'\u1EF2': // Ỳ  [LATIN CAPITAL LETTER Y WITH GRAVE]
      case L'\u1EF4': // Ỵ  [LATIN CAPITAL LETTER Y WITH DOT BELOW]
      case L'\u1EF6': // Ỷ  [LATIN CAPITAL LETTER Y WITH HOOK ABOVE]
      case L'\u1EF8': // Ỹ  [LATIN CAPITAL LETTER Y WITH TILDE]
      case L'\u1EFE': // Ỿ  [LATIN CAPITAL LETTER Y WITH LOOP]
      case L'\u24CE': // Ⓨ  [CIRCLED LATIN CAPITAL LETTER Y]
      case L'\uFF39': // Ｙ  [FULLWIDTH LATIN CAPITAL LETTER Y]
        output[outputPos++] = L'Y';
        break;
      case L'\u00FD': // ý  [LATIN SMALL LETTER Y WITH ACUTE]
      case L'\u00FF': // ÿ  [LATIN SMALL LETTER Y WITH DIAERESIS]
      case L'\u0177': // ŷ  [LATIN SMALL LETTER Y WITH CIRCUMFLEX]
      case L'\u01B4': // ƴ  [LATIN SMALL LETTER Y WITH HOOK]
      case L'\u0233': // ȳ  [LATIN SMALL LETTER Y WITH MACRON]
      case L'\u024F': // ɏ  [LATIN SMALL LETTER Y WITH STROKE]
      case L'\u028E': // ʎ  [LATIN SMALL LETTER TURNED Y]
      case L'\u1E8F': // ẏ  [LATIN SMALL LETTER Y WITH DOT ABOVE]
      case L'\u1E99': // ẙ  [LATIN SMALL LETTER Y WITH RING ABOVE]
      case L'\u1EF3': // ỳ  [LATIN SMALL LETTER Y WITH GRAVE]
      case L'\u1EF5': // ỵ  [LATIN SMALL LETTER Y WITH DOT BELOW]
      case L'\u1EF7': // ỷ  [LATIN SMALL LETTER Y WITH HOOK ABOVE]
      case L'\u1EF9': // ỹ  [LATIN SMALL LETTER Y WITH TILDE]
      case L'\u1EFF': // ỿ  [LATIN SMALL LETTER Y WITH LOOP]
      case L'\u24E8': // ⓨ  [CIRCLED LATIN SMALL LETTER Y]
      case L'\uFF59': // ｙ  [FULLWIDTH LATIN SMALL LETTER Y]
        output[outputPos++] = L'y';
        break;
      case L'\u24B4': // ⒴  [PARENTHESIZED LATIN SMALL LETTER Y]
        output[outputPos++] = L'(';
        output[outputPos++] = L'y';
        output[outputPos++] = L')';
        break;
      case L'\u0179': // Ź  [LATIN CAPITAL LETTER Z WITH ACUTE]
      case L'\u017B': // Ż  [LATIN CAPITAL LETTER Z WITH DOT ABOVE]
      case L'\u017D': // Ž  [LATIN CAPITAL LETTER Z WITH CARON]
      case L'\u01B5': // Ƶ  [LATIN CAPITAL LETTER Z WITH STROKE]
      case L'\u021C': // Ȝ  http://en.wikipedia.org/wiki/Yogh  [LATIN CAPITAL
                      // LETTER YOGH]
      case L'\u0224': // Ȥ  [LATIN CAPITAL LETTER Z WITH HOOK]
      case L'\u1D22': // ᴢ  [LATIN LETTER SMALL CAPITAL Z]
      case L'\u1E90': // Ẑ  [LATIN CAPITAL LETTER Z WITH CIRCUMFLEX]
      case L'\u1E92': // Ẓ  [LATIN CAPITAL LETTER Z WITH DOT BELOW]
      case L'\u1E94': // Ẕ  [LATIN CAPITAL LETTER Z WITH LINE BELOW]
      case L'\u24CF': // Ⓩ  [CIRCLED LATIN CAPITAL LETTER Z]
      case L'\u2C6B': // Ⱬ  [LATIN CAPITAL LETTER Z WITH DESCENDER]
      case L'\uA762': // Ꝣ  [LATIN CAPITAL LETTER VISIGOTHIC Z]
      case L'\uFF3A': // Ｚ  [FULLWIDTH LATIN CAPITAL LETTER Z]
        output[outputPos++] = L'Z';
        break;
      case L'\u017A': // ź  [LATIN SMALL LETTER Z WITH ACUTE]
      case L'\u017C': // ż  [LATIN SMALL LETTER Z WITH DOT ABOVE]
      case L'\u017E': // ž  [LATIN SMALL LETTER Z WITH CARON]
      case L'\u01B6': // ƶ  [LATIN SMALL LETTER Z WITH STROKE]
      case L'\u021D': // ȝ  http://en.wikipedia.org/wiki/Yogh  [LATIN SMALL
                      // LETTER YOGH]
      case L'\u0225': // ȥ  [LATIN SMALL LETTER Z WITH HOOK]
      case L'\u0240': // ɀ  [LATIN SMALL LETTER Z WITH SWASH TAIL]
      case L'\u0290': // ʐ  [LATIN SMALL LETTER Z WITH RETROFLEX HOOK]
      case L'\u0291': // ʑ  [LATIN SMALL LETTER Z WITH CURL]
      case L'\u1D76': // ᵶ  [LATIN SMALL LETTER Z WITH MIDDLE TILDE]
      case L'\u1D8E': // ᶎ  [LATIN SMALL LETTER Z WITH PALATAL HOOK]
      case L'\u1E91': // ẑ  [LATIN SMALL LETTER Z WITH CIRCUMFLEX]
      case L'\u1E93': // ẓ  [LATIN SMALL LETTER Z WITH DOT BELOW]
      case L'\u1E95': // ẕ  [LATIN SMALL LETTER Z WITH LINE BELOW]
      case L'\u24E9': // ⓩ  [CIRCLED LATIN SMALL LETTER Z]
      case L'\u2C6C': // ⱬ  [LATIN SMALL LETTER Z WITH DESCENDER]
      case L'\uA763': // ꝣ  [LATIN SMALL LETTER VISIGOTHIC Z]
      case L'\uFF5A': // ｚ  [FULLWIDTH LATIN SMALL LETTER Z]
        output[outputPos++] = L'z';
        break;
      case L'\u24B5': // ⒵  [PARENTHESIZED LATIN SMALL LETTER Z]
        output[outputPos++] = L'(';
        output[outputPos++] = L'z';
        output[outputPos++] = L')';
        break;
      case L'\u2070': // ⁰  [SUPERSCRIPT ZERO]
      case L'\u2080': // ₀  [SUBSCRIPT ZERO]
      case L'\u24EA': // ⓪  [CIRCLED DIGIT ZERO]
      case L'\u24FF': // ⓿  [NEGATIVE CIRCLED DIGIT ZERO]
      case L'\uFF10': // ０  [FULLWIDTH DIGIT ZERO]
        output[outputPos++] = L'0';
        break;
      case L'\u00B9': // ¹  [SUPERSCRIPT ONE]
      case L'\u2081': // ₁  [SUBSCRIPT ONE]
      case L'\u2460': // ①  [CIRCLED DIGIT ONE]
      case L'\u24F5': // ⓵  [DOUBLE CIRCLED DIGIT ONE]
      case L'\u2776': // ❶  [DINGBAT NEGATIVE CIRCLED DIGIT ONE]
      case L'\u2780': // ➀  [DINGBAT CIRCLED SANS-SERIF DIGIT ONE]
      case L'\u278A': // ➊  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT ONE]
      case L'\uFF11': // １  [FULLWIDTH DIGIT ONE]
        output[outputPos++] = L'1';
        break;
      case L'\u2488': // ⒈  [DIGIT ONE FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'.';
        break;
      case L'\u2474': // ⑴  [PARENTHESIZED DIGIT ONE]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L')';
        break;
      case L'\u00B2': // ²  [SUPERSCRIPT TWO]
      case L'\u2082': // ₂  [SUBSCRIPT TWO]
      case L'\u2461': // ②  [CIRCLED DIGIT TWO]
      case L'\u24F6': // ⓶  [DOUBLE CIRCLED DIGIT TWO]
      case L'\u2777': // ❷  [DINGBAT NEGATIVE CIRCLED DIGIT TWO]
      case L'\u2781': // ➁  [DINGBAT CIRCLED SANS-SERIF DIGIT TWO]
      case L'\u278B': // ➋  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT TWO]
      case L'\uFF12': // ２  [FULLWIDTH DIGIT TWO]
        output[outputPos++] = L'2';
        break;
      case L'\u2489': // ⒉  [DIGIT TWO FULL STOP]
        output[outputPos++] = L'2';
        output[outputPos++] = L'.';
        break;
      case L'\u2475': // ⑵  [PARENTHESIZED DIGIT TWO]
        output[outputPos++] = L'(';
        output[outputPos++] = L'2';
        output[outputPos++] = L')';
        break;
      case L'\u00B3': // ³  [SUPERSCRIPT THREE]
      case L'\u2083': // ₃  [SUBSCRIPT THREE]
      case L'\u2462': // ③  [CIRCLED DIGIT THREE]
      case L'\u24F7': // ⓷  [DOUBLE CIRCLED DIGIT THREE]
      case L'\u2778': // ❸  [DINGBAT NEGATIVE CIRCLED DIGIT THREE]
      case L'\u2782': // ➂  [DINGBAT CIRCLED SANS-SERIF DIGIT THREE]
      case L'\u278C': // ➌  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT THREE]
      case L'\uFF13': // ３  [FULLWIDTH DIGIT THREE]
        output[outputPos++] = L'3';
        break;
      case L'\u248A': // ⒊  [DIGIT THREE FULL STOP]
        output[outputPos++] = L'3';
        output[outputPos++] = L'.';
        break;
      case L'\u2476': // ⑶  [PARENTHESIZED DIGIT THREE]
        output[outputPos++] = L'(';
        output[outputPos++] = L'3';
        output[outputPos++] = L')';
        break;
      case L'\u2074': // ⁴  [SUPERSCRIPT FOUR]
      case L'\u2084': // ₄  [SUBSCRIPT FOUR]
      case L'\u2463': // ④  [CIRCLED DIGIT FOUR]
      case L'\u24F8': // ⓸  [DOUBLE CIRCLED DIGIT FOUR]
      case L'\u2779': // ❹  [DINGBAT NEGATIVE CIRCLED DIGIT FOUR]
      case L'\u2783': // ➃  [DINGBAT CIRCLED SANS-SERIF DIGIT FOUR]
      case L'\u278D': // ➍  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FOUR]
      case L'\uFF14': // ４  [FULLWIDTH DIGIT FOUR]
        output[outputPos++] = L'4';
        break;
      case L'\u248B': // ⒋  [DIGIT FOUR FULL STOP]
        output[outputPos++] = L'4';
        output[outputPos++] = L'.';
        break;
      case L'\u2477': // ⑷  [PARENTHESIZED DIGIT FOUR]
        output[outputPos++] = L'(';
        output[outputPos++] = L'4';
        output[outputPos++] = L')';
        break;
      case L'\u2075': // ⁵  [SUPERSCRIPT FIVE]
      case L'\u2085': // ₅  [SUBSCRIPT FIVE]
      case L'\u2464': // ⑤  [CIRCLED DIGIT FIVE]
      case L'\u24F9': // ⓹  [DOUBLE CIRCLED DIGIT FIVE]
      case L'\u277A': // ❺  [DINGBAT NEGATIVE CIRCLED DIGIT FIVE]
      case L'\u2784': // ➄  [DINGBAT CIRCLED SANS-SERIF DIGIT FIVE]
      case L'\u278E': // ➎  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT FIVE]
      case L'\uFF15': // ５  [FULLWIDTH DIGIT FIVE]
        output[outputPos++] = L'5';
        break;
      case L'\u248C': // ⒌  [DIGIT FIVE FULL STOP]
        output[outputPos++] = L'5';
        output[outputPos++] = L'.';
        break;
      case L'\u2478': // ⑸  [PARENTHESIZED DIGIT FIVE]
        output[outputPos++] = L'(';
        output[outputPos++] = L'5';
        output[outputPos++] = L')';
        break;
      case L'\u2076': // ⁶  [SUPERSCRIPT SIX]
      case L'\u2086': // ₆  [SUBSCRIPT SIX]
      case L'\u2465': // ⑥  [CIRCLED DIGIT SIX]
      case L'\u24FA': // ⓺  [DOUBLE CIRCLED DIGIT SIX]
      case L'\u277B': // ❻  [DINGBAT NEGATIVE CIRCLED DIGIT SIX]
      case L'\u2785': // ➅  [DINGBAT CIRCLED SANS-SERIF DIGIT SIX]
      case L'\u278F': // ➏  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SIX]
      case L'\uFF16': // ６  [FULLWIDTH DIGIT SIX]
        output[outputPos++] = L'6';
        break;
      case L'\u248D': // ⒍  [DIGIT SIX FULL STOP]
        output[outputPos++] = L'6';
        output[outputPos++] = L'.';
        break;
      case L'\u2479': // ⑹  [PARENTHESIZED DIGIT SIX]
        output[outputPos++] = L'(';
        output[outputPos++] = L'6';
        output[outputPos++] = L')';
        break;
      case L'\u2077': // ⁷  [SUPERSCRIPT SEVEN]
      case L'\u2087': // ₇  [SUBSCRIPT SEVEN]
      case L'\u2466': // ⑦  [CIRCLED DIGIT SEVEN]
      case L'\u24FB': // ⓻  [DOUBLE CIRCLED DIGIT SEVEN]
      case L'\u277C': // ❼  [DINGBAT NEGATIVE CIRCLED DIGIT SEVEN]
      case L'\u2786': // ➆  [DINGBAT CIRCLED SANS-SERIF DIGIT SEVEN]
      case L'\u2790': // ➐  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT SEVEN]
      case L'\uFF17': // ７  [FULLWIDTH DIGIT SEVEN]
        output[outputPos++] = L'7';
        break;
      case L'\u248E': // ⒎  [DIGIT SEVEN FULL STOP]
        output[outputPos++] = L'7';
        output[outputPos++] = L'.';
        break;
      case L'\u247A': // ⑺  [PARENTHESIZED DIGIT SEVEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'7';
        output[outputPos++] = L')';
        break;
      case L'\u2078': // ⁸  [SUPERSCRIPT EIGHT]
      case L'\u2088': // ₈  [SUBSCRIPT EIGHT]
      case L'\u2467': // ⑧  [CIRCLED DIGIT EIGHT]
      case L'\u24FC': // ⓼  [DOUBLE CIRCLED DIGIT EIGHT]
      case L'\u277D': // ❽  [DINGBAT NEGATIVE CIRCLED DIGIT EIGHT]
      case L'\u2787': // ➇  [DINGBAT CIRCLED SANS-SERIF DIGIT EIGHT]
      case L'\u2791': // ➑  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT EIGHT]
      case L'\uFF18': // ８  [FULLWIDTH DIGIT EIGHT]
        output[outputPos++] = L'8';
        break;
      case L'\u248F': // ⒏  [DIGIT EIGHT FULL STOP]
        output[outputPos++] = L'8';
        output[outputPos++] = L'.';
        break;
      case L'\u247B': // ⑻  [PARENTHESIZED DIGIT EIGHT]
        output[outputPos++] = L'(';
        output[outputPos++] = L'8';
        output[outputPos++] = L')';
        break;
      case L'\u2079': // ⁹  [SUPERSCRIPT NINE]
      case L'\u2089': // ₉  [SUBSCRIPT NINE]
      case L'\u2468': // ⑨  [CIRCLED DIGIT NINE]
      case L'\u24FD': // ⓽  [DOUBLE CIRCLED DIGIT NINE]
      case L'\u277E': // ❾  [DINGBAT NEGATIVE CIRCLED DIGIT NINE]
      case L'\u2788': // ➈  [DINGBAT CIRCLED SANS-SERIF DIGIT NINE]
      case L'\u2792': // ➒  [DINGBAT NEGATIVE CIRCLED SANS-SERIF DIGIT NINE]
      case L'\uFF19': // ９  [FULLWIDTH DIGIT NINE]
        output[outputPos++] = L'9';
        break;
      case L'\u2490': // ⒐  [DIGIT NINE FULL STOP]
        output[outputPos++] = L'9';
        output[outputPos++] = L'.';
        break;
      case L'\u247C': // ⑼  [PARENTHESIZED DIGIT NINE]
        output[outputPos++] = L'(';
        output[outputPos++] = L'9';
        output[outputPos++] = L')';
        break;
      case L'\u2469': // ⑩  [CIRCLED NUMBER TEN]
      case L'\u24FE': // ⓾  [DOUBLE CIRCLED NUMBER TEN]
      case L'\u277F': // ❿  [DINGBAT NEGATIVE CIRCLED NUMBER TEN]
      case L'\u2789': // ➉  [DINGBAT CIRCLED SANS-SERIF NUMBER TEN]
      case L'\u2793': // ➓  [DINGBAT NEGATIVE CIRCLED SANS-SERIF NUMBER TEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'0';
        break;
      case L'\u2491': // ⒑  [NUMBER TEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'0';
        output[outputPos++] = L'.';
        break;
      case L'\u247D': // ⑽  [PARENTHESIZED NUMBER TEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'0';
        output[outputPos++] = L')';
        break;
      case L'\u246A': // ⑪  [CIRCLED NUMBER ELEVEN]
      case L'\u24EB': // ⓫  [NEGATIVE CIRCLED NUMBER ELEVEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'1';
        break;
      case L'\u2492': // ⒒  [NUMBER ELEVEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'1';
        output[outputPos++] = L'.';
        break;
      case L'\u247E': // ⑾  [PARENTHESIZED NUMBER ELEVEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'1';
        output[outputPos++] = L')';
        break;
      case L'\u246B': // ⑫  [CIRCLED NUMBER TWELVE]
      case L'\u24EC': // ⓬  [NEGATIVE CIRCLED NUMBER TWELVE]
        output[outputPos++] = L'1';
        output[outputPos++] = L'2';
        break;
      case L'\u2493': // ⒓  [NUMBER TWELVE FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'2';
        output[outputPos++] = L'.';
        break;
      case L'\u247F': // ⑿  [PARENTHESIZED NUMBER TWELVE]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'2';
        output[outputPos++] = L')';
        break;
      case L'\u246C': // ⑬  [CIRCLED NUMBER THIRTEEN]
      case L'\u24ED': // ⓭  [NEGATIVE CIRCLED NUMBER THIRTEEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'3';
        break;
      case L'\u2494': // ⒔  [NUMBER THIRTEEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'3';
        output[outputPos++] = L'.';
        break;
      case L'\u2480': // ⒀  [PARENTHESIZED NUMBER THIRTEEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'3';
        output[outputPos++] = L')';
        break;
      case L'\u246D': // ⑭  [CIRCLED NUMBER FOURTEEN]
      case L'\u24EE': // ⓮  [NEGATIVE CIRCLED NUMBER FOURTEEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'4';
        break;
      case L'\u2495': // ⒕  [NUMBER FOURTEEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'4';
        output[outputPos++] = L'.';
        break;
      case L'\u2481': // ⒁  [PARENTHESIZED NUMBER FOURTEEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'4';
        output[outputPos++] = L')';
        break;
      case L'\u246E': // ⑮  [CIRCLED NUMBER FIFTEEN]
      case L'\u24EF': // ⓯  [NEGATIVE CIRCLED NUMBER FIFTEEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'5';
        break;
      case L'\u2496': // ⒖  [NUMBER FIFTEEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'5';
        output[outputPos++] = L'.';
        break;
      case L'\u2482': // ⒂  [PARENTHESIZED NUMBER FIFTEEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'5';
        output[outputPos++] = L')';
        break;
      case L'\u246F': // ⑯  [CIRCLED NUMBER SIXTEEN]
      case L'\u24F0': // ⓰  [NEGATIVE CIRCLED NUMBER SIXTEEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'6';
        break;
      case L'\u2497': // ⒗  [NUMBER SIXTEEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'6';
        output[outputPos++] = L'.';
        break;
      case L'\u2483': // ⒃  [PARENTHESIZED NUMBER SIXTEEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'6';
        output[outputPos++] = L')';
        break;
      case L'\u2470': // ⑰  [CIRCLED NUMBER SEVENTEEN]
      case L'\u24F1': // ⓱  [NEGATIVE CIRCLED NUMBER SEVENTEEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'7';
        break;
      case L'\u2498': // ⒘  [NUMBER SEVENTEEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'7';
        output[outputPos++] = L'.';
        break;
      case L'\u2484': // ⒄  [PARENTHESIZED NUMBER SEVENTEEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'7';
        output[outputPos++] = L')';
        break;
      case L'\u2471': // ⑱  [CIRCLED NUMBER EIGHTEEN]
      case L'\u24F2': // ⓲  [NEGATIVE CIRCLED NUMBER EIGHTEEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'8';
        break;
      case L'\u2499': // ⒙  [NUMBER EIGHTEEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'8';
        output[outputPos++] = L'.';
        break;
      case L'\u2485': // ⒅  [PARENTHESIZED NUMBER EIGHTEEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'8';
        output[outputPos++] = L')';
        break;
      case L'\u2472': // ⑲  [CIRCLED NUMBER NINETEEN]
      case L'\u24F3': // ⓳  [NEGATIVE CIRCLED NUMBER NINETEEN]
        output[outputPos++] = L'1';
        output[outputPos++] = L'9';
        break;
      case L'\u249A': // ⒚  [NUMBER NINETEEN FULL STOP]
        output[outputPos++] = L'1';
        output[outputPos++] = L'9';
        output[outputPos++] = L'.';
        break;
      case L'\u2486': // ⒆  [PARENTHESIZED NUMBER NINETEEN]
        output[outputPos++] = L'(';
        output[outputPos++] = L'1';
        output[outputPos++] = L'9';
        output[outputPos++] = L')';
        break;
      case L'\u2473': // ⑳  [CIRCLED NUMBER TWENTY]
      case L'\u24F4': // ⓴  [NEGATIVE CIRCLED NUMBER TWENTY]
        output[outputPos++] = L'2';
        output[outputPos++] = L'0';
        break;
      case L'\u249B': // ⒛  [NUMBER TWENTY FULL STOP]
        output[outputPos++] = L'2';
        output[outputPos++] = L'0';
        output[outputPos++] = L'.';
        break;
      case L'\u2487': // ⒇  [PARENTHESIZED NUMBER TWENTY]
        output[outputPos++] = L'(';
        output[outputPos++] = L'2';
        output[outputPos++] = L'0';
        output[outputPos++] = L')';
        break;
      case L'\u00AB': // «  [LEFT-POINTING DOUBLE ANGLE QUOTATION MARK]
      case L'\u00BB': // »  [RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK]
      case L'\u201C': // "  [LEFT DOUBLE QUOTATION MARK]
      case L'\u201D': // "  [RIGHT DOUBLE QUOTATION MARK]
      case L'\u201E': // „  [DOUBLE LOW-9 QUOTATION MARK]
      case L'\u2033': // ″  [DOUBLE PRIME]
      case L'\u2036': // ‶  [REVERSED DOUBLE PRIME]
      case L'\u275D': // ❝  [HEAVY DOUBLE TURNED COMMA QUOTATION MARK ORNAMENT]
      case L'\u275E': // ❞  [HEAVY DOUBLE COMMA QUOTATION MARK ORNAMENT]
      case L'\u276E': // ❮  [HEAVY LEFT-POINTING ANGLE QUOTATION MARK ORNAMENT]
      case L'\u276F': // ❯  [HEAVY RIGHT-POINTING ANGLE QUOTATION MARK ORNAMENT]
      case L'\uFF02': // ＂  [FULLWIDTH QUOTATION MARK]
        output[outputPos++] = L'"';
        break;
      case L'\u2018': // ‘  [LEFT SINGLE QUOTATION MARK]
      case L'\u2019': // ’  [RIGHT SINGLE QUOTATION MARK]
      case L'\u201A': // ‚  [SINGLE LOW-9 QUOTATION MARK]
      case L'\u201B': // ‛  [SINGLE HIGH-REVERSED-9 QUOTATION MARK]
      case L'\u2032': // ′  [PRIME]
      case L'\u2035': // ‵  [REVERSED PRIME]
      case L'\u2039': // ‹  [SINGLE LEFT-POINTING ANGLE QUOTATION MARK]
      case L'\u203A': // ›  [SINGLE RIGHT-POINTING ANGLE QUOTATION MARK]
      case L'\u275B': // ❛  [HEAVY SINGLE TURNED COMMA QUOTATION MARK ORNAMENT]
      case L'\u275C': // ❜  [HEAVY SINGLE COMMA QUOTATION MARK ORNAMENT]
      case L'\uFF07': // ＇  [FULLWIDTH APOSTROPHE]
        output[outputPos++] = L'\'';
        break;
      case L'\u2010': // ‐  [HYPHEN]
      case L'\u2011': // ‑  [NON-BREAKING HYPHEN]
      case L'\u2012': // ‒  [FIGURE DASH]
      case L'\u2013': // –  [EN DASH]
      case L'\u2014': // —  [EM DASH]
      case L'\u207B': // ⁻  [SUPERSCRIPT MINUS]
      case L'\u208B': // ₋  [SUBSCRIPT MINUS]
      case L'\uFF0D': // －  [FULLWIDTH HYPHEN-MINUS]
        output[outputPos++] = L'-';
        break;
      case L'\u2045': // ⁅  [LEFT SQUARE BRACKET WITH QUILL]
      case L'\u2772': // ❲  [LIGHT LEFT TORTOISE SHELL BRACKET ORNAMENT]
      case L'\uFF3B': // ［  [FULLWIDTH LEFT SQUARE BRACKET]
        output[outputPos++] = L'[';
        break;
      case L'\u2046': // ⁆  [RIGHT SQUARE BRACKET WITH QUILL]
      case L'\u2773': // ❳  [LIGHT RIGHT TORTOISE SHELL BRACKET ORNAMENT]
      case L'\uFF3D': // ］  [FULLWIDTH RIGHT SQUARE BRACKET]
        output[outputPos++] = L']';
        break;
      case L'\u207D': // ⁽  [SUPERSCRIPT LEFT PARENTHESIS]
      case L'\u208D': // ₍  [SUBSCRIPT LEFT PARENTHESIS]
      case L'\u2768': // ❨  [MEDIUM LEFT PARENTHESIS ORNAMENT]
      case L'\u276A': // ❪  [MEDIUM FLATTENED LEFT PARENTHESIS ORNAMENT]
      case L'\uFF08': // （  [FULLWIDTH LEFT PARENTHESIS]
        output[outputPos++] = L'(';
        break;
      case L'\u2E28': // ⸨  [LEFT DOUBLE PARENTHESIS]
        output[outputPos++] = L'(';
        output[outputPos++] = L'(';
        break;
      case L'\u207E': // ⁾  [SUPERSCRIPT RIGHT PARENTHESIS]
      case L'\u208E': // ₎  [SUBSCRIPT RIGHT PARENTHESIS]
      case L'\u2769': // ❩  [MEDIUM RIGHT PARENTHESIS ORNAMENT]
      case L'\u276B': // ❫  [MEDIUM FLATTENED RIGHT PARENTHESIS ORNAMENT]
      case L'\uFF09': // ）  [FULLWIDTH RIGHT PARENTHESIS]
        output[outputPos++] = L')';
        break;
      case L'\u2E29': // ⸩  [RIGHT DOUBLE PARENTHESIS]
        output[outputPos++] = L')';
        output[outputPos++] = L')';
        break;
      case L'\u276C': // ❬  [MEDIUM LEFT-POINTING ANGLE BRACKET ORNAMENT]
      case L'\u2770': // ❰  [HEAVY LEFT-POINTING ANGLE BRACKET ORNAMENT]
      case L'\uFF1C': // ＜  [FULLWIDTH LESS-THAN SIGN]
        output[outputPos++] = L'<';
        break;
      case L'\u276D': // ❭  [MEDIUM RIGHT-POINTING ANGLE BRACKET ORNAMENT]
      case L'\u2771': // ❱  [HEAVY RIGHT-POINTING ANGLE BRACKET ORNAMENT]
      case L'\uFF1E': // ＞  [FULLWIDTH GREATER-THAN SIGN]
        output[outputPos++] = L'>';
        break;
      case L'\u2774': // ❴  [MEDIUM LEFT CURLY BRACKET ORNAMENT]
      case L'\uFF5B': // ｛  [FULLWIDTH LEFT CURLY BRACKET]
        output[outputPos++] = L'{';
        break;
      case L'\u2775': // ❵  [MEDIUM RIGHT CURLY BRACKET ORNAMENT]
      case L'\uFF5D': // ｝  [FULLWIDTH RIGHT CURLY BRACKET]
        output[outputPos++] = L'}';
        break;
      case L'\u207A': // ⁺  [SUPERSCRIPT PLUS SIGN]
      case L'\u208A': // ₊  [SUBSCRIPT PLUS SIGN]
      case L'\uFF0B': // ＋  [FULLWIDTH PLUS SIGN]
        output[outputPos++] = L'+';
        break;
      case L'\u207C': // ⁼  [SUPERSCRIPT EQUALS SIGN]
      case L'\u208C': // ₌  [SUBSCRIPT EQUALS SIGN]
      case L'\uFF1D': // ＝  [FULLWIDTH EQUALS SIGN]
        output[outputPos++] = L'=';
        break;
      case L'\uFF01': // ！  [FULLWIDTH EXCLAMATION MARK]
        output[outputPos++] = L'!';
        break;
      case L'\u203C': // ‼  [DOUBLE EXCLAMATION MARK]
        output[outputPos++] = L'!';
        output[outputPos++] = L'!';
        break;
      case L'\u2049': // ⁉  [EXCLAMATION QUESTION MARK]
        output[outputPos++] = L'!';
        output[outputPos++] = L'?';
        break;
      case L'\uFF03': // ＃  [FULLWIDTH NUMBER SIGN]
        output[outputPos++] = L'#';
        break;
      case L'\uFF04': // ＄  [FULLWIDTH DOLLAR SIGN]
        output[outputPos++] = L'$';
        break;
      case L'\u2052': // ⁒  [COMMERCIAL MINUS SIGN]
      case L'\uFF05': // ％  [FULLWIDTH PERCENT SIGN]
        output[outputPos++] = L'%';
        break;
      case L'\uFF06': // ＆  [FULLWIDTH AMPERSAND]
        output[outputPos++] = L'&';
        break;
      case L'\u204E': // ⁎  [LOW ASTERISK]
      case L'\uFF0A': // ＊  [FULLWIDTH ASTERISK]
        output[outputPos++] = L'*';
        break;
      case L'\uFF0C': // ，  [FULLWIDTH COMMA]
        output[outputPos++] = L',';
        break;
      case L'\uFF0E': // ．  [FULLWIDTH FULL STOP]
        output[outputPos++] = L'.';
        break;
      case L'\u2044': // ⁄  [FRACTION SLASH]
      case L'\uFF0F': // ／  [FULLWIDTH SOLIDUS]
        output[outputPos++] = L'/';
        break;
      case L'\uFF1A': // ：  [FULLWIDTH COLON]
        output[outputPos++] = L':';
        break;
      case L'\u204F': // ⁏  [REVERSED SEMICOLON]
      case L'\uFF1B': // ；  [FULLWIDTH SEMICOLON]
        output[outputPos++] = L';';
        break;
      case L'\uFF1F': // ？  [FULLWIDTH QUESTION MARK]
        output[outputPos++] = L'?';
        break;
      case L'\u2047': // ⁇  [DOUBLE QUESTION MARK]
        output[outputPos++] = L'?';
        output[outputPos++] = L'?';
        break;
      case L'\u2048': // ⁈  [QUESTION EXCLAMATION MARK]
        output[outputPos++] = L'?';
        output[outputPos++] = L'!';
        break;
      case L'\uFF20': // ＠  [FULLWIDTH COMMERCIAL AT]
        output[outputPos++] = L'@';
        break;
      case L'\uFF3C': // ＼  [FULLWIDTH REVERSE SOLIDUS]
        output[outputPos++] = L'\\';
        break;
      case L'\u2038': // ‸  [CARET]
      case L'\uFF3E': // ＾  [FULLWIDTH CIRCUMFLEX ACCENT]
        output[outputPos++] = L'^';
        break;
      case L'\uFF3F': // ＿  [FULLWIDTH LOW LINE]
        output[outputPos++] = L'_';
        break;
      case L'\u2053': // ⁓  [SWUNG DASH]
      case L'\uFF5E': // ～  [FULLWIDTH TILDE]
        output[outputPos++] = L'~';
        break;
      default:
        output[outputPos++] = c;
        break;
      }
    }
  }
  return outputPos;
}
} // namespace org::apache::lucene::analysis::miscellaneous