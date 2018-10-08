using namespace std;

#include "PortugueseStemmer.h"

namespace org::apache::lucene::analysis::pt
{

const shared_ptr<Step> plural, feminine, adverb, augmentative, noun, verb,
    PortugueseStemmer::vowel;

PortugueseStemmer::StaticConstructor::StaticConstructor()
{
  unordered_map<wstring, std::shared_ptr<Step>> steps =
      parse(PortugueseStemmer::typeid, L"portuguese.rslp");
  plural = steps[L"Plural"];
  feminine = steps[L"Feminine"];
  adverb = steps[L"Adverb"];
  augmentative = steps[L"Augmentative"];
  noun = steps[L"Noun"];
  verb = steps[L"Verb"];
  vowel = steps[L"Vowel"];
}

PortugueseStemmer::StaticConstructor PortugueseStemmer::staticConstructor;

int PortugueseStemmer::stem(std::deque<wchar_t> &s, int len)
{
  assert((s.size() >= len + 1,
          L"this stemmer requires an oversized array of at least 1"));

  len = plural->apply(s, len);
  len = adverb->apply(s, len);
  len = feminine->apply(s, len);
  len = augmentative->apply(s, len);

  int oldlen = len;
  len = noun->apply(s, len);

  if (len == oldlen) { // suffix not removed
    oldlen = len;

    len = verb->apply(s, len);

    if (len == oldlen) { // suffix not removed
      len = vowel->apply(s, len);
    }
  }

  // rslp accent removal
  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'à':
    case L'á':
    case L'â':
    case L'ã':
    case L'ä':
    case L'å':
      s[i] = L'a';
      break;
    case L'ç':
      s[i] = L'c';
      break;
    case L'è':
    case L'é':
    case L'ê':
    case L'ë':
      s[i] = L'e';
      break;
    case L'ì':
    case L'í':
    case L'î':
    case L'ï':
      s[i] = L'i';
      break;
    case L'ñ':
      s[i] = L'n';
      break;
    case L'ò':
    case L'ó':
    case L'ô':
    case L'õ':
    case L'ö':
      s[i] = L'o';
      break;
    case L'ù':
    case L'ú':
    case L'û':
    case L'ü':
      s[i] = L'u';
      break;
    case L'ý':
    case L'ÿ':
      s[i] = L'y';
      break;
    }
  }
  return len;
}
} // namespace org::apache::lucene::analysis::pt