using namespace std;

#include "PortugueseLightStemmer.h"

namespace org::apache::lucene::analysis::pt
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int PortugueseLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 4) {
    return len;
  }

  len = removeSuffix(s, len);

  if (len > 3 && s[len - 1] == L'a') {
    len = normFeminine(s, len);
  }

  if (len > 4) {
    switch (s[len - 1]) {
    case L'e':
    case L'a':
    case L'o':
      len--;
      break;
    }
  }

  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'à':
    case L'á':
    case L'â':
    case L'ä':
    case L'ã':
      s[i] = L'a';
      break;
    case L'ò':
    case L'ó':
    case L'ô':
    case L'ö':
    case L'õ':
      s[i] = L'o';
      break;
    case L'è':
    case L'é':
    case L'ê':
    case L'ë':
      s[i] = L'e';
      break;
    case L'ù':
    case L'ú':
    case L'û':
    case L'ü':
      s[i] = L'u';
      break;
    case L'ì':
    case L'í':
    case L'î':
    case L'ï':
      s[i] = L'i';
      break;
    case L'ç':
      s[i] = L'c';
      break;
    }
  }

  return len;
}

int PortugueseLightStemmer::removeSuffix(std::deque<wchar_t> &s, int len)
{
  if (len > 4 && StemmerUtil::endsWith(s, len, L"es")) {
    switch (s[len - 3]) {
    case L'r':
    case L's':
    case L'l':
    case L'z':
      return len - 2;
    }
  }

  if (len > 3 && StemmerUtil::endsWith(s, len, L"ns")) {
    s[len - 2] = L'm';
    return len - 1;
  }

  if (len > 4 && (StemmerUtil::endsWith(s, len, L"eis") ||
                  StemmerUtil::endsWith(s, len, L"éis"))) {
    s[len - 3] = L'e';
    s[len - 2] = L'l';
    return len - 1;
  }

  if (len > 4 && StemmerUtil::endsWith(s, len, L"ais")) {
    s[len - 2] = L'l';
    return len - 1;
  }

  if (len > 4 && StemmerUtil::endsWith(s, len, L"óis")) {
    s[len - 3] = L'o';
    s[len - 2] = L'l';
    return len - 1;
  }

  if (len > 4 && StemmerUtil::endsWith(s, len, L"is")) {
    s[len - 1] = L'l';
    return len;
  }

  if (len > 3 && (StemmerUtil::endsWith(s, len, L"ões") ||
                  StemmerUtil::endsWith(s, len, L"ães"))) {
    len--;
    s[len - 2] = L'ã';
    s[len - 1] = L'o';
    return len;
  }

  if (len > 6 && StemmerUtil::endsWith(s, len, L"mente")) {
    return len - 5;
  }

  if (len > 3 && s[len - 1] == L's') {
    return len - 1;
  }
  return len;
}

int PortugueseLightStemmer::normFeminine(std::deque<wchar_t> &s, int len)
{
  if (len > 7 && (StemmerUtil::endsWith(s, len, L"inha") ||
                  StemmerUtil::endsWith(s, len, L"iaca") ||
                  StemmerUtil::endsWith(s, len, L"eira"))) {
    s[len - 1] = L'o';
    return len;
  }

  if (len > 6) {
    if (StemmerUtil::endsWith(s, len, L"osa") ||
        StemmerUtil::endsWith(s, len, L"ica") ||
        StemmerUtil::endsWith(s, len, L"ida") ||
        StemmerUtil::endsWith(s, len, L"ada") ||
        StemmerUtil::endsWith(s, len, L"iva") ||
        StemmerUtil::endsWith(s, len, L"ama")) {
      s[len - 1] = L'o';
      return len;
    }

    if (StemmerUtil::endsWith(s, len, L"ona")) {
      s[len - 3] = L'ã';
      s[len - 2] = L'o';
      return len - 1;
    }

    if (StemmerUtil::endsWith(s, len, L"ora")) {
      return len - 1;
    }

    if (StemmerUtil::endsWith(s, len, L"esa")) {
      s[len - 3] = L'ê';
      return len - 1;
    }

    if (StemmerUtil::endsWith(s, len, L"na")) {
      s[len - 1] = L'o';
      return len;
    }
  }
  return len;
}
} // namespace org::apache::lucene::analysis::pt