using namespace std;

#include "FrenchLightStemmer.h"

namespace org::apache::lucene::analysis::fr
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int FrenchLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && s[len - 1] == L'x') {
    if (s[len - 3] == L'a' && s[len - 2] == L'u' && s[len - 4] != L'e') {
      s[len - 2] = L'l';
    }
    len--;
  }

  if (len > 3 && s[len - 1] == L'x') {
    len--;
  }

  if (len > 3 && s[len - 1] == L's') {
    len--;
  }

  if (len > 9 && StemmerUtil::endsWith(s, len, L"issement")) {
    len -= 6;
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 8 && StemmerUtil::endsWith(s, len, L"issant")) {
    len -= 4;
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 6 && StemmerUtil::endsWith(s, len, L"ement")) {
    len -= 4;
    if (len > 3 && StemmerUtil::endsWith(s, len, L"ive")) {
      len--;
      s[len - 1] = L'f';
    }
    return norm(s, len);
  }

  if (len > 11 && StemmerUtil::endsWith(s, len, L"ficatrice")) {
    len -= 5;
    s[len - 2] = L'e';
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 10 && StemmerUtil::endsWith(s, len, L"ficateur")) {
    len -= 4;
    s[len - 2] = L'e';
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 9 && StemmerUtil::endsWith(s, len, L"catrice")) {
    len -= 3;
    s[len - 4] = L'q';
    s[len - 3] = L'u';
    s[len - 2] = L'e';
    // s[len-1] = 'r' <-- unnecessary, already 'r'.
    return norm(s, len);
  }

  if (len > 8 && StemmerUtil::endsWith(s, len, L"cateur")) {
    len -= 2;
    s[len - 4] = L'q';
    s[len - 3] = L'u';
    s[len - 2] = L'e';
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 8 && StemmerUtil::endsWith(s, len, L"atrice")) {
    len -= 4;
    s[len - 2] = L'e';
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 7 && StemmerUtil::endsWith(s, len, L"ateur")) {
    len -= 3;
    s[len - 2] = L'e';
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 6 && StemmerUtil::endsWith(s, len, L"trice")) {
    len--;
    s[len - 3] = L'e';
    s[len - 2] = L'u';
    s[len - 1] = L'r';
  }

  if (len > 5 && StemmerUtil::endsWith(s, len, L"ième")) {
    return norm(s, len - 4);
  }

  if (len > 7 && StemmerUtil::endsWith(s, len, L"teuse")) {
    len -= 2;
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 6 && StemmerUtil::endsWith(s, len, L"teur")) {
    len--;
    s[len - 1] = L'r';
    return norm(s, len);
  }

  if (len > 5 && StemmerUtil::endsWith(s, len, L"euse")) {
    return norm(s, len - 2);
  }

  if (len > 8 && StemmerUtil::endsWith(s, len, L"ère")) {
    len--;
    s[len - 2] = L'e';
    return norm(s, len);
  }

  if (len > 7 && StemmerUtil::endsWith(s, len, L"ive")) {
    len--;
    s[len - 1] = L'f';
    return norm(s, len);
  }

  if (len > 4 && (StemmerUtil::endsWith(s, len, L"folle") ||
                  StemmerUtil::endsWith(s, len, L"molle"))) {
    len -= 2;
    s[len - 1] = L'u';
    return norm(s, len);
  }

  if (len > 9 && StemmerUtil::endsWith(s, len, L"nnelle")) {
    return norm(s, len - 5);
  }

  if (len > 9 && StemmerUtil::endsWith(s, len, L"nnel")) {
    return norm(s, len - 3);
  }

  if (len > 4 && StemmerUtil::endsWith(s, len, L"ète")) {
    len--;
    s[len - 2] = L'e';
  }

  if (len > 8 && StemmerUtil::endsWith(s, len, L"ique")) {
    len -= 4;
  }

  if (len > 8 && StemmerUtil::endsWith(s, len, L"esse")) {
    return norm(s, len - 3);
  }

  if (len > 7 && StemmerUtil::endsWith(s, len, L"inage")) {
    return norm(s, len - 3);
  }

  if (len > 9 && StemmerUtil::endsWith(s, len, L"isation")) {
    len -= 7;
    if (len > 5 && StemmerUtil::endsWith(s, len, L"ual")) {
      s[len - 2] = L'e';
    }
    return norm(s, len);
  }

  if (len > 9 && StemmerUtil::endsWith(s, len, L"isateur")) {
    return norm(s, len - 7);
  }

  if (len > 8 && StemmerUtil::endsWith(s, len, L"ation")) {
    return norm(s, len - 5);
  }

  if (len > 8 && StemmerUtil::endsWith(s, len, L"ition")) {
    return norm(s, len - 5);
  }

  return norm(s, len);
}

int FrenchLightStemmer::norm(std::deque<wchar_t> &s, int len)
{
  if (len > 4) {
    for (int i = 0; i < len; i++) {
      switch (s[i]) {
      case L'à':
      case L'á':
      case L'â':
        s[i] = L'a';
        break;
      case L'ô':
        s[i] = L'o';
        break;
      case L'è':
      case L'é':
      case L'ê':
        s[i] = L'e';
        break;
      case L'ù':
      case L'û':
        s[i] = L'u';
        break;
      case L'î':
        s[i] = L'i';
        break;
      case L'ç':
        s[i] = L'c';
        break;
      }
    }

    wchar_t ch = s[0];
    for (int i = 1; i < len; i++) {
      if (s[i] == ch && isalpha(ch)) {
        len = StemmerUtil::delete (s, i--, len);
      } else {
        ch = s[i];
      }
    }
  }

  if (len > 4 && StemmerUtil::endsWith(s, len, L"ie")) {
    len -= 2;
  }

  if (len > 4) {
    if (s[len - 1] == L'r') {
      len--;
    }
    if (s[len - 1] == L'e') {
      len--;
    }
    if (s[len - 1] == L'e') {
      len--;
    }
    if (s[len - 1] == s[len - 2] && isalpha(s[len - 1])) {
      len--;
    }
  }
  return len;
}
} // namespace org::apache::lucene::analysis::fr