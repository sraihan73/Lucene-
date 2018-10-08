using namespace std;

#include "HindiNormalizer.h"

namespace org::apache::lucene::analysis::hi
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int HindiNormalizer::normalize(std::deque<wchar_t> &s, int len)
{

  for (int i = 0; i < len; i++) {
    switch (s[i]) {
      // dead n -> bindu
    case L'\u0928':
      if (i + 1 < len && s[i + 1] == L'\u094D') {
        s[i] = L'\u0902';
        len = StemmerUtil::delete (s, i + 1, len);
      }
      break;
    // candrabindu -> bindu
    case L'\u0901':
      s[i] = L'\u0902';
      break;
    // nukta deletions
    case L'\u093C':
      len = StemmerUtil::delete (s, i, len);
      i--;
      break;
    case L'\u0929':
      s[i] = L'\u0928';
      break;
    case L'\u0931':
      s[i] = L'\u0930';
      break;
    case L'\u0934':
      s[i] = L'\u0933';
      break;
    case L'\u0958':
      s[i] = L'\u0915';
      break;
    case L'\u0959':
      s[i] = L'\u0916';
      break;
    case L'\u095A':
      s[i] = L'\u0917';
      break;
    case L'\u095B':
      s[i] = L'\u091C';
      break;
    case L'\u095C':
      s[i] = L'\u0921';
      break;
    case L'\u095D':
      s[i] = L'\u0922';
      break;
    case L'\u095E':
      s[i] = L'\u092B';
      break;
    case L'\u095F':
      s[i] = L'\u092F';
      break;
      // zwj/zwnj -> delete
    case L'\u200D':
    case L'\u200C':
      len = StemmerUtil::delete (s, i, len);
      i--;
      break;
      // virama -> delete
    case L'\u094D':
      len = StemmerUtil::delete (s, i, len);
      i--;
      break;
      // chandra/short -> replace
    case L'\u0945':
    case L'\u0946':
      s[i] = L'\u0947';
      break;
    case L'\u0949':
    case L'\u094A':
      s[i] = L'\u094B';
      break;
    case L'\u090D':
    case L'\u090E':
      s[i] = L'\u090F';
      break;
    case L'\u0911':
    case L'\u0912':
      s[i] = L'\u0913';
      break;
    case L'\u0972':
      s[i] = L'\u0905';
      break;
      // long -> short ind. vowels
    case L'\u0906':
      s[i] = L'\u0905';
      break;
    case L'\u0908':
      s[i] = L'\u0907';
      break;
    case L'\u090A':
      s[i] = L'\u0909';
      break;
    case L'\u0960':
      s[i] = L'\u090B';
      break;
    case L'\u0961':
      s[i] = L'\u090C';
      break;
    case L'\u0910':
      s[i] = L'\u090F';
      break;
    case L'\u0914':
      s[i] = L'\u0913';
      break;
      // long -> short dep. vowels
    case L'\u0940':
      s[i] = L'\u093F';
      break;
    case L'\u0942':
      s[i] = L'\u0941';
      break;
    case L'\u0944':
      s[i] = L'\u0943';
      break;
    case L'\u0963':
      s[i] = L'\u0962';
      break;
    case L'\u0948':
      s[i] = L'\u0947';
      break;
    case L'\u094C':
      s[i] = L'\u094B';
      break;
    default:
      break;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::hi