using namespace std;

#include "HungarianLightStemmer.h"

namespace org::apache::lucene::analysis::hu
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int HungarianLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'á':
      s[i] = L'a';
      break;
    case L'ë':
    case L'é':
      s[i] = L'e';
      break;
    case L'í':
      s[i] = L'i';
      break;
    case L'ó':
    case L'ő':
    case L'õ':
    case L'ö':
      s[i] = L'o';
      break;
    case L'ú':
    case L'ű':
    case L'ũ':
    case L'û':
    case L'ü':
      s[i] = L'u';
      break;
    }
  }

  len = removeCase(s, len);
  len = removePossessive(s, len);
  len = removePlural(s, len);
  return normalize(s, len);
}

int HungarianLightStemmer::removeCase(std::deque<wchar_t> &s, int len)
{
  if (len > 6 && StemmerUtil::endsWith(s, len, L"kent")) {
    return len - 4;
  }

  if (len > 5) {
    if (StemmerUtil::endsWith(s, len, L"nak") ||
        StemmerUtil::endsWith(s, len, L"nek") ||
        StemmerUtil::endsWith(s, len, L"val") ||
        StemmerUtil::endsWith(s, len, L"vel") ||
        StemmerUtil::endsWith(s, len, L"ert") ||
        StemmerUtil::endsWith(s, len, L"rol") ||
        StemmerUtil::endsWith(s, len, L"ban") ||
        StemmerUtil::endsWith(s, len, L"ben") ||
        StemmerUtil::endsWith(s, len, L"bol") ||
        StemmerUtil::endsWith(s, len, L"nal") ||
        StemmerUtil::endsWith(s, len, L"nel") ||
        StemmerUtil::endsWith(s, len, L"hoz") ||
        StemmerUtil::endsWith(s, len, L"hez") ||
        StemmerUtil::endsWith(s, len, L"tol")) {
      return len - 3;
    }

    if (StemmerUtil::endsWith(s, len, L"al") ||
        StemmerUtil::endsWith(s, len, L"el")) {
      if (!isVowel(s[len - 3]) && s[len - 3] == s[len - 4]) {
        return len - 3;
      }
    }
  }

  if (len > 4) {
    if (StemmerUtil::endsWith(s, len, L"at") ||
        StemmerUtil::endsWith(s, len, L"et") ||
        StemmerUtil::endsWith(s, len, L"ot") ||
        StemmerUtil::endsWith(s, len, L"va") ||
        StemmerUtil::endsWith(s, len, L"ve") ||
        StemmerUtil::endsWith(s, len, L"ra") ||
        StemmerUtil::endsWith(s, len, L"re") ||
        StemmerUtil::endsWith(s, len, L"ba") ||
        StemmerUtil::endsWith(s, len, L"be") ||
        StemmerUtil::endsWith(s, len, L"ul") ||
        StemmerUtil::endsWith(s, len, L"ig")) {
      return len - 2;
    }

    if ((StemmerUtil::endsWith(s, len, L"on") ||
         StemmerUtil::endsWith(s, len, L"en")) &&
        !isVowel(s[len - 3])) {
      return len - 2;
    }

    switch (s[len - 1]) {
    case L't':
    case L'n':
      return len - 1;
    case L'a':
    case L'e':
      if (s[len - 2] == s[len - 3] && !isVowel(s[len - 2])) {
        return len - 2;
      }
    }
  }

  return len;
}

int HungarianLightStemmer::removePossessive(std::deque<wchar_t> &s, int len)
{
  if (len > 6) {
    if (!isVowel(s[len - 5]) && (StemmerUtil::endsWith(s, len, L"atok") ||
                                 StemmerUtil::endsWith(s, len, L"otok") ||
                                 StemmerUtil::endsWith(s, len, L"etek"))) {
      return len - 4;
    }

    if (StemmerUtil::endsWith(s, len, L"itek") ||
        StemmerUtil::endsWith(s, len, L"itok")) {
      return len - 4;
    }
  }

  if (len > 5) {
    if (!isVowel(s[len - 4]) && (StemmerUtil::endsWith(s, len, L"unk") ||
                                 StemmerUtil::endsWith(s, len, L"tok") ||
                                 StemmerUtil::endsWith(s, len, L"tek"))) {
      return len - 3;
    }

    if (isVowel(s[len - 4]) && StemmerUtil::endsWith(s, len, L"juk")) {
      return len - 3;
    }

    if (StemmerUtil::endsWith(s, len, L"ink")) {
      return len - 3;
    }
  }

  if (len > 4) {
    if (!isVowel(s[len - 3]) && (StemmerUtil::endsWith(s, len, L"am") ||
                                 StemmerUtil::endsWith(s, len, L"em") ||
                                 StemmerUtil::endsWith(s, len, L"om") ||
                                 StemmerUtil::endsWith(s, len, L"ad") ||
                                 StemmerUtil::endsWith(s, len, L"ed") ||
                                 StemmerUtil::endsWith(s, len, L"od") ||
                                 StemmerUtil::endsWith(s, len, L"uk"))) {
      return len - 2;
    }

    if (isVowel(s[len - 3]) && (StemmerUtil::endsWith(s, len, L"nk") ||
                                StemmerUtil::endsWith(s, len, L"ja") ||
                                StemmerUtil::endsWith(s, len, L"je"))) {
      return len - 2;
    }

    if (StemmerUtil::endsWith(s, len, L"im") ||
        StemmerUtil::endsWith(s, len, L"id") ||
        StemmerUtil::endsWith(s, len, L"ik")) {
      return len - 2;
    }
  }

  if (len > 3) {
    switch (s[len - 1]) {
    case L'a':
    case L'e':
      if (!isVowel(s[len - 2])) {
        return len - 1;
      }
      break;
    case L'm':
    case L'd':
      if (isVowel(s[len - 2])) {
        return len - 1;
      }
      break;
    case L'i':
      return len - 1;
    }
  }

  return len;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") private int removePlural(char
// s[], int len)
int HungarianLightStemmer::removePlural(std::deque<wchar_t> &s, int len)
{
  if (len > 3 && s[len - 1] == L'k') {
    switch (s[len - 2]) {
    case L'a':
    case L'o':
    case L'e':
      if (len > 4) {
        return len - 2; // intentional fallthru
      }
    default:
      return len - 1;
    }
  }
  return len;
}

int HungarianLightStemmer::normalize(std::deque<wchar_t> &s, int len)
{
  if (len > 3) {
    switch (s[len - 1]) {
    case L'a':
    case L'e':
    case L'i':
    case L'o':
      return len - 1;
    }
  }
  return len;
}

bool HungarianLightStemmer::isVowel(wchar_t ch)
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
} // namespace org::apache::lucene::analysis::hu