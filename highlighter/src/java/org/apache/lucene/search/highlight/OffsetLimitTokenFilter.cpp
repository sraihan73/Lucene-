using namespace std;

#include "OffsetLimitTokenFilter.h"

namespace org::apache::lucene::search::highlight
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

OffsetLimitTokenFilter::OffsetLimitTokenFilter(shared_ptr<TokenStream> input,
                                               int offsetLimit)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  this->offsetLimit = offsetLimit;
}

bool OffsetLimitTokenFilter::incrementToken() 
{
  if (offsetCount < offsetLimit && input->incrementToken()) {
    int offsetLength = offsetAttrib->endOffset() - offsetAttrib->startOffset();
    offsetCount += offsetLength;
    return true;
  }
  return false;
}

void OffsetLimitTokenFilter::reset() 
{
  TokenFilter::reset();
  offsetCount = 0;
}
} // namespace org::apache::lucene::search::highlight