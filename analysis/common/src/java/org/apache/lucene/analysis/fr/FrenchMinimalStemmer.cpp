using namespace std;

#include "FrenchMinimalStemmer.h"

namespace org::apache::lucene::analysis::fr
{

int FrenchMinimalStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 6) {
    return len;
  }

  if (s[len - 1] == L'x') {
    if (s[len - 3] == L'a' && s[len - 2] == L'u') {
      s[len - 2] = L'l';
    }
    return len - 1;
  }

  if (s[len - 1] == L's') {
    len--;
  }
  if (s[len - 1] == L'r') {
    len--;
  }
  if (s[len - 1] == L'e') {
    len--;
  }
  if (s[len - 1] == L'é') {
    len--;
  }
  if (s[len - 1] == s[len - 2]) {
    len--;
  }
  return len;
}
} // namespace org::apache::lucene::analysis::fr