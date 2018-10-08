using namespace std;

#include "GermanLightStemmer.h"

namespace org::apache::lucene::analysis::de
{

int GermanLightStemmer::stem(std::deque<wchar_t> &s, int len)
{
  for (int i = 0; i < len; i++) {
    switch (s[i]) {
    case L'ä':
    case L'à':
    case L'á':
    case L'â':
      s[i] = L'a';
      break;
    case L'ö':
    case L'ò':
    case L'ó':
    case L'ô':
      s[i] = L'o';
      break;
    case L'ï':
    case L'ì':
    case L'í':
    case L'î':
      s[i] = L'i';
      break;
    case L'ü':
    case L'ù':
    case L'ú':
    case L'û':
      s[i] = L'u';
      break;
    }
  }

  len = step1(s, len);
  return step2(s, len);
}

bool GermanLightStemmer::stEnding(wchar_t ch)
{
  switch (ch) {
  case L'b':
  case L'd':
  case L'f':
  case L'g':
  case L'h':
  case L'k':
  case L'l':
  case L'm':
  case L'n':
  case L't':
    return true;
  default:
    return false;
  }
}

int GermanLightStemmer::step1(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && s[len - 3] == L'e' && s[len - 2] == L'r' &&
      s[len - 1] == L'n') {
    return len - 3;
  }

  if (len > 4 && s[len - 2] == L'e') {
    switch (s[len - 1]) {
    case L'm':
    case L'n':
    case L'r':
    case L's':
      return len - 2;
    }
  }

  if (len > 3 && s[len - 1] == L'e') {
    return len - 1;
  }

  if (len > 3 && s[len - 1] == L's' && stEnding(s[len - 2])) {
    return len - 1;
  }

  return len;
}

int GermanLightStemmer::step2(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && s[len - 3] == L'e' && s[len - 2] == L's' &&
      s[len - 1] == L't') {
    return len - 3;
  }

  if (len > 4 && s[len - 2] == L'e' &&
      (s[len - 1] == L'r' || s[len - 1] == L'n')) {
    return len - 2;
  }

  if (len > 4 && s[len - 2] == L's' && s[len - 1] == L't' &&
      stEnding(s[len - 3])) {
    return len - 2;
  }

  return len;
}
} // namespace org::apache::lucene::analysis::de