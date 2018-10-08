using namespace std;

#include "ArabicStemmer.h"

namespace org::apache::lucene::analysis::ar
{
using namespace org::apache::lucene::analysis::util;
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;
std::deque<std::deque<wchar_t>> const ArabicStemmer::prefixes = {
    (L"" + StringHelper::toString(ALEF) + StringHelper::toString(LAM))
        ->toCharArray(),
    (L"" + StringHelper::toString(WAW) + StringHelper::toString(ALEF) +
     StringHelper::toString(LAM))
        ->toCharArray(),
    (L"" + StringHelper::toString(BEH) + StringHelper::toString(ALEF) +
     StringHelper::toString(LAM))
        ->toCharArray(),
    (L"" + StringHelper::toString(KAF) + StringHelper::toString(ALEF) +
     StringHelper::toString(LAM))
        ->toCharArray(),
    (L"" + StringHelper::toString(FEH) + StringHelper::toString(ALEF) +
     StringHelper::toString(LAM))
        ->toCharArray(),
    (L"" + StringHelper::toString(LAM) + StringHelper::toString(LAM))
        ->toCharArray(),
    (L"" + StringHelper::toString(WAW))->toCharArray()};
std::deque<std::deque<wchar_t>> const ArabicStemmer::suffixes = {
    (L"" + StringHelper::toString(HEH) + StringHelper::toString(ALEF))
        ->toCharArray(),
    (L"" + StringHelper::toString(ALEF) + StringHelper::toString(NOON))
        ->toCharArray(),
    (L"" + StringHelper::toString(ALEF) + StringHelper::toString(TEH))
        ->toCharArray(),
    (L"" + StringHelper::toString(WAW) + StringHelper::toString(NOON))
        ->toCharArray(),
    (L"" + StringHelper::toString(YEH) + StringHelper::toString(NOON))
        ->toCharArray(),
    (L"" + StringHelper::toString(YEH) + StringHelper::toString(HEH))
        ->toCharArray(),
    (L"" + StringHelper::toString(YEH) + StringHelper::toString(TEH_MARBUTA))
        ->toCharArray(),
    (L"" + StringHelper::toString(HEH))->toCharArray(),
    (L"" + StringHelper::toString(TEH_MARBUTA))->toCharArray(),
    (L"" + StringHelper::toString(YEH))->toCharArray()};

int ArabicStemmer::stem(std::deque<wchar_t> &s, int len)
{
  len = stemPrefix(s, len);
  len = stemSuffix(s, len);

  return len;
}

int ArabicStemmer::stemPrefix(std::deque<wchar_t> &s, int len)
{
  for (int i = 0; i < prefixes.size(); i++) {
    if (startsWithCheckLength(s, len, prefixes[i])) {
      return StemmerUtil::deleteN(s, 0, len, prefixes[i].length);
    }
  }
  return len;
}

int ArabicStemmer::stemSuffix(std::deque<wchar_t> &s, int len)
{
  for (int i = 0; i < suffixes.size(); i++) {
    if (endsWithCheckLength(s, len, suffixes[i])) {
      len = StemmerUtil::deleteN(s, len - suffixes[i].length, len,
                                 suffixes[i].length);
    }
  }
  return len;
}

bool ArabicStemmer::startsWithCheckLength(std::deque<wchar_t> &s, int len,
                                          std::deque<wchar_t> &prefix)
{
  if (prefix.size() == 1 &&
      len < 4) { // wa- prefix requires at least 3 characters
    return false;
  } else if (len < prefix.size() + 2) { // other prefixes require only 2.
    return false;
  } else {
    for (int i = 0; i < prefix.size(); i++) {
      if (s[i] != prefix[i]) {
        return false;
      }
    }

    return true;
  }
}

bool ArabicStemmer::endsWithCheckLength(std::deque<wchar_t> &s, int len,
                                        std::deque<wchar_t> &suffix)
{
  if (len <
      suffix.size() +
          2) { // all suffixes require at least 2 characters after stemming
    return false;
  } else {
    for (int i = 0; i < suffix.size(); i++) {
      if (s[len - suffix.size() + i] != suffix[i]) {
        return false;
      }
    }

    return true;
  }
}
} // namespace org::apache::lucene::analysis::ar