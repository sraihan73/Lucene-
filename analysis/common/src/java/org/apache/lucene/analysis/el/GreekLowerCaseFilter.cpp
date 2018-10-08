using namespace std;

#include "GreekLowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::el
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

GreekLowerCaseFilter::GreekLowerCaseFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool GreekLowerCaseFilter::incrementToken() 
{
  if (input->incrementToken()) {
    std::deque<wchar_t> chArray = termAtt->buffer();
    int chLen = termAtt->length();
    for (int i = 0; i < chLen;) {
      i += Character::toChars(
          lowerCase(Character::codePointAt(chArray, i, chLen)), chArray, i);
    }
    return true;
  } else {
    return false;
  }
}

int GreekLowerCaseFilter::lowerCase(int codepoint)
{
  switch (codepoint) {
  /* There are two lowercase forms of sigma:
   *   U+03C2: small final sigma (end of word)
   *   U+03C3: small sigma (otherwise)
   *
   * Standardize both to U+03C3
   */
  case L'\u03C2':     // small final sigma
    return L'\u03C3'; // small sigma

    /* Some greek characters contain diacritics.
     * This filter removes these, converting to the lowercase base form.
     */

  case L'\u0386':     // capital alpha with tonos
  case L'\u03AC':     // small alpha with tonos
    return L'\u03B1'; // small alpha

  case L'\u0388':     // capital epsilon with tonos
  case L'\u03AD':     // small epsilon with tonos
    return L'\u03B5'; // small epsilon

  case L'\u0389':     // capital eta with tonos
  case L'\u03AE':     // small eta with tonos
    return L'\u03B7'; // small eta

  case L'\u038A':     // capital iota with tonos
  case L'\u03AA':     // capital iota with dialytika
  case L'\u03AF':     // small iota with tonos
  case L'\u03CA':     // small iota with dialytika
  case L'\u0390':     // small iota with dialytika and tonos
    return L'\u03B9'; // small iota

  case L'\u038E':     // capital upsilon with tonos
  case L'\u03AB':     // capital upsilon with dialytika
  case L'\u03CD':     // small upsilon with tonos
  case L'\u03CB':     // small upsilon with dialytika
  case L'\u03B0':     // small upsilon with dialytika and tonos
    return L'\u03C5'; // small upsilon

  case L'\u038C':     // capital omicron with tonos
  case L'\u03CC':     // small omicron with tonos
    return L'\u03BF'; // small omicron

  case L'\u038F':     // capital omega with tonos
  case L'\u03CE':     // small omega with tonos
    return L'\u03C9'; // small omega

    /* The previous implementation did the conversion below.
     * Only implemented for backwards compatibility with old indexes.
     */

  case L'\u03A2':     // reserved
    return L'\u03C2'; // small final sigma

  default:
    return towlower(codepoint);
  }
}
} // namespace org::apache::lucene::analysis::el