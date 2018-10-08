using namespace std;

#include "SwedishLightStemmer.h"

namespace org::apache::lucene::analysis::sv
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

int SwedishLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len > 4 && s[len - 1] == L's') {
    len--;
  }

  if (len > 7 && (StemmerUtil::endsWith(s, len, L"elser") ||
                  StemmerUtil::endsWith(s, len, L"heten"))) {
    return len - 5;
  }

  if (len > 6 && (StemmerUtil::endsWith(s, len, L"arne") ||
                  StemmerUtil::endsWith(s, len, L"erna") ||
                  StemmerUtil::endsWith(s, len, L"ande") ||
                  StemmerUtil::endsWith(s, len, L"else") ||
                  StemmerUtil::endsWith(s, len, L"aste") ||
                  StemmerUtil::endsWith(s, len, L"orna") ||
                  StemmerUtil::endsWith(s, len, L"aren"))) {
    return len - 4;
  }

  if (len > 5 && (StemmerUtil::endsWith(s, len, L"are") ||
                  StemmerUtil::endsWith(s, len, L"ast") ||
                  StemmerUtil::endsWith(s, len, L"het"))) {
    return len - 3;
  }

  if (len > 4 && (StemmerUtil::endsWith(s, len, L"ar") ||
                  StemmerUtil::endsWith(s, len, L"er") ||
                  StemmerUtil::endsWith(s, len, L"or") ||
                  StemmerUtil::endsWith(s, len, L"en") ||
                  StemmerUtil::endsWith(s, len, L"at") ||
                  StemmerUtil::endsWith(s, len, L"te") ||
                  StemmerUtil::endsWith(s, len, L"et"))) {
    return len - 2;
  }

  if (len > 3) {
    switch (s[len - 1]) {
    case L't':
    case L'a':
    case L'e':
    case L'n':
      return len - 1;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::sv