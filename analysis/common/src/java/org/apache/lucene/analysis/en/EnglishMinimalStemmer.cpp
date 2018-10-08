using namespace std;

#include "EnglishMinimalStemmer.h"

namespace org::apache::lucene::analysis::en
{

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") public int stem(char s[], int
// len)
int EnglishMinimalStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 3 || s[len - 1] != L's') {
    return len;
  }

  switch (s[len - 2]) {
  case L'u':
  case L's':
    return len;
  case L'e':
    if (len > 3 && s[len - 3] == L'i' && s[len - 4] != L'a' &&
        s[len - 4] != L'e') {
      s[len - 3] = L'y';
      return len - 2;
    }
    if (s[len - 3] == L'i' || s[len - 3] == L'a' || s[len - 3] == L'o' ||
        s[len - 3] == L'e') {
      return len; // intentional fallthrough
    }
  default:
    return len - 1;
  }
}
} // namespace org::apache::lucene::analysis::en