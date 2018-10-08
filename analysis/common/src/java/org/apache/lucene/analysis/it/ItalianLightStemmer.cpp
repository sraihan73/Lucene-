using namespace std;

#include "ItalianLightStemmer.h"

namespace org::apache::lucene::analysis::it
{

int ItalianLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 6) {
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
  case L'e':
    if (s[len - 2] == L'i' || s[len - 2] == L'h') {
      return len - 2;
    } else {
      return len - 1;
    }
  case L'i':
    if (s[len - 2] == L'h' || s[len - 2] == L'i') {
      return len - 2;
    } else {
      return len - 1;
    }
  case L'a':
    if (s[len - 2] == L'i') {
      return len - 2;
    } else {
      return len - 1;
    }
  case L'o':
    if (s[len - 2] == L'i') {
      return len - 2;
    } else {
      return len - 1;
    }
  }

  return len;
}
} // namespace org::apache::lucene::analysis::it