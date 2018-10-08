using namespace std;

#include "GalicianStemmer.h"

namespace org::apache::lucene::analysis::gl
{
using RSLPStemmerBase = org::apache::lucene::analysis::pt::RSLPStemmerBase;
const shared_ptr<Step> plural, unification, adverb, augmentative, noun, verb,
    GalicianStemmer::vowel;

GalicianStemmer::StaticConstructor::StaticConstructor()
{
  unordered_map<wstring, std::shared_ptr<RSLPStemmerBase::Step>> steps =
      RSLPStemmerBase::parse(GalicianStemmer::typeid, L"galician.rslp");
  plural = steps[L"Plural"];
  unification = steps[L"Unification"];
  adverb = steps[L"Adverb"];
  augmentative = steps[L"Augmentative"];
  noun = steps[L"Noun"];
  verb = steps[L"Verb"];
  vowel = steps[L"Vowel"];
}

GalicianStemmer::StaticConstructor GalicianStemmer::staticConstructor;

int GalicianStemmer::stem(std::deque<wchar_t> &s, int len)
{
  assert((s.size() >= len + 1,
          L"this stemmer requires an oversized array of at least 1"));

  len = plural->apply(s, len);
  len = unification->apply(s, len);
  len = adverb->apply(s, len);

  int oldlen;
  do {
    oldlen = len;
    len = augmentative->apply(s, len);
  } while (len != oldlen);

  oldlen = len;
  len = noun->apply(s, len);
  if (len == oldlen) { // suffix not removed
    len = verb->apply(s, len);
  }

  len = vowel->apply(s, len);

  // RSLG accent removal
  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'á':
      s[i] = L'a';
      break;
    case L'é':
    case L'ê':
      s[i] = L'e';
      break;
    case L'í':
      s[i] = L'i';
      break;
    case L'ó':
      s[i] = L'o';
      break;
    case L'ú':
      s[i] = L'u';
      break;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::gl