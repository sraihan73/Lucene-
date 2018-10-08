using namespace std;

#include "ReusableStringReader.h"

namespace org::apache::lucene::analysis
{

void ReusableStringReader::setValue(const wstring &s)
{
  this->s = s;
  this->size = s.length();
  this->pos = 0;
}

int ReusableStringReader::read()
{
  if (pos < size) {
    return s[pos++];
  } else {
    s = L"";
    return -1;
  }
}

int ReusableStringReader::read(std::deque<wchar_t> &c, int off, int len)
{
  if (pos < size) {
    len = min(len, size - pos);
    s.getChars(pos, pos + len, c, off);
    pos += len;
    return len;
  } else {
    s = L"";
    return -1;
  }
}

ReusableStringReader::~ReusableStringReader()
{
  pos = size; // this prevents NPE when reading after close!
  s = L"";
}
} // namespace org::apache::lucene::analysis