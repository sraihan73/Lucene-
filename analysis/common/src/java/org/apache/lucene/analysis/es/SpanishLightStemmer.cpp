using namespace std;

#include "SpanishLightStemmer.h"

namespace org::apache::lucene::analysis::es
{

int SpanishLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 5) {
    return len;
  }

  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'à':
    case L'á':
    case L'â':
    case L'ä':
      s[i] = L'a';
      break;
    case L'ò':
    case L'ó':
    case L'ô':
    case L'ö':
      s[i] = L'o';
      break;
    case L'è':
    case L'é':
    case L'ê':
    case L'ë':
      s[i] = L'e';
      break;
    case L'ù':
    case L'ú':
    case L'û':
    case L'ü':
      s[i] = L'u';
      break;
    case L'ì':
    case L'í':
    case L'î':
    case L'ï':
      s[i] = L'i';
      break;
    }
  }

  switch (s[len - 1]) {
  case L'o':
  case L'a':
  case L'e':
    return len - 1;
  case L's':
    if (s[len - 2] == L'e' && s[len - 3] == L's' && s[len - 4] == L'e') {
      return len - 2;
    }
    if (s[len - 2] == L'e' && s[len - 3] == L'c') {
      s[len - 3] = L'z';
      return len - 2;
    }
    if (s[len - 2] == L'o' || s[len - 2] == L'a' || s[len - 2] == L'e') {
      return len - 2;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::es