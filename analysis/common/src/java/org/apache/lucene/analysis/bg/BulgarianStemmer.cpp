using namespace std;

#include "BulgarianStemmer.h"

namespace org::apache::lucene::analysis::bg
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int BulgarianStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 4) // do not stem
  {
    return len;
  }

  if (len > 5 && StemmerUtil::endsWith(s, len, L"ища")) {
    return len - 3;
  }

  len = removeArticle(s, len);
  len = removePlural(s, len);

  if (len > 3) {
    if (StemmerUtil::endsWith(s, len, L"я")) {
      len--;
    }
    if (StemmerUtil::endsWith(s, len, L"а") ||
        StemmerUtil::endsWith(s, len, L"о") ||
        StemmerUtil::endsWith(s, len, L"е")) {
      len--;
    }
  }

  // the rule to rewrite ен -> н is duplicated in the paper.
  // in the perl implementation referenced by the paper, this is fixed.
  // (it is fixed here as well)
  if (len > 4 && StemmerUtil::endsWith(s, len, L"ен")) {
    s[len - 2] = L'н'; // replace with н
    len--;
  }

  if (len > 5 && s[len - 2] == L'ъ') {
    s[len - 2] = s[len - 1]; // replace ъN with N
    len--;
  }

  return len;
}

int BulgarianStemmer::removeArticle(std::deque<wchar_t> &s, int const len)
{
  if (len > 6 && StemmerUtil::endsWith(s, len, L"ият")) {
    return len - 3;
  }

  if (len > 5) {
    if (StemmerUtil::endsWith(s, len, L"ът") ||
        StemmerUtil::endsWith(s, len, L"то") ||
        StemmerUtil::endsWith(s, len, L"те") ||
        StemmerUtil::endsWith(s, len, L"та") ||
        StemmerUtil::endsWith(s, len, L"ия")) {
      return len - 2;
    }
  }

  if (len > 4 && StemmerUtil::endsWith(s, len, L"ят")) {
    return len - 2;
  }

  return len;
}

int BulgarianStemmer::removePlural(std::deque<wchar_t> &s, int const len)
{
  if (len > 6) {
    if (StemmerUtil::endsWith(s, len, L"овци")) {
      return len - 3; // replace with о
    }
    if (StemmerUtil::endsWith(s, len, L"ове")) {
      return len - 3;
    }
    if (StemmerUtil::endsWith(s, len, L"еве")) {
      s[len - 3] = L'й'; // replace with й
      return len - 2;
    }
  }

  if (len > 5) {
    if (StemmerUtil::endsWith(s, len, L"ища")) {
      return len - 3;
    }
    if (StemmerUtil::endsWith(s, len, L"та")) {
      return len - 2;
    }
    if (StemmerUtil::endsWith(s, len, L"ци")) {
      s[len - 2] = L'к'; // replace with к
      return len - 1;
    }
    if (StemmerUtil::endsWith(s, len, L"зи")) {
      s[len - 2] = L'г'; // replace with г
      return len - 1;
    }

    if (s[len - 3] == L'е' && s[len - 1] == L'и') {
      s[len - 3] = L'я'; // replace е with я, remove и
      return len - 1;
    }
  }

  if (len > 4) {
    if (StemmerUtil::endsWith(s, len, L"си")) {
      s[len - 2] = L'х'; // replace with х
      return len - 1;
    }
    if (StemmerUtil::endsWith(s, len, L"и")) {
      return len - 1;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::bg