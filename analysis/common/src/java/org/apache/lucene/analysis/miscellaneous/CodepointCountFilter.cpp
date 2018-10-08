using namespace std;

#include "CodepointCountFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

CodepointCountFilter::CodepointCountFilter(shared_ptr<TokenStream> in_, int min,
                                           int max)
    : org::apache::lucene::analysis::FilteringTokenFilter(in_), min(min),
      max(max)
{
  if (min < 0) {
    throw invalid_argument(
        L"minimum length must be greater than or equal to zero");
  }
  if (min > max) {
    throw invalid_argument(
        L"maximum length must not be greater than minimum length");
  }
}

bool CodepointCountFilter::accept()
{
  constexpr int max32 = termAtt->length();
  constexpr int min32 = max32 >> 1;
  if (min32 >= min && max32 <= max) {
    // definitely within range
    return true;
  } else if (min32 > max || max32 < min) {
    // definitely not
    return false;
  } else {
    // we must count to be sure
    int len =
        Character::codePointCount(termAtt->buffer(), 0, termAtt->length());
    return (len >= min && len <= max);
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous