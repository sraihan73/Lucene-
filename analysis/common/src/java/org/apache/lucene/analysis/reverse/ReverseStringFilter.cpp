using namespace std;

#include "ReverseStringFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::reverse
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

ReverseStringFilter::ReverseStringFilter(shared_ptr<TokenStream> in_)
    : ReverseStringFilter(in_, NOMARKER)
{
}

ReverseStringFilter::ReverseStringFilter(shared_ptr<TokenStream> in_,
                                         wchar_t marker)
    : org::apache::lucene::analysis::TokenFilter(in_), marker(marker)
{
}

bool ReverseStringFilter::incrementToken() 
{
  if (input->incrementToken()) {
    int len = termAtt->length();
    if (marker != NOMARKER) {
      len++;
      termAtt->resizeBuffer(len);
      termAtt->buffer()[len - 1] = marker;
    }
    reverse(termAtt->buffer(), 0, len);
    termAtt->setLength(len);
    return true;
  } else {
    return false;
  }
}

wstring ReverseStringFilter::reverse(const wstring &input)
{
  const std::deque<wchar_t> charInput = input.toCharArray();
  reverse(charInput, 0, charInput.size());
  return wstring(charInput);
}

void ReverseStringFilter::reverse(std::deque<wchar_t> &buffer)
{
  reverse(buffer, 0, buffer.size());
}

void ReverseStringFilter::reverse(std::deque<wchar_t> &buffer, int const len)
{
  reverse(buffer, 0, len);
}

void ReverseStringFilter::reverse(std::deque<wchar_t> &buffer, int const start,
                                  int const len)
{
  /* modified version of Apache Harmony AbstractStringBuilder reverse0() */
  if (len < 2) {
    return;
  }
  int end = (start + len) - 1;
  wchar_t frontHigh = buffer[start];
  wchar_t endLow = buffer[end];
  bool allowFrontSur = true, allowEndSur = true;
  constexpr int mid = start + (len >> 1);
  for (int i = start; i < mid; ++i, --end) {
    constexpr wchar_t frontLow = buffer[i + 1];
    constexpr wchar_t endHigh = buffer[end - 1];
    constexpr bool surAtFront =
        allowFrontSur && Character::isSurrogatePair(frontHigh, frontLow);
    if (surAtFront && (len < 3)) {
      // nothing to do since surAtFront is allowed and 1 char left
      return;
    }
    constexpr bool surAtEnd =
        allowEndSur && Character::isSurrogatePair(endHigh, endLow);
    allowFrontSur = allowEndSur = true;
    if (surAtFront == surAtEnd) {
      if (surAtFront) {
        // both surrogates
        buffer[end] = frontLow;
        buffer[--end] = frontHigh;
        buffer[i] = endHigh;
        buffer[++i] = endLow;
        frontHigh = buffer[i + 1];
        endLow = buffer[end - 1];
      } else {
        // neither surrogates
        buffer[end] = frontHigh;
        buffer[i] = endLow;
        frontHigh = frontLow;
        endLow = endHigh;
      }
    } else {
      if (surAtFront) {
        // surrogate only at the front
        buffer[end] = frontLow;
        buffer[i] = endLow;
        endLow = endHigh;
        allowFrontSur = false;
      } else {
        // surrogate only at the end
        buffer[end] = frontHigh;
        buffer[i] = endHigh;
        frontHigh = frontLow;
        allowEndSur = false;
      }
    }
  }
  if ((len & 0x01) == 1 && !(allowFrontSur && allowEndSur)) {
    // only if odd length
    buffer[end] = allowFrontSur ? endLow : frontHigh;
  }
}
} // namespace org::apache::lucene::analysis::reverse