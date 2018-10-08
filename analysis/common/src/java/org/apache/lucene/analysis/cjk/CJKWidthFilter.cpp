using namespace std;

#include "CJKWidthFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../util/StemmerUtil.h"

namespace org::apache::lucene::analysis::cjk
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using StemmerUtil = org::apache::lucene::analysis::util::StemmerUtil;
std::deque<wchar_t> const CJKWidthFilter::KANA_NORM = std::deque<wchar_t>{
    0x30fb, 0x30f2, 0x30a1, 0x30a3, 0x30a5, 0x30a7, 0x30a9, 0x30e3, 0x30e5,
    0x30e7, 0x30c3, 0x30fc, 0x30a2, 0x30a4, 0x30a6, 0x30a8, 0x30aa, 0x30ab,
    0x30ad, 0x30af, 0x30b1, 0x30b3, 0x30b5, 0x30b7, 0x30b9, 0x30bb, 0x30bd,
    0x30bf, 0x30c1, 0x30c4, 0x30c6, 0x30c8, 0x30ca, 0x30cb, 0x30cc, 0x30cd,
    0x30ce, 0x30cf, 0x30d2, 0x30d5, 0x30d8, 0x30db, 0x30de, 0x30df, 0x30e0,
    0x30e1, 0x30e2, 0x30e4, 0x30e6, 0x30e8, 0x30e9, 0x30ea, 0x30eb, 0x30ec,
    0x30ed, 0x30ef, 0x30f3, 0x3099, 0x309A};

CJKWidthFilter::CJKWidthFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool CJKWidthFilter::incrementToken() 
{
  if (input->incrementToken()) {
    std::deque<wchar_t> text = termAtt->buffer();
    int length = termAtt->length();
    for (int i = 0; i < length; i++) {
      constexpr wchar_t ch = text[i];
      if (ch >= 0xFF01 && ch <= 0xFF5E) {
        // Fullwidth ASCII variants
        text[i] -= 0xFEE0;
      } else if (ch >= 0xFF65 && ch <= 0xFF9F) {
        // Halfwidth Katakana variants
        if ((ch == 0xFF9E || ch == 0xFF9F) && i > 0 && combine(text, i, ch)) {
          length = StemmerUtil::delete (text, i--, length);
        } else {
          text[i] = KANA_NORM[ch - 0xFF65];
        }
      }
    }
    termAtt->setLength(length);
    return true;
  } else {
    return false;
  }
}

std::deque<char> const CJKWidthFilter::KANA_COMBINE_VOICED = std::deque<char>{
    78, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
    0,  1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    1,  0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
std::deque<char> const CJKWidthFilter::KANA_COMBINE_HALF_VOICED =
    std::deque<char>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

bool CJKWidthFilter::combine(std::deque<wchar_t> &text, int pos, wchar_t ch)
{
  constexpr wchar_t prev = text[pos - 1];
  if (prev >= 0x30A6 && prev <= 0x30FD) {
    text[pos - 1] += (ch == 0xFF9F) ? KANA_COMBINE_HALF_VOICED[prev - 0x30A6]
                                    : KANA_COMBINE_VOICED[prev - 0x30A6];
    return text[pos - 1] != prev;
  }
  return false;
}
} // namespace org::apache::lucene::analysis::cjk