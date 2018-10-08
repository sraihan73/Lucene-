using namespace std;

#include "NorwegianLightStemmer.h"

namespace org::apache::lucene::analysis::no
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

NorwegianLightStemmer::NorwegianLightStemmer(int flags)
    : useBokmaal((flags & BOKMAAL) != 0), useNynorsk((flags & NYNORSK) != 0)
{
  if (flags <= 0 || flags > BOKMAAL + NYNORSK) {
    throw invalid_argument(L"invalid flags");
  }
}

int NorwegianLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  // Remove posessive -s (bilens -> bilen) and continue checking
  if (len > 4 && s[len - 1] == L's') {
    len--;
  }

  // Remove common endings, single-pass
  if (len > 7 && ((StemmerUtil::endsWith(s, len, L"heter") && useBokmaal) ||
                  (StemmerUtil::endsWith(s, len, L"heten") && useBokmaal) ||
                  (StemmerUtil::endsWith(s, len, L"heita") &&
                   useNynorsk))) // general ending (hemmeleg-heita -> hemmeleg)
  {
    return len - 5;
  }

  // Remove Nynorsk common endings, single-pass
  if (len > 8 && useNynorsk &&
      (StemmerUtil::endsWith(s, len, L"heiter") ||
       StemmerUtil::endsWith(s, len, L"leiken") ||
       StemmerUtil::endsWith(
           s, len, L"leikar"))) // general ending (trygg-leikar -> trygg)
  {
    return len - 6;
  }

  if (len > 5 && (StemmerUtil::endsWith(s, len, L"dom") ||
                  (StemmerUtil::endsWith(s, len, L"het") &&
                   useBokmaal))) // general ending (hemmelig-het -> hemmelig)
  {
    return len - 3;
  }

  if (len > 6 && useNynorsk &&
      (StemmerUtil::endsWith(s, len, L"heit") ||
       StemmerUtil::endsWith(s, len, L"semd") ||
       StemmerUtil::endsWith(s, len,
                             L"leik"))) // general ending (trygg-leik -> trygg)
  {
    return len - 4;
  }

  if (len > 7 && (StemmerUtil::endsWith(s, len, L"elser") ||
                  StemmerUtil::endsWith(
                      s, len, L"elsen"))) // general ending (føl-elsen -> føl)
  {
    return len - 5;
  }

  if (len > 6 &&
      ((StemmerUtil::endsWith(s, len, L"ende") && useBokmaal) ||
       (StemmerUtil::endsWith(s, len, L"ande") && useNynorsk) ||
       StemmerUtil::endsWith(s, len, L"else") ||
       (StemmerUtil::endsWith(s, len, L"este") && useBokmaal) ||
       (StemmerUtil::endsWith(s, len, L"aste") && useNynorsk) ||
       (StemmerUtil::endsWith(s, len, L"eren") && useBokmaal) ||
       (StemmerUtil::endsWith(s, len, L"aren") && useNynorsk))) // masc
  {
    return len - 4;
  }

  if (len > 5 && ((StemmerUtil::endsWith(s, len, L"ere") && useBokmaal) ||
                  (StemmerUtil::endsWith(s, len, L"are") && useNynorsk) ||
                  (StemmerUtil::endsWith(s, len, L"est") && useBokmaal) ||
                  (StemmerUtil::endsWith(s, len, L"ast") && useNynorsk) ||
                  StemmerUtil::endsWith(s, len, L"ene") ||
                  (StemmerUtil::endsWith(s, len, L"ane") &&
                   useNynorsk))) // masc pl definite (gut-ane)
  {
    return len - 3;
  }

  if (len > 4 && (StemmerUtil::endsWith(s, len, L"er") ||
                  StemmerUtil::endsWith(s, len, L"en") ||
                  StemmerUtil::endsWith(s, len, L"et") ||
                  (StemmerUtil::endsWith(s, len, L"ar") && useNynorsk) ||
                  (StemmerUtil::endsWith(s, len, L"st") && useBokmaal) ||
                  StemmerUtil::endsWith(s, len, L"te"))) {
    return len - 2;
  }

  if (len > 3) {
    switch (s[len - 1]) {
    case L'a': // fem definite
    case L'e': // to get correct stem for nouns ending in -e (kake -> kak, kaker
               // -> kak)
    case L'n':
      return len - 1;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::no