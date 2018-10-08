using namespace std;

#include "LengthFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

LengthFilter::LengthFilter(shared_ptr<TokenStream> in_, int min, int max)
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

bool LengthFilter::accept()
{
  constexpr int len = termAtt->length();
  return (len >= min && len <= max);
}
} // namespace org::apache::lucene::analysis::miscellaneous