using namespace std;

#include "StrictStringTokenizer.h"

namespace org::apache::lucene::util
{

StrictStringTokenizer::StrictStringTokenizer(const wstring &s,
                                             wchar_t delimiter)
    : s(s), delimiter(delimiter)
{
}

wstring StrictStringTokenizer::nextToken()
{
  if (pos < 0) {
    throw make_shared<IllegalStateException>(L"no more tokens");
  }

  int pos1 = (int)s.find(delimiter, pos);
  wstring s1;
  if (pos1 >= 0) {
    s1 = s.substr(pos, pos1 - pos);
    pos = pos1 + 1;
  } else {
    s1 = s.substr(pos);
    pos = -1;
  }

  return s1;
}

bool StrictStringTokenizer::hasMoreTokens() { return pos >= 0; }
} // namespace org::apache::lucene::util