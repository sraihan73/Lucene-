using namespace std;

#include "StemmerUtil.h"

namespace org::apache::lucene::analysis::util
{

StemmerUtil::StemmerUtil() {}

bool StemmerUtil::startsWith(std::deque<wchar_t> &s, int len,
                             const wstring &prefix)
{
  constexpr int prefixLen = prefix.length();
  if (prefixLen > len) {
    return false;
  }
  for (int i = 0; i < prefixLen; i++) {
    if (s[i] != prefix[i]) {
      return false;
    }
  }
  return true;
}

bool StemmerUtil::endsWith(std::deque<wchar_t> &s, int len,
                           const wstring &suffix)
{
  constexpr int suffixLen = suffix.length();
  if (suffixLen > len) {
    return false;
  }
  for (int i = suffixLen - 1; i >= 0; i--) {
    if (s[len - (suffixLen - i)] != suffix[i]) {
      return false;
    }
  }

  return true;
}

bool StemmerUtil::endsWith(std::deque<wchar_t> &s, int len,
                           std::deque<wchar_t> &suffix)
{
  constexpr int suffixLen = suffix.size();
  if (suffixLen > len) {
    return false;
  }
  for (int i = suffixLen - 1; i >= 0; i--) {
    if (s[len - (suffixLen - i)] != suffix[i]) {
      return false;
    }
  }

  return true;
}

int StemmerUtil::delete_(std::deque<wchar_t> &s, int pos, int len)
{
  assert(pos < len);
  if (pos < len - 1) { // don't arraycopy if asked to delete last character
    System::arraycopy(s, pos + 1, s, pos, len - pos - 1);
  }
  return len - 1;
}

int StemmerUtil::deleteN(std::deque<wchar_t> &s, int pos, int len, int nChars)
{
  assert(pos + nChars <= len);
  if (pos + nChars <
      len) { // don't arraycopy if asked to delete the last characters
    System::arraycopy(s, pos + nChars, s, pos, len - pos - nChars);
  }
  return len - nChars;
}
} // namespace org::apache::lucene::analysis::util