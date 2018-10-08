using namespace std;

#include "Utility.h"
#include "CharType.h"

namespace org::apache::lucene::analysis::cn::smart
{
using SegTokenFilter =
    org::apache::lucene::analysis::cn::smart::hhmm::SegTokenFilter;
//    import static Character.isSurrogate;
std::deque<wchar_t> const Utility::STRING_CHAR_ARRAY =
    L"未##串"->toCharArray();
std::deque<wchar_t> const Utility::NUMBER_CHAR_ARRAY =
    L"未##数"->toCharArray();
std::deque<wchar_t> const Utility::START_CHAR_ARRAY = L"始##始"->toCharArray();
std::deque<wchar_t> const Utility::END_CHAR_ARRAY =
    (wstring(L"末##末")).toCharArray();
std::deque<wchar_t> const Utility::COMMON_DELIMITER =
    std::deque<wchar_t>{L','};
const wstring Utility::SPACES = L" 　\t\r\n";

int Utility::compareArray(std::deque<wchar_t> &larray, int lstartIndex,
                          std::deque<wchar_t> &rarray, int rstartIndex)
{

  if (larray.empty()) {
    if (rarray.empty() || rstartIndex >= rarray.size()) {
      return 0;
    } else {
      return -1;
    }
  } else {
    // larray != null
    if (rarray.empty()) {
      if (lstartIndex >= larray.size()) {
        return 0;
      } else {
        return 1;
      }
    }
  }

  int li = lstartIndex, ri = rstartIndex;
  while (li < larray.size() && ri < rarray.size() && larray[li] == rarray[ri]) {
    li++;
    ri++;
  }
  if (li == larray.size()) {
    if (ri == rarray.size()) {
      // Both arrays are equivalent, return 0.
      return 0;
    } else {
      // larray < rarray because larray has ended first.
      return -1;
    }
  } else {
    // differing lengths
    if (ri == rarray.size()) {
      // larray > rarray because rarray has ended first.
      return 1;
    } else {
      // determine by comparison
      if (larray[li] > rarray[ri]) {
        return 1;
      } else {
        return -1;
      }
    }
  }
}

int Utility::compareArrayByPrefix(std::deque<wchar_t> &shortArray,
                                  int shortIndex,
                                  std::deque<wchar_t> &longArray,
                                  int longIndex)
{

  // a null prefix is a prefix of longArray
  if (shortArray.empty()) {
    return 0;
  } else if (longArray.empty()) {
    return (shortIndex < shortArray.size()) ? 1 : 0;
  }

  int si = shortIndex, li = longIndex;
  while (si < shortArray.size() && li < longArray.size() &&
         shortArray[si] == longArray[li]) {
    si++;
    li++;
  }
  if (si == shortArray.size()) {
    // shortArray is a prefix of longArray
    return 0;
  } else {
    // shortArray > longArray because longArray ended first.
    if (li == longArray.size()) {
      return 1;
    } else {
      // determine by comparison
      return (shortArray[si] > longArray[li]) ? 1 : -1;
    }
  }
}

int Utility::getCharType(wchar_t ch)
{
  if (isSurrogate(ch)) {
    return CharType::SURROGATE;
  }
  // Most (but not all!) of these are Han Ideographic Characters
  if (ch >= 0x4E00 && ch <= 0x9FA5) {
    return CharType::HANZI;
  }
  if ((ch >= 0x0041 && ch <= 0x005A) || (ch >= 0x0061 && ch <= 0x007A)) {
    return CharType::LETTER;
  }
  if (ch >= 0x0030 && ch <= 0x0039) {
    return CharType::DIGIT;
  }
  if (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n' || ch == L'　') {
    return CharType::SPACE_LIKE;
  }
  // Punctuation Marks
  if ((ch >= 0x0021 && ch <= 0x00BB) || (ch >= 0x2010 && ch <= 0x2642) ||
      (ch >= 0x3001 && ch <= 0x301E)) {
    return CharType::DELIMITER;
  }

  // Full-Width range
  if ((ch >= 0xFF21 && ch <= 0xFF3A) || (ch >= 0xFF41 && ch <= 0xFF5A)) {
    return CharType::FULLWIDTH_LETTER;
  }
  if (ch >= 0xFF10 && ch <= 0xFF19) {
    return CharType::FULLWIDTH_DIGIT;
  }
  if (ch >= 0xFE30 && ch <= 0xFF63) {
    return CharType::DELIMITER;
  }
  return CharType::OTHER;
}
} // namespace org::apache::lucene::analysis::cn::smart