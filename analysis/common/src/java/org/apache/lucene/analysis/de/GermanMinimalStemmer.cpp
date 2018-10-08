using namespace std;

#include "GermanMinimalStemmer.h"

namespace org::apache::lucene::analysis::de
{

int GermanMinimalStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 5) {
    return len;
  }

  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'ä':
      s[i] = L'a';
      break;
    case L'ö':
      s[i] = L'o';
      break;
    case L'ü':
      s[i] = L'u';
      break;
    }
  }

  if (len > 6 && s[len - 3] == L'n' && s[len - 2] == L'e' &&
      s[len - 1] == L'n') {
    return len - 3;
  }

  if (len > 5) {
    switch (s[len - 1]) {
    case L'n':
      if (s[len - 2] == L'e') {
        return len - 2;
      } else {
        break;
      }
    case L'e':
      if (s[len - 2] == L's') {
        return len - 2;
      } else {
        break;
      }
    case L's':
      if (s[len - 2] == L'e') {
        return len - 2;
      } else {
        break;
      }
    case L'r':
      if (s[len - 2] == L'e') {
        return len - 2;
      } else {
        break;
      }
    }
  }

  switch (s[len - 1]) {
  case L'n':
  case L'e':
  case L's':
  case L'r':
    return len - 1;
  }

  return len;
}
} // namespace org::apache::lucene::analysis::de