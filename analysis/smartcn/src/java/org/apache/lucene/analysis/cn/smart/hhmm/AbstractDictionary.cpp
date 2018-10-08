using namespace std;

#include "AbstractDictionary.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{

wstring AbstractDictionary::getCCByGB2312Id(int ccid)
{
  if (ccid < 0 || ccid > AbstractDictionary::GB2312_CHAR_NUM) {
    return L"";
  }
  int cc1 = ccid / 94 + 161;
  int cc2 = ccid % 94 + 161;
  std::deque<char> buffer(2);
  buffer[0] = static_cast<char>(cc1);
  buffer[1] = static_cast<char>(cc2);
  try {
    wstring cchar = wstring(buffer, L"GB2312");
    return cchar;
  } catch (const UnsupportedEncodingException &e) {
    return L"";
  }
}

short AbstractDictionary::getGB2312Id(wchar_t ch)
{
  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    std::deque<char> buffer = Character::toString(ch).getBytes(L"GB2312");
    if (buffer.size() != 2) {
      // Should be a two-byte character
      return -1;
    }
    int b0 = (buffer[0] & 0x0FF) -
             161; // Code starts from A1, therefore subtract 0xA1=161
    int b1 = (buffer[1] & 0x0FF) -
             161; // There is no Chinese char for the first and last symbol.
                  // Therefore, each code page only has 16*6-2=94 characters.
    return static_cast<short>(b0 * 94 + b1);
  } catch (const UnsupportedEncodingException &e) {
    throw runtime_error(e);
  }
}

int64_t AbstractDictionary::hash1(wchar_t c)
{
  constexpr int64_t p = 1099511628211LL;
  int64_t hash = 0xcbf29ce484222325LL;
  hash = (hash ^ (c & 0x00FF)) * p;
  hash = (hash ^ (c >> 8)) * p;
  hash += hash << 13;
  hash ^= hash >> 7;
  hash += hash << 3;
  hash ^= hash >> 17;
  hash += hash << 5;
  return hash;
}

int64_t AbstractDictionary::hash1(std::deque<wchar_t> &carray)
{
  constexpr int64_t p = 1099511628211LL;
  int64_t hash = 0xcbf29ce484222325LL;
  for (int i = 0; i < carray.size(); i++) {
    wchar_t d = carray[i];
    hash = (hash ^ (d & 0x00FF)) * p;
    hash = (hash ^ (d >> 8)) * p;
  }

  // hash += hash << 13;
  // hash ^= hash >> 7;
  // hash += hash << 3;
  // hash ^= hash >> 17;
  // hash += hash << 5;
  return hash;
}

int AbstractDictionary::hash2(wchar_t c)
{
  int hash = 5381;

  /* hash 33 + c */
  hash = ((hash << 5) + hash) + c & 0x00FF;
  hash = ((hash << 5) + hash) + c >> 8;

  return hash;
}

int AbstractDictionary::hash2(std::deque<wchar_t> &carray)
{
  int hash = 5381;

  /* hash 33 + c */
  for (int i = 0; i < carray.size(); i++) {
    wchar_t d = carray[i];
    hash = ((hash << 5) + hash) + d & 0x00FF;
    hash = ((hash << 5) + hash) + d >> 8;
  }

  return hash;
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm