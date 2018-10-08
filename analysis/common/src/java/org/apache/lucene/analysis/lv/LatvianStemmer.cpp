using namespace std;

#include "LatvianStemmer.h"

namespace org::apache::lucene::analysis::lv
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int LatvianStemmer::stem(std::deque<wchar_t> &s, int len)
{
  int numVowels = this->numVowels(s, len);

  for (int i = 0; i < affixes.size(); i++) {
    shared_ptr<Affix> affix = affixes[i];
    if (numVowels > affix->vc && len >= affix->affix.size() + 3 &&
        StemmerUtil::endsWith(s, len, affix->affix)) {
      len -= affix->affix.size();
      return affix->palatalizes ? unpalatalize(s, len) : len;
    }
  }

  return len;
}

std::deque<std::shared_ptr<Affix>> const LatvianStemmer::affixes = {
    make_shared<Affix>(L"ajiem", 3, false),
    make_shared<Affix>(L"ajai", 3, false),
    make_shared<Affix>(L"ajam", 2, false),
    make_shared<Affix>(L"ajām", 2, false),
    make_shared<Affix>(L"ajos", 2, false),
    make_shared<Affix>(L"ajās", 2, false),
    make_shared<Affix>(L"iem", 2, true),
    make_shared<Affix>(L"ajā", 2, false),
    make_shared<Affix>(L"ais", 2, false),
    make_shared<Affix>(L"ai", 2, false),
    make_shared<Affix>(L"ei", 2, false),
    make_shared<Affix>(L"ām", 1, false),
    make_shared<Affix>(L"am", 1, false),
    make_shared<Affix>(L"ēm", 1, false),
    make_shared<Affix>(L"īm", 1, false),
    make_shared<Affix>(L"im", 1, false),
    make_shared<Affix>(L"um", 1, false),
    make_shared<Affix>(L"us", 1, true),
    make_shared<Affix>(L"as", 1, false),
    make_shared<Affix>(L"ās", 1, false),
    make_shared<Affix>(L"es", 1, false),
    make_shared<Affix>(L"os", 1, true),
    make_shared<Affix>(L"ij", 1, false),
    make_shared<Affix>(L"īs", 1, false),
    make_shared<Affix>(L"ēs", 1, false),
    make_shared<Affix>(L"is", 1, false),
    make_shared<Affix>(L"ie", 1, false),
    make_shared<Affix>(L"u", 1, true),
    make_shared<Affix>(L"a", 1, true),
    make_shared<Affix>(L"i", 1, true),
    make_shared<Affix>(L"e", 1, false),
    make_shared<Affix>(L"ā", 1, false),
    make_shared<Affix>(L"ē", 1, false),
    make_shared<Affix>(L"ī", 1, false),
    make_shared<Affix>(L"ū", 1, false),
    make_shared<Affix>(L"o", 1, false),
    make_shared<Affix>(L"s", 0, false),
    make_shared<Affix>(L"š", 0, false)};

LatvianStemmer::Affix::Affix(const wstring &affix, int vc, bool palatalizes)
{
  this->affix = affix.toCharArray();
  this->vc = vc;
  this->palatalizes = palatalizes;
}

int LatvianStemmer::unpalatalize(std::deque<wchar_t> &s, int len)
{
  // we check the character removed: if it's -u then
  // it's 2,5, or 6 gen pl., and these two can only apply then.
  if (s[len] == L'u') {
    // kš -> kst
    if (StemmerUtil::endsWith(s, len, L"kš")) {
      len++;
      s[len - 2] = L's';
      s[len - 1] = L't';
      return len;
    }
    // ņņ -> nn
    if (StemmerUtil::endsWith(s, len, L"ņņ")) {
      s[len - 2] = L'n';
      s[len - 1] = L'n';
      return len;
    }
  }

  // otherwise all other rules
  if (StemmerUtil::endsWith(s, len, L"pj") ||
      StemmerUtil::endsWith(s, len, L"bj") ||
      StemmerUtil::endsWith(s, len, L"mj") ||
      StemmerUtil::endsWith(s, len, L"vj")) {
    // labial consonant
    return len - 1;
  } else if (StemmerUtil::endsWith(s, len, L"šņ")) {
    s[len - 2] = L's';
    s[len - 1] = L'n';
    return len;
  } else if (StemmerUtil::endsWith(s, len, L"žņ")) {
    s[len - 2] = L'z';
    s[len - 1] = L'n';
    return len;
  } else if (StemmerUtil::endsWith(s, len, L"šļ")) {
    s[len - 2] = L's';
    s[len - 1] = L'l';
    return len;
  } else if (StemmerUtil::endsWith(s, len, L"žļ")) {
    s[len - 2] = L'z';
    s[len - 1] = L'l';
    return len;
  } else if (StemmerUtil::endsWith(s, len, L"ļņ")) {
    s[len - 2] = L'l';
    s[len - 1] = L'n';
    return len;
  } else if (StemmerUtil::endsWith(s, len, L"ļļ")) {
    s[len - 2] = L'l';
    s[len - 1] = L'l';
    return len;
  } else if (s[len - 1] == L'č') {
    s[len - 1] = L'c';
    return len;
  } else if (s[len - 1] == L'ļ') {
    s[len - 1] = L'l';
    return len;
  } else if (s[len - 1] == L'ņ') {
    s[len - 1] = L'n';
    return len;
  }

  return len;
}

int LatvianStemmer::numVowels(std::deque<wchar_t> &s, int len)
{
  int n = 0;
  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'a':
    case L'e':
    case L'i':
    case L'o':
    case L'u':
    case L'ā':
    case L'ī':
    case L'ē':
    case L'ū':
      n++;
    }
  }
  return n;
}
} // namespace org::apache::lucene::analysis::lv