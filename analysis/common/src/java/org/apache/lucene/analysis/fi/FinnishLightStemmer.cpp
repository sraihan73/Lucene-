using namespace std;

#include "FinnishLightStemmer.h"

namespace org::apache::lucene::analysis::fi
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int FinnishLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 4) {
    return len;
  }

  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'ä':
    case L'å':
      s[i] = L'a';
      break;
    case L'ö':
      s[i] = L'o';
      break;
    }
  }

  len = step1(s, len);
  len = step2(s, len);
  len = step3(s, len);
  len = norm1(s, len);
  len = norm2(s, len);
  return len;
}

int FinnishLightStemmer::step1(std::deque<wchar_t> &s, int len)
{
  if (len > 8) {
    if (StemmerUtil::endsWith(s, len, L"kin")) {
      return step1(s, len - 3);
    }
    if (StemmerUtil::endsWith(s, len, L"ko")) {
      return step1(s, len - 2);
    }
  }

  if (len > 11) {
    if (StemmerUtil::endsWith(s, len, L"dellinen")) {
      return len - 8;
    }
    if (StemmerUtil::endsWith(s, len, L"dellisuus")) {
      return len - 9;
    }
  }
  return len;
}

int FinnishLightStemmer::step2(std::deque<wchar_t> &s, int len)
{
  if (len > 5) {
    if (StemmerUtil::endsWith(s, len, L"lla") ||
        StemmerUtil::endsWith(s, len, L"tse") ||
        StemmerUtil::endsWith(s, len, L"sti")) {
      return len - 3;
    }

    if (StemmerUtil::endsWith(s, len, L"ni")) {
      return len - 2;
    }

    if (StemmerUtil::endsWith(s, len, L"aa")) {
      return len - 1; // aa -> a
    }
  }

  return len;
}

int FinnishLightStemmer::step3(std::deque<wchar_t> &s, int len)
{
  if (len > 8) {
    if (StemmerUtil::endsWith(s, len, L"nnen")) {
      s[len - 4] = L's';
      return len - 3;
    }

    if (StemmerUtil::endsWith(s, len, L"ntena")) {
      s[len - 5] = L's';
      return len - 4;
    }

    if (StemmerUtil::endsWith(s, len, L"tten")) {
      return len - 4;
    }

    if (StemmerUtil::endsWith(s, len, L"eiden")) {
      return len - 5;
    }
  }

  if (len > 6) {
    if (StemmerUtil::endsWith(s, len, L"neen") ||
        StemmerUtil::endsWith(s, len, L"niin") ||
        StemmerUtil::endsWith(s, len, L"seen") ||
        StemmerUtil::endsWith(s, len, L"teen") ||
        StemmerUtil::endsWith(s, len, L"inen")) {
      return len - 4;
    }

    if (s[len - 3] == L'h' && isVowel(s[len - 2]) && s[len - 1] == L'n') {
      return len - 3;
    }

    if (StemmerUtil::endsWith(s, len, L"den")) {
      s[len - 3] = L's';
      return len - 2;
    }

    if (StemmerUtil::endsWith(s, len, L"ksen")) {
      s[len - 4] = L's';
      return len - 3;
    }

    if (StemmerUtil::endsWith(s, len, L"ssa") ||
        StemmerUtil::endsWith(s, len, L"sta") ||
        StemmerUtil::endsWith(s, len, L"lla") ||
        StemmerUtil::endsWith(s, len, L"lta") ||
        StemmerUtil::endsWith(s, len, L"tta") ||
        StemmerUtil::endsWith(s, len, L"ksi") ||
        StemmerUtil::endsWith(s, len, L"lle")) {
      return len - 3;
    }
  }

  if (len > 5) {
    if (StemmerUtil::endsWith(s, len, L"na") ||
        StemmerUtil::endsWith(s, len, L"ne")) {
      return len - 2;
    }

    if (StemmerUtil::endsWith(s, len, L"nei")) {
      return len - 3;
    }
  }

  if (len > 4) {
    if (StemmerUtil::endsWith(s, len, L"ja") ||
        StemmerUtil::endsWith(s, len, L"ta")) {
      return len - 2;
    }

    if (s[len - 1] == L'a') {
      return len - 1;
    }

    if (s[len - 1] == L'n' && isVowel(s[len - 2])) {
      return len - 2;
    }

    if (s[len - 1] == L'n') {
      return len - 1;
    }
  }

  return len;
}

int FinnishLightStemmer::norm1(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && StemmerUtil::endsWith(s, len, L"hde")) {
    s[len - 3] = L'k';
    s[len - 2] = L's';
    s[len - 1] = L'i';
  }

  if (len > 4) {
    if (StemmerUtil::endsWith(s, len, L"ei") ||
        StemmerUtil::endsWith(s, len, L"at")) {
      return len - 2;
    }
  }

  if (len > 3) {
    switch (s[len - 1]) {
    case L't':
    case L's':
    case L'j':
    case L'e':
    case L'a':
    case L'i':
      return len - 1;
    }
  }

  return len;
}

int FinnishLightStemmer::norm2(std::deque<wchar_t> &s, int len)
{
  if (len > 8) {
    if (s[len - 1] == L'e' || s[len - 1] == L'o' || s[len - 1] == L'u') {
      len--;
    }
  }

  if (len > 4) {
    if (s[len - 1] == L'i') {
      len--;
    }

    if (len > 4) {
      wchar_t ch = s[0];
      for (int i = 1; i < len; i++) {
        if (s[i] == ch && (ch == L'k' || ch == L'p' || ch == L't')) {
          len = StemmerUtil::delete (s, i--, len);
        } else {
          ch = s[i];
        }
      }
    }
  }

  return len;
}

bool FinnishLightStemmer::isVowel(wchar_t ch)
{
  switch (ch) {
  case L'a':
  case L'e':
  case L'i':
  case L'o':
  case L'u':
  case L'y':
    return true;
  default:
    return false;
  }
}
} // namespace org::apache::lucene::analysis::fi