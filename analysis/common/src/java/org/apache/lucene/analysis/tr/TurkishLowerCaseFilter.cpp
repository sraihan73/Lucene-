using namespace std;

#include "TurkishLowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::tr
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

TurkishLowerCaseFilter::TurkishLowerCaseFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool TurkishLowerCaseFilter::incrementToken() 
{
  bool iOrAfter = false;

  if (input->incrementToken()) {
    const std::deque<wchar_t> buffer = termAtt->buffer();
    int length = termAtt->length();
    for (int i = 0; i < length;) {
      constexpr int ch = Character::codePointAt(buffer, i, length);

      iOrAfter =
          (ch == LATIN_CAPITAL_LETTER_I ||
           (iOrAfter && Character::getType(ch) == Character::NON_SPACING_MARK));

      if (iOrAfter) { // all the special I turkish handling happens here.
        switch (ch) {
        // remove COMBINING_DOT_ABOVE to mimic composed lowercase
        case COMBINING_DOT_ABOVE:
          length = delete (buffer, i, length);
          continue;
        // i itself, it depends if it is followed by COMBINING_DOT_ABOVE
        // if it is, we will make it small i and later remove the dot
        case LATIN_CAPITAL_LETTER_I:
          if (isBeforeDot(buffer, i + 1, length)) {
            buffer[i] = LATIN_SMALL_LETTER_I;
          } else {
            buffer[i] = LATIN_SMALL_LETTER_DOTLESS_I;
            // below is an optimization. no COMBINING_DOT_ABOVE follows,
            // so don't waste time calculating Character.getType(), etc
            iOrAfter = false;
          }
          i++;
          continue;
        }
      }

      i += Character::toChars(towlower(ch), buffer, i);
    }

    termAtt->setLength(length);
    return true;
  } else {
    return false;
  }
}

bool TurkishLowerCaseFilter::isBeforeDot(std::deque<wchar_t> &s, int pos,
                                         int len)
{
  for (int i = pos; i < len;) {
    constexpr int ch = Character::codePointAt(s, i, len);
    if (Character::getType(ch) != Character::NON_SPACING_MARK) {
      return false;
    }
    if (ch == COMBINING_DOT_ABOVE) {
      return true;
    }
    i += Character::charCount(ch);
  }

  return false;
}

int TurkishLowerCaseFilter::delete_(std::deque<wchar_t> &s, int pos, int len)
{
  if (pos < len) {
    System::arraycopy(s, pos + 1, s, pos, len - pos - 1);
  }

  return len - 1;
}
} // namespace org::apache::lucene::analysis::tr