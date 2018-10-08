using namespace std;

#include "NorwegianMinimalStemmer.h"

namespace org::apache::lucene::analysis::no
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;
//    import static org.apache.lucene.analysis.no.NorwegianLightStemmer.BOKMAAL;
//    import static org.apache.lucene.analysis.no.NorwegianLightStemmer.NYNORSK;

NorwegianMinimalStemmer::NorwegianMinimalStemmer(int flags)
    : useBokmaal((flags & BOKMAAL) != 0), useNynorsk((flags & NYNORSK) != 0)
{
  if (flags <= 0 || flags > BOKMAAL + NYNORSK) {
    throw invalid_argument(L"invalid flags");
  }
}

int NorwegianMinimalStemmer::stem(std::deque<wchar_t> &s, int len)
{
  // Remove genitiv s
  if (len > 4 && s[len - 1] == L's') {
    len--;
  }

  if (len > 5 && (StemmerUtil::endsWith(s, len, L"ene") ||
                  (StemmerUtil::endsWith(s, len, L"ane") && useNynorsk))) {
    return len - 3;
  }

  if (len > 4 && (StemmerUtil::endsWith(s, len, L"er") ||
                  StemmerUtil::endsWith(s, len, L"en") ||
                  StemmerUtil::endsWith(s, len, L"et") ||
                  (StemmerUtil::endsWith(s, len, L"ar") && useNynorsk))) {
    return len - 2;
  }

  if (len > 3) {
    switch (s[len - 1]) {
    case L'a': // fem definite
    case L'e': // to get correct stem for nouns ending in -e (kake -> kak, kaker
               // -> kak)
      return len - 1;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::no