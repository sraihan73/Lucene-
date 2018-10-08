using namespace std;

#include "RussianLightStemmer.h"

namespace org::apache::lucene::analysis::ru
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int RussianLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  len = removeCase(s, len);
  return normalize(s, len);
}

int RussianLightStemmer::normalize(std::deque<wchar_t> &s, int len)
{
  if (len > 3) {
    switch (s[len - 1]) {
    case L'ь':
    case L'и':
      return len - 1;
    case L'н':
      if (s[len - 2] == L'н') {
        return len - 1;
      }
    }
  }
  return len;
}

int RussianLightStemmer::removeCase(std::deque<wchar_t> &s, int len)
{
  if (len > 6 && (StemmerUtil::endsWith(s, len, L"иями") ||
                  StemmerUtil::endsWith(s, len, L"оями"))) {
    return len - 4;
  }

  if (len > 5 && (StemmerUtil::endsWith(s, len, L"иям") ||
                  StemmerUtil::endsWith(s, len, L"иях") ||
                  StemmerUtil::endsWith(s, len, L"оях") ||
                  StemmerUtil::endsWith(s, len, L"ями") ||
                  StemmerUtil::endsWith(s, len, L"оям") ||
                  StemmerUtil::endsWith(s, len, L"оьв") ||
                  StemmerUtil::endsWith(s, len, L"ами") ||
                  StemmerUtil::endsWith(s, len, L"его") ||
                  StemmerUtil::endsWith(s, len, L"ему") ||
                  StemmerUtil::endsWith(s, len, L"ери") ||
                  StemmerUtil::endsWith(s, len, L"ими") ||
                  StemmerUtil::endsWith(s, len, L"ого") ||
                  StemmerUtil::endsWith(s, len, L"ому") ||
                  StemmerUtil::endsWith(s, len, L"ыми") ||
                  StemmerUtil::endsWith(s, len, L"оев"))) {
    return len - 3;
  }

  if (len > 4 && (StemmerUtil::endsWith(s, len, L"ая") ||
                  StemmerUtil::endsWith(s, len, L"яя") ||
                  StemmerUtil::endsWith(s, len, L"ях") ||
                  StemmerUtil::endsWith(s, len, L"юю") ||
                  StemmerUtil::endsWith(s, len, L"ах") ||
                  StemmerUtil::endsWith(s, len, L"ею") ||
                  StemmerUtil::endsWith(s, len, L"их") ||
                  StemmerUtil::endsWith(s, len, L"ия") ||
                  StemmerUtil::endsWith(s, len, L"ию") ||
                  StemmerUtil::endsWith(s, len, L"ьв") ||
                  StemmerUtil::endsWith(s, len, L"ою") ||
                  StemmerUtil::endsWith(s, len, L"ую") ||
                  StemmerUtil::endsWith(s, len, L"ям") ||
                  StemmerUtil::endsWith(s, len, L"ых") ||
                  StemmerUtil::endsWith(s, len, L"ея") ||
                  StemmerUtil::endsWith(s, len, L"ам") ||
                  StemmerUtil::endsWith(s, len, L"ем") ||
                  StemmerUtil::endsWith(s, len, L"ей") ||
                  StemmerUtil::endsWith(s, len, L"ём") ||
                  StemmerUtil::endsWith(s, len, L"ев") ||
                  StemmerUtil::endsWith(s, len, L"ий") ||
                  StemmerUtil::endsWith(s, len, L"им") ||
                  StemmerUtil::endsWith(s, len, L"ое") ||
                  StemmerUtil::endsWith(s, len, L"ой") ||
                  StemmerUtil::endsWith(s, len, L"ом") ||
                  StemmerUtil::endsWith(s, len, L"ов") ||
                  StemmerUtil::endsWith(s, len, L"ые") ||
                  StemmerUtil::endsWith(s, len, L"ый") ||
                  StemmerUtil::endsWith(s, len, L"ым") ||
                  StemmerUtil::endsWith(s, len, L"ми"))) {
    return len - 2;
  }

  if (len > 3) {
    switch (s[len - 1]) {
    case L'а':
    case L'е':
    case L'и':
    case L'о':
    case L'у':
    case L'й':
    case L'ы':
    case L'я':
    case L'ь':
      return len - 1;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::ru