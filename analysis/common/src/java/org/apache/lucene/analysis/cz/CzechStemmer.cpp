using namespace std;

#include "CzechStemmer.h"

namespace org::apache::lucene::analysis::cz
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int CzechStemmer::stem(std::deque<wchar_t> &s, int len)
{
  len = removeCase(s, len);
  len = removePossessives(s, len);
  if (len > 0) {
    len = normalize(s, len);
  }
  return len;
}

int CzechStemmer::removeCase(std::deque<wchar_t> &s, int len)
{
  if (len > 7 && StemmerUtil::endsWith(s, len, L"atech")) {
    return len - 5;
  }

  if (len > 6 && (StemmerUtil::endsWith(s, len, L"ětem") ||
                  StemmerUtil::endsWith(s, len, L"etem") ||
                  StemmerUtil::endsWith(s, len, L"atům"))) {
    return len - 4;
  }

  if (len > 5 && (StemmerUtil::endsWith(s, len, L"ech") ||
                  StemmerUtil::endsWith(s, len, L"ich") ||
                  StemmerUtil::endsWith(s, len, L"ích") ||
                  StemmerUtil::endsWith(s, len, L"ého") ||
                  StemmerUtil::endsWith(s, len, L"ěmi") ||
                  StemmerUtil::endsWith(s, len, L"emi") ||
                  StemmerUtil::endsWith(s, len, L"ému") ||
                  StemmerUtil::endsWith(s, len, L"ěte") ||
                  StemmerUtil::endsWith(s, len, L"ete") ||
                  StemmerUtil::endsWith(s, len, L"ěti") ||
                  StemmerUtil::endsWith(s, len, L"eti") ||
                  StemmerUtil::endsWith(s, len, L"ího") ||
                  StemmerUtil::endsWith(s, len, L"iho") ||
                  StemmerUtil::endsWith(s, len, L"ími") ||
                  StemmerUtil::endsWith(s, len, L"ímu") ||
                  StemmerUtil::endsWith(s, len, L"imu") ||
                  StemmerUtil::endsWith(s, len, L"ách") ||
                  StemmerUtil::endsWith(s, len, L"ata") ||
                  StemmerUtil::endsWith(s, len, L"aty") ||
                  StemmerUtil::endsWith(s, len, L"ých") ||
                  StemmerUtil::endsWith(s, len, L"ama") ||
                  StemmerUtil::endsWith(s, len, L"ami") ||
                  StemmerUtil::endsWith(s, len, L"ové") ||
                  StemmerUtil::endsWith(s, len, L"ovi") ||
                  StemmerUtil::endsWith(s, len, L"ými"))) {
    return len - 3;
  }

  if (len > 4 && (StemmerUtil::endsWith(s, len, L"em") ||
                  StemmerUtil::endsWith(s, len, L"es") ||
                  StemmerUtil::endsWith(s, len, L"ém") ||
                  StemmerUtil::endsWith(s, len, L"ím") ||
                  StemmerUtil::endsWith(s, len, L"ům") ||
                  StemmerUtil::endsWith(s, len, L"at") ||
                  StemmerUtil::endsWith(s, len, L"ám") ||
                  StemmerUtil::endsWith(s, len, L"os") ||
                  StemmerUtil::endsWith(s, len, L"us") ||
                  StemmerUtil::endsWith(s, len, L"ým") ||
                  StemmerUtil::endsWith(s, len, L"mi") ||
                  StemmerUtil::endsWith(s, len, L"ou"))) {
    return len - 2;
  }

  if (len > 3) {
    switch (s[len - 1]) {
    case L'a':
    case L'e':
    case L'i':
    case L'o':
    case L'u':
    case L'ů':
    case L'y':
    case L'á':
    case L'é':
    case L'í':
    case L'ý':
    case L'ě':
      return len - 1;
    }
  }

  return len;
}

int CzechStemmer::removePossessives(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && (StemmerUtil::endsWith(s, len, L"ov") ||
                  StemmerUtil::endsWith(s, len, L"in") ||
                  StemmerUtil::endsWith(s, len, L"ův"))) {
    return len - 2;
  }

  return len;
}

int CzechStemmer::normalize(std::deque<wchar_t> &s, int len)
{
  if (StemmerUtil::endsWith(s, len, L"čt")) { // čt -> ck
    s[len - 2] = L'c';
    s[len - 1] = L'k';
    return len;
  }

  if (StemmerUtil::endsWith(s, len, L"št")) { // št -> sk
    s[len - 2] = L's';
    s[len - 1] = L'k';
    return len;
  }

  switch (s[len - 1]) {
  case L'c': // [cč] -> k
  case L'č':
    s[len - 1] = L'k';
    return len;
  case L'z': // [zž] -> h
  case L'ž':
    s[len - 1] = L'h';
    return len;
  }

  if (len > 1 && s[len - 2] == L'e') {
    s[len - 2] = s[len - 1]; // e* > *
    return len - 1;
  }

  if (len > 2 && s[len - 2] == L'ů') {
    s[len - 2] = L'o'; // *ů* -> *o*
    return len;
  }

  return len;
}
} // namespace org::apache::lucene::analysis::cz